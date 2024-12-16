#ifndef _SERVER_H
#define _SERVER_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <http.h>
#include <routing.h>
#include <signal.h>
#include <parser.h>
#include <openframe.h>

typedef struct
{
    volatile sig_atomic_t shutdown_requested;
    int server_fd;
    int port;
    router_t *router;
    pthread_key_t frame_key;
    pthread_mutex_t lock;
} server_t;

extern server_t *global_server;

void cleanup_server(void);
void sigint_handler(int sig_num);
void *handle_client(void *client_socket);
void process_client_request(int sock);
void init_server(int port, router_t *router);
int start_server();
#endif // _SERVER_H
