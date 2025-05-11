#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>

int init_logging(int argc, char *argv[]);
void log_message(const char* format, ...);
void close_logging();

#endif
