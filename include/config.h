#ifndef _CONFIG_H
#define _CONFIG_H

#include <hash.h>
#include <stdlib.h>

typedef struct _config_t {
    hashmap_map *config;
} config_t;


config_t *config_new();
void config_free(config_t *c);

const char *config_get(config_t *c, const char *key, char *def);

#endif // _CONFIG_H