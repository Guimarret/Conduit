#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>

void init_logging(const char* filename);
void log_message(const char* message);
void close_logging();

#endif
