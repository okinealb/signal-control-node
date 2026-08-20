#include "sigctl/signal.h"
#include "sigctl_node/command.h"
#include "sigctl_node/control.h"
#include "sigctl_node/status.h"

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
  sigctl_state_t state;
} decision_t;

static const uint16_t input_values[SAMPLE_COUNT] = {0, 0, 10, 10, 0, 0};
static const uint16_t expected_averages[SAMPLE_COUNT] = {0, 0, 5, 10, 5, 0};
static const sigctl_state_t expected_states[SAMPLE_COUNT] = {
    SIGCTL_STATE_LOW,  SIGCTL_STATE_LOW,  SIGCTL_STATE_LOW,
    SIGCTL_STATE_HIGH, SIGCTL_STATE_HIGH, SIGCTL_STATE_LOW};

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
static volatile BaseType_t processing_resumed = pdFALSE;

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
  sigctl_filter_t filter;
  sigctl_state_t state = SIGCTL_STATE_LOW;

  (void)parameters;
  assert(sigctl_filter_init(&filter, FILTER_WINDOW));

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

    assert(sigctl_filter_push(&filter, sample.value, &average));
    assert(sigctl_state_update(&state, average, 2, 8));

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
  sigctl_control_t control;
  sigctl_command_t command;
  sigctl_status_t status = {0};
  uint32_t decisions_received = 0;
  bool status_captured = false;

  (void)parameters;
  assert(sigctl_control_init(&control));
  assert(sigctl_command_parse("STATUS", sizeof("STATUS") - 1u, &command));
  assert(command.type == SIGCTL_COMMAND_STATUS);

  while (decisions_received < SAMPLE_COUNT) {
    decision_t decision;

    if (xQueueReceive(decision_queue, &decision, PROCESSING_DEADLINE_TICKS) !=
        pdPASS) {
      assert(sigctl_control_record_stall(&control));
      continue;
    }

    assert(decision.sequence == decisions_received);
    assert(decision.average == expected_averages[decisions_received]);
    assert(decision.state == expected_states[decisions_received]);

    const bool fault_active = control.fault_active;
    assert(sigctl_control_apply_decision(&control, decision.state));

    if (fault_active) {
      processing_resumed = pdTRUE;
      if (!status_captured && command.type == SIGCTL_COMMAND_STATUS &&
          decision.state == SIGCTL_STATE_HIGH) {
        status.sample_available = true;
        status.sample_sequence = decision.sequence;
        status.sample_average = decision.average;
        status.signal_state = decision.state;
        status.output_state = control.output_state;
        status.fault_active = control.fault_active;
        status_captured = true;
      }
      assert(control.output_state == SIGCTL_STATE_LOW);
    }

    decisions_received++;
  }

  assert(control.fault_active == true);
  assert(processing_resumed == pdTRUE);
  assert(control.output_state == SIGCTL_STATE_LOW);
  assert(status_captured);
  assert(status.sample_available);
  assert(status.sample_sequence == 3U);
  assert(status.sample_average == 10U);
  assert(status.signal_state == SIGCTL_STATE_HIGH);
  assert(status.output_state == SIGCTL_STATE_LOW);
  assert(status.fault_active);

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
  assert(processing_resumed == pdTRUE);
  puts("PASS");
  return 0;
}
