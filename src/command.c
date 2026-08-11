#include "signal_control/command.h"

#include <string.h>

static void skip_spaces(const char **cursor, const char *end) {
  while (*cursor < end && **cursor == ' ') {
    (*cursor)++;
  }
}

static bool match_word(const char **cursor, const char *end, const char *word) {
  size_t word_length = strlen(word);

  if ((size_t)(end - *cursor) < word_length ||
      memcmp(*cursor, word, word_length) != 0) {
    return false;
  }

  *cursor += word_length;
  return *cursor == end || **cursor == ' ';
}

static bool parse_uint16(const char **cursor, const char *end,
                         uint16_t *value) {
  uint32_t parsed = 0;

  if (*cursor == end || **cursor < '0' || **cursor > '9') {
    return false;
  }

  while (*cursor < end && **cursor >= '0' && **cursor <= '9') {
    parsed = parsed * 10u + (uint32_t)(**cursor - '0');
    if (parsed > UINT16_MAX) {
      return false;
    }
    (*cursor)++;
  }

  *value = (uint16_t)parsed;
  return true;
}

bool signal_command_parse(const char *line, size_t length,
                          signal_command_t *command) {
  signal_command_t parsed = {0};
  const char *end;
  const char *cursor;

  if (line == NULL || command == NULL ||
      length > SIGNAL_COMMAND_MAX_LINE_LENGTH) {
    return false;
  }

  end = line + length;

  cursor = line;
  if (match_word(&cursor, end, "STATUS")) {
    skip_spaces(&cursor, end);
    if (cursor == end) {
      parsed.type = SIGNAL_COMMAND_STATUS;
      *command = parsed;
      return true;
    }
  }

  cursor = line;
  if (match_word(&cursor, end, "SET")) {
    skip_spaces(&cursor, end);
    if (match_word(&cursor, end, "THRESHOLDS")) {
      skip_spaces(&cursor, end);
      if (parse_uint16(&cursor, end, &parsed.low_threshold)) {
        skip_spaces(&cursor, end);
        if (parse_uint16(&cursor, end, &parsed.high_threshold)) {
          skip_spaces(&cursor, end);
          if (cursor == end && parsed.low_threshold < parsed.high_threshold) {
            parsed.type = SIGNAL_COMMAND_SET_THRESHOLDS;
            *command = parsed;
            return true;
          }
        }
      }
    }
  }

  return false;
}
