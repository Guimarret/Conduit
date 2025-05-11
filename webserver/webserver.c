#include "civetweb/civetweb.h"
#include <stdio.h>
#include "../logger.h"

static int home_handler(struct mg_connection *conn, void *cbdata) {
    mg_send_file(conn, "./webroot/html/home.html");
    return 1;
}

static int dash_handler(struct mg_connection *conn, void *cbdata) {
    mg_send_file(conn, "./webroot/html/dash.html");
    return 1;
}

int main(void) {
    struct mg_context *ctx;
    mg_init_library(0);

    const char *options[] = {
        "listening_ports", "9000",
        "document_root", "./webroot", // Path to static files
        "enable_directory_listing", "no", // Optional
        NULL
    };
    ctx = mg_start(NULL, 0, options);

    mg_set_request_handler(ctx, "/home", home_handler, NULL);
    mg_set_request_handler(ctx, "/dash", dash_handler, NULL);

    log_message("Server running at http://localhost:9000. Press Enter to quit.\n");
    getchar();

    mg_stop(ctx);
    mg_exit_library();
    return 0;
}
