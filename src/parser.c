#include <parser.h>

void add_get_param(http_request *request, char *key, const char *value)
{
    hashmap_put(request->getParams, key, value);
}

void parser_parse_query_string(char *query, http_request *request)
{
    // Ensure there's a query to parse.
    if (query == NULL || *query == '\0') {
        return;
    }

    char *current = query;
    char *next;
    char *saveptr = NULL; // Single saveptr for consistent tokenization context

    // Use a loop to iterate over each parameter-value pair
    while ((next = strchr(current, '&')) != NULL || *current) {
        if (next != NULL) {
            *next = '\0'; // Temporarily end the string to isolate the current part
        }

        // Extract parameter and value
        char *param = strtok_r(current, "=", &saveptr);
        const char *value = strtok_r(NULL, "=", &saveptr); // Continue with the same saveptr

        if (param && value) {
            add_get_param(request, param, value);
        }

        if (next != NULL) {
            *next = '&'; // Restore the '&' character if we modified the string
            current = next + 1;
        } else {
            break; // No more parameters to process
        }
    }

    return;
}

void parse_start_line(const char *startLine, http_request *request) {
    sscanf(startLine, "%s %s %s", request->method, request->uri, request->version);
}

void parse_uri_and_query_string(const char *uri, http_request *request) {
    char *queryStart = strchr(uri, '?');
    if (queryStart) {
        *queryStart = '\0'; // Terminate URI
        request->_query_string = strdup(queryStart + 1);
        parser_parse_query_string(request->_query_string, request);
    } else {
        request->_query_string = NULL;
    }
}

void parse_cookies(char *cookies, http_request *request) {
    char *rest;
    char *cookie = strtok_r(cookies, ";", &rest);
    while (cookie) {
        char *cookieName = strtok_r(cookie, "=", &rest);
        const char *cookieValue = strtok_r(NULL, ";", &rest);
        if (cookieName && cookieValue) {
            trim_whitespace(cookieName);
            hashmap_put(request->cookies, cookieName, cookieValue);
        }
        cookie = strtok_r(NULL, ";", &rest);
    }
}

void process_header_line(char *line, http_request *request) {
    char *savePtr;
    char *key = strtok_r(line, ": ", &savePtr);
    char *value = strtok_r(NULL, "\r\n", &savePtr);

    if (strcmp(key, "Cookie") == 0) {
        parse_cookies(value, request);
    } else if (key && value) {
        hashmap_put(request->headers, key, value);
    }
}

void parser_parse_request(const char *req, http_request *request) {
    const char *currentLine = req;
    const char *nextLine = strchr(currentLine, '\r');
    if (!nextLine) {
        request->error = 1;
        return;
    }

    ptrdiff_t lineSize = nextLine - currentLine;
    char startLine[lineSize + 1];
    strncpy(startLine, currentLine, lineSize);
    startLine[lineSize] = '\0';
    parse_start_line(startLine, request);

    parse_uri_and_query_string(request->uri, request);

    currentLine = nextLine + 2; // Move past the start line
    while ((nextLine = strstr(currentLine, "\r\n")) && currentLine[0] != '\r') {
        lineSize = nextLine - currentLine;
        char lineBuffer[lineSize + 1];
        strncpy(lineBuffer, currentLine, lineSize);
        lineBuffer[lineSize] = '\0';
        process_header_line(lineBuffer, request);
        currentLine = nextLine + 2; // Move to the next line
    }

    if (strlen(currentLine) > 0) {
        request->body = strdup(currentLine);
    } else {
        request->body = NULL;
    }
    request->error = 0;
}

