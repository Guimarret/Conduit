#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

int match_cron_field(const char *field, int value, int min, int max) {
    if (strcmp(field, "*") == 0) return 1;

    char *copy = strdup(field);
    char *token = strtok(copy, ",");
    int match_found = 0;

    while (token) {
        if (strchr(token, '-')) {
            int start, end;
            if (sscanf(token, "%d-%d", &start, &end) == 2 &&
                value >= start && value <= end) {
                match_found = 1;
                break;
            }
        } else if (strchr(token, '/')) {
            int base = min, step;
            if (sscanf(token, "*/%d", &step) == 1) {
                if (step > 0 && (value - min) % step == 0) {
                    match_found = 1;
                    break;
                }
            } else if (sscanf(token, "%d/%d", &base, &step) == 2) {
                if (value >= base && (value - base) % step == 0) {
                    match_found = 1;
                    break;
                }
            }
        } else {
            int target;
            if (sscanf(token, "%d", &target) == 1 && target == value) {
                match_found = 1;
                break;
            }
        }
        token = strtok(NULL, ",");
    }

    free(copy);
    return match_found;
}
