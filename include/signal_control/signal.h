/**
 * @file signal.h
 * @brief Moving-average filtering and threshold classification.
 */

#ifndef SIGNAL_CONTROL_SIGNAL_H
#define SIGNAL_CONTROL_SIGNAL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/** Maximum number of samples supported by a moving-average filter. */
#define SIGNAL_FILTER_MAX_WINDOW 16u

/**
 * @brief State for a bounded moving-average filter.
 *
 * Initialize the filter with signal_filter_init() before using it. Treat the
 * fields as implementation state and do not modify them directly.
 */
typedef struct {
  uint16_t samples[SIGNAL_FILTER_MAX_WINDOW];
  uint32_t sum;
  size_t next;
  size_t count;
  size_t window_size;
} signal_filter_t;

/**
 * @brief Initialize a moving-average filter.
 *
 * A successful initialization starts the filter with no samples received.
 *
 * @param[out] filter Storage to initialize.
 * @param window_size Number of samples in the moving window.
 * @retval true The filter was initialized.
 * @retval false filter is NULL or window_size is outside the supported range.
 */
bool signal_filter_init(signal_filter_t *filter, size_t window_size);

/**
 * @brief Add a sample and compute the current moving average.
 *
 * During warm-up, the average uses the samples received so far. Once the
 * window is full, each new sample replaces the oldest one.
 *
 * @param[in,out] filter Filter state to update.
 * @param sample New sample value.
 * @param[out] average Output location for the computed average. Must not be
 *                      NULL.
 * @pre The filter was successfully initialized with signal_filter_init().
 * @retval true The sample was accepted and the average was written.
 * @retval false An argument or stored filter bound was invalid.
 */
bool signal_filter_push(signal_filter_t *filter, uint16_t sample,
                        uint16_t *average);

/** Result of comparing a sample with a threshold. */
typedef enum {
  SIGNAL_STATE_BELOW_THRESHOLD,
  SIGNAL_STATE_AT_OR_ABOVE_THRESHOLD
} signal_state_t;

/**
 * @brief Classify a sample against an inclusive threshold.
 *
 * @param sample Sample value to classify.
 * @param threshold Boundary between states.
 * @retval SIGNAL_STATE_AT_OR_ABOVE_THRESHOLD sample >= threshold.
 * @retval SIGNAL_STATE_BELOW_THRESHOLD sample < threshold.
 */
signal_state_t signal_classify(uint16_t sample, uint16_t threshold);

#endif
