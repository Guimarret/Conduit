#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <cjson/cJSON.h>
#include "civetweb/civetweb.h"
#include "database.h"
#include "logger.h"
#include "responses.h"

static int api_data_handler(struct mg_connection *conn, void *cbdata) {
    sqlite3 *db = (sqlite3 *)cbdata;
    char *json_data = dags_status(db);
    if (!json_data) {
        mg_printf(conn, HTTP_HEADER_200_JSON, (int)strlen(RESPONSE_EMPTY_TASKS));
        return 1;
    }

    size_t data_len = strlen(json_data);
    mg_printf(conn, HTTP_HEADER_200_JSON, (int)data_len);
    mg_write(conn, json_data, data_len);

    free(json_data);
    return 1;
}

static int create_task_handler(struct mg_connection *conn, void *cbdata) {
    const struct mg_request_info *req_info = mg_get_request_info(conn);
    sqlite3 *db = (sqlite3 *)cbdata;

    if (strcmp(req_info->request_method, "POST") != 0) {
        mg_printf(conn, HTTP_HEADER_405_METHOD_NOT_ALLOWED, (int)strlen(RESPONSE_ERROR_METHOD_NOT_ALLOWED));
        return 1;
    }

    long long content_length = req_info->content_length;
    if (content_length <= 0 || content_length > 1024*1024) {  // Limit to 1MB for now
        mg_printf(conn, HTTP_HEADER_400_BAD_REQUEST, (int)strlen(RESPONSE_ERROR_INVALID_BODY));
        return 1;
    }

    char *buffer = malloc(content_length + 1);
    if (!buffer) {
        mg_printf(conn, HTTP_HEADER_500_SERVER_ERROR, (int)strlen(RESPONSE_ERROR_MEMORY_ALLOCATION));
        return 1;
    }

    int bytes_read = mg_read(conn, buffer, content_length);
    buffer[bytes_read] = '\0';

    cJSON *json = cJSON_Parse(buffer);
    free(buffer);
    if (json == NULL) {
        mg_printf(conn, HTTP_HEADER_400_BAD_REQUEST , (int)strlen(RESPONSE_ERROR_INVALID_JSON));
        return 1;
    }

    int task_count = 0;
    int error_count = 0;
    cJSON *tasks_array = NULL;

    if (cJSON_IsArray(json)) {
        tasks_array = json;
    } else if (cJSON_IsObject(json)) {
        tasks_array = cJSON_CreateArray();
        cJSON_AddItemReferenceToArray(tasks_array, json);
    } else {
        cJSON_Delete(json);
        mg_printf(conn, HTTP_HEADER_400_BAD_REQUEST,(int)strlen(RESPONSE_ERROR_JSON_FORMAT));
        return 1;
    }

    int array_size = cJSON_GetArraySize(tasks_array);
    for (int i = 0; i < array_size; i++) {
        cJSON *task_obj = cJSON_GetArrayItem(tasks_array, i);

        cJSON *taskName = cJSON_GetObjectItem(task_obj, "taskName");
        cJSON *cronExpression = cJSON_GetObjectItem(task_obj, "cronExpression");
        cJSON *taskExecution = cJSON_GetObjectItem(task_obj, "taskExecution");

        if (!taskName || !cronExpression || !taskExecution || !cJSON_IsString(taskName) || !cJSON_IsString(cronExpression) || !cJSON_IsString(taskExecution)) {
            error_count++;
            continue;
        }

        Task task;
        memset(&task, 0, sizeof(Task));
        task.next = NULL;

        strncpy(task.taskName, taskName->valuestring, sizeof(task.taskName) - 1);
        strncpy(task.cronExpression, cronExpression->valuestring, sizeof(task.cronExpression) - 1);
        strncpy(task.taskExecution, taskExecution->valuestring, sizeof(task.taskExecution) - 1);

        insert_new_dag(db, &task);
        task_count++;
    }

    cJSON_Delete(json);

    if (task_count > 0) {
        char response_buffer[256];
        snprintf(response_buffer, sizeof(response_buffer),
                RESPONSE_SUCCESS_MULTIPLE,
                task_count,
                error_count > 0 ? ", " : "",
                error_count > 0 ? "with some errors" : "");

        mg_printf(conn, HTTP_HEADER_201_CREATED, (int)strlen(response_buffer));
        mg_write(conn, response_buffer, strlen(response_buffer));
    } else {
        mg_printf(conn, HTTP_HEADER_400_BAD_REQUEST "%s",
                  (int)strlen(RESPONSE_ERROR_NO_VALID_TASKS),
                  RESPONSE_ERROR_NO_VALID_TASKS);
    }

    return 1;
}

static int redirect_wrong_endpoint(struct mg_connection *conn, void *cbdata) {
    const struct mg_request_info *req_info = mg_get_request_info(conn);

        char response_buffer[512];
        snprintf(response_buffer, sizeof(response_buffer), RESPONSE_ERROR_ENDPOINT_NOT_FOUND, req_info->request_uri);

        mg_printf(conn, HTTP_HEADER_404_NOT_FOUND, (int)strlen(response_buffer));
        mg_write(conn, response_buffer, strlen(response_buffer));

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
    mg_set_request_handler(ctx, "/api/new_dag", create_task_handler, db);
    mg_set_request_handler(ctx, "**", redirect_wrong_endpoint, NULL);

    log_message("Server running at http://localhost:8080.\n");
    getchar();

    mg_stop(ctx);
    mg_exit_library();
    return 0;
}
