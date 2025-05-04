#include <stdio.h>
#include <unistd.h>
#include "thread.h"
#include "scheduler.h"
#include "database.h"

void initialize_test_tasks(void) {

    add_task("Daily Backup", "0 0 * * *", "backup_system");
    add_task("Hourly Log Rotation", "0 * * * *", "rotate_logs");
    add_task("System Health Check", "*/15 * * * *", "check_health");
    add_task("Weekly Cleanup", "0 0 * * 0", "cleanup_temp");
    add_task("print_bin", "* * * * *", "print_bin");

    printf("Test tasks initialized successfully\n");
}

int main(void){
    sqlite3 *db;
    db = initialize_database();
    dag_migration(db);

    initialize_test_tasks();
    start_scheduler_thread();

    while(1){
        printf("Main program is running\n");
        sleep(20);
    }

    shutdown_database(db);
    return 0;
}
