#include "signal_control/signal.h"

bool signal_filter_init(signal_filter_t *filter, size_t window_size) {
  if (filter == NULL || window_size == 0 ||
      window_size > SIGNAL_FILTER_MAX_WINDOW) {
    return false;
  }

  filter->sum = 0;
  filter->next = 0;
  filter->count = 0;
  filter->window_size = window_size;
  return true;
}

bool signal_filter_push(signal_filter_t *filter, uint16_t sample,
                        uint16_t *average) {
  if (filter == NULL || average == NULL || filter->window_size == 0 ||
      filter->window_size > SIGNAL_FILTER_MAX_WINDOW ||
      filter->next >= filter->window_size ||
      filter->count > filter->window_size) {
    return false;
  }

  if (filter->count == filter->window_size) {
    filter->sum -= filter->samples[filter->next];
  } else {
    filter->count++;
  }

  filter->samples[filter->next] = sample;
  filter->sum += sample;
  filter->next = (filter->next + 1) % filter->window_size;
  *average = (uint16_t)(filter->sum / filter->count);
  return true;
}

signal_state_t signal_classify(uint16_t sample, uint16_t threshold) {
  if (sample >= threshold) {
    return SIGNAL_STATE_HIGH;
  }

  return SIGNAL_STATE_LOW;
}

bool signal_state_update(signal_state_t *state, uint16_t value,
                         uint16_t low_threshold, uint16_t high_threshold) {
  if (state == NULL ||
      (*state != SIGNAL_STATE_LOW && *state != SIGNAL_STATE_HIGH) ||
      low_threshold >= high_threshold) {
    return false;
  }

  if (*state == SIGNAL_STATE_LOW && value >= high_threshold) {
    *state = SIGNAL_STATE_HIGH;
  } else if (*state == SIGNAL_STATE_HIGH && value <= low_threshold) {
    *state = SIGNAL_STATE_LOW;
  }

  return true;
}
