#include <session.h>

static char *getFileName(session_t *session)
{
    if (session->filename) return session->filename;
    char *id = session->id;
    char *filename = (char *)malloc(strlen(id) + 15);
    strcpy(filename, "sessions/");
    strcat(filename, id);
    strcat(filename, ".txt");
    session->filename = filename;
    return filename;
}

static char *generate_session_id()
{
    // generate random file name string
    char *id = (char *)malloc(33);
    for (int i = 0; i < 32; i++)
    {
        id[i] = (rand() % 26) + 97;
    }
    id[32] = '\0';
    return id;
}

static char *load_session_from_file(session_t *session, char *id)
{
    char *filename = getFileName(session);
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        printf("Could not read file %s\n", filename);
        return NULL;
    }
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    char *state = NULL;
    while ((read = getline(&line, &len, file)) != -1)
    {
        char *key = strtok_r(line, "=", &state);
        char *value = strtok_r(NULL, "=", &state);
        value[strlen(value) - 1] = '\0';
        hashmap_put(session->data, key, value);
    }
    free(line);
    fclose(file);
    return id;
}

static void writeLine(const char *key, const char *value, void *file)
{
    fprintf((FILE *)file, "%s=%s\n", key, value);
}

static void save_session_to_file(session_t *session)
{
    char *filename = getFileName(session);
    FILE *file = fopen(filename, "w");
    if (file == NULL)
    {
        printf("Could not write file %s\n", filename);
        return;
    }
    hashmap_iterate(session->data, &writeLine, file);
    fclose(file);
}

session_t *session_create()
{
    session_t *session = (session_t *)malloc(sizeof(session_t));
    session->id = NULL;
    session->filename = NULL;
    session->data = hashmap_new();
    session->started = false;
    return session;
}

void session_destroy(session_t *session)
{
    remove(getFileName(session));
    session_free(session);
}

void session_start(session_t *session)
{
    if (session->started)
    {
        // Session already started
        return;
    }
    srand((unsigned int)time(NULL));
    if (session->id == NULL)
    {
        char *id = generate_session_id();
        session->id = id;
        session->filename = getFileName(session);
        session->started = 1;
    }

    load_session_from_file(session, session->id);
    session->started = 1;
}

void session_set(session_t *session, char *key, char *value)
{
    hashmap_put(session->data, key, value);
}

char *session_get(session_t *session, char *key)
{
    return hashmap_get(session->data, key);
}

void session_unset(session_t *session, char *key)
{
    hashmap_remove(session->data, key);
}

void session_free(session_t *session)
{
    if (!session) return; // If session ended by user not by end of request lifecycle
    if (session->id != NULL)
    {
        free(session->id);
    }
    if (session->filename != NULL)
    {
        free(session->filename);
    }
    hashmap_free(session->data);
    free(session);
}

void session_end(session_t *session)
{
    save_session_to_file(session);
    printf("Session ended\n");
    session_free(session);
}