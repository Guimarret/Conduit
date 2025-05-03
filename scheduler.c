#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "scheduler.h"
#include "cron.h"
#include "thread.h"

Task *taskListHead = NULL;

int is_time_to_run(const char *cronExpression, struct CronTime now) {
    char *fields[5];
    char *copy = strdup(cronExpression);
    char *token = strtok(copy, " ");

    for (int i = 0; i < 5; i++) {
        if (!token) {
            free(copy);
            return 0;
        }
        fields[i] = token;
        token = strtok(NULL, " ");
    }

    int result = 1;
    result &= match_cron_field(fields[0], now.minute, 0, 59);
    result &= match_cron_field(fields[1], now.hour, 0, 23);
    result &= match_cron_field(fields[2], now.day_of_month, 1, 31);
    result &= match_cron_field(fields[3], now.month, 1, 12);
    result &= match_cron_field(fields[4], now.day_of_week, 0, 6);

    free(copy);
    return result;
}

void execute_task(Task task) {
    spawn_worker_thread(&task);
    printf("Task triggered: %s\n", task.taskName);
}

Task* add_task(const char *name, const char *cronExpression, const char *execution) {
    Task *new_task = malloc(sizeof(Task));
    if (new_task == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }

    strlcpy(new_task->taskName, name, sizeof(new_task->taskName));
    strlcpy(new_task->taskExecution, execution, sizeof(new_task->taskExecution));
    strlcpy(new_task->cronExpression, cronExpression, sizeof(new_task->cronExpression));

    new_task->next = taskListHead;
    taskListHead = new_task;

    return new_task;
}

void free_tasks() {
    Task *current = taskListHead;
    while (current != NULL) {
        Task *next = current->next;
        free(current);
        current = next;
    }
    taskListHead = NULL;
    fprintf(stderr, "Tasks freed successfully\n");
}

void scheduler() {
    while(1) {
        time_t now = time(NULL);
        struct tm *current_time = localtime(&now);

        struct CronTime cronTime = {
                    current_time->tm_min,
                    current_time->tm_hour,
                    current_time->tm_mday,
                    current_time->tm_mon + 1,  // tm_mon is 0-11, CronTime expects 1-12
                    current_time->tm_wday  // 0-6, Sunday = 0
                };

        Task *current = taskListHead; // *current is the current task what will iterate all the other ones through the linked list
        while (current != NULL) {
            if (is_time_to_run(current->cronExpression, cronTime)) {
                execute_task(*current);
            }
            current = current->next;
        }
        sleep(30);
    }
}
