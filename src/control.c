#include "sigctl_node/control.h"

bool sigctl_control_init(sigctl_control_t *control) {
  if (control == NULL) {
    return false;
  }

  control->fault_active = false;
  control->output_state = SIGCTL_STATE_LOW;
  return true;
}

bool sigctl_control_apply_decision(sigctl_control_t *control,
                                   sigctl_state_t decision_state) {
  if (control == NULL || (decision_state != SIGCTL_STATE_LOW &&
                          decision_state != SIGCTL_STATE_HIGH)) {
    return false;
  }

  if (!control->fault_active) {
    control->output_state = decision_state;
  }

  return true;
}

bool sigctl_control_record_stall(sigctl_control_t *control) {
  if (control == NULL) {
    return false;
  }

  control->fault_active = true;
  control->output_state = SIGCTL_STATE_LOW;
  return true;
}
