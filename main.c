#include <stdio.h>
#include <unistd.h>
#include "thread.h"

int main(void){
    start_scheduler_thread();

    while(1){
        printf("Main program is running");
        sleep(20);
    }

    return 0;
}
