
#include <hash.h>


#define INITIAL_SIZE 512


hashmap_map *hashmap_new()
{
    hashmap_map *m = (hashmap_map *)malloc(sizeof(hashmap_map));
    if (!m)
    {
        return NULL;
    }
    m->table_size = 0;
    m->size = 0;
    m->data = (hashmap_element *)calloc(INITIAL_SIZE, sizeof(hashmap_element));
    if (!m->data)
    {
        free(m);
        return NULL;
    }
    m->table_size = INITIAL_SIZE;
    return m;
}

int hashmap_put(hashmap_map *m, char *key, const char *value)
{

    if (value == NULL || strlen(value) == 0)
    {
        value = " ";
    }
    if (key == NULL || strlen(key) == 0)
    {
        key = " ";
    }
    if (m->size >= m->table_size && hashmap_rehash(m) != MAP_OK)
    {
        return MAP_FULL;
    }

    int index = hashmap_hash_string(key) % m->table_size;
    // Linear probing
    while (m->data[index].in_use && strcmp(m->data[index].key, key) != 0) // Overwrite possible
    {
        printf("Collision: %s\n", key);
        index = (index + 1) % m->table_size;
    }
    hashmap_element *e = m->data + index;

    // since overwrite is possible we need to free existing values
    if (e->key) free(e->key);
    if (e->value) free(e->value);
    e->key = (char *)malloc(strlen(key) + 1);
    e->value = (char *)malloc(strlen(value) + 1);
    strcpy(e->key, key);
    strcpy(e->value, value);
    e->in_use = 1;
    m->size++;
    return MAP_OK;
}

char *hashmap_get(hashmap_map *m, const char *key)
{
    if (!m) {
        printf("Map is NULL\n");
        return NULL;
    }

    int index = hashmap_hash_string(key) % m->table_size;
    while (m->data[index].in_use)
    {
        if (strcmp(m->data[index].key, key) == 0)
        {
            return m->data[index].value;
        }
        index = (index + 1) % m->table_size;
    }

    printf("Key not found\n");
    return NULL;
}

int hashmap_remove(hashmap_map *m, const char *key)
{
    int index = hashmap_hash_string(key) % m->table_size;
    while (m->data[index].in_use)
    {
        if (strcmp(m->data[index].key, key) == 0)
        {
            m->data[index].in_use = 0;
            if (m->data[index].key)
            {
                free(m->data[index].key);
            }
            if (m->data[index].value)
            {
                free(m->data[index].value);
            }
            m->size--;
            return MAP_OK;
        }
        index = (index + 1) % m->table_size;
    }
    return MAP_MISSING;
}

void hashmap_free(hashmap_map *m) {

    if (!m) {
        return;
    }

    for (int i = 0; i < m->table_size; i++) {
        if (m->data[i].in_use) {
            if (m->data[i].key) {
                free(m->data[i].key);
            }
            if (m->data[i].value) {
                free(m->data[i].value);
            }
        }
    }
    free(m->data);
    free(m);
}

int hashmap_length(const hashmap_map *m) {
    return m->size;
}

unsigned int hashmap_hash_string(const char *str) {
    unsigned int hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

int hashmap_rehash(hashmap_map *m) {
    printf("Rehashing\n");
    int old_size = m->table_size;
    hashmap_element *old_data = m->data;

    m->table_size *= 2;
    m->size = 0;
    m->data = (hashmap_element *)calloc(m->table_size, sizeof(hashmap_element));
    if (!m->data) {
        m->data = old_data;
        m->table_size = old_size;
        return MAP_OMEM;
    }

    for (int i = 0; i < old_size; i++) {
        if (old_data[i].in_use) {
            hashmap_put(m, old_data[i].key, old_data[i].value);
        }
    }

    free(old_data);
    return MAP_OK;
}

void hashmap_iterate(const hashmap_map *m, hashmap_iter_cb cb, void *data) {
    for (int i = 0; i < m->table_size; i++) {
        if (m->data[i].in_use) {
            cb(m->data[i].key, m->data[i].value, data);
        }
    }
}