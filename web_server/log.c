/**
 * @author      : alex (alexbora@gmail.com)
 * @file        : log
 * @created     : Duminică Noi 26, 2023 10:05:22 EET
 */

#include "log.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void inline log_message(enum LogLevel level, const char *message) {
  time_t rawtime = 0;
  struct tm *timeinfo = NULL;
  const char *level_str = NULL;
  const char *color = ANSI_COLOR_RESET;
  switch (level) {
    case INFO:
      level_str = "INFO";
      color = ANSI_COLOR_GREEN;
      break;
    case WARN:
      level_str = "WARN";
      color = ANSI_COLOR_YELLOW;
      break;
    case ERROR:
      level_str = "ERROR";
      color = ANSI_COLOR_RED;
      break;
    default:
      level_str = "UNKNOWN";
      color = ANSI_COLOR_RESET;
      break;
  }

  rawtime = time(0);
  time(&rawtime);
  timeinfo = localtime(&rawtime);

  fprintf(stderr, "%s[%s]  %s  %s", color, level_str, message,
          asctime(timeinfo));
  fprintf(stderr, "%s\n", ANSI_COLOR_RESET);
  if (level == ERROR) exit(errno);
}

