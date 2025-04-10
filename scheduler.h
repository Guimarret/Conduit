typedef struct Task{
    char taskName[64];
    int hour;
    int minute;
    char task_execution[64];
    struct Task *next; // I think this one should be deleted and implement something like this in stack or heap future implementation
} Task;

void scheduler(void);
