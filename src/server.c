#include <server.h>

server_t *global_server = NULL;

void cleanup_server(void)
{
    printf("Cleaning up server resources...\n");
    close(global_server->server_fd);
    printf("Server socket closed.\n");
    router_free(global_server->router);
    printf("Router resources freed.\n");
    pthread_key_delete(global_server->frame_key);
    printf("Thread key deleted.\n");
    pthread_mutex_destroy(&global_server->lock);
    printf("Mutex destroyed.\n");
    free(global_server);
    printf("Server resources freed.\n");
    exit(1);
}

void sigint_handler(int sig_num)
{
    (void)sig_num;                         // Unused parameter warning
    global_server->shutdown_requested = 1; // Set the flag to indicate shutdown is requested
    printf("\nReceived SIGINT, shutting down...\n");
    cleanup_server();
}

void init_server(int port, router_t *router)
{
    global_server = (server_t *)malloc(sizeof(server_t));
    global_server->port = port;
    global_server->shutdown_requested = 0;
    global_server->router = router;
    pthread_key_create(&global_server->frame_key, NULL);
    pthread_mutex_init(&global_server->lock, NULL);
}

int start_server()
{
    struct sockaddr_in server;
    struct sockaddr_in client;
    socklen_t c = sizeof(struct sockaddr_in);
    pthread_t thread_id;
    int *new_sock = NULL;

    // Signal handling setup
    signal(SIGINT, sigint_handler);

    global_server->server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (global_server->server_fd == -1)
    {
        perror("Could not create socket");
        return 1;
    }

    // Set the SO_REUSEADDR socket option
    int optval = 1;
    if (setsockopt(global_server->server_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0)
    {
        perror("setsockopt(SO_REUSEADDR) failed");
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons((uint16_t)global_server->port);

    if (bind(global_server->server_fd, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("bind failed. Error");
        return 1;
    }

    listen(global_server->server_fd, 3);
    printf("Ready! \n");

    while (!global_server->shutdown_requested && (new_sock = malloc(sizeof(int))) &&
           (*new_sock = accept(global_server->server_fd, (struct sockaddr *)&client, &c), *new_sock > 0))
    {

        if (pthread_create(&thread_id, NULL, handle_client, (void *)new_sock) < 0)
        {
            perror("could not create thread");
            free(new_sock); // Ensure memory allocated for new_sock is freed on error
            continue;       // Continue accepting new connections until shutdown_requested
        }
        // Detach the thread to allow for independent operation
        pthread_detach(thread_id);
    }
    if (new_sock == NULL) perror("accept failed");

    if (new_sock) free(new_sock); // Ensure memory allocated for new_sock is freed
    cleanup_server(); // Cleanup server resources
    return 0;
}

void *handle_client(void *client_socket)
{
    int sock = *((int *)client_socket);
    free(client_socket); // Early free to avoid forgetting later
    process_client_request(sock);
    return NULL;
}

void process_client_request(int sock)
{
    char buffer[2048];
    ssize_t read_size = recv(sock, buffer, sizeof(buffer) - 1, 0);
    if (read_size < 0)
    {
        perror("recv failed");
        close(sock);
        return;
    }
    buffer[read_size] = '\0';

    openframe_start();
    openframe_handle(sock, buffer);
    openframe_terminate();
}
