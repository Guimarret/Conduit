#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

int validate_cron_field(const char *field, int min, int max) {
    if (strcmp(field, "*") == 0) {
        return 1;
    }
    if (strchr(field, ',') != NULL) {
        char *token = strtok(strdup(field), ",");
        while (token != NULL) {
            int value;
            if (sscanf(token, "%d", &value) != 1 || value < min || value > max) {
                free(token);
                return 0;
            }
            token = strtok(NULL, ",");
        }
        return 1;
    }
     if (strchr(field, '/') != NULL) {
        int base, step;
        if (sscanf(field, "*/%d", &step) == 1 && step > 0) {
            return 1;
        }
         if (sscanf(field, "%d/%d", &base, &step) == 2 && base >= min && base <= max && step > 0) {
            return 1;
        }
        return 0;
    }
    if (strchr(field, '-') != NULL) {
        int start, end;
        if (sscanf(field, "%d-%d", &start, &end) == 2 && start >= min && end <= max && start <= end) {
            return 1;
        }
        return 0;
    }
    int value;
    if (sscanf(field, "%d", &value) == 1 && value >= min && value <= max) {
        return 1;
    }
    return 0;
}

int validate_cron_expression(const char *cron_expression) {
    char *fields[5];
    char *token = strtok(strdup(cron_expression), " ");
    int i = 0;
    while (token != NULL && i < 5) {
        fields[i++] = token;
        token = strtok(NULL, " ");
    }
    if (i != 5) {
        return 0;
    }

    if (!validate_cron_field(fields[0], 0, 59)) return 0;
    if (!validate_cron_field(fields[1], 0, 23)) return 0;
    if (!validate_cron_field(fields[2], 1, 31)) return 0;
    if (!validate_cron_field(fields[3], 1, 12)) return 0;
    if (!validate_cron_field(fields[4], 0, 6)) return 0;
    return 1;
}
