#ifndef RESPONSES_H
#define RESPONSES_H

// Success responses
#define RESPONSE_SUCCESS_CREATED "{\"success\":true,\"message\":\"Task created successfully\"}"
#define RESPONSE_SUCCESS_MULTIPLE "{\"success\":true,\"message\":\"%d task(s) created successfully%s%s\"}"
#define RESPONSE_SUCCESS_UPDATED "{\"success\":true,\"message\":\"Task updated successfully\"}"
#define RESPONSE_SUCCESS_DELETED "{\"success\":true,\"message\":\"Task deleted successfully\"}"

// Error responses
#define RESPONSE_ERROR_METHOD_NOT_ALLOWED "{\"error\":true,\"message\":\"Only POST method is allowed\"}"
#define RESPONSE_ERROR_INVALID_BODY "{\"error\":true,\"message\":\"Invalid or missing request body\"}"
#define RESPONSE_ERROR_MEMORY_ALLOCATION "{\"error\":true,\"message\":\"Server error: Memory allocation failed\"}"
#define RESPONSE_ERROR_INVALID_JSON "{\"error\":true,\"message\":\"Invalid JSON format\"}"
#define RESPONSE_ERROR_MISSING_FIELDS "{\"error\":true,\"message\":\"Missing or invalid fields\"}"
#define RESPONSE_ERROR_NO_VALID_TASKS "{\"error\":true,\"message\":\"No valid tasks found in request\"}"
#define RESPONSE_ERROR_JSON_FORMAT "{\"error\":true,\"message\":\"JSON must be an object or array of objects\"}"
#define RESPONSE_ERROR_ENDPOINT_NOT_FOUND "{\"error\":true,\"message\":\"Endpoint not found\",\"status\":404,\"path\":\"%s\"}"
#define RESPONSE_ERROR_TASK_NOT_FOUND "{\"error\":true,\"message\":\"Task not found\"}"
#define RESPONSE_ERROR_MISSING_ID "{\"error\":true,\"message\":\"Missing required field: id\"}"
#define RESPONSE_ERROR_UPDATE_FAILED "{\"error\":true,\"message\":\"Failed to update task\"}"
#define RESPONSE_ERROR_DELETE_FAILED "{\"error\":true,\"message\":\"Failed to delete task\"}"

// Empty responses
#define RESPONSE_EMPTY_TASKS "{\"tasks\":[]}"

// HTTP header templates
#define HTTP_HEADER_200_JSON "HTTP/1.1 200 OK\r\n"\
                             "Content-Type: application/json\r\n"\
                             "Access-Control-Allow-Origin: *\r\n"\
                             "Content-Length: %d\r\n"\
                             "\r\n"

#define HTTP_HEADER_201_CREATED "HTTP/1.1 201 Created\r\n"\
                                "Content-Type: application/json\r\n"\
                                "Content-Length: %d\r\n"\
                                "Access-Control-Allow-Origin: *\r\n"\
                                "\r\n"

#define HTTP_HEADER_400_BAD_REQUEST "HTTP/1.1 400 Bad Request\r\n"\
                                   "Content-Type: application/json\r\n"\
                                   "Content-Length: %d\r\n"\
                                   "Access-Control-Allow-Origin: *\r\n"\
                                   "\r\n"

#define HTTP_HEADER_404_NOT_FOUND "HTTP/1.1 404 Not Found\r\n"\
                                 "Content-Type: application/json\r\n"\
                                 "Content-Length: %d\r\n"\
                                 "Access-Control-Allow-Origin: *\r\n"\
                                 "\r\n"

#define HTTP_HEADER_405_METHOD_NOT_ALLOWED "HTTP/1.1 405 Method Not Allowed\r\n"\
                                         "Content-Type: application/json\r\n"\
                                         "Content-Length: %d\r\n"\
                                         "Access-Control-Allow-Origin: *\r\n"\
                                         "\r\n"

#define HTTP_HEADER_500_SERVER_ERROR "HTTP/1.1 500 Internal Server Error\r\n"\
                                    "Content-Type: application/json\r\n"\
                                    "Content-Length: %d\r\n"\
                                    "Access-Control-Allow-Origin: *\r\n"\
                                    "\r\n"

#define HTTP_HEADER_302_REDIRECT "HTTP/1.1 302 Found\r\n"\
                "Location: %s\r\n"\
                "Content-Length: 0\r\n"\
                "Access-Control-Allow-Origin: *\r\n"\
                "\r\n"
#endif /* RESPONSES_H */
