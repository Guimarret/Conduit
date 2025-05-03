void start_scheduler_thread();
void spawn_worker_thread();
void spawn_worker_thread(struct Task *task);

typedef struct {
    int taskId;
    char taskName[64];
    char taskExecution[64];
} ThreadParams;
