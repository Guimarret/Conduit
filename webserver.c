#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <cjson/cJSON.h>
#include "mongoose.h"
#include "database.h"
#include "logger.h"
#include "responses.h"

// Global database pointer for the webserver
static sqlite3 *g_db = NULL;

// HTTP response helper
static void send_json_response(struct mg_connection *c, int status_code, const char *body) {
    mg_http_reply(c, status_code, 
                  "Content-Type: application/json\r\n"
                  "Access-Control-Allow-Origin: *\r\n", 
                  "%s", body);
}

static void api_data_handler(struct mg_connection *c, struct mg_http_message *hm) {
    char *json_data = dags_status(g_db);
    if (!json_data) {
        send_json_response(c, 200, RESPONSE_EMPTY_TASKS);
        return;
    }
    
    send_json_response(c, 200, json_data);
    free(json_data);
}

static void create_task_handler(struct mg_connection *c, struct mg_http_message *hm) {
    if (mg_strcmp(hm->method, mg_str("POST")) != 0) {
        send_json_response(c, 405, RESPONSE_ERROR_METHOD_NOT_ALLOWED);
        return;
    }

    if (hm->body.len <= 0 || hm->body.len > 1024*1024) {  // Limit to 1MB
        send_json_response(c, 400, RESPONSE_ERROR_INVALID_BODY);
        return;
    }

    char *body_str = malloc(hm->body.len + 1);
    if (!body_str) {
        send_json_response(c, 500, RESPONSE_ERROR_MEMORY_ALLOCATION);
        return;
    }
    
    memcpy(body_str, hm->body.buf, hm->body.len);
    body_str[hm->body.len] = '\0';

    cJSON *json = cJSON_Parse(body_str);
    free(body_str);
    
    if (!json) {
        send_json_response(c, 400, RESPONSE_ERROR_INVALID_JSON);
        return;
    }

    int task_count = 0;
    int error_count = 0;
    cJSON *tasks_array = NULL;
    int created_new_array = 0;

    if (cJSON_IsArray(json)) {
        tasks_array = json;
    } else if (cJSON_IsObject(json)) {
        tasks_array = cJSON_CreateArray();
        if (!tasks_array) {
            cJSON_Delete(json);
            send_json_response(c, 500, RESPONSE_ERROR_MEMORY_ALLOCATION);
            return;
        }
        cJSON_AddItemReferenceToArray(tasks_array, json);
        created_new_array = 1;
    } else {
        cJSON_Delete(json);
        send_json_response(c, 400, RESPONSE_ERROR_JSON_FORMAT);
        return;
    }

    int array_size = cJSON_GetArraySize(tasks_array);
    for (int i = 0; i < array_size; i++) {
        cJSON *task_obj = cJSON_GetArrayItem(tasks_array, i);

        cJSON *taskName = cJSON_GetObjectItem(task_obj, "taskName");
        cJSON *cronExpression = cJSON_GetObjectItem(task_obj, "cronExpression");
        cJSON *taskExecution = cJSON_GetObjectItem(task_obj, "taskExecution");

        if (!taskName || !cronExpression || !taskExecution || 
            !cJSON_IsString(taskName) || !cJSON_IsString(cronExpression) || !cJSON_IsString(taskExecution)) {
            error_count++;
            continue;
        }

        Task task;
        memset(&task, 0, sizeof(Task));
        task.next = NULL;

        strncpy(task.taskName, taskName->valuestring, sizeof(task.taskName) - 1);
        strncpy(task.cronExpression, cronExpression->valuestring, sizeof(task.cronExpression) - 1);
        strncpy(task.taskExecution, taskExecution->valuestring, sizeof(task.taskExecution) - 1);

        insert_new_dag(g_db, &task);
        task_count++;
    }

    if (created_new_array) {
        cJSON_Delete(tasks_array);  
    } else {
        cJSON_Delete(json);  // Just delete the original json
    }

    if (task_count > 0) {
        char response_buffer[256];
        snprintf(response_buffer, sizeof(response_buffer),
                RESPONSE_SUCCESS_MULTIPLE,
                task_count,
                error_count > 0 ? ", " : "",
                error_count > 0 ? "with some errors" : "");

        send_json_response(c, 201, response_buffer);
    } else {
        send_json_response(c, 400, RESPONSE_ERROR_NO_VALID_TASKS);
    }
}

// Main event handler
static void event_handler(struct mg_connection *c, int ev, void *ev_data) {
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;
        
        // Route handling
        if (mg_match(hm->uri, mg_str("/api/dag_data"), NULL)) {
            api_data_handler(c, hm);
        } else if (mg_match(hm->uri, mg_str("/api/new_dag"), NULL)) {
            create_task_handler(c, hm);
        } else {
            // Handle 404 - endpoint not found
            char response_buffer[1024];
            char uri_str[512];
            
            // Safely extract URI
            size_t uri_len = hm->uri.len < sizeof(uri_str) - 1 ? hm->uri.len : sizeof(uri_str) - 1;
            memcpy(uri_str, hm->uri.buf, uri_len);
            uri_str[uri_len] = '\0';
            
            snprintf(response_buffer, sizeof(response_buffer), 
                    RESPONSE_ERROR_ENDPOINT_NOT_FOUND, uri_str);
            
            send_json_response(c, 404, response_buffer);
        }
    }
}

int initialize_webserver(sqlite3 *db) {
    struct mg_mgr mgr;
    struct mg_connection *c;

    g_db = db;
    
    mg_mgr_init(&mgr);

    c = mg_http_listen(&mgr, "http://0.0.0.0:8080", event_handler, &mgr);
    if (c == NULL) {
        log_message("Failed to create HTTP listener\n");
        return -1;
    }
    
    log_message("Server running at http://localhost:8080\n");
    
    while (1) {
        mg_mgr_poll(&mgr, 1000);
    }
    
    mg_mgr_free(&mgr);
    return 0;
}
