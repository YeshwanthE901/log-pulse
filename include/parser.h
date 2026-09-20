#ifndef PARSER_H
#define PARSER_H

#include "logpulse.h"

int time_to_minute(const char *time);

int parse_log_line(const char *line, LogRecord *record);

#endif