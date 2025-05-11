#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <string.h>
#include "scheduler.h"
#include "logger.h"

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
    log_message("Migration executed");
    return db;
}

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
