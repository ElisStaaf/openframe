#ifndef _http_h
#define _http_h

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hash.h>
#include <stdbool.h>
#include <ctype.h>
#include <utils.h>

typedef struct {
    char method[10];
    char uri[4096];
    char version[10];

    char *body;
    size_t body_length;

    hashmap_map *headers;

    char *_query_string;
    size_t query_string_length;

    hashmap_map *params; // /users/{id} -> /users/1 -> {id: 1}
    int param_count;

    hashmap_map *cookies;

    hashmap_map *getParams; // /users?id=1 -> {id: 1}

    int error;
} http_request;

// struct http_response
typedef struct {
    char version[10];
    int status_code;
    char status_message[50];
    char *body;
    size_t body_length;
    hashmap_map *headers;
    hashmap_map *cookies;
} http_response;

typedef struct {
    char* name;              // Cookie name
    char* value;             // Cookie value
    char* domain;            // Domain
    char* path;              // Path
    char* expires;           // Expires
    bool secure;             // Secure
    bool httpOnly;           // HttpOnly
    char* sameSite;          // SameSite
} http_cookie;

char *http_request_to_string(http_request *request);
http_request *http_create_request();
void http_free_request(http_request *request);

http_response *http_create_response(int status_code, const char *status_message, char *body);
char *http_response_to_string(http_response *response);
void http_free_response(http_response *response);
void http_free_cookie(http_cookie *cookie);

int http_response_add_header(http_response *response, char *key, char *value);
int http_request_add_header(http_request *req, char *key, char *value);
char *http_request_get_header_value(http_request *req, char *key);

char *http_request_get_param(http_request *req, char *key);
http_cookie *http_request_get_cookie(http_request *req, char *key);
char *http_request_get_get_param(http_request *req, char *key);



char *http_response_status_message(int status_code); // maps status code to status message
http_response *http_response_json(int status_code, char *json_data);
http_response *http_response_text(int status_code, char *text);
http_response *http_response_redirect(char *path);
http_response *http_response_view(int status_code, char *view_name, hashmap_map *context);


void http_response_set_cookie(http_response *resp, char *name, char *value, char *path, int max_age);
/*
    Calls http_response_text with the status code and error message
    It does the same thing as http_response_text but it's more readable
*/
http_response *http_response_error(int status_code, char *error_message);


#endif