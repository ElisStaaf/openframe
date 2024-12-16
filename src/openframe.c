#include <openframe.h>

static openframe_t *openframe_create();
static void openframe_destroy();
static void openframe_send_response(int sock, openframe_t *openframe);
static void openframe_set_response(openframe_t *openframe, http_response *response);
static void openframe_set_request(openframe_t *openframe, http_request *request);


static openframe_t *get_openframe()
{
    return (openframe_t *)pthread_getspecific(global_server->frame_key);
}

static openframe_t *openframe_create()
{
    openframe_t *openframe = (openframe_t *)malloc(sizeof(openframe_t));
    if (openframe == NULL)
    {
        perror("Could not allocate memory for openframe_t");
        openframe_terminate();
        exit(1);
    }
    config_t *config = config_new();
    openframe->config = config;
    return openframe;
}

static void openframe_destroy()
{
    openframe_t *openframe = get_openframe();
    if (openframe->request)
    {
        http_free_request(openframe->request);
    }
    if (openframe->response)
    {
        http_free_response(openframe->response);
    }
    config_free(openframe->config);
    free(openframe);
}

static void openframe_send_response(int sock, openframe_t *openframe)
{
    char *response = http_response_to_string(openframe->response);

    char *req_str = http_request_to_string(openframe->request);
    printf("%s %d s:%ld\r\n", req_str, openframe->response->status_code, strlen(response));
    free(req_str);

    ssize_t sent = send(sock, response, strlen(response), 0);
    if (sent < 0)
    {
        perror("send failed");
    }
    free(response);
    close(sock);
}

static void openframe_set_response(openframe_t *openframe, http_response *response)
{
    openframe->response = response;
}

static void openframe_set_request(openframe_t *openframe, http_request *request)
{
    openframe->request = request;
}

void openframe_start()
{
    openframe_t *openframe = openframe_create();
    pthread_setspecific(global_server->frame_key, openframe);
    database_init();
}

void openframe_terminate()
{
    openframe_destroy();
}

static char *openframe_handle_session(openframe_t *openframe)
{
    session_t *session = session_create();
    openframe->session = session;

    char *id = hashmap_get(openframe->request->cookies, "SESSIONID");
    if (id)
    {
        session->id = strdup(id);
    } else {
        session->id = NULL;
    }

    return session->id;
}

static void openframe_set_session_cookie(openframe_t *openframe)
{
    if (openframe->session->id)
    {
        http_response_set_cookie(openframe->response, "SESSIONID", openframe->session->id, "/", 3600);
    }
}

void openframe_handle(int sock, char *buffer)
{
    openframe_t *openframe = get_openframe();
    openframe_set_request(openframe, http_create_request());
    if (openframe->request == NULL)
    {
        openframe_set_response(openframe, http_response_error(500, "Internal Server Error"));
        openframe_send_response(sock, openframe);
        return;
    }
    parser_parse_request(buffer, openframe->request);
    if (openframe->request->error)
    {
        openframe_set_response(openframe, http_response_error(400, "Bad Request"));
        openframe_send_response(sock, openframe);
        return;
    }
    openframe_handle_session(openframe);
    session_start(openframe->session);


    pthread_mutex_lock(&global_server->lock);
    openframe_set_response(openframe, router_handle_request(global_server->router, openframe->request));
    pthread_mutex_unlock(&global_server->lock);

    openframe_set_session_cookie(openframe);
    session_end(openframe->session);

    openframe_send_response(sock, openframe);
}

void openframe_response_set_header(char *key, char *value)
{
    openframe_t *openframe = get_openframe();
    http_response_add_header(openframe->response, key, value);
}

void openframe_response_set_status(int status)
{
    openframe_t *openframe = get_openframe();
    openframe->response->status_code = status;
}

http_request *openframe_get_request()
{
    return get_openframe()->request;
}

char *openframe_request(char *key, char *def)
{
    openframe_t *openframe = get_openframe();
    char *value = http_request_get_param(openframe->request, key);
    if (!value)
    {
        value = http_request_get_get_param(openframe->request, key);
    }

    if (value) return value;
    return def;
}

void openframe_session_set(char *key, char *value)
{
    openframe_t *openframe = get_openframe();
    session_t *session = openframe->session;

    session_set(session, key, value);
}

char *openframe_session_get(char *key, char *def)
{
    openframe_t *openframe = get_openframe();
    session_t *session = openframe->session;

    char *value = session_get(session, key);
    if (!value)
    {
        return def;
    }
    return value;
}

const char *openframe_env(char *key, char *def)
{
    openframe_t *openframe = get_openframe();
    return config_get(openframe->config, key, def);
}
