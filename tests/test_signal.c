#include "signal_control/signal.h"

#include <assert.h>

static void test_filter(void) {
  signal_filter_t filter;
  uint16_t average;

  assert(!signal_filter_init(NULL, 3));
  assert(!signal_filter_init(&filter, 0));
  assert(!signal_filter_init(&filter, SIGNAL_FILTER_MAX_WINDOW + 1u));
  assert(signal_filter_init(&filter, 3));
  assert(!signal_filter_push(&filter, 1, NULL));
  assert(signal_filter_push(&filter, 3, &average));
  assert(average == 3);
  assert(signal_filter_push(&filter, 5, &average));
  assert(average == 4);
  assert(signal_filter_push(&filter, 7, &average));
  assert(average == 5);
  assert(signal_filter_push(&filter, 10, &average));
  assert(average == 7);
}

static void test_classifier(void) {
  assert(signal_classify(9, 10) == SIGNAL_STATE_BELOW_THRESHOLD);
  assert(signal_classify(10, 10) == SIGNAL_STATE_AT_OR_ABOVE_THRESHOLD);
  assert(signal_classify(11, 10) == SIGNAL_STATE_AT_OR_ABOVE_THRESHOLD);
}

int main(void) {
  test_filter();
  test_classifier();
  return 0;
}
