#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "dag.h"
#include "logger.h"

// Utility Functions Implementation

const char* execution_status_to_string(ExecutionStatus status) {
    switch (status) {
        case EXECUTION_STATUS_PENDING: return "pending";
        case EXECUTION_STATUS_RUNNING: return "running";
        case EXECUTION_STATUS_SUCCESS: return "success";
        case EXECUTION_STATUS_FAILED: return "failed";
        case EXECUTION_STATUS_CANCELLED: return "cancelled";
        case EXECUTION_STATUS_SKIPPED: return "skipped";
        default: return "unknown";
    }
}

ExecutionStatus string_to_execution_status(const char *status) {
    if (strcmp(status, "pending") == 0) return EXECUTION_STATUS_PENDING;
    if (strcmp(status, "running") == 0) return EXECUTION_STATUS_RUNNING;
    if (strcmp(status, "success") == 0) return EXECUTION_STATUS_SUCCESS;
    if (strcmp(status, "failed") == 0) return EXECUTION_STATUS_FAILED;
    if (strcmp(status, "cancelled") == 0) return EXECUTION_STATUS_CANCELLED;
    if (strcmp(status, "skipped") == 0) return EXECUTION_STATUS_SKIPPED;
    return EXECUTION_STATUS_PENDING;
}

const char* dag_status_to_string(DAGStatus status) {
    switch (status) {
        case DAG_STATUS_ACTIVE: return "active";
        case DAG_STATUS_INACTIVE: return "inactive";
        case DAG_STATUS_PAUSED: return "paused";
        default: return "active";
    }
}

DAGStatus string_to_dag_status(const char *status) {
    if (strcmp(status, "active") == 0) return DAG_STATUS_ACTIVE;
    if (strcmp(status, "inactive") == 0) return DAG_STATUS_INACTIVE;
    if (strcmp(status, "paused") == 0) return DAG_STATUS_PAUSED;
    return DAG_STATUS_ACTIVE;
}

// DAG Management Functions

DAG* create_dag(const char *name, const char *cron_expression, const char *description) {
    DAG *dag = malloc(sizeof(DAG));
    if (!dag) {
        log_message("Failed to allocate memory for DAG\n");
        return NULL;
    }
    
    memset(dag, 0, sizeof(DAG));
    strncpy(dag->name, name, MAX_DAG_NAME_LENGTH - 1);
    strncpy(dag->cron_expression, cron_expression, MAX_CRON_EXPRESSION_LENGTH - 1);
    strncpy(dag->description, description ? description : "", MAX_DESCRIPTION_LENGTH - 1);
    
    dag->status = DAG_STATUS_ACTIVE;
    dag->created_at = time(NULL);
    dag->updated_at = time(NULL);
    dag->tasks = NULL;
    dag->task_count = 0;
    dag->next = NULL;
    
    return dag;
}

void free_dag(DAG *dag) {
    if (!dag) return;
    
    DAGTask *current_task = dag->tasks;
    while (current_task) {
        DAGTask *next_task = current_task->next;
        free_dag_task(current_task);
        current_task = next_task;
    }
    
    free(dag);
}

void free_dag_list(DAG *dag_list) {
    DAG *current = dag_list;
    while (current) {
        DAG *next = current->next;
        free_dag(current);
        current = next;
    }
}

// DAG Task Management Functions

DAGTask* create_dag_task(int dag_id, const char *task_name, const char *task_execution) {
    DAGTask *task = malloc(sizeof(DAGTask));
    if (!task) {
        log_message("Failed to allocate memory for DAG task\n");
        return NULL;
    }
    
    memset(task, 0, sizeof(DAGTask));
    task->dag_id = dag_id;
    strncpy(task->task_name, task_name, MAX_TASK_NAME_LENGTH - 1);
    strncpy(task->task_execution, task_execution, MAX_TASK_EXECUTION_LENGTH - 1);
    
    task->dependencies = NULL;
    task->dependency_count = 0;
    task->next = NULL;
    
    return task;
}

int add_task_dependency(DAGTask *task, int dependency_task_id, const char *dependency_task_name) {
    if (!task || task->dependency_count >= MAX_DEPENDENCIES) {
        return -1;
    }
    
    TaskDependency *dep = malloc(sizeof(TaskDependency));
    if (!dep) {
        log_message("Failed to allocate memory for task dependency\n");
        return -1;
    }
    
    dep->task_id = dependency_task_id;
    strncpy(dep->task_name, dependency_task_name, MAX_TASK_NAME_LENGTH - 1);
    dep->next = task->dependencies;
    task->dependencies = dep;
    task->dependency_count++;
    
    return 0;
}

void free_dag_task(DAGTask *task) {
    if (!task) return;
    
    TaskDependency *current_dep = task->dependencies;
    while (current_dep) {
        TaskDependency *next_dep = current_dep->next;
        free(current_dep);
        current_dep = next_dep;
    }
    
    free(task);
}

// Dependency Resolution Functions

int validate_dag_dependencies(DAG *dag) {
    if (!dag || !dag->tasks) return 1;
    
    // Check for cycle detection
    if (has_cycle(dag)) {
        log_message("Cycle detected in DAG %s\n", dag->name);
        return 0;
    }
    
    // Validate that all dependencies exist in the DAG
    DAGTask *task = dag->tasks;
    while (task) {
        TaskDependency *dep = task->dependencies;
        while (dep) {
            // Find if dependency task exists in DAG
            DAGTask *dep_task = dag->tasks;
            int found = 0;
            while (dep_task) {
                if (dep_task->id == dep->task_id) {
                    found = 1;
                    break;
                }
                dep_task = dep_task->next;
            }
            if (!found) {
                log_message("Dependency task %d not found in DAG %s\n", dep->task_id, dag->name);
                return 0;
            }
            dep = dep->next;
        }
        task = task->next;
    }
    
    return 1;
}

int has_cycle(DAG *dag) {
    if (!dag || !dag->tasks) return 0;
    
    // Simple cycle detection using DFS
    // For a more robust implementation, consider using Kahn's algorithm
    int visited[1000] = {0}; // Assuming max 1000 tasks per DAG
    int rec_stack[1000] = {0};
    
    DAGTask *task = dag->tasks;
    while (task) {
        if (task->id < 1000 && !visited[task->id]) {
            if (dfs_cycle_check(dag, task->id, visited, rec_stack)) {
                return 1;
            }
        }
        task = task->next;
    }
    
    return 0;
}

int dfs_cycle_check(DAG *dag, int task_id, int *visited, int *rec_stack) {
    visited[task_id] = 1;
    rec_stack[task_id] = 1;
    
    // Find the task with this ID
    DAGTask *task = dag->tasks;
    while (task && task->id != task_id) {
        task = task->next;
    }
    
    if (!task) return 0;
    
    // Check all dependencies
    TaskDependency *dep = task->dependencies;
    while (dep) {
        if (dep->task_id < 1000) {
            if (!visited[dep->task_id] && dfs_cycle_check(dag, dep->task_id, visited, rec_stack)) {
                return 1;
            }
            if (rec_stack[dep->task_id]) {
                return 1;
            }
        }
        dep = dep->next;
    }
    
    rec_stack[task_id] = 0;
    return 0;
}

ExecutionQueue* create_execution_queue(DAG *dag) {
    if (!dag || !dag->tasks) return NULL;
    
    ExecutionQueue *queue_head = NULL;
    DAGTask *task = dag->tasks;
    
    while (task) {
        ExecutionQueue *queue_item = malloc(sizeof(ExecutionQueue));
        if (!queue_item) {
            log_message("Failed to allocate memory for execution queue item\n");
            continue;
        }
        
        queue_item->task = task;
        queue_item->ready_to_run = (task->dependency_count == 0) ? 1 : 0;
        queue_item->completed_dependencies = 0;
        queue_item->next = queue_head;
        queue_head = queue_item;
        
        task = task->next;
    }
    
    return queue_head;
}

ExecutionQueue* get_ready_tasks(ExecutionQueue *queue) {
    ExecutionQueue *ready_tasks = NULL;
    ExecutionQueue *current = queue;
    
    while (current) {
        if (current->ready_to_run) {
            ExecutionQueue *ready_item = malloc(sizeof(ExecutionQueue));
            if (ready_item) {
                *ready_item = *current;
                ready_item->next = ready_tasks;
                ready_tasks = ready_item;
            }
        }
        current = current->next;
    }
    
    return ready_tasks;
}

void mark_task_completed(ExecutionQueue *queue, int task_id) {
    ExecutionQueue *current = queue;
    
    // Mark the completed task as not ready to run
    while (current) {
        if (current->task->id == task_id) {
            current->ready_to_run = 0;
            break;
        }
        current = current->next;
    }
    
    // Update dependency counts for tasks that depend on this one
    current = queue;
    while (current) {
        if (current->task->id != task_id) {
            TaskDependency *dep = current->task->dependencies;
            while (dep) {
                if (dep->task_id == task_id) {
                    current->completed_dependencies++;
                    if (current->completed_dependencies >= current->task->dependency_count) {
                        current->ready_to_run = 1;
                    }
                    break;
                }
                dep = dep->next;
            }
        }
        current = current->next;
    }
}

// DAG Execution Functions

char* generate_execution_id(int dag_id) {
    char *execution_id = malloc(64);
    if (!execution_id) return NULL;
    
    time_t now = time(NULL);
    snprintf(execution_id, 64, "dag_%d_%ld", dag_id, now);
    return execution_id;
}

int start_dag_execution(sqlite3 *db, int dag_id, char *execution_id) {
    DAGExecution execution = {0};
    execution.dag_id = dag_id;
    strncpy(execution.execution_id, execution_id, sizeof(execution.execution_id) - 1);
    execution.status = EXECUTION_STATUS_RUNNING;
    execution.started_at = time(NULL);
    
    return insert_dag_execution_db(db, &execution);
}