#ifndef CONDUIT_THREAD_H
#define CONDUIT_THREAD_H

// Forward declaration instead of including scheduler.h
struct Task;  // Just declare the struct without defining it
typedef struct Task Task;  // Define the typedef

void start_scheduler_thread(void);
void spawn_worker_thread(Task *task);

typedef struct ThreadParams{
    int taskId;
    char taskName[64];
    char taskExecution[64];
} ThreadParams;

#endif
