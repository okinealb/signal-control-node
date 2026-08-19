/**
 * @file control.h
 * @brief Fault-aware output control for the signal node.
 */

#ifndef SIGCTL_NODE_CONTROL_H
#define SIGCTL_NODE_CONTROL_H

#include <stdbool.h>

#include "sigctl/signal.h"

/** State owned by the node's output controller. */
typedef struct {
  bool fault_active;
  sigctl_state_t output_state;
} sigctl_control_t;

/**
 * @brief Initialize node control state.
 *
 * Initialization starts with no fault and a safe low output.
 *
 * @param[out] control Storage to initialize.
 * @retval true The control state was initialized.
 * @retval false control is NULL.
 */
bool sigctl_control_init(sigctl_control_t *control);

/**
 * @brief Apply a processed signal decision to the node output.
 *
 * A latched fault keeps the output low even when later decisions arrive.
 *
 * @param[in,out] control Control state to update.
 * @param decision_state State produced by signal processing.
 * @pre control was initialized with sigctl_control_init().
 * @retval true The decision was accepted.
 * @retval false control is NULL or decision_state is invalid.
 */
bool sigctl_control_apply_decision(sigctl_control_t *control,
                                   sigctl_state_t decision_state);

/**
 * @brief Latch a processing-stalled fault and force a safe output.
 *
 * The fault remains active until a future application-owned clear operation
 * is defined.
 *
 * @param[in,out] control Control state to update.
 * @pre control was initialized with sigctl_control_init().
 * @retval true The fault was recorded.
 * @retval false control is NULL.
 */
bool sigctl_control_record_stall(sigctl_control_t *control);

#endif
