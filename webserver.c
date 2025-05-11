#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "civetweb/civetweb.h"
#include "database.h"
#include "logger.h"

static int api_data_handler(struct mg_connection *conn, void *cbdata) {
    sqlite3 *db = (sqlite3 *)cbdata;
    char *json_data = dags_status(db);
    if (!json_data) {
        // Send empty response if there's an error
        const char *empty_json = "{\"tasks\":[]}";
        mg_printf(conn, "HTTP/1.1 200 OK\r\n"
                      "Content-Type: application/json\r\n"
                      "Access-Control-Allow-Origin: *\r\n"
                      "Content-Length: 11\r\n"
                      "\r\n%s", empty_json);
        return 1;
    }

    size_t data_len = strlen(json_data);

    mg_printf(conn, "HTTP/1.1 200 OK\r\n"
                   "Content-Type: application/json\r\n"
                   "Access-Control-Allow-Origin: *\r\n"
                   "Content-Length: %lu\r\n"
                   "\r\n", (unsigned long)data_len);


    mg_write(conn, json_data, data_len);

    free(json_data);
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
    mg_set_request_handler(ctx, "/api/dag_data", api_data_handler, db);

    log_message("Server running at http://localhost:8080.\n");
    getchar();

    mg_stop(ctx);
    mg_exit_library();
    return 0;
}
