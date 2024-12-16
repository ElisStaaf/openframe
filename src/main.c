#include <server.h>
#include <routing.h>
#include <openframe.h>

http_response *handle_index()
{
    hashmap_map *context = hashmap_new();
    hashmap_put(context, "title", openframe_env("APPNAME", "FrameC"));
    hashmap_put(context, "name", "Noah Scholz");
    http_response *r = http_response_view(200, "index", context);
    hashmap_free(context);
    return r;
}

// GET /hello
http_response *handle_hello() {
    return http_response_text(200, "Hello, World! <GET>");
}

// POST /hello
http_response *handle_hello_post() {
    return http_response_text(200, "Hello, World! <POST>");
}

// GET /hello/{id}
http_response *handle_hello_id() {
    char *id = openframe_request("id","nullID");
    char *response = malloc(strlen(id) + 8);
    sprintf(response, "Hello %s!", id);
    http_response *res = http_response_text(200, response);
    free(response);
    return res;
}

// GET /multiple/{id}/{name}
http_response *handle_multiple() {
    char *id = openframe_request("id", "nullID");
    char *name = openframe_request("name", "nullName");
    char *response = malloc(strlen(id) + strlen(name) + 3);
    sprintf(response, "%s %s", id, name);
    http_response *res = http_response_text(200, response);
    free(response);
    return res;
}

http_response *handle_session() {
    // Retrieve the current value from the session or "0" if not set
    char *valueStr = openframe_session_get("value", "0");
    int value = atoi(valueStr);
    
    // Increment the value
    value++;
    char newValueStr[12]; // Enough to hold int values, including negative numbers
    sprintf(newValueStr, "%d", value);
    
    // Update the session with the new value
    openframe_session_set("value", newValueStr);
    return http_response_text(200, newValueStr);
}

int main()
{
    router_t *router = router_create();
    router_get(router, "/", &handle_index);
    router_get(router, "/hello", &handle_hello);
    router_post(router, "/hello", &handle_hello_post);
    router_get(router, "/hello/{id}", &handle_hello_id);
    router_get(router, "/multiple/{id}/{name}", &handle_multiple);
    router_get(router, "/session", &handle_session);

    init_server(80, router);
    start_server();
    return 0;
}
