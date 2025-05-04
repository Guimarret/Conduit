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

int insert_into_db(sqlite3 *db, Task *task){

    return 0;
};
