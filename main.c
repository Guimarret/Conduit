#include <stdio.h>
#include <unistd.h>
#include "thread.h"

int main(void){
    start_scheduler_thread();
    init_worker_thread();

    while(1){
        printf("Main program is running\n");
        sleep(20);
    }
    return 0;
}
