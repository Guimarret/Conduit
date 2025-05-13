#include <stdio.h>
#include <unistd.h>
#include "thread.h"
#include "scheduler.h"
#include "database.h"
#include "logger.h"

void initialize_test_tasks(void) {

    add_task("Daily Backup", "0 0 * * *", "backup_system");
    add_task("Hourly Log Rotation", "0 * * * *", "rotate_logs");
    add_task("System Health Check", "*/15 * * * *", "check_health");
    add_task("Weekly Cleanup", "0 0 * * 0", "cleanup_temp");
    add_task("print_bin", "* * * * *", "print_bin");

    log_message("Test tasks initialized successfully\n");
}

int main(int argc, char *argv[]){
    sqlite3 *db;
    int log_status = init_logging(argc, argv);
    if (log_status != 0) {
        fprintf(stderr, "Failed to initialize logging. Exiting.\n");
        return 1;
    }

    db = initialize_database();
    dag_migration(db);

    initialize_test_tasks(); // Remove today if possible (13-05)
    while(1){
        free_tasks();
        dag_import(db, taskListHead);
        sleep(30);
    }

    start_webserver_thread(db);
    start_scheduler_thread();

    while(1){
        log_message("Main program is running\n");
        sleep(20);
    }

    shutdown_database(db);
    close_logging();
    return 0;
}
