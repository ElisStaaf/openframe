#ifndef _FRAMEC_H
#define _FRAMEC_H

#include <http.h>
#include <routing.h>
#include <pthread.h>
#include <server.h>
#include <http.h>
#include <session.h>
#include <config.h>
#include <database.h>

// Framework control structure
typedef struct _openframe_t{
    http_request *request; // current request
    http_response *response; // response that will be sent
    session_t *session; // current session
    config_t *config; // current configuration
} openframe_t;


void openframe_start(); // Start the framework
void openframe_terminate(); // Terminate the framework
void openframe_handle(int sock, char *r); // takes raw request and handles it


// facade
void openframe_response_set_header(char *key, char *value);
void openframe_response_set_status(int status);

http_request *openframe_get_request();

char *openframe_request(char *key, char *def);

void openframe_session_set(char *key, char *value);
char *openframe_session_get(char *key, char *def);


const char *openframe_env(char *key, char *def);

#endif // _FRAMEC_H
