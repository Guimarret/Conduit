#include "civetweb/civetweb.h"
#include <stdio.h>

static int home_handler(struct mg_connection *conn, void *cbdata) {
    mg_send_file(conn, "./webroot/home.html");
    return 1; // Handled
}

int main(void) {
    struct mg_context *ctx;
    mg_init_library(0);

    // Server configuration
    const char *options[] = {
        "listening_ports", "8080",
        "document_root", "./webroot", // Path to static files
        "enable_directory_listing", "no", // Optional
        NULL
    };
    ctx = mg_start(NULL, 0, options);

    mg_set_request_handler(ctx, "/home", home_handler, NULL);

    printf("Server running at http://localhost:8080. Press Enter to quit.\n");
    getchar();

    mg_stop(ctx);
    mg_exit_library();
    return 0;
}
