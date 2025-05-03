#ifndef CONDUIT_SCHEDULER_H
#define CONDUIT_SCHEDULER_H

typedef struct Task{
    char taskName[64];
    char cronExpression[256];
    char taskExecution[64];
    struct Task *next;
} Task;

void scheduler(void);
Task* add_task(const char *name, const char *cronExpression, const char *execution);
void free_tasks(void);

#endif
