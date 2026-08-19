#include "sigctl/signal.h"

#include <assert.h>

static void test_filter(void) {
  sigctl_filter_t filter;
  uint16_t average;

  assert(!sigctl_filter_init(NULL, 3));
  assert(!sigctl_filter_init(&filter, 0));
  assert(!sigctl_filter_init(&filter, SIGCTL_FILTER_MAX_WINDOW + 1u));
  assert(sigctl_filter_init(&filter, 3));
  assert(!sigctl_filter_push(&filter, 1, NULL));
  assert(sigctl_filter_push(&filter, 3, &average));
  assert(average == 3);
  assert(sigctl_filter_push(&filter, 5, &average));
  assert(average == 4);
  assert(sigctl_filter_push(&filter, 7, &average));
  assert(average == 5);
  assert(sigctl_filter_push(&filter, 10, &average));
  assert(average == 7);
}

static void test_classifier(void) {
  assert(sigctl_classify(9, 10) == SIGCTL_STATE_LOW);
  assert(sigctl_classify(10, 10) == SIGCTL_STATE_HIGH);
  assert(sigctl_classify(11, 10) == SIGCTL_STATE_HIGH);
}

static void test_hysteresis(void) {
  sigctl_state_t state = SIGCTL_STATE_LOW;

  assert(!sigctl_state_update(NULL, 5, 4, 8));
  assert(!sigctl_state_update(&state, 5, 8, 8));
  assert(!sigctl_state_update(&state, 5, 9, 8));
  assert(sigctl_state_update(&state, 4, 4, 8));
  assert(state == SIGCTL_STATE_LOW);
  assert(sigctl_state_update(&state, 8, 4, 8));
  assert(state == SIGCTL_STATE_HIGH);
  assert(sigctl_state_update(&state, 7, 4, 8));
  assert(state == SIGCTL_STATE_HIGH);
  assert(sigctl_state_update(&state, 4, 4, 8));
  assert(state == SIGCTL_STATE_LOW);

  state = (sigctl_state_t)2;
  assert(!sigctl_state_update(&state, 5, 4, 8));
}

int main(void) {
  test_filter();
  test_classifier();
  test_hysteresis();
  return 0;
}
