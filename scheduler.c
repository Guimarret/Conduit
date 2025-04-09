#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include "scheduler.h"

int is_time_to_run(struct tm current_time, int hour, int minute){
    return (current_time.tm_hour == hour && current_time.tm_min == minute);
}

// This task struct probably should be in sqlite db, because technically it can scale a lot
// Actually not sure because it will keep looking up every minute to match the time

int execute_task(char task_execution[64]){
    printf("Task to be executed trigged, %s", task_execution);
    return 1;
}

// I have to find a way to pass the schedule_time and task itself via struct, but the struct should be put in a list and maybe a stack of lists, not sure
void scheduler(struct Task *task){
    while(1){
        time_t now = time(NULL);
        struct tm *current_time = localtime(&now);
        if (is_time_to_run(*current_time, task->hour, task->minute)) {
                    execute_task(task->task_execution);
                    sleep(60);
        }
    }
}
