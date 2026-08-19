/**
 * @file status.h
 * @brief Snapshot data for signal-node status responses.
 */
#ifndef SIGCTL_NODE_STATUS_H
#define SIGCTL_NODE_STATUS_H

#include <stdbool.h>
#include <stdint.h>

#include "sigctl/signal.h"

/**
 * @brief Snapshot of the latest processed sample and controlled output.
 *
 * When sample_available is false, the sample fields do not describe a
 * processed sample. The signal and output states can differ when a latched
 * fault forces the output to its safe state.
 */
typedef struct {
  bool sample_available; /**< Whether a processed sample is available. */
  uint32_t
      sample_sequence; /**< Sequence number of the latest processed sample. */
  uint16_t
      sample_average; /**< Moving average for the latest processed sample. */
  sigctl_state_t signal_state; /**< Classified state for the latest average. */
  sigctl_state_t output_state; /**< State selected for the controlled output. */
  bool fault_active; /**< Whether output control has a latched fault. */
} sigctl_status_t;

#endif
