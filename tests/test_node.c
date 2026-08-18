#include "signal_control/signal.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

enum {
  SAMPLE_COUNT = 6,
  QUEUE_LENGTH = 2,
  FILTER_WINDOW = 2,
  STALL_SEQUENCE = 2,
  PROCESSING_DEADLINE_TICKS = 2,
  PROCESSING_STALL_TICKS = 4,
};

typedef struct {
  uint16_t value;
  uint32_t sequence;
} sample_t;

typedef struct {
  uint16_t average;
  uint32_t sequence;
  signal_state_t state;
} decision_t;

static const uint16_t input_values[SAMPLE_COUNT] = {0, 0, 10, 10, 0, 0};
static const uint16_t expected_averages[SAMPLE_COUNT] = {0, 0, 5, 10, 5, 0};
static const signal_state_t expected_states[SAMPLE_COUNT] = {
    SIGNAL_STATE_LOW,  SIGNAL_STATE_LOW,  SIGNAL_STATE_LOW,
    SIGNAL_STATE_HIGH, SIGNAL_STATE_HIGH, SIGNAL_STATE_LOW};

static QueueHandle_t sample_queue;
static QueueHandle_t decision_queue;
static StaticQueue_t sample_queue_storage;
static StaticQueue_t decision_queue_storage;
static uint8_t sample_queue_buffer[QUEUE_LENGTH * sizeof(sample_t)];
static uint8_t decision_queue_buffer[QUEUE_LENGTH * sizeof(decision_t)];

static StaticTask_t sample_task_tcb;
static StaticTask_t processing_task_tcb;
static StaticTask_t control_task_tcb;
static StackType_t sample_task_stack[configMINIMAL_STACK_SIZE];
static StackType_t processing_task_stack[configMINIMAL_STACK_SIZE];
static StackType_t control_task_stack[configMINIMAL_STACK_SIZE];

static volatile BaseType_t test_complete = pdFALSE;
static volatile BaseType_t processing_stalled = pdFALSE;
static volatile BaseType_t processing_resumed = pdFALSE;
static volatile signal_state_t output_state = SIGNAL_STATE_LOW;

static void sample_task(void *parameters) {
  (void)parameters;

  for (uint32_t sequence = 0; sequence < SAMPLE_COUNT; sequence++) {
    sample_t sample = {input_values[sequence], sequence};

    assert(xQueueSend(sample_queue, &sample, pdMS_TO_TICKS(1000U)) == pdPASS);
  }

  for (;;) {
    vTaskDelay(portMAX_DELAY);
  }
}

static void processing_task(void *parameters) {
  signal_filter_t filter;
  signal_state_t state = SIGNAL_STATE_LOW;

  (void)parameters;
  assert(signal_filter_init(&filter, FILTER_WINDOW));

  for (uint32_t sequence = 0; sequence < SAMPLE_COUNT; sequence++) {
    sample_t sample;
    decision_t decision;
    uint16_t average;

    assert(xQueueReceive(sample_queue, &sample, pdMS_TO_TICKS(1000U)) ==
           pdPASS);
    assert(sample.sequence == sequence);

    if (sequence == STALL_SEQUENCE) {
      vTaskDelay(PROCESSING_STALL_TICKS);
    }

    assert(signal_filter_push(&filter, sample.value, &average));
    assert(signal_state_update(&state, average, 2, 8));

    decision.average = average;
    decision.sequence = sequence;
    decision.state = state;
    assert(xQueueSend(decision_queue, &decision, pdMS_TO_TICKS(1000U)) ==
           pdPASS);
  }

  for (;;) {
    vTaskDelay(portMAX_DELAY);
  }
}

static void control_task(void *parameters) {
  bool fault_active = false;
  uint32_t decisions_received = 0;

  (void)parameters;

  while (decisions_received < SAMPLE_COUNT) {
    decision_t decision;

    if (xQueueReceive(decision_queue, &decision, PROCESSING_DEADLINE_TICKS) !=
        pdPASS) {
      fault_active = true;
      processing_stalled = pdTRUE;
      output_state = SIGNAL_STATE_LOW;
      continue;
    }

    assert(decision.sequence == decisions_received);
    assert(decision.average == expected_averages[decisions_received]);
    assert(decision.state == expected_states[decisions_received]);

    if (fault_active) {
      processing_resumed = pdTRUE;
      assert(output_state == SIGNAL_STATE_LOW);
    } else {
      output_state = decision.state;
    }

    decisions_received++;
  }

  assert(processing_stalled == pdTRUE);
  assert(processing_resumed == pdTRUE);
  assert(output_state == SIGNAL_STATE_LOW);

  test_complete = pdTRUE;
  vTaskEndScheduler();

  for (;;) {
    vTaskDelay(portMAX_DELAY);
  }
}

int main(void) {
  sample_queue = xQueueCreateStatic(QUEUE_LENGTH, sizeof(sample_t),
                                    sample_queue_buffer, &sample_queue_storage);
  decision_queue =
      xQueueCreateStatic(QUEUE_LENGTH, sizeof(decision_t),
                         decision_queue_buffer, &decision_queue_storage);
  assert(sample_queue != NULL);
  assert(decision_queue != NULL);

  assert(xTaskCreateStatic(sample_task, "sample", configMINIMAL_STACK_SIZE,
                           NULL, 1U, sample_task_stack,
                           &sample_task_tcb) != NULL);
  assert(xTaskCreateStatic(
             processing_task, "processing", configMINIMAL_STACK_SIZE, NULL, 2U,
             processing_task_stack, &processing_task_tcb) != NULL);
  assert(xTaskCreateStatic(control_task, "control", configMINIMAL_STACK_SIZE,
                           NULL, 3U, control_task_stack,
                           &control_task_tcb) != NULL);

  vTaskStartScheduler();

  assert(test_complete == pdTRUE);
  assert(processing_stalled == pdTRUE);
  assert(processing_resumed == pdTRUE);
  assert(output_state == SIGNAL_STATE_LOW);
  puts("PASS");
  return 0;
}
