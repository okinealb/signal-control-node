#include "sigctl_node/command.h"

#include <assert.h>
#include <string.h>

static void test_valid_commands(void) {
  sigctl_command_t command;

  assert(sigctl_command_parse("STATUS", strlen("STATUS"), &command));
  assert(command.type == SIGCTL_COMMAND_STATUS);
  assert(command.low_threshold == 0);
  assert(command.high_threshold == 0);

  assert(sigctl_command_parse("SET THRESHOLDS 0 65535",
                              strlen("SET THRESHOLDS 0 65535"), &command));
  assert(command.type == SIGCTL_COMMAND_SET_THRESHOLDS);
  assert(command.low_threshold == 0);
  assert(command.high_threshold == UINT16_MAX);

  assert(sigctl_command_parse("  STATUS ", strlen("  STATUS "), &command) ==
         false);
  assert(sigctl_command_parse("STATUS ", strlen("STATUS "), &command));
}

static void test_invalid_commands(void) {
  sigctl_command_t command = {SIGCTL_COMMAND_STATUS, 10, 20};
  sigctl_command_t before = command;
  char long_line[SIGCTL_COMMAND_MAX_LINE_LENGTH + 1u];

  memset(long_line, 'X', sizeof(long_line));
  assert(!sigctl_command_parse(NULL, 0, &command));
  assert(!sigctl_command_parse("STATUS", 6, NULL));
  assert(!sigctl_command_parse("STATUS", SIGCTL_COMMAND_MAX_LINE_LENGTH + 1u,
                               &command));
  assert(!sigctl_command_parse(long_line, SIGCTL_COMMAND_MAX_LINE_LENGTH + 1u,
                               &command));
  assert(!sigctl_command_parse("STATUS\n", strlen("STATUS\n"), &command));
  assert(!sigctl_command_parse("UNKNOWN", strlen("UNKNOWN"), &command));
  assert(!sigctl_command_parse("SET THRESHOLDS 8 8",
                               strlen("SET THRESHOLDS 8 8"), &command));
  assert(!sigctl_command_parse("SET THRESHOLDS 9 8",
                               strlen("SET THRESHOLDS 9 8"), &command));
  assert(!sigctl_command_parse("SET THRESHOLDS -1 8",
                               strlen("SET THRESHOLDS -1 8"), &command));
  assert(!sigctl_command_parse("SET THRESHOLDS 0 65536",
                               strlen("SET THRESHOLDS 0 65536"), &command));
  assert(!sigctl_command_parse("SET THRESHOLDS 0", strlen("SET THRESHOLDS 0"),
                               &command));

  assert(memcmp(&command, &before, sizeof(command)) == 0);
}

int main(void) {
  test_valid_commands();
  test_invalid_commands();
  return 0;
}
