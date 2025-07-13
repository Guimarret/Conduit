#ifndef CONDUIT_THREAD_H
#define CONDUIT_THREAD_H

#include "database.h"

struct Task;
typedef struct Task Task;

void start_scheduler_thread(sqlite3 *db);
void spawn_worker_thread(Task *task);
void start_webserver_thread(sqlite3 *db);

typedef struct ThreadParams{
    int taskId;
    char taskName[64];
    char taskExecution[64];
} ThreadParams;

#endif
