#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include "dag_scheduler.h"
#include "dag.h"
#include "database.h"
#include "logger.h"
#include "thread.h"

// Global DAG list
static DAG *dag_list_head = NULL;
static pthread_mutex_t dag_list_mutex = PTHREAD_MUTEX_INITIALIZER;

// DAG Scheduler Functions

void load_dags_from_database(sqlite3 *db) {
    pthread_mutex_lock(&dag_list_mutex);
    
    // Free existing DAG list
    if (dag_list_head) {
        free_dag_list(dag_list_head);
        dag_list_head = NULL;
    }
    
    // Load DAGs from database
    dag_list_head = load_all_dags_db(db);
    
    pthread_mutex_unlock(&dag_list_mutex);
    
    if (dag_list_head) {
        log_message("Loaded DAGs from database\n");
    } else {
        log_message("No DAGs found in database\n");
    }
}

int is_dag_time_to_run(const char *cronExpression, struct CronTime now) {
    char *fields[5];
    char *copy = strdup(cronExpression);
    char *token = strtok(copy, " ");

    for (int i = 0; i < 5; i++) {
        if (!token) {
            free(copy);
            return 0;
        }
        fields[i] = token;
        token = strtok(NULL, " ");
    }

    int result = 1;
    result &= match_cron_field(fields[0], now.minute, 0, 59);
    result &= match_cron_field(fields[1], now.hour, 0, 23);
    result &= match_cron_field(fields[2], now.day_of_month, 1, 31);
    result &= match_cron_field(fields[3], now.month, 1, 12);
    result &= match_cron_field(fields[4], now.day_of_week, 0, 6);

    free(copy);
    return result;
}

int execute_dag(sqlite3 *db, DAG *dag) {
    if (!dag || dag->status != DAG_STATUS_ACTIVE) {
        return -1;
    }
    
    log_message("Starting execution of DAG: %s\n", dag->name);
    
    // Validate DAG dependencies before execution
    if (!validate_dag_dependencies(dag)) {
        log_message("DAG %s has invalid dependencies, skipping execution\n", dag->name);
        return -1;
    }
    
    // Generate unique execution ID
    char *execution_id = generate_execution_id(dag->id);
    if (!execution_id) {
        log_message("Failed to generate execution ID for DAG %s\n", dag->name);
        return -1;
    }
    
    // Start DAG execution record
    int dag_execution_db_id = start_dag_execution(db, dag->id, execution_id);
    if (dag_execution_db_id < 0) {
        log_message("Failed to start DAG execution record for %s\n", dag->name);
        free(execution_id);
        return -1;
    }
    
    // Create execution queue based on dependencies
    ExecutionQueue *queue = create_execution_queue(dag);
    if (!queue) {
        log_message("Failed to create execution queue for DAG %s\n", dag->name);
        free(execution_id);
        return -1;
    }
    
    // Execute tasks based on dependency order
    int total_tasks = dag->task_count;
    int completed_tasks = 0;
    int failed_tasks = 0;
    
    while (completed_tasks + failed_tasks < total_tasks) {
        ExecutionQueue *ready_tasks = get_ready_tasks(queue);
        
        if (!ready_tasks) {
            // Check if we're deadlocked (no ready tasks but not all completed)
            log_message("No ready tasks found for DAG %s, possible deadlock\n", dag->name);
            break;
        }
        
        // Execute all ready tasks in parallel
        ExecutionQueue *current_ready = ready_tasks;
        while (current_ready) {
            DAGTask *task = current_ready->task;
            
            // Create task execution record
            TaskExecution task_exec = {0};
            task_exec.dag_execution_id = dag_execution_db_id;
            task_exec.task_id = task->id;
            strncpy(task_exec.task_name, task->task_name, MAX_TASK_NAME_LENGTH - 1);
            task_exec.status = EXECUTION_STATUS_RUNNING;
            task_exec.started_at = time(NULL);
            
            int task_exec_id = insert_task_execution_db(db, &task_exec);
            
            log_message("Executing task: %s (ID: %d) in DAG: %s\n", 
                       task->task_name, task->id, dag->name);
            
            // Log task status
            log_dag_task_status(db, task->id, dag->id, dag_execution_db_id, 
                               "STARTED", task->task_execution);
            
            // Execute the task (spawn worker thread)
            ThreadParams *params = malloc(sizeof(ThreadParams));
            if (params) {
                params->taskId = task->id;
                strncpy(params->taskName, task->task_name, sizeof(params->taskName) - 1);
                strncpy(params->taskExecution, task->task_execution, sizeof(params->taskExecution) - 1);
                
                // For now, execute synchronously for dependency management
                // In a production system, you'd want async execution with proper coordination
                int execution_result = execute_task_sync(params);
                
                if (execution_result == 0) {
                    // Task succeeded
                    mark_task_completed(queue, task->id);
                    update_task_execution_status_db(db, task_exec_id, EXECUTION_STATUS_SUCCESS, NULL);
                    log_dag_task_status(db, task->id, dag->id, dag_execution_db_id, 
                                       "COMPLETED", "Task completed successfully");
                    completed_tasks++;
                    log_message("Task %s completed successfully\n", task->task_name);
                } else {
                    // Task failed
                    update_task_execution_status_db(db, task_exec_id, EXECUTION_STATUS_FAILED, 
                                                   "Task execution failed");
                    log_dag_task_status(db, task->id, dag->id, dag_execution_db_id, 
                                       "FAILED", "Task execution failed");
                    failed_tasks++;
                    log_message("Task %s failed\n", task->task_name);
                }
                
                free(params);
            }
            
            current_ready = current_ready->next;
        }
        
        // Free ready tasks list
        ExecutionQueue *temp = ready_tasks;
        while (temp) {
            ExecutionQueue *next = temp->next;
            free(temp);
            temp = next;
        }
        
        // If any task failed, decide whether to continue or abort
        if (failed_tasks > 0) {
            log_message("DAG %s has %d failed tasks, aborting execution\n", dag->name, failed_tasks);
            break;
        }
        
        // Small delay to prevent tight loops
        usleep(100000); // 100ms
    }
    
    // Update DAG execution status
    ExecutionStatus final_status = (failed_tasks > 0) ? EXECUTION_STATUS_FAILED : EXECUTION_STATUS_SUCCESS;
    char completion_message[256];
    snprintf(completion_message, sizeof(completion_message), 
             "DAG execution completed: %d successful, %d failed", completed_tasks, failed_tasks);
    
    update_dag_execution_status_db(db, dag_execution_db_id, final_status, 
                                  (failed_tasks > 0) ? completion_message : NULL);
    
    // Free execution queue
    ExecutionQueue *temp = queue;
    while (temp) {
        ExecutionQueue *next = temp->next;
        free(temp);
        temp = next;
    }
    
    free(execution_id);
    
    log_message("DAG %s execution completed: %d successful, %d failed\n", 
               dag->name, completed_tasks, failed_tasks);
    
    return (failed_tasks > 0) ? -1 : 0;
}

int execute_task_sync(ThreadParams *params) {
    if (!params) return -1;
    
    log_message("Executing task: %s with command: %s\n", params->taskName, params->taskExecution);
    
    // Execute the task command
    int result = system(params->taskExecution);
    
    if (result == 0) {
        log_message("Task %s completed successfully\n", params->taskName);
        return 0;
    } else {
        log_message("Task %s failed with exit code %d\n", params->taskName, result);
        return -1;
    }
}

void dag_scheduler(sqlite3 *db) {
    log_message("Starting DAG scheduler\n");
    
    // Load DAGs from database
    load_dags_from_database(db);
    
    while (1) {
        time_t now = time(NULL);
        struct tm *current_time = localtime(&now);

        struct CronTime cronTime = {
            current_time->tm_min,
            current_time->tm_hour,
            current_time->tm_mday,
            current_time->tm_mon + 1,  // tm_mon is 0-11, CronTime expects 1-12
            current_time->tm_wday      // 0-6, Sunday = 0
        };

        pthread_mutex_lock(&dag_list_mutex);
        
        DAG *current_dag = dag_list_head;
        while (current_dag != NULL) {
            if (current_dag->status == DAG_STATUS_ACTIVE && 
                is_dag_time_to_run(current_dag->cron_expression, cronTime)) {
                
                log_message("DAG %s is scheduled to run\n", current_dag->name);
                
                // Execute DAG in a separate thread to allow parallel DAG execution
                pthread_t dag_thread;
                DAGExecutionContext *context = malloc(sizeof(DAGExecutionContext));
                if (context) {
                    context->db = db;
                    context->dag = current_dag;
                    
                    if (pthread_create(&dag_thread, NULL, dag_execution_thread, context) != 0) {
                        log_message("Failed to create thread for DAG %s\n", current_dag->name);
                        free(context);
                    } else {
                        pthread_detach(dag_thread); // Allow thread to clean up automatically
                    }
                }
            }
            current_dag = current_dag->next;
        }
        
        pthread_mutex_unlock(&dag_list_mutex);
        
        // Sleep for 30 seconds before checking again
        sleep(30);
    }
}

void* dag_execution_thread(void *arg) {
    DAGExecutionContext *context = (DAGExecutionContext*)arg;
    if (context) {
        execute_dag(context->db, context->dag);
        free(context);
    }
    return NULL;
}

void reload_dags(sqlite3 *db) {
    log_message("Reloading DAGs from database\n");
    load_dags_from_database(db);
}

int trigger_dag_execution(sqlite3 *db, int dag_id) {
    pthread_mutex_lock(&dag_list_mutex);
    
    DAG *current_dag = dag_list_head;
    while (current_dag) {
        if (current_dag->id == dag_id) {
            pthread_mutex_unlock(&dag_list_mutex);
            
            log_message("Manually triggering DAG %s (ID: %d)\n", current_dag->name, dag_id);
            return execute_dag(db, current_dag);
        }
        current_dag = current_dag->next;
    }
    
    pthread_mutex_unlock(&dag_list_mutex);
    
    log_message("DAG with ID %d not found\n", dag_id);
    return -1;
}