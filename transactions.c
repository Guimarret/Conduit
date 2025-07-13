#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include "scheduler.h"
#include "logger.h"

sqlite3* transactions_status_migration(sqlite3 *db){
    char *ErrMsg = 0;
    const char *sql;
    sql = "CREATE TABLE IF NOT EXISTS transaction_status ("
          "id INTEGER PRIMARY KEY AUTOINCREMENT, "
          "task_id INTEGER NOT NULL, "
          "status TEXT NOT NULL, "
          "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP, "
          "details TEXT, "
          "FOREIGN KEY(task_id) REFERENCES tasks(id)"
          ");";

    int rc = sqlite3_exec(db, sql, 0, 0, &ErrMsg);
    if (rc != SQLITE_OK) {
        log_message("SQL error: %s", ErrMsg);
        sqlite3_free(ErrMsg);
        return NULL;
    }
    log_message("Migration executed");
    return db;
}

int log_task_status(sqlite3 *db, int task_id, const char *status, const char *details) {
    const char *sql = "INSERT INTO transaction_status (task_id, status, details) VALUES (?, ?, ?)";
    sqlite3_stmt *stmt;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        log_message("Failed to prepare statement: %s", sqlite3_errmsg(db));
        return rc;
    }

    sqlite3_bind_int(stmt, 1, task_id);
    sqlite3_bind_text(stmt, 2, status, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, details ? details : "", -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc == SQLITE_DONE) {
        log_message("Task %d status logged: %s", task_id, status);
        return SQLITE_OK;
    } else {
        log_message("Failed to log task status: %s", sqlite3_errmsg(db));
        return rc;
    }
}