#include "sigctl/signal.h"

bool sigctl_filter_init(sigctl_filter_t *filter, size_t window_size) {
  if (filter == NULL || window_size == 0 ||
      window_size > SIGCTL_FILTER_MAX_WINDOW) {
    return false;
  }

  filter->sum = 0;
  filter->next = 0;
  filter->count = 0;
  filter->window_size = window_size;
  return true;
}

bool sigctl_filter_push(sigctl_filter_t *filter, uint16_t sample,
                        uint16_t *average) {
  if (filter == NULL || average == NULL || filter->window_size == 0 ||
      filter->window_size > SIGCTL_FILTER_MAX_WINDOW ||
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

sigctl_state_t sigctl_classify(uint16_t sample, uint16_t threshold) {
  if (sample >= threshold) {
    return SIGCTL_STATE_HIGH;
  }

  return SIGCTL_STATE_LOW;
}

bool sigctl_state_update(sigctl_state_t *state, uint16_t value,
                         uint16_t low_threshold, uint16_t high_threshold) {
  if (state == NULL ||
      (*state != SIGCTL_STATE_LOW && *state != SIGCTL_STATE_HIGH) ||
      low_threshold >= high_threshold) {
    return false;
  }

  if (*state == SIGCTL_STATE_LOW && value >= high_threshold) {
    *state = SIGCTL_STATE_HIGH;
  } else if (*state == SIGCTL_STATE_HIGH && value <= low_threshold) {
    *state = SIGCTL_STATE_LOW;
  }

  return true;
}
