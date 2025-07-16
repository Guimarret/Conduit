#include <sqlite3.h>
#include "scheduler.h"
#include "dag.h"

// Buffer size constants for JSON generation
#define JSON_BUFFER_INITIAL_SIZE 2048
#define JSON_DEPENDENCY_BUFFER_SIZE 1024
#define JSON_DEP_STRING_SIZE 128

// Core database functions
sqlite3* initialize_database();
void shutdown_database(sqlite3 *db);
sqlite3* dag_migration(sqlite3 *db);

// Legacy task functions (maintained for backward compatibility)
sqlite3* dag_import(sqlite3 *db, Task *task);
char* dags_status(sqlite3 *db);
sqlite3* insert_new_dag(sqlite3 *db, Task *task);
int update_dag(sqlite3 *db, int id, const char *taskName, const char *cronExpression, const char *taskExecution);
int delete_dag(sqlite3 *db, int id);

// DAG Management Functions
int insert_dag_db(sqlite3 *db, DAG *dag);
int insert_dag_task_db(sqlite3 *db, DAGTask *task);
int insert_dag_execution_db(sqlite3 *db, DAGExecution *execution);
int insert_task_execution_db(sqlite3 *db, TaskExecution *execution);
int update_dag_execution_status_db(sqlite3 *db, int execution_id, ExecutionStatus status, const char *error_message);
int update_task_execution_status_db(sqlite3 *db, int execution_id, ExecutionStatus status, const char *error_message);

// DAG Query Functions  
DAG* load_dag_by_id_db(sqlite3 *db, int dag_id);
DAG* load_all_dags_db(sqlite3 *db);
DAGTask* load_dag_tasks_db(sqlite3 *db, int dag_id);
DAGExecution* load_dag_executions_db(sqlite3 *db, int dag_id);
TaskExecution* load_task_executions_db(sqlite3 *db, int dag_execution_id);
char* get_dags_json(sqlite3 *db);
char* get_dag_status_json(sqlite3 *db, int dag_id);

// DAG Utility Functions
int count_dag_tasks(DAGTask *task_list);
int count_dependencies(TaskDependency *dep_list); 
TaskDependency* parse_dependencies_json(const char *json_str);

// DAG Modification Functions
int delete_dag_by_id_db(sqlite3 *db, int dag_id);
int update_dag_db(sqlite3 *db, int dag_id, const char *name, const char *cron_expression, const char *description);

// DAG Task Dependency Functions
int insert_task_dependencies_db(sqlite3 *db, int task_id, TaskDependency *dependencies);
TaskDependency* load_task_dependencies_db(sqlite3 *db, int task_id);

// Enhanced transaction logging with DAG context
int log_dag_task_status(sqlite3 *db, int task_id, int dag_id, int dag_execution_id, const char *status, const char *details);

// Buffer management utilities
char* ensure_buffer_capacity(char **buffer, size_t *current_size, size_t needed_size);
