/**
 * @author      : alex (alexbora@gmail.com)
 * @file        : log
 * @created     : Duminică Noi 26, 2023 10:07:34 EET
 */

#ifndef LOG_H

#define LOG_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

enum LogLevel { INFO, WARN, ERROR };
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_RESET "\x1b[0m"

void log_message(enum LogLevel, const char *);

#endif /* end of include guard LOG_H */

