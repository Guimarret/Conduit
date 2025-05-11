#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "worker.h"
#include "thread.h"
#include "scheduler.h"
#include "hash.h"
#include "webserver.h"
#include "logger.h"

void *thread_scheduler_function(void *arg) {
    scheduler();
    return NULL;
}

void *thread_worker_function(void *arg) {
    ThreadParams* params = (ThreadParams*)arg;

    // Now you can use the parameters
    log_message("Worker processing task ID: %d, execution: %s\n",
            params->taskId, params->taskExecution);

    worker(params->taskId, params->taskExecution);

    free(params);
    return NULL;
}

void *thread_webserver_function(void *arg) {
    sqlite3 *db = (sqlite3 *)arg;
    initialize_webserver(db);
    return NULL;
}

void start_scheduler_thread() {
    pthread_t thread_id;

    if (pthread_create(&thread_id, NULL, thread_scheduler_function, NULL) != 0) {
        perror("Failed to create thread");
        exit(EXIT_FAILURE);
    }
    pthread_detach(thread_id);
    // Getting warn with just the thread_id and the makefile Werror flag dont compile irra!!
    log_message("Thread started with ID: %ld\n", (unsigned long)thread_id);

    return;
}

void start_webserver_thread(sqlite3 *db) {
    pthread_t thread_id;

    if (pthread_create(&thread_id, NULL, thread_webserver_function, db) != 0) {
        perror("Failed to create thread");
        exit(EXIT_FAILURE);
    }
    pthread_detach(thread_id);
    log_message("Thread started with ID: %ld\n", (unsigned long)thread_id);

    return;
}

void spawn_worker_thread(Task *task) {
    pthread_t thread_id;

    if (task == NULL) {
           log_message("Error: Task is NULL\n");
           return;
       }

       // Allocate memory for parameters
       ThreadParams* params = malloc(sizeof(ThreadParams));
       if (params == NULL) {
           perror("Failed to allocate memory for thread parameters");
           exit(EXIT_FAILURE);
    }

    // To be fair this is more like testing things than anything
    int taskId = 0;
    taskId = hashString(task->taskName);
    params->taskId = taskId;
    strcpy(params->taskExecution, task->taskExecution);

    if (pthread_create(&thread_id, NULL, thread_worker_function, params) != 0) {
        perror("Failed to create worker thread");
        free(params);  // Clean up on error
        exit(EXIT_FAILURE);
    }
    pthread_detach(thread_id);

    log_message("Worker thread start with ID: %ld\n", (unsigned long)thread_id);

    return;
}
