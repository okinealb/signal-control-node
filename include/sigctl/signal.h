/**
 * @file signal.h
 * @brief Moving-average filtering and threshold state decisions.
 */

#ifndef SIGCTL_SIGNAL_H
#define SIGCTL_SIGNAL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/** Maximum number of samples supported by a moving-average filter. */
#define SIGCTL_FILTER_MAX_WINDOW 16u

/**
 * @brief State for a bounded moving-average filter.
 *
 * Initialize the filter with sigctl_filter_init() before using it. Treat the
 * fields as implementation state and do not modify them directly.
 */
typedef struct {
  uint16_t samples[SIGCTL_FILTER_MAX_WINDOW];
  uint32_t sum;
  size_t next;
  size_t count;
  size_t window_size;
} sigctl_filter_t;

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
bool sigctl_filter_init(sigctl_filter_t *filter, size_t window_size);

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
 * @pre The filter was successfully initialized with sigctl_filter_init().
 * @retval true The sample was accepted and the average was written.
 * @retval false An argument or stored filter bound was invalid.
 */
bool sigctl_filter_push(sigctl_filter_t *filter, uint16_t sample,
                        uint16_t *average);

/** Low or high state for a signal decision. */
typedef enum { SIGCTL_STATE_LOW, SIGCTL_STATE_HIGH } sigctl_state_t;

/**
 * @brief Classify a value against an inclusive threshold.
 *
 * This comparison does not retain state.
 *
 * @param sample Value to classify.
 * @param threshold Boundary between states.
 * @retval SIGCTL_STATE_HIGH sample >= threshold.
 * @retval SIGCTL_STATE_LOW sample < threshold.
 */
sigctl_state_t sigctl_classify(uint16_t sample, uint16_t threshold);

/**
 * @brief Update a signal state with two-threshold hysteresis.
 *
 * The caller owns the state and must initialize it to a valid enum value.
 * Values between the thresholds preserve the current state.
 * The low threshold must be strictly less than the high threshold.
 *
 * @param[in,out] state State to update.
 * @param value Value to compare.
 * @param low_threshold Value at or below which the high state clears.
 * @param high_threshold Value at or above which the low state sets.
 * @retval true The state was valid and the transition was evaluated.
 * @retval false An argument, state, or threshold bound was invalid.
 */
bool sigctl_state_update(sigctl_state_t *state, uint16_t value,
                         uint16_t low_threshold, uint16_t high_threshold);

#endif
