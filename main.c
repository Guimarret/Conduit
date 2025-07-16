#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "thread.h"
#include "scheduler.h"
#include "database.h"
#include "logger.h"
#include "transactions.h"
#include "dag_scheduler.h"

void initialize_test_tasks(void) {

    add_task("Daily Backup", "0 0 * * *", "backup_system");
    add_task("Hourly Log Rotation", "0 * * * *", "rotate_logs");
    add_task("System Health Check", "*/15 * * * *", "check_health");
    add_task("Weekly Cleanup", "0 0 * * 0", "cleanup_temp");
    add_task("print_bin", "* * * * *", "print_bin");

    log_message("Test tasks initialized successfully\n");
}

// Start DAG scheduler in separate thread
void start_dag_scheduler_thread(sqlite3 *db) {
    pthread_t dag_scheduler_thread;
    int result = pthread_create(&dag_scheduler_thread, NULL, (void*)dag_scheduler, db);
    if (result != 0) {
        log_message("Failed to create DAG scheduler thread\n");
    } else {
        log_message("DAG scheduler thread started\n");
        pthread_detach(dag_scheduler_thread);
    }
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
    transactions_status_migration(db);

    initialize_test_tasks(); // Remove today if possible (13-05)
    
    log_message("Starting Conduit application with DAG support\n");
    
    // Start the webserver thread
    start_webserver_thread(db);
    
    // Start both legacy task scheduler and new DAG scheduler
    start_scheduler_thread(db);      // Legacy individual task scheduling
    start_dag_scheduler_thread(db);  // New DAG scheduling with dependencies
    
    log_message("All threads started successfully\n");

    // Main loop for DAG import and general operation
    while(1){
        // Legacy task management (can be removed once fully migrated to DAGs)
        free_tasks();
        dag_import(db, taskListHead);
        
        log_message("Main program is running - both legacy tasks and DAGs active\n");
        sleep(60); // Increased to 60 seconds to reduce log noise
    }

    shutdown_database(db);
    close_logging();
    return 0;
}
