#include <stdio.h>
#include <string.h>
#include "civetweb/civetweb.h"
#include "../logger.h"
// #include "../responses.h"

static int dash_handler(struct mg_connection *conn, void *cbdata) {
    mg_send_file(conn, "./webroot/html/dash.html");
    return 1;
}

// static int redirect_handler(struct mg_connection *conn, void *cbdata) {
//     mg_printf(conn, HTTP_HEADER_302_REDIRECT, "/dash");
//     return 1;
// }

int main(void) {
    struct mg_context *ctx;
    mg_init_library(0);

    const char *options[] = {
        "listening_ports", "9000",
        "document_root", "./webroot",
        "enable_directory_listing", "no",
        NULL
    };
    ctx = mg_start(NULL, 0, options);

    mg_set_request_handler(ctx, "/dash", dash_handler, NULL);

    log_message("Server running at http://localhost:9000. Press Enter to quit.\n");
    getchar();

    mg_stop(ctx);
    mg_exit_library();
    return 0;
}
