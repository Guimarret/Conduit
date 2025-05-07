#include <stdio.h>
#include <string.h>
#include <time.h>
#include <string.h>
#include "civetweb/civetweb.h"
#include "database.h"

static int api_data_handler(struct mg_connection *conn, void *cbdata) {
    sqlite3 *db = (sqlite3 *)cbdata;
    char *json_data = dags_status(db);

    mg_send_http_ok(conn, "application/json; charset=utf-8", -1);
    mg_write(conn, json_data, strlen(json_data));

    return 1;
}

int initialize_webserver(sqlite3 *db) {
    struct mg_context *ctx;
    mg_init_library(0);

    const char *options[] = {
        "listening_ports", "8080",
        "enable_directory_listing", "no",
        NULL
    };
    ctx = mg_start(NULL, 0, options);
    mg_set_request_handler(ctx, "/api/dag_data", api_data_handler, NULL);

    printf("Server running at http://localhost:8080.\n");
    getchar();

    mg_stop(ctx);
    mg_exit_library();
    return 0;
}
