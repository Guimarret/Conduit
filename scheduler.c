#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "scheduler.h"

Task *taskListHead = NULL;

int is_time_to_run(struct tm current_time, int hour, int minute){
    return (current_time.tm_hour == hour && current_time.tm_min == minute);
}

int execute_task(char task_execution[64]){
    // #TODO add file execution, also decide if the file is gonna be in C or something else
    printf("Task to be executed trigged, %s", task_execution);
    return 1;
}

Task* add_task(const char *name, int hour, int minute, const char *execution) {
    Task *new_task = malloc(sizeof(Task));
    if (new_task == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }

    strlcpy(new_task->taskName, name, sizeof(new_task->taskName));
    strlcpy(new_task->task_execution, execution, sizeof(new_task->task_execution));

    new_task->hour = hour;
    new_task->minute = minute;

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
        Task *current = taskListHead;
        while (current != NULL) {
            if (is_time_to_run(*current_time, current->hour, current->minute)) {
                execute_task(current->task_execution);
            }
            current = current->next;
        }
        sleep(30);
    }
}
