#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
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

sqlite3* insert_into_db(sqlite3 *db, Task *task){
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

sqlite3* dag_import(sqlite3 *db, Task *task){
    while (task != NULL) {
        insert_into_db(db, task);
        task = task->next;
    }
    return db;
}
