#ifndef CONDUIT_DAG_H
#define CONDUIT_DAG_H

#include <sqlite3.h>
#include <time.h>

// Maximum limits for DAG components
#define MAX_DAG_NAME_LENGTH 128
#define MAX_TASK_NAME_LENGTH 64
#define MAX_TASK_EXECUTION_LENGTH 256
#define MAX_CRON_EXPRESSION_LENGTH 64
#define MAX_DESCRIPTION_LENGTH 512
#define MAX_ERROR_MESSAGE_LENGTH 1024
#define MAX_DEPENDENCIES 32

// DAG and task status definitions
typedef enum {
    DAG_STATUS_ACTIVE,
    DAG_STATUS_INACTIVE,
    DAG_STATUS_PAUSED
} DAGStatus;

typedef enum {
    EXECUTION_STATUS_PENDING,
    EXECUTION_STATUS_RUNNING,
    EXECUTION_STATUS_SUCCESS,
    EXECUTION_STATUS_FAILED,
    EXECUTION_STATUS_CANCELLED,
    EXECUTION_STATUS_SKIPPED
} ExecutionStatus;

// Forward declarations
struct DAGTask;
struct DAG;

// Dependency structure for tasks
typedef struct TaskDependency {
    int task_id;
    char task_name[MAX_TASK_NAME_LENGTH];
    struct TaskDependency *next;
} TaskDependency;

// DAG Task structure
typedef struct DAGTask {
    int id;
    int dag_id;
    char task_name[MAX_TASK_NAME_LENGTH];
    char task_execution[MAX_TASK_EXECUTION_LENGTH];
    TaskDependency *dependencies;
    int dependency_count;
    struct DAGTask *next;
} DAGTask;

// DAG structure
typedef struct DAG {
    int id;
    char name[MAX_DAG_NAME_LENGTH];
    char cron_expression[MAX_CRON_EXPRESSION_LENGTH];
    char description[MAX_DESCRIPTION_LENGTH];
    DAGStatus status;
    time_t created_at;
    time_t updated_at;
    DAGTask *tasks;
    int task_count;
    struct DAG *next;
} DAG;

// DAG Execution tracking
typedef struct DAGExecution {
    int id;
    int dag_id;
    char execution_id[64];
    ExecutionStatus status;
    time_t started_at;
    time_t completed_at;
    char error_message[MAX_ERROR_MESSAGE_LENGTH];
    struct DAGExecution *next;
} DAGExecution;

// Task Execution tracking
typedef struct TaskExecution {
    int id;
    int dag_execution_id;
    int task_id;
    char task_name[MAX_TASK_NAME_LENGTH];
    ExecutionStatus status;
    time_t started_at;
    time_t completed_at;
    char error_message[MAX_ERROR_MESSAGE_LENGTH];
    struct TaskExecution *next;
} TaskExecution;

// Execution queue for managing task dependencies
typedef struct ExecutionQueue {
    DAGTask *task;
    int ready_to_run;
    int completed_dependencies;
    struct ExecutionQueue *next;
} ExecutionQueue;

// DAG Management Functions
DAG* create_dag(const char *name, const char *cron_expression, const char *description);
void free_dag(DAG *dag);
void free_dag_list(DAG *dag_list);

// DAG Task Management Functions
DAGTask* create_dag_task(int dag_id, const char *task_name, const char *task_execution);
int add_task_dependency(DAGTask *task, int dependency_task_id, const char *dependency_task_name);
void free_dag_task(DAGTask *task);

// Dependency Resolution Functions
int validate_dag_dependencies(DAG *dag);
int has_cycle(DAG *dag);
int dfs_cycle_check(DAG *dag, int task_id, int *visited, int *rec_stack);
ExecutionQueue* create_execution_queue(DAG *dag);
ExecutionQueue* get_ready_tasks(ExecutionQueue *queue);
void mark_task_completed(ExecutionQueue *queue, int task_id);

// Utility Functions
const char* execution_status_to_string(ExecutionStatus status);
ExecutionStatus string_to_execution_status(const char *status);
const char* dag_status_to_string(DAGStatus status);
DAGStatus string_to_dag_status(const char *status);

// Database Functions (declared here, implemented in database.c)
int insert_dag_db(sqlite3 *db, DAG *dag);
int insert_dag_task_db(sqlite3 *db, DAGTask *task);
int insert_dag_execution_db(sqlite3 *db, DAGExecution *execution);
int insert_task_execution_db(sqlite3 *db, TaskExecution *execution);
int update_dag_execution_status_db(sqlite3 *db, int execution_id, ExecutionStatus status, const char *error_message);
int update_task_execution_status_db(sqlite3 *db, int execution_id, ExecutionStatus status, const char *error_message);

DAG* load_dag_by_id_db(sqlite3 *db, int dag_id);
DAG* load_all_dags_db(sqlite3 *db);
DAGExecution* load_dag_executions_db(sqlite3 *db, int dag_id);
TaskExecution* load_task_executions_db(sqlite3 *db, int dag_execution_id);

int delete_dag_by_id_db(sqlite3 *db, int dag_id);
int update_dag_db(sqlite3 *db, int dag_id, const char *name, const char *cron_expression, const char *description);

// DAG Execution Functions
char* generate_execution_id(int dag_id);
int start_dag_execution(sqlite3 *db, int dag_id, char *execution_id);
int execute_dag_task(sqlite3 *db, TaskExecution *task_execution);

#endif