#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <cjson/cJSON.h>
#include "mongoose.h"
#include "database.h"
#include "logger.h"
#include "responses.h"
#include "transactions.h"
#include "dag.h"
#include "dag_scheduler.h"

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

static void dag_edit_handler(struct mg_connection *c, struct mg_http_message *hm) {
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

    if (!cJSON_IsObject(json)) {
        cJSON_Delete(json);
        send_json_response(c, 400, RESPONSE_ERROR_JSON_FORMAT);
        return;
    }

    cJSON *id_obj = cJSON_GetObjectItem(json, "id");
    if (!id_obj || !cJSON_IsNumber(id_obj)) {
        cJSON_Delete(json);
        send_json_response(c, 400, RESPONSE_ERROR_MISSING_ID);
        return;
    }

    int id = id_obj->valueint;

    cJSON *taskName = cJSON_GetObjectItem(json, "taskName");
    cJSON *cronExpression = cJSON_GetObjectItem(json, "cronExpression");
    cJSON *taskExecution = cJSON_GetObjectItem(json, "taskExecution");

    if ((!taskName || !cJSON_IsString(taskName)) ||
        (!cronExpression || !cJSON_IsString(cronExpression)) ||
        (!taskExecution || !cJSON_IsString(taskExecution))) {
        cJSON_Delete(json);
        send_json_response(c, 400, RESPONSE_ERROR_MISSING_FIELDS);
        return;
    }

    int result = update_dag(g_db, id, taskName->valuestring, 
                           cronExpression->valuestring, taskExecution->valuestring);
    
    if (result == 1) {
        log_task_status(g_db, id, "UPDATED", "Updated fields: taskName, cronExpression, taskExecution via API");
        send_json_response(c, 200, RESPONSE_SUCCESS_UPDATED);
    } else if (result == 0) {
        send_json_response(c, 404, RESPONSE_ERROR_TASK_NOT_FOUND);
    } else {
        send_json_response(c, 500, RESPONSE_ERROR_UPDATE_FAILED);
    }
    
    cJSON_Delete(json);
}

static void dag_delete_handler(struct mg_connection *c, struct mg_http_message *hm) {
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

    if (!cJSON_IsObject(json)) {
        cJSON_Delete(json);
        send_json_response(c, 400, RESPONSE_ERROR_JSON_FORMAT);
        return;
    }

    cJSON *id_obj = cJSON_GetObjectItem(json, "id");
    if (!id_obj || !cJSON_IsNumber(id_obj)) {
        cJSON_Delete(json);
        send_json_response(c, 400, RESPONSE_ERROR_MISSING_ID);
        return;
    }

    int id = id_obj->valueint;
    
    int result = delete_dag(g_db, id);
    
    if (result == 1) {
        log_task_status(g_db, id, "DELETED", "Task deleted via API");
        send_json_response(c, 200, RESPONSE_SUCCESS_DELETED);
    } else if (result == 0) {
        send_json_response(c, 404, RESPONSE_ERROR_TASK_NOT_FOUND);
    } else {
        send_json_response(c, 500, RESPONSE_ERROR_DELETE_FAILED);
    }
    
    cJSON_Delete(json);
}

// DAG Management Handlers

static void create_dag_handler(struct mg_connection *c, struct mg_http_message *hm) {
    if (mg_strcmp(hm->method, mg_str("POST")) != 0) {
        send_json_response(c, 405, RESPONSE_ERROR_METHOD_NOT_ALLOWED);
        return;
    }

    if (hm->body.len <= 0 || hm->body.len > 1024*1024) {
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

    if (!cJSON_IsObject(json)) {
        cJSON_Delete(json);
        send_json_response(c, 400, RESPONSE_ERROR_JSON_FORMAT);
        return;
    }

    // Extract DAG fields
    cJSON *name = cJSON_GetObjectItem(json, "name");
    cJSON *cron_expression = cJSON_GetObjectItem(json, "cron_expression");
    cJSON *description = cJSON_GetObjectItem(json, "description");
    cJSON *tasks = cJSON_GetObjectItem(json, "tasks");

    if (!name || !cJSON_IsString(name)) {
        cJSON_Delete(json);
        send_json_response(c, 400, RESPONSE_ERROR_MISSING_DAG_NAME);
        return;
    }

    if (!cron_expression || !cJSON_IsString(cron_expression)) {
        cJSON_Delete(json);
        send_json_response(c, 400, RESPONSE_ERROR_MISSING_CRON_EXPRESSION);
        return;
    }

    if (!tasks || !cJSON_IsArray(tasks)) {
        cJSON_Delete(json);
        send_json_response(c, 400, RESPONSE_ERROR_MISSING_TASKS);
        return;
    }

    // Create DAG
    DAG *dag = create_dag(name->valuestring, cron_expression->valuestring, 
                         description ? description->valuestring : "");
    if (!dag) {
        cJSON_Delete(json);
        send_json_response(c, 500, RESPONSE_ERROR_DAG_CREATE_FAILED);
        return;
    }

    // Insert DAG into database
    int dag_id = insert_dag_db(g_db, dag);
    if (dag_id < 0) {
        free_dag(dag);
        cJSON_Delete(json);
        send_json_response(c, 500, RESPONSE_ERROR_DAG_CREATE_FAILED);
        return;
    }

    // Process tasks in two phases: first create all tasks, then resolve dependencies
    
    // Phase 1: Create all tasks without dependencies
    int task_count = cJSON_GetArraySize(tasks);
    DAGTask **task_array = NULL;
    if (task_count > 0) {
        task_array = malloc(task_count * sizeof(DAGTask*));
        if (!task_array) {
            free_dag(dag);
            cJSON_Delete(json);
            send_json_response(c, 500, RESPONSE_ERROR_MEMORY_ALLOCATION);
            return;
        }
    }
    
    for (int i = 0; i < task_count; i++) {
        cJSON *task_obj = cJSON_GetArrayItem(tasks, i);
        
        cJSON *task_name = cJSON_GetObjectItem(task_obj, "task_name");
        cJSON *task_execution = cJSON_GetObjectItem(task_obj, "task_execution");

        if (!task_name || !cJSON_IsString(task_name) ||
            !task_execution || !cJSON_IsString(task_execution)) {
            task_array[i] = NULL;
            continue;
        }

        DAGTask *dag_task = create_dag_task(dag_id, task_name->valuestring, 
                                           task_execution->valuestring);
        if (!dag_task) {
            task_array[i] = NULL;
            continue;
        }

        // Insert task into database to get ID
        int task_id = insert_dag_task_db(g_db, dag_task);
        if (task_id >= 0) {
            dag_task->id = task_id;
            // Add to DAG task list
            dag_task->next = dag->tasks;
            dag->tasks = dag_task;
            dag->task_count++;
            task_array[i] = dag_task;
        } else {
            free_dag_task(dag_task);
            task_array[i] = NULL;
        }
    }
    
    // Phase 2: Resolve dependencies by name and update database
    for (int i = 0; i < task_count; i++) {
        if (!task_array[i]) continue;
        
        cJSON *task_obj = cJSON_GetArrayItem(tasks, i);
        cJSON *dependencies = cJSON_GetObjectItem(task_obj, "dependencies");

        if (dependencies && cJSON_IsArray(dependencies)) {
            int dep_count = cJSON_GetArraySize(dependencies);
            for (int j = 0; j < dep_count; j++) {
                cJSON *dep = cJSON_GetArrayItem(dependencies, j);
                if (cJSON_IsString(dep)) {
                    // Find the task with this name
                    const char *dep_name = dep->valuestring;
                    for (int k = 0; k < task_count; k++) {
                        if (task_array[k] && strcmp(task_array[k]->task_name, dep_name) == 0) {
                            add_task_dependency(task_array[i], task_array[k]->id, dep_name);
                            break;
                        }
                    }
                }
            }
            
            // Update the task dependencies in database
            insert_dag_task_db(g_db, task_array[i]); // This will update the dependencies JSON
        }
    }
    
    free(task_array);

    char response_buffer[256];
    snprintf(response_buffer, sizeof(response_buffer), RESPONSE_DAG_SUCCESS_CREATED, dag_id);
    send_json_response(c, 201, response_buffer);

    // Reload DAGs in scheduler
    reload_dags(g_db);

    free_dag(dag);
    cJSON_Delete(json);
}

static void get_dags_handler(struct mg_connection *c, struct mg_http_message *hm) {
    char *json_data = get_dags_json(g_db);
    if (!json_data) {
        send_json_response(c, 200, "{\"dags\":[]}");
        return;
    }
    
    send_json_response(c, 200, json_data);
    free(json_data);
}

static void get_dag_status_handler(struct mg_connection *c, struct mg_http_message *hm) {
    // Extract DAG ID from URI path
    char uri_str[256];
    size_t uri_len = hm->uri.len < sizeof(uri_str) - 1 ? hm->uri.len : sizeof(uri_str) - 1;
    memcpy(uri_str, hm->uri.buf, uri_len);
    uri_str[uri_len] = '\0';
    
    // Parse ID from /api/dag/{id}/status
    int dag_id = 0;
    if (sscanf(uri_str, "/api/dag/%d/status", &dag_id) != 1) {
        send_json_response(c, 400, RESPONSE_ERROR_MISSING_ID);
        return;
    }
    
    char *json_data = get_dag_status_json(g_db, dag_id);
    if (!json_data) {
        send_json_response(c, 404, RESPONSE_ERROR_DAG_NOT_FOUND);
        return;
    }
    
    send_json_response(c, 200, json_data);
    free(json_data);
}

static void trigger_dag_handler(struct mg_connection *c, struct mg_http_message *hm) {
    if (mg_strcmp(hm->method, mg_str("POST")) != 0) {
        send_json_response(c, 405, RESPONSE_ERROR_METHOD_NOT_ALLOWED);
        return;
    }

    // Extract DAG ID from URI path
    char uri_str[256];
    size_t uri_len = hm->uri.len < sizeof(uri_str) - 1 ? hm->uri.len : sizeof(uri_str) - 1;
    memcpy(uri_str, hm->uri.buf, uri_len);
    uri_str[uri_len] = '\0';
    
    // Parse ID from /api/dag/{id}/trigger
    int dag_id = 0;
    if (sscanf(uri_str, "/api/dag/%d/trigger", &dag_id) != 1) {
        send_json_response(c, 400, RESPONSE_ERROR_MISSING_ID);
        return;
    }
    
    int result = trigger_dag_execution(g_db, dag_id);
    if (result == 0) {
        send_json_response(c, 200, RESPONSE_DAG_SUCCESS_TRIGGERED);
    } else {
        send_json_response(c, 404, RESPONSE_ERROR_DAG_NOT_FOUND);
    }
}

static void delete_dag_handler(struct mg_connection *c, struct mg_http_message *hm) {
    if (mg_strcmp(hm->method, mg_str("DELETE")) != 0) {
        send_json_response(c, 405, RESPONSE_ERROR_METHOD_NOT_ALLOWED);
        return;
    }

    // Extract DAG ID from URI path
    char uri_str[256];
    size_t uri_len = hm->uri.len < sizeof(uri_str) - 1 ? hm->uri.len : sizeof(uri_str) - 1;
    memcpy(uri_str, hm->uri.buf, uri_len);
    uri_str[uri_len] = '\0';
    
    // Parse ID from /api/dag/{id}
    int dag_id = 0;
    if (sscanf(uri_str, "/api/dag/%d", &dag_id) != 1) {
        send_json_response(c, 400, RESPONSE_ERROR_MISSING_ID);
        return;
    }
    
    int result = delete_dag_by_id_db(g_db, dag_id);
    if (result == 1) {
        send_json_response(c, 200, RESPONSE_DAG_SUCCESS_DELETED);
        reload_dags(g_db);
    } else if (result == 0) {
        send_json_response(c, 404, RESPONSE_ERROR_DAG_NOT_FOUND);
    } else {
        send_json_response(c, 500, RESPONSE_ERROR_DAG_DELETE_FAILED);
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
        } else if (mg_match(hm->uri, mg_str("/api/dag_edit"), NULL)) {
            dag_edit_handler(c, hm);
        } else if (mg_match(hm->uri, mg_str("/api/dag_delete"), NULL)) {
            dag_delete_handler(c, hm);
        // DAG Management Routes
        } else if (mg_match(hm->uri, mg_str("/api/dag"), NULL)) {
            if (mg_strcmp(hm->method, mg_str("POST")) == 0) {
                create_dag_handler(c, hm);
            } else {
                send_json_response(c, 405, RESPONSE_ERROR_METHOD_NOT_ALLOWED);
            }
        } else if (mg_match(hm->uri, mg_str("/api/dags"), NULL)) {
            get_dags_handler(c, hm);
        } else if (mg_match(hm->uri, mg_str("/api/dag/*/status"), NULL)) {
            get_dag_status_handler(c, hm);
        } else if (mg_match(hm->uri, mg_str("/api/dag/*/trigger"), NULL)) {
            trigger_dag_handler(c, hm);
        } else if (mg_match(hm->uri, mg_str("/api/dag/*"), NULL)) {
            delete_dag_handler(c, hm);
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
