#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BUFFER_SIZE 1024
#define MAX_CLIENTS 100
#define MAX_MONITORS 100

typedef struct {
    int index;
    char encoded_text[BUFFER_SIZE];
} EncodedFragment;

EncodedFragment fragments[MAX_CLIENTS];
int fragment_count = 0;
int total_fragments = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

int monitor_sockets[MAX_MONITORS];
int monitor_count = 0;
int server_fd;
int server_running = 1;

void *handle_client(void *client_socket) {
    int new_socket = *((int *)client_socket);
    free(client_socket);
    char buffer[BUFFER_SIZE];
    int bytes_read;

    // Read the index of the fragment
    int fragment_index;
    read(new_socket, &fragment_index, sizeof(int));

    // Read the encoded fragment
    bytes_read = read(new_socket, buffer, BUFFER_SIZE);
    buffer[bytes_read] = '\0';

    // Store the fragment in the correct order
    pthread_mutex_lock(&mutex);
    fragments[fragment_count].index = fragment_index;
    strcpy(fragments[fragment_count].encoded_text, buffer);
    fragment_count++;

    // Notify all monitors about the received fragment
    char monitor_message[BUFFER_SIZE];
    snprintf(monitor_message, BUFFER_SIZE, "Received fragment %d: %s\n", fragment_index, buffer);
    for (int i = 0; i < monitor_count; i++) {
        send(monitor_sockets[i], monitor_message, strlen(monitor_message), 0);
    }

    // Check if all fragments have been received
    if (fragment_count >= total_fragments) {
        server_running = 0;
        pthread_cond_broadcast(&cond); // Notify all waiting threads
    }

    pthread_mutex_unlock(&mutex);
    close(new_socket);
    return NULL;
}

void *handle_monitor(void *monitor_socket) {
    int new_socket = *((int *)monitor_socket);
    free(monitor_socket);

    pthread_mutex_lock(&mutex);
    monitor_sockets[monitor_count++] = new_socket;
    pthread_mutex_unlock(&mutex);

    // Keep the connection open
    char buffer[BUFFER_SIZE];
    while (server_running) {
        int bytes_read = recv(new_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_read <= 0) {
            close(new_socket);
            pthread_mutex_lock(&mutex);
            for (int i = 0; i < monitor_count; i++) {
                if (monitor_sockets[i] == new_socket) {
                    monitor_sockets[i] = monitor_sockets[--monitor_count];
                    break;
                }
            }
            pthread_mutex_unlock(&mutex);
            break;
        }
    }

    close(new_socket);
    return NULL;
}

void start_server(char *ip_address, int port) {
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    pthread_t thread_id[MAX_CLIENTS + MAX_MONITORS];

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(ip_address);
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on %s:%d\n", ip_address, port);

    int i = 0, j = 0;
    while (server_running) {
        int new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (new_socket < 0) {
            if (!server_running) break; // Exit if server is shutting down
            perror("accept");
            continue;
        }

        int *client_socket = malloc(sizeof(int));
        *client_socket = new_socket;

        // Read the first byte to determine if it's a monitor or client
        char first_byte;
        recv(new_socket, &first_byte, 1, MSG_PEEK);
        if (first_byte == 'M') {  // Assuming monitors send 'M' initially
            pthread_create(&thread_id[MAX_CLIENTS + i++], NULL, handle_monitor, (void *)client_socket);
        } else {
            pthread_create(&thread_id[j++], NULL, handle_client, (void *)client_socket);
        }
    }

    // Wait for all client threads to finish
    for (int k = 0; k < j; k++) {
        pthread_join(thread_id[k], NULL);
    }

    // Notify and wait for all monitor threads to finish
    char completion_message[] = "All fragments received. Server is closing.\n";
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < monitor_count; i++) {
        send(monitor_sockets[i], completion_message, strlen(completion_message), 0);
        close(monitor_sockets[i]);
    }
    monitor_count = 0;
    pthread_mutex_unlock(&mutex);

    for (int k = 0; k < i; k++) {
        pthread_cancel(thread_id[MAX_CLIENTS + k]);
    }

    close(server_fd);
}

void print_encoded_text() {
    printf("Encoded Text:\n");
    for (int i = 0; i < fragment_count; i++) {
        printf("%s", fragments[i].encoded_text);
    }
    printf("\n");
}

int main(int argc, char const *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <IP Address> <Port> <Total Fragments>\n", argv[0]);
        return EXIT_FAILURE;
    }
    char *ip_address = argv[1];
    int port = atoi(argv[2]);
    total_fragments = atoi(argv[3]) - 1;

    start_server(ip_address, port);

    pthread_mutex_lock(&mutex);
    while (fragment_count < total_fragments) {
        pthread_cond_wait(&cond, &mutex);
    }
    pthread_mutex_unlock(&mutex);

    print_encoded_text();

    return 0;
}
