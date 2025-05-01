typedef struct Task{
    char taskName[64];
    char cron_expression[256];
    char task_execution[64];
    struct Task *next;
} Task;

void scheduler(void);
