#include <stdio.h>

static FILE* log_file = NULL;

void init_logging(const char* filename) {
    log_file = fopen(filename, "a");
}

void log_message(const char* message) {
    if (log_file) {
        fprintf(log_file, "%s\n", message);
        fflush(log_file);
    }
}

void close_logging() {
    if (log_file) {
        fclose(log_file);
        log_file = NULL;
    }
}
