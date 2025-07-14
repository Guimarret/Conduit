#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <string.h>
#include "scheduler.h"
#include "logger.h"
#include "dag.h"

sqlite3* initialize_database() {
    sqlite3 *db;

    int rc;

    rc = sqlite3_open("sqlite_test.db", &db);
    if (rc != SQLITE_OK) {
        log_message("Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }

    log_message("Database created or already initialized.\n");

    return db;
}

void shutdown_database(sqlite3 *db) {
    sqlite3_close(db);
    log_message("Database shutdown.\n");
}

sqlite3* dag_migration(sqlite3 *db){
    char *ErrMsg = 0;
    const char *sql;

    sql = "CREATE TABLE IF NOT EXISTS tasks (id INTEGER PRIMARY KEY AUTOINCREMENT, taskName TEXT NOT NULL, cronExpression TEXT NOT NULL, taskExecution TEXT NOT NULL)";
    sqlite3_exec(db, sql,0,0, &ErrMsg);
    log_message("Migration tasks executed \n");

    // Create DAGs table
    sql = "CREATE TABLE IF NOT EXISTS dags ("
          "id INTEGER PRIMARY KEY AUTOINCREMENT, "
          "name TEXT NOT NULL UNIQUE, "
          "cron_expression TEXT NOT NULL, "
          "description TEXT, "
          "status TEXT DEFAULT 'active', "
          "created_at DATETIME DEFAULT CURRENT_TIMESTAMP, "
          "updated_at DATETIME DEFAULT CURRENT_TIMESTAMP"
          ")";
    sqlite3_exec(db, sql, 0, 0, &ErrMsg);
    if (ErrMsg) {
        log_message("DAGs table creation error: %s\n", ErrMsg);
        sqlite3_free(ErrMsg);
        ErrMsg = 0;
    }

    // Create DAG tasks table
    sql = "CREATE TABLE IF NOT EXISTS dag_tasks ("
          "id INTEGER PRIMARY KEY AUTOINCREMENT, "
          "dag_id INTEGER NOT NULL, "
          "task_name TEXT NOT NULL, "
          "task_execution TEXT NOT NULL, "
          "dependencies TEXT, "
          "created_at DATETIME DEFAULT CURRENT_TIMESTAMP, "
          "FOREIGN KEY(dag_id) REFERENCES dags(id) ON DELETE CASCADE"
          ")";
    sqlite3_exec(db, sql, 0, 0, &ErrMsg);
    if (ErrMsg) {
        log_message("DAG tasks table creation error: %s\n", ErrMsg);
        sqlite3_free(ErrMsg);
        ErrMsg = 0;
    }

    // Create DAG executions table
    sql = "CREATE TABLE IF NOT EXISTS dag_executions ("
          "id INTEGER PRIMARY KEY AUTOINCREMENT, "
          "dag_id INTEGER NOT NULL, "
          "execution_id TEXT NOT NULL, "
          "status TEXT DEFAULT 'pending', "
          "started_at DATETIME, "
          "completed_at DATETIME, "
          "error_message TEXT, "
          "FOREIGN KEY(dag_id) REFERENCES dags(id) ON DELETE CASCADE"
          ")";
    sqlite3_exec(db, sql, 0, 0, &ErrMsg);
    if (ErrMsg) {
        log_message("DAG executions table creation error: %s\n", ErrMsg);
        sqlite3_free(ErrMsg);
        ErrMsg = 0;
    }

    // Create task executions table
    sql = "CREATE TABLE IF NOT EXISTS task_executions ("
          "id INTEGER PRIMARY KEY AUTOINCREMENT, "
          "dag_execution_id INTEGER NOT NULL, "
          "task_id INTEGER NOT NULL, "
          "task_name TEXT NOT NULL, "
          "status TEXT DEFAULT 'pending', "
          "started_at DATETIME, "
          "completed_at DATETIME, "
          "error_message TEXT, "
          "FOREIGN KEY(dag_execution_id) REFERENCES dag_executions(id) ON DELETE CASCADE, "
          "FOREIGN KEY(task_id) REFERENCES dag_tasks(id) ON DELETE CASCADE"
          ")";
    sqlite3_exec(db, sql, 0, 0, &ErrMsg);
    if (ErrMsg) {
        log_message("Task executions table creation error: %s\n", ErrMsg);
        sqlite3_free(ErrMsg);
        ErrMsg = 0;
    }

    // Update transaction_status table for DAG context
    sql = "ALTER TABLE transaction_status ADD COLUMN dag_id INTEGER";
    sqlite3_exec(db, sql, 0, 0, &ErrMsg);
    if (ErrMsg) {
        sqlite3_free(ErrMsg);
        ErrMsg = 0;
    }

    sql = "ALTER TABLE transaction_status ADD COLUMN dag_execution_id INTEGER";
    sqlite3_exec(db, sql, 0, 0, &ErrMsg);
    if (ErrMsg) {
        sqlite3_free(ErrMsg);
        ErrMsg = 0;
    }

    log_message("DAG migration completed\n");
    return db;
}

sqlite3* insert_new_dag(sqlite3 *db, Task *task){
    const char *sql;
    sqlite3_stmt *stmt;

    sql = "INSERT INTO tasks (taskName, cronExpression, taskExecution) VALUES (?, ?, ?)";
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, task->taskName, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, task->cronExpression, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, task->taskExecution, -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return db;
};

// Change to overwrite cause its creating new in every run
sqlite3* insert_into_db(sqlite3 *db, Task *task){
    const char *sql;
    sqlite3_stmt *stmt;

    sql = "REPLACE INTO tasks (taskName, cronExpression, taskExecution) VALUES (?, ?, ?)";
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, task->taskName, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, task->cronExpression, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, task->taskExecution, -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return db;
};

sqlite3* dag_import(sqlite3 *db, Task *task){
    const char *sql;

    sql = "SELECT * from tasks";
    sqlite3_stmt *stmt;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        log_message("Failed to prepare statement: %s\n", sqlite3_errmsg(db));
    } else {
        rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW) {
            log_message("Table has data.\n");
            sqlite3_finalize(stmt);
            return db;
        } else if (rc == SQLITE_DONE) {
            log_message("Table is empty.\n");
            while (task != NULL) {
                insert_into_db(db, task);
                task = task->next;
            }
            log_message("Dags imported");
            sqlite3_finalize(stmt);
            return db;
        } else {
            log_message("Error executing statement: %s\n", sqlite3_errmsg(db));
        }
        sqlite3_finalize(stmt);
    }
    return db;
}

char* dags_status(sqlite3 *db){
    const char *sql = "SELECT id, taskName, cronExpression, taskExecution FROM tasks";
    sqlite3_stmt *stmt;

    size_t buffer_size = 1024;
    char *json_result = malloc(buffer_size);
    if (!json_result) return NULL;

    int pos = 0;
    pos += snprintf(json_result + pos, buffer_size - pos, "{\"tasks\":[");

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        free(json_result);
        return NULL;
    }

    int first_row = 1;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const char *name = (const char*)sqlite3_column_text(stmt, 1);
        const char *cron = (const char*)sqlite3_column_text(stmt, 2);
        const char *exec = (const char*)sqlite3_column_text(stmt, 3);

        if (!name) name = "";
        if (!cron) cron = "";
        if (!exec) exec = "";

        int needed = snprintf(NULL, 0, "%s{\"id\":%d,\"name\":\"%s\",\"schedule\":\"%s\",\"execution\":\"%s\"}",
                             first_row ? "" : ",", id, name, cron, exec);

        if (pos + needed + 10 >= buffer_size) {
            buffer_size = buffer_size * 2 + needed;
            char *new_buffer = realloc(json_result, buffer_size);
            if (!new_buffer) {
                sqlite3_finalize(stmt);
                free(json_result);
                return NULL;
            }
            json_result = new_buffer;
        }

        pos += snprintf(json_result + pos, buffer_size - pos,
                      "%s{\"id\":%d,\"name\":\"%s\",\"schedule\":\"%s\",\"execution\":\"%s\"}",
                      first_row ? "" : ",", id, name, cron, exec);
        first_row = 0;
    }

    pos += snprintf(json_result + pos, buffer_size - pos, "]}");
    sqlite3_finalize(stmt);
    return json_result;
}

int update_dag(sqlite3 *db, int id, const char *taskName, const char *cronExpression, const char *taskExecution) {
    const char *sql = "UPDATE tasks SET taskName = ?, cronExpression = ?, taskExecution = ? WHERE id = ?";
    sqlite3_stmt *stmt;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        log_message("Failed to prepare update statement: %s\n", sqlite3_errmsg(db));
        return -1;
    }
    
    sqlite3_bind_text(stmt, 1, taskName, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, cronExpression, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, taskExecution, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 4, id);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc != SQLITE_DONE) {
        log_message("Failed to update task: %s\n", sqlite3_errmsg(db));
        return -1;
    }
    
    int changes = sqlite3_changes(db);
    if (changes == 0) {
        log_message("No task found with id %d\n", id);
        return 0;
    }
    
    log_message("Successfully updated task with id %d\n", id);
    return 1;
}

int delete_dag(sqlite3 *db, int id) {
    const char *sql = "DELETE FROM tasks WHERE id = ?";
    sqlite3_stmt *stmt;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        log_message("Failed to prepare delete statement: %s\n", sqlite3_errmsg(db));
        return -1;
    }
    
    sqlite3_bind_int(stmt, 1, id);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc != SQLITE_DONE) {
        log_message("Failed to delete task: %s\n", sqlite3_errmsg(db));
        return -1;
    }
    
    int changes = sqlite3_changes(db);
    if (changes == 0) {
        log_message("No task found with id %d\n", id);
        return 0;
    }
    
    log_message("Successfully deleted task with id %d\n", id);
    return 1;
}

// DAG Management Functions Implementation

int insert_dag_db(sqlite3 *db, DAG *dag) {
    const char *sql = "INSERT INTO dags (name, cron_expression, description, status) VALUES (?, ?, ?, ?)";
    sqlite3_stmt *stmt;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        log_message("Failed to prepare DAG insert statement: %s\n", sqlite3_errmsg(db));
        return -1;
    }
    
    sqlite3_bind_text(stmt, 1, dag->name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, dag->cron_expression, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, dag->description, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, dag_status_to_string(dag->status), -1, SQLITE_TRANSIENT);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc != SQLITE_DONE) {
        log_message("Failed to insert DAG: %s\n", sqlite3_errmsg(db));
        return -1;
    }
    
    dag->id = sqlite3_last_insert_rowid(db);
    log_message("Successfully inserted DAG with id %d\n", dag->id);
    return dag->id;
}

int insert_dag_task_db(sqlite3 *db, DAGTask *task) {
    const char *sql = "INSERT INTO dag_tasks (dag_id, task_name, task_execution, dependencies) VALUES (?, ?, ?, ?)";
    sqlite3_stmt *stmt;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        log_message("Failed to prepare DAG task insert statement: %s\n", sqlite3_errmsg(db));
        return -1;
    }
    
    // Convert dependencies to JSON string
    char dependencies_json[1024] = "[";
    TaskDependency *dep = task->dependencies;
    int first = 1;
    while (dep != NULL) {
        if (!first) strcat(dependencies_json, ",");
        char dep_str[128];
        snprintf(dep_str, sizeof(dep_str), "{\"task_id\":%d,\"task_name\":\"%s\"}", 
                dep->task_id, dep->task_name);
        strcat(dependencies_json, dep_str);
        first = 0;
        dep = dep->next;
    }
    strcat(dependencies_json, "]");
    
    sqlite3_bind_int(stmt, 1, task->dag_id);
    sqlite3_bind_text(stmt, 2, task->task_name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, task->task_execution, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, dependencies_json, -1, SQLITE_TRANSIENT);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc != SQLITE_DONE) {
        log_message("Failed to insert DAG task: %s\n", sqlite3_errmsg(db));
        return -1;
    }
    
    task->id = sqlite3_last_insert_rowid(db);
    log_message("Successfully inserted DAG task with id %d\n", task->id);
    return task->id;
}

int insert_dag_execution_db(sqlite3 *db, DAGExecution *execution) {
    const char *sql = "INSERT INTO dag_executions (dag_id, execution_id, status, started_at) VALUES (?, ?, ?, CURRENT_TIMESTAMP)";
    sqlite3_stmt *stmt;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        log_message("Failed to prepare DAG execution insert statement: %s\n", sqlite3_errmsg(db));
        return -1;
    }
    
    sqlite3_bind_int(stmt, 1, execution->dag_id);
    sqlite3_bind_text(stmt, 2, execution->execution_id, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, execution_status_to_string(execution->status), -1, SQLITE_TRANSIENT);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc != SQLITE_DONE) {
        log_message("Failed to insert DAG execution: %s\n", sqlite3_errmsg(db));
        return -1;
    }
    
    execution->id = sqlite3_last_insert_rowid(db);
    log_message("Successfully inserted DAG execution with id %d\n", execution->id);
    return execution->id;
}

int insert_task_execution_db(sqlite3 *db, TaskExecution *execution) {
    const char *sql = "INSERT INTO task_executions (dag_execution_id, task_id, task_name, status, started_at) VALUES (?, ?, ?, ?, CURRENT_TIMESTAMP)";
    sqlite3_stmt *stmt;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        log_message("Failed to prepare task execution insert statement: %s\n", sqlite3_errmsg(db));
        return -1;
    }
    
    sqlite3_bind_int(stmt, 1, execution->dag_execution_id);
    sqlite3_bind_int(stmt, 2, execution->task_id);
    sqlite3_bind_text(stmt, 3, execution->task_name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, execution_status_to_string(execution->status), -1, SQLITE_TRANSIENT);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc != SQLITE_DONE) {
        log_message("Failed to insert task execution: %s\n", sqlite3_errmsg(db));
        return -1;
    }
    
    execution->id = sqlite3_last_insert_rowid(db);
    log_message("Successfully inserted task execution with id %d\n", execution->id);
    return execution->id;
}

int update_dag_execution_status_db(sqlite3 *db, int execution_id, ExecutionStatus status, const char *error_message) {
    const char *sql = "UPDATE dag_executions SET status = ?, completed_at = CURRENT_TIMESTAMP, error_message = ? WHERE id = ?";
    sqlite3_stmt *stmt;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        log_message("Failed to prepare DAG execution update statement: %s\n", sqlite3_errmsg(db));
        return -1;
    }
    
    sqlite3_bind_text(stmt, 1, execution_status_to_string(status), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, error_message ? error_message : "", -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, execution_id);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc != SQLITE_DONE) {
        log_message("Failed to update DAG execution status: %s\n", sqlite3_errmsg(db));
        return -1;
    }
    
    log_message("Successfully updated DAG execution %d status to %s\n", execution_id, execution_status_to_string(status));
    return 1;
}

int update_task_execution_status_db(sqlite3 *db, int execution_id, ExecutionStatus status, const char *error_message) {
    const char *sql = "UPDATE task_executions SET status = ?, completed_at = CURRENT_TIMESTAMP, error_message = ? WHERE id = ?";
    sqlite3_stmt *stmt;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        log_message("Failed to prepare task execution update statement: %s\n", sqlite3_errmsg(db));
        return -1;
    }
    
    sqlite3_bind_text(stmt, 1, execution_status_to_string(status), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, error_message ? error_message : "", -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, execution_id);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc != SQLITE_DONE) {
        log_message("Failed to update task execution status: %s\n", sqlite3_errmsg(db));
        return -1;
    }
    
    log_message("Successfully updated task execution %d status to %s\n", execution_id, execution_status_to_string(status));
    return 1;
}

int delete_dag_by_id_db(sqlite3 *db, int dag_id) {
    const char *sql = "DELETE FROM dags WHERE id = ?";
    sqlite3_stmt *stmt;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        log_message("Failed to prepare DAG delete statement: %s\n", sqlite3_errmsg(db));
        return -1;
    }
    
    sqlite3_bind_int(stmt, 1, dag_id);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc != SQLITE_DONE) {
        log_message("Failed to delete DAG: %s\n", sqlite3_errmsg(db));
        return -1;
    }
    
    int changes = sqlite3_changes(db);
    if (changes == 0) {
        log_message("No DAG found with id %d\n", dag_id);
        return 0;
    }
    
    log_message("Successfully deleted DAG with id %d\n", dag_id);
    return 1;
}

// Enhanced transaction logging with DAG context
int log_dag_task_status(sqlite3 *db, int task_id, int dag_id, int dag_execution_id, const char *status, const char *details) {
    const char *sql = "INSERT INTO transaction_status (task_id, status, details, dag_id, dag_execution_id) VALUES (?, ?, ?, ?, ?)";
    sqlite3_stmt *stmt;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        log_message("Failed to prepare statement: %s", sqlite3_errmsg(db));
        return rc;
    }

    sqlite3_bind_int(stmt, 1, task_id);
    sqlite3_bind_text(stmt, 2, status, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, details ? details : "", -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 4, dag_id);
    sqlite3_bind_int(stmt, 5, dag_execution_id);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc == SQLITE_DONE) {
        log_message("DAG Task %d status logged: %s (DAG: %d, Execution: %d)", task_id, status, dag_id, dag_execution_id);
        return SQLITE_OK;
    } else {
        log_message("Failed to log DAG task status: %s", sqlite3_errmsg(db));
        return rc;
    }
}

// Forward declarations for internal functions
DAGTask* load_dag_tasks_db(sqlite3 *db, int dag_id);
int count_dag_tasks(DAGTask *task_list);
int count_dependencies(TaskDependency *dep_list);
TaskDependency* parse_dependencies_json(const char *json_str);

// DAG Query Functions

DAG* load_all_dags_db(sqlite3 *db) {
    const char *sql = "SELECT id, name, cron_expression, description, status, created_at, updated_at FROM dags WHERE status = 'active'";
    sqlite3_stmt *stmt;
    DAG *dag_list = NULL;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        log_message("Failed to prepare DAG load statement: %s\n", sqlite3_errmsg(db));
        return NULL;
    }

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        DAG *dag = malloc(sizeof(DAG));
        if (!dag) continue;

        memset(dag, 0, sizeof(DAG));
        dag->id = sqlite3_column_int(stmt, 0);
        strncpy(dag->name, (const char*)sqlite3_column_text(stmt, 1), MAX_DAG_NAME_LENGTH - 1);
        strncpy(dag->cron_expression, (const char*)sqlite3_column_text(stmt, 2), MAX_CRON_EXPRESSION_LENGTH - 1);
        strncpy(dag->description, (const char*)sqlite3_column_text(stmt, 3), MAX_DESCRIPTION_LENGTH - 1);
        dag->status = string_to_dag_status((const char*)sqlite3_column_text(stmt, 4));
        dag->created_at = sqlite3_column_int64(stmt, 5);
        dag->updated_at = sqlite3_column_int64(stmt, 6);

        // Load tasks for this DAG
        dag->tasks = load_dag_tasks_db(db, dag->id);
        dag->task_count = count_dag_tasks(dag->tasks);

        dag->next = dag_list;
        dag_list = dag;
    }

    sqlite3_finalize(stmt);
    return dag_list;
}

DAGTask* load_dag_tasks_db(sqlite3 *db, int dag_id) {
    const char *sql = "SELECT id, task_name, task_execution, dependencies FROM dag_tasks WHERE dag_id = ?";
    sqlite3_stmt *stmt;
    DAGTask *task_list = NULL;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        log_message("Failed to prepare DAG tasks load statement: %s\n", sqlite3_errmsg(db));
        return NULL;
    }

    sqlite3_bind_int(stmt, 1, dag_id);

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        DAGTask *task = malloc(sizeof(DAGTask));
        if (!task) continue;

        memset(task, 0, sizeof(DAGTask));
        task->id = sqlite3_column_int(stmt, 0);
        task->dag_id = dag_id;
        strncpy(task->task_name, (const char*)sqlite3_column_text(stmt, 1), MAX_TASK_NAME_LENGTH - 1);
        strncpy(task->task_execution, (const char*)sqlite3_column_text(stmt, 2), MAX_TASK_EXECUTION_LENGTH - 1);

        // Parse dependencies JSON (simplified for now)
        const char *deps_json = (const char*)sqlite3_column_text(stmt, 3);
        task->dependencies = parse_dependencies_json(deps_json);
        task->dependency_count = count_dependencies(task->dependencies);

        task->next = task_list;
        task_list = task;
    }

    sqlite3_finalize(stmt);
    return task_list;
}

int count_dag_tasks(DAGTask *task_list) {
    int count = 0;
    DAGTask *current = task_list;
    while (current) {
        count++;
        current = current->next;
    }
    return count;
}

int count_dependencies(TaskDependency *dep_list) {
    int count = 0;
    TaskDependency *current = dep_list;
    while (current) {
        count++;
        current = current->next;
    }
    return count;
}

TaskDependency* parse_dependencies_json(const char *json_str) {
    if (!json_str) return NULL;
    
    // Simple JSON parsing for dependencies - in production use proper JSON parser
    // For now, return NULL (no dependencies)
    return NULL;
}

char* get_dags_json(sqlite3 *db) {
    const char *sql = "SELECT id, name, cron_expression, description, status FROM dags";
    sqlite3_stmt *stmt;
    
    size_t buffer_size = 2048;
    char *json_result = malloc(buffer_size);
    if (!json_result) return NULL;

    int pos = 0;
    pos += snprintf(json_result + pos, buffer_size - pos, "{\"dags\":[");

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        free(json_result);
        return NULL;
    }

    int first_row = 1;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const char *name = (const char*)sqlite3_column_text(stmt, 1);
        const char *cron = (const char*)sqlite3_column_text(stmt, 2);
        const char *desc = (const char*)sqlite3_column_text(stmt, 3);
        const char *status = (const char*)sqlite3_column_text(stmt, 4);

        if (!name) name = "";
        if (!cron) cron = "";
        if (!desc) desc = "";
        if (!status) status = "";

        int needed = snprintf(NULL, 0, "%s{\"id\":%d,\"name\":\"%s\",\"cron_expression\":\"%s\",\"description\":\"%s\",\"status\":\"%s\"}",
                             first_row ? "" : ",", id, name, cron, desc, status);

        if (pos + needed + 10 >= buffer_size) {
            buffer_size = buffer_size * 2 + needed;
            char *new_buffer = realloc(json_result, buffer_size);
            if (!new_buffer) {
                sqlite3_finalize(stmt);
                free(json_result);
                return NULL;
            }
            json_result = new_buffer;
        }

        pos += snprintf(json_result + pos, buffer_size - pos,
                      "%s{\"id\":%d,\"name\":\"%s\",\"cron_expression\":\"%s\",\"description\":\"%s\",\"status\":\"%s\"}",
                      first_row ? "" : ",", id, name, cron, desc, status);
        first_row = 0;
    }

    pos += snprintf(json_result + pos, buffer_size - pos, "]}");
    sqlite3_finalize(stmt);
    return json_result;
}

char* get_dag_status_json(sqlite3 *db, int dag_id) {
    const char *sql = "SELECT d.id, d.name, d.status, de.execution_id, de.status, de.started_at, de.completed_at "
                     "FROM dags d LEFT JOIN dag_executions de ON d.id = de.dag_id "
                     "WHERE d.id = ? ORDER BY de.started_at DESC LIMIT 10";
    sqlite3_stmt *stmt;
    
    size_t buffer_size = 2048;
    char *json_result = malloc(buffer_size);
    if (!json_result) return NULL;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        free(json_result);
        return NULL;
    }

    sqlite3_bind_int(stmt, 1, dag_id);

    int pos = 0;
    int first_row = 1;
    pos += snprintf(json_result + pos, buffer_size - pos, "{\"dag_id\":%d,\"executions\":[", dag_id);

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        const char *exec_id = (const char*)sqlite3_column_text(stmt, 3);
        const char *status = (const char*)sqlite3_column_text(stmt, 4);
        const char *started = (const char*)sqlite3_column_text(stmt, 5);
        const char *completed = (const char*)sqlite3_column_text(stmt, 6);

        if (!exec_id) continue;

        if (!first_row) pos += snprintf(json_result + pos, buffer_size - pos, ",");
        pos += snprintf(json_result + pos, buffer_size - pos,
                      "{\"execution_id\":\"%s\",\"status\":\"%s\",\"started_at\":\"%s\",\"completed_at\":\"%s\"}",
                      exec_id ? exec_id : "", status ? status : "", 
                      started ? started : "", completed ? completed : "");
        first_row = 0;
    }

    pos += snprintf(json_result + pos, buffer_size - pos, "]}");
    sqlite3_finalize(stmt);
    return json_result;
}
