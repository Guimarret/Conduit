#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <string.h>
#include "scheduler.h"

sqlite3* initialize_database() {
    sqlite3 *db;

    int rc;

    rc = sqlite3_open("sqlite_test.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }

    printf("Database created or already initialized.\n");

    return db;
}

void shutdown_database(sqlite3 *db) {
    sqlite3_close(db);
    printf("Database shutdown.\n");
}

sqlite3* dag_migration(sqlite3 *db){
    char *ErrMsg = 0;
    const char *sql;

    sql = "CREATE TABLE IF NOT EXISTS tasks (id INTEGER PRIMARY KEY AUTOINCREMENT, taskName TEXT NOT NULL, cronExpression TEXT NOT NULL, taskExecution TEXT NOT NULL)";
    sqlite3_exec(db, sql,0,0, &ErrMsg);
    printf("Migration executed");
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
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
    } else {
        rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW) {
            printf("Table has data.\n");
            sqlite3_finalize(stmt);
            return db;
        } else if (rc == SQLITE_DONE) {
            printf("Table is empty.\n");
            while (task != NULL) {
                insert_into_db(db, task);
                task = task->next;
            }
            printf("Dags imported");
            sqlite3_finalize(stmt);
            return db;
        } else {
            fprintf(stderr, "Error executing statement: %s\n", sqlite3_errmsg(db));
        }
        sqlite3_finalize(stmt);
    }
    return db;
}

char* dags_status(sqlite3 *db){
    const char *sql = "SELECT id, taskName, cronExpression, taskExecution FROM tasks";
    sqlite3_stmt *stmt;
    char *json_result = NULL;
    size_t json_size = 0;
    size_t json_capacity = 1024;

    json_result = malloc(json_capacity);
    if (!json_result) return NULL;

    strcpy(json_result, "{\"tasks\": [");
    json_size = strlen(json_result);


    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK){
        fprintf(stderr, "Database error: %s\n", sqlite3_errmsg(db));
        free(json_result);
        return NULL;
    }

    int first_row = 1;
    while((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const char *name = (const char*)sqlite3_column_text(stmt, 1);
        const char *cron = (const char*)sqlite3_column_text(stmt, 2);
        const char *exec = (const char*)sqlite3_column_text(stmt, 3);

        char row_buffer[512];
        snprintf(row_buffer, sizeof(row_buffer),
                         "%s{\"id\":%d,\"name\":\"%s\",\"schedule\":\"%s\",\"execution\":\"%s\"}",
                         first_row ? "" : ",", id, name, cron, exec);
        first_row = 0;
        if(json_size + strlen(row_buffer) >= json_capacity){
            json_capacity *= 2;
            json_result = realloc(json_result, json_capacity);
            if (!json_result) return NULL;
        }

        strcat(json_result, row_buffer);
        json_size += strlen(row_buffer);
    }

    strcat(json_result, "}]");
    sqlite3_finalize(stmt);

    return json_result;
}
