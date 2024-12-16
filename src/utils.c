#include <utils.h>

void trim_whitespace(char *str)
{
    char *start = str;
    char *end;
    char *temp = str;

    // Skip leading whitespace without modifying the str pointer
    while (isspace((unsigned char)*start))
        start++;

    // Copy the trimmed start to the beginning of the string
    while (*start)
    {
        *temp++ = *start++;
    }
    *temp = '\0'; // Null-terminate the possibly moved string

    // If the string only contained whitespace, return early
    if (*str == 0)
        return;

    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end))
        end--;

    // Write new null terminator to remove trailing whitespace
    *(end + 1) = '\0';
}