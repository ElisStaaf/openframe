#include <http.h>

http_request *http_create_request()
{
    http_request *req = (http_request *)malloc(sizeof(http_request));
    if (!req)
        return NULL; // Failed to allocate memory for request

    *req = (http_request){0};

    req->headers = hashmap_new();
    if (!req->headers)
    {
        free(req);
        return NULL; // Failed to create headers map
    }

    req->params = hashmap_new();
    if (!req->params)
    {
        hashmap_free(req->headers);
        free(req);
        return NULL; // Failed to create params map
    }

    req->getParams = hashmap_new();
    if (!req->getParams)
    {
        hashmap_free(req->headers);
        hashmap_free(req->params);
        free(req);
        return NULL; // Failed to create getParams map
    }

    req->cookies = hashmap_new();
    if (!req->cookies)
    {
        hashmap_free(req->headers);
        hashmap_free(req->params);
        hashmap_free(req->getParams);
        free(req);
        return NULL; // Failed to create cookies map
    }

    return req;
}

void http_free_request(http_request *req)
{
    if (req->_query_string)
    {
        free(req->_query_string);
    }
    if (req->headers)
    {
        hashmap_free(req->headers);
    }
    if (req->body)
    {
        free(req->body);
    }

    if (req->params)
        hashmap_free(req->params);
    if (req->getParams)
        hashmap_free(req->getParams);
    if (req->cookies)
        hashmap_free(req->cookies);
    free(req);
}

char *http_request_to_string(http_request *request)
{
    // Calculate the total length needed for the string representation
    size_t totalLength = strlen(request->method) + strlen(request->uri) + strlen(request->version) + 4; // Space for spaces and \r\n
    if (request->_query_string)
    {
        totalLength += strlen(request->_query_string) + 1; // +1 for '?'
    }

    // Allocate memory for the string representation
    char *requestString = (char *)malloc(totalLength + 1); // +1 for null terminator
    if (!requestString)
    {
        return NULL; // Memory allocation failed
    }

    // Start constructing the string
    char *ptr = requestString;
    sprintf(ptr, "<%s %s%s%s %s>",
            request->method,
            request->uri,
            request->_query_string ? "?" : "",
            request->_query_string ? request->_query_string : "",
            request->version);

    return requestString;
}

http_response *http_create_response(int status_code, const char *status_message, char *body)
{
    http_response *response = (http_response *)malloc(sizeof(http_response));
    if (!response)
        return NULL; // Failed to allocate memory for response

    strcpy(response->version, "HTTP/1.1");
    response->status_code = status_code;
    strncpy(response->status_message, status_message, sizeof(response->status_message) - 1);
    response->status_message[sizeof(response->status_message) - 1] = '\0'; // Ensure null-termination

    response->body = body ? strdup(body) : NULL;
    response->body_length = body ? strlen(body) : 0;

    response->headers = hashmap_new();
    if (!response->headers)
    {
        free(response);
        return NULL; // Failed to create headers map
    }
    response->cookies = hashmap_new();
    if (!response->cookies)
    {
        hashmap_free(response->headers);
        free(response);
        return NULL; // Failed to create cookies map
    }

    http_response_add_header(response, "Content-Type", "text/plain");
    if (body)
    {
        char contentLength[32];
        sprintf(contentLength, "%zu", response->body_length);
        http_response_add_header(response, "Content-Length", contentLength);
    }

    return response;
}

void http_free_response(http_response *response)
{
    if (response->body)
        free(response->body);
    // Free headers
    hashmap_free(response->headers);
    // Free cookies
    hashmap_free(response->cookies);
    free(response);
}

void append_header(const char *key, const char *value, void *responseString)
{
    // cast responseString to char*
    responseString = (char *)responseString;
    strcat(responseString, key);
    strcat(responseString, ": ");
    strcat(responseString, value);
    strcat(responseString, "\r\n");
}

// cookie value format: name=value; Path=/; Max-Age=3600
void append_cookie(const char *key, const char *value, void *responseString)
{
    (void)key; // unused
    // cast responseString to char*
    responseString = (char *)responseString;
    strcat(responseString, "Set-Cookie: ");
    strcat(responseString, value); // key is already included in value
    strcat(responseString, "\r\n");
}

void calculateLength(const char *key, const char *value, void *count)
{
    int *c = (int *)count;
    *c += strlen(key) + 2 + strlen(value) + 2; // key: value\r\n
}

char *http_response_to_string(http_response *response)
{
    // Estimate initial size
    size_t totalLength = strlen(response->version) + 4 + strlen(response->status_message) + 2 + response->body_length + 2;

    // Calculate headers length

    int count = 0;
    hashmap_iterate(response->headers, &calculateLength, &count);
    totalLength += count;
    // Calculate cookies length
    count = 0;
    hashmap_iterate(response->cookies, &calculateLength, &count);
    totalLength += count;

    char *responseString = (char *)malloc(totalLength + 100);
    if (!responseString)
        return NULL;

    sprintf(responseString, "%s %d %s\r\n", response->version, response->status_code, response->status_message);

    hashmap_iterate(response->headers, &append_header, responseString);
    hashmap_iterate(response->cookies, &append_cookie, responseString);

    strcat(responseString, "\r\n"); // Headers and body separator

    // Append body if exists
    if (response->body && response->body_length > 0)
    {
        strcat(responseString, response->body);
    }

    return responseString;
}

int http_response_add_header(http_response *response, char *key, char *value)
{
    if (!response || !key || !value)
        return -1; // Basic validation
    if (!response->headers)
    {
        response->headers = hashmap_new();
        if (!response->headers)
            return -1; // Failed to create headers map
    }
    return hashmap_put(response->headers, key, value);
}

int http_request_add_header(http_request *req, char *key, char *value)
{
    if (!req || !key || !value)
        return -1; // Basic validation
    if (!req->headers)
    {
        req->headers = hashmap_new();
        if (!req->headers)
            return -1; // Failed to create headers map
    }
    return hashmap_put(req->headers, key, value);
}

char *http_request_get_header_value(http_request *req, char *key)
{
    if (!req || !key)
        return NULL; // Basic validation
    return hashmap_get(req->headers, key);
}

char *http_request_get_param(http_request *req, char *key)
{
    if (!req || !key)
        return NULL; // Basic validation
    return hashmap_get(req->params, key);
}

http_cookie *http_parse_cookie_string(char *cookieString)
{
    http_cookie *cookie = malloc(sizeof(http_cookie));
    if (!cookie)
        return NULL; // Memory allocation failed

    *cookie = (http_cookie){0};

    char *rest = cookieString;
    char *token;
    char *innerRest;

    while ((token = strtok_r(rest, ";", &rest)))
    {
        char *key = strtok_r(token, "=", &innerRest);
        char *value = strtok_r(NULL, "", &innerRest); // Use an empty string to get the rest of the token

        if (key != NULL && value != NULL)
        {
            trim_whitespace(key);
            trim_whitespace(value);

            if (strcmp(key, "Expires") == 0)
            {
                cookie->expires = strdup(value);
            }
            else if (strcmp(key, "Path") == 0)
            {
                cookie->path = strdup(value);
            }
            else if (strcmp(key, "Domain") == 0)
            {
                cookie->domain = strdup(value);
            }
            else if (strcmp(key, "SameSite") == 0)
            {
                cookie->sameSite = strdup(value);
            }
            else if (cookie->name == NULL && cookie->value == NULL)
            {
                cookie->name = strdup(key);
                cookie->value = strdup(value);
            }
        }
        else if (key != NULL)
        {
            // Handle boolean attributes
            trim_whitespace(key);
            if (strcmp(key, "Secure") == 0)
            {
                cookie->secure = true;
            }
            else if (strcmp(key, "HttpOnly") == 0)
            {
                cookie->httpOnly = true;
            }
        }
    }

    return cookie;
}

http_cookie *http_request_get_cookie(http_request *req, char *key)
{
    const char *cookieString = hashmap_get(req->cookies, key);
    if (!cookieString)
    {
        printf("Cookie %s not found\n", key);
        return NULL; // Cookie not found
    }

    // Make a copy of the cookie string since strtok modifies the string
    char *cookieStringCopy = strdup(cookieString);
    if (!cookieStringCopy)
        return NULL; // Memory allocation failed

    http_cookie *cookie = http_parse_cookie_string(cookieStringCopy);

    free(cookieStringCopy); // Clean up the temporary copy

    return cookie;
}

void http_free_cookie(http_cookie *cookie)
{
    if (cookie == NULL)
    {
        return; // Nothing to free
    }

    // Free each dynamically allocated field if it's not NULL
    if (cookie->name != NULL)
    {
        free(cookie->name);
    }
    if (cookie->value != NULL)
    {
        free(cookie->value);
    }
    if (cookie->domain != NULL)
    {
        free(cookie->domain);
    }
    if (cookie->path != NULL)
    {
        free(cookie->path);
    }
    if (cookie->expires != NULL)
    {
        free(cookie->expires);
    }
    if (cookie->sameSite != NULL)
    {
        free(cookie->sameSite);
    }

    // Finally, free the struct itself
    free(cookie);
}

char *http_request_get_get_param(http_request *req, char *key)
{
    if (!req || !key)
        return NULL; // Basic validation
    return hashmap_get(req->getParams, key);
}