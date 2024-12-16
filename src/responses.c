#include <http.h>
#include <hash.h>
#include <template.h>

http_response *http_response_json(int status_code, char *json_data)
{
    http_response *resp = http_create_response(status_code, http_response_status_message(status_code), json_data);
    http_response_add_header(resp, "Content-Type", "application/json");
    return resp;
}

http_response *http_response_text(int status_code, char *text)
{
    http_response *resp = http_create_response(status_code, http_response_status_message(status_code), text);
    // No additional content type header needed since text/plain is default
    return resp;
}

http_response *http_response_redirect(char *path)
{
    http_response *resp = http_create_response(301, http_response_status_message(301), "");
    http_response_add_header(resp, "Location", path);
    // Content-Type not defined in redirect responses
    // Leave it text/plain will be ignored
    return resp;
}

char *http_get_file_content(char *filename)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        // File not found
        printf("File not found: %s\n", filename);
        // perror
        perror("fopen");
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *buffer = NULL;
    if (length > 0)
    {
        buffer = malloc(length + 1);
    }
    else
    {
        fclose(file);
        return "";
    }
    buffer[length] = '\0';
    fread(buffer, 1, length, file);
    fclose(file);
    return buffer;
}

char *http_viewname_to_filename(char *view_name)
{
    size_t filename_size = strlen(view_name) + 6 + 5 + 1; // "views/", ".html", and null terminator
    char *filename = malloc(filename_size);

    if (filename == NULL)
    {
        printf("Failed to allocate memory for filename\n");
        return NULL; // Handle malloc failure
    }

    // Initialize the memory to ensure it's clean
    memset(filename, 0, filename_size);

    // Safely concatenate the strings
    strcpy(filename, "views/");
    strcat(filename, view_name);
    strcat(filename, ".html");
    return filename;
}

char *http_render_template(char *view_name, hashmap_map *context)
{
    char *filename = http_viewname_to_filename(view_name);
    char *template = http_get_file_content(filename);
    free(filename);
    if (template == NULL)
    {
        return NULL;
    }
    char *result = template_compute(template, context);
    free(template);
    if (result == NULL)
    {
        return NULL;
    }
    return result;
}

http_response *http_response_view(int status_code, char *view_name, hashmap_map *context)
{
    char *html = http_render_template(view_name, context);
    if (html == NULL)
    {
        char *error_message = malloc(strlen(view_name) + 50);
        sprintf(error_message, "Template not found: %s", view_name);
        http_response *r = http_response_error(404, error_message);
        free(error_message);
        free(html);
        return r;
    }
    http_response *resp = http_create_response(status_code, http_response_status_message(status_code), html);
    free(html);
    http_response_add_header(resp, "Content-Type", "text/html");
    return resp;
}

char *http_response_status_message(int status_code)
{
    switch (status_code)
    {
    case 200:
        return "OK";
    case 301:
        return "Moved Permanently";
    case 400:
        return "Bad Request";
    case 401:
        return "Unauthorized";
    case 403:
        return "Forbidden";
    case 404:
        return "Not Found";
    case 405:
        return "Method Not Allowed";
    case 500:
        return "Internal Server Error";
    default:
        return "Unknown Status";
    }
}

http_response *http_response_error(int status_code, char *error_message)
{
    return http_response_text(status_code, error_message);
}

// key = cookie name, value = cookie value
// cookie value format: name=value; Path=/; Max-Age=3600
int http_response_add_cookie(http_response *response, char *key, char *value)
{
    return hashmap_put(response->cookies, key, value);
}

void http_response_set_cookie(http_response *resp, char *name, char *value, char *path, int max_age)
{
    char *cookie = malloc(strlen(name) + strlen(value) + strlen(path) + 50);
    sprintf(cookie, "%s=%s; Path=%s; Max-Age=%d", name, value, path, max_age);
    http_response_add_cookie(resp, name, cookie);
    free(cookie);
}