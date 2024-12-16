#include <template.h>

char *template_compute(char *template, hashmap_map *map)
{
    // Buffer to hold the final result, dynamically resizing as needed
    size_t buffer_size = strlen(template) * 2; // Initial buffer size
    char *result = malloc(buffer_size);
    if (!result)
        return NULL; // Check for malloc failure

    result[0] = '\0'; // Initialize the result as an empty string

    char *current_position = template;
    char *end = NULL;
    while ((current_position = strstr(current_position, "{{")) != NULL)
    {
        end = strstr(current_position, "}}");
        if (!end)
            break; // No closing braces found

        // Resize result buffer if necessary
        size_t new_length = strlen(result) + (current_position - template) + (end - current_position);
        if (new_length >= buffer_size)
        {
            buffer_size *= 2; // Double the buffer size
            char *temp = realloc(result, buffer_size);
            if (!temp)
            {
                free(result);
                return NULL; // Realloc failure
            }
            result = temp;
        }

        // Copy everything up to the '{{' into result
        strncat(result, template, current_position - template);

        // Prepare to look up the key in the hashmap
        char *key = malloc(sizeof(char) * (end - current_position - 1));
        if (!key)
        {
            free(result);
            return NULL; // Malloc failure for key
        }
        strncpy(key, current_position + 2, end - (current_position + 2));
        key[end - (current_position + 2)] = '\0'; // Null-terminate the key

        trim_whitespace(key); // Remove leading and trailing whitespace

        // Get the value from the hashmap
        const char *value = hashmap_get(map, key);
        free(key); // Free the key
        if (value)
        {
            strcat(result, value); // Append the value to the result
        }
        else
        {
            strcat(result, "null"); // Append "null" if the key is not found
        }

        // Move past the current placeholder
        current_position = end + 2;
        template = current_position;
    }

    // Ensure there's enough space for the remaining part of the template string
    if (strlen(result) + strlen(template) >= buffer_size)
    {
        buffer_size += strlen(template);
        char *temp = realloc(result, buffer_size);
        if (!temp)
        {
            free(result);
            return NULL; // Realloc failure
        }
        result = temp;
    }

    // Append any remaining part of the template string
    strcat(result, template);

    return result;
}