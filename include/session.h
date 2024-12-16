#ifndef _SESSION_H
#define _SESSION_H

#include <hash.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

typedef struct session {
    char *id;
    char *filename;
    bool started;

    hashmap_map *data;
} session_t;

// Allocation functions
session_t *session_create();
// deallocate session
void session_free(session_t *session);

// Session functions

/*
* Discard session array changes and finish session
*/
void session_abort(session_t *session);
/*
* Destroys session
*/
void session_destroy(session_t *session);
/*
* Starts new or resumes existing session
*/
void session_start(session_t *session);
/**
 * Set session data
 */
void session_set(session_t *session, char *key, char *value);
/**
 * Get session data
 */
char *session_get(session_t *session, char *key);
/**
 * Unset session data
 */
void session_unset(session_t *session, char *key);
/**
 * End session
 */
void session_end(session_t *session);




#endif // _SESSION_H