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
    // #TODO add file execution, also decide if the file is gonna be in C or something else
    printf("Task to be executed trigged, %s", task_execution);
    return 1;
}

// I have to find a way to pass the schedule_time and task itself via struct, but the struct should be put in a list and maybe a stack of lists, not sure
// Completely alucinated here, this is a listenet there is no param to be passed, but it should look for some struct to check if there is any hour && minute that match the actual hour so it pass the task_execution to the execution_task
// Just melted so i gonna catch up from here tomorrow i think
void scheduler(){
    while(1){
        time_t now = time(NULL);
        struct tm *current_time = localtime(&now);
        if (is_time_to_run(*current_time, task->hour, task->minute)) {
                    execute_task(task->task_execution);
                    sleep(60);
        }
    }
}
