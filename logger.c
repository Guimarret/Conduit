#include <stdio.h>
#include <string.h>
#include <stdarg.h>

int debug_mode = 0;
FILE *log_file = NULL;

void log_message(const char* format, ...) {
    va_list args;
    va_start(args, format);
    if (debug_mode) {
        vfprintf(stdout, format, args);
        fflush(stdout);
    } else if (log_file != NULL) {
        vfprintf(log_file, format, args);
        fflush(log_file);
    }
    va_end(args);
}

int init_logging(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "--debug") == 0) {
                debug_mode = 1;
                break;
            }
        }

    if (!debug_mode) {
        log_file = fopen("app.log", "a");
        if (!log_file) {
            fprintf(stderr, "Could not open log file.\n");
            return 1;
        }
    }

    log_message("This is a test message: %d\n", 42);
    return 0;
}

void close_logging() {
    if (log_file) {
        fclose(log_file);
        log_file = NULL;
    }
}
