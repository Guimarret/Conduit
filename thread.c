#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "scheduler.h"

void *thread_function(void *arg) {
    scheduler();
    return NULL;
}

void start_scheduler_thread() {
    pthread_t thread_id;

    if (pthread_create(&thread_id, NULL, thread_function, NULL) != 0) {
        perror("Failed to create thread");
        exit(EXIT_FAILURE);
    }
    pthread_detach(thread_id);
    // Getting warn with just the thread_id and the makefile Werror flag dont compile irra!!
    printf("Scheduler thread started with ID: %ld\n", (unsigned long)thread_id);

    return;
}
