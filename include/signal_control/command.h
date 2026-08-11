/**
 * @file command.h
 * @brief Bounded text commands for the signal-control node.
 */
#ifndef SIGNAL_CONTROL_COMMAND_H
#define SIGNAL_CONTROL_COMMAND_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/** Maximum number of characters accepted in one command line. */
#define SIGNAL_COMMAND_MAX_LINE_LENGTH 48u

/** Kind of command decoded from a command line. */
typedef enum {
  SIGNAL_COMMAND_STATUS,
  SIGNAL_COMMAND_SET_THRESHOLDS
} signal_command_type_t;

/**
 * @brief Parsed command request.
 *
 * The threshold fields are used only for SIGNAL_COMMAND_SET_THRESHOLDS. They
 * are zero for the other command types after a successful parse.
 */
typedef struct {
  signal_command_type_t type;
  uint16_t low_threshold;
  uint16_t high_threshold;
} signal_command_t;

/**
 * @brief Parse one bounded command line.
 *
 * Accepted commands are STATUS and SET THRESHOLDS followed by two decimal
 * uint16 values. The caller supplies a line without its line-ending
 * characters. Leading whitespace and tabs are not accepted; spaces between
 * tokens and at the end of a line are accepted.
 *
 * @param[in] line Command characters to parse.
 * @param length Number of characters in line.
 * @param[out] command Parsed command storage.
 * @post On failure, command is unchanged. On success, command is complete.
 * @retval true The line matched a supported command.
 * @retval false An argument, length, token, or threshold bound was invalid.
 */
bool signal_command_parse(const char *line, size_t length,
                          signal_command_t *command);

#endif
