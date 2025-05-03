typedef struct Task{
    char taskName[64];
    char cronExpression[256];
    char taskExecution[64];
    struct Task *next;
} Task;

void scheduler(void);
