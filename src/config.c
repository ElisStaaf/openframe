#include <config.h>


// ------------------------------------------------------------
// Config
// Modify this function to add your own configuration
static void config_fill(hashmap_map *config) {

    // Add your own configuration here
    hashmap_put(config, "APPNAME", "Best framework");
    hashmap_put(config, "VERSION", "0.1");
    hashmap_put(config, "DATABASE", "openframe.sqlite");
    // End of your configuration
}
// ------------------------------------------------------------

config_t *config_new() {
    config_t *c = malloc(sizeof(config_t));
    c->config = hashmap_new();
    config_fill(c->config);
    return c;
}

void config_free(config_t *c) {
    hashmap_free(c->config);
    free(c);
}

const char *config_get(config_t *c, const char *key, char *def) {
    char *value = hashmap_get(c->config, key);
    if (value == NULL) {
        return def;
    }
    return value;
}
