#ifndef _ROUTING_H
#define _ROUTING_H

#define ROUTE_METHOD_GET 0
#define ROUTE_METHOD_POST 1
#define ROUTE_METHOD_PUT 2
#define ROUTE_METHOD_DELETE 3

#include <http.h>
#include <stdlib.h>
#include <string.h>
#include <hash.h>

typedef struct {
    char *path;
    int method;
    http_response *(*handler)();
} route_t;

typedef struct routing_table_t{
    route_t *route;
    struct routing_table_t *next;
} routing_table_t;

typedef struct {
    routing_table_t *table;
    int count;
} router_t;

router_t *router_create();
route_t *route_create();
routing_table_t *routing_table_create(route_t *route);
void router_add_route(router_t *router, route_t *route);

void router_get(router_t *router, const char *path, http_response *(*handler)(http_request *req));
void router_post(router_t *router, const char *path, http_response *(*handler)(http_request *req));
void router_put(router_t *router, const char *path, http_response *(*handler)(http_request *req));
void router_delete(router_t *router, const char *path, http_response *(*handler)(http_request *req));
int str_to_method(const char *method); // "GET" -> 0, "POST" -> 1, "PUT" -> 2, "DELETE" -> 3
http_response *router_handle_request(router_t *router, http_request *req);
route_t *match_route(http_request *req, router_t *router);
char **split_string(const char *str, const char *delimiter, int *count);
int add_param_to_request(http_request *req, char *key, const char *value);

void route_free(route_t *route);
void router_free(router_t *router);



#endif // _ROUTING_H