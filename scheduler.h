typedef struct Task{
    char taskName[64];
    int hour;
    int minute;
    char task_execution[64];
    struct Task *next;
} Task;
