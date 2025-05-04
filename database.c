#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>

sqlite3* initialize_database() {
    sqlite3 *db;
    char *err_msg = 0;
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
