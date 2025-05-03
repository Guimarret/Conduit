#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "scheduler.h"
#include "worker.h"

void *thread_scheduler_function(void *arg) {
    scheduler();
    return NULL;
}

void *thread_worker_function(void *arg) {
    ThreadParams* params = (ThreadParams*)arg;

    // Now you can use the parameters
    printf("Worker processing task ID: %d, execution: %s\n",
            params->taskId, params->taskExecution);

    worker(params->taskId, params->taskExecution);

    free(params->taskExecution);
    free(params);
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
    printf("Thread started with ID: %ld\n", (unsigned long)thread_id);

    return;
}

void init_worker_thread() {
    pthread_t thread_id;

    if (pthread_create(&thread_id, NULL, thread_worker_function, NULL) != 0) {
        perror("Failed to create worker thread");
        exit(EXIT_FAILURE);
    }
    pthread_detach(thread_id);

    printf("Worker thread start with ID: %ld\n", (unsigned long)thread_id);

    return;
}
