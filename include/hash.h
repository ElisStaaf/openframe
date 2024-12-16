#ifndef _HASH_H
#define _HASH_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <utils.h>

#define MAP_MISSING -3
#define MAP_FULL -2  
#define MAP_OMEM -1 
#define MAP_OK 0   

// callback function for iterating over hashmap
typedef void (*hashmap_iter_cb)(const char *key, const char *value, void *data);

typedef struct _hashmap_element {
    char *key;
    char *value;
    int in_use;
} hashmap_element;

typedef struct _hashmap_map {
    int table_size;
    int size;
    hashmap_element *data;
} hashmap_map;

// return empty hashmap. Returns NULL if empty
hashmap_map *hashmap_new();

// add an element to the hashmap. Return MAP_0K or MAP_OMEM
int hashmap_put(hashmap_map *in, char *key, const char *value);

// get an element from the hashmap. Return MAP_0K or MAP_MISSING
char *hashmap_get(hashmap_map *in, const char *key);

// remove an element from the hashmap. Return MAP_0K or MAP_MISSING
int hashmap_remove(hashmap_map *in, const char *key);

// free the hashmap
void hashmap_free(hashmap_map *in);

// get the current size of a hashmap
int hashmap_length(const hashmap_map *in);

unsigned int hashmap_hash_string(const char *str);

int hashmap_rehash(hashmap_map *in);

void hashmap_iterate(const hashmap_map *in, hashmap_iter_cb cb, void *data);
#endif // _HASH_H
