#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BUFFER_SIZE 1024
#define MAX_CLIENTS 100

typedef struct {
    int index;
    char encoded_text[BUFFER_SIZE];
} EncodedFragment;

EncodedFragment fragments[MAX_CLIENTS];
int fragment_count = 0;
int total_fragments = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

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
    pthread_cond_signal(&cond); // Signal the condition variable
    pthread_mutex_unlock(&mutex);

    printf("Received fragment %d: %s\n", fragment_index, buffer); // Debug output

    close(new_socket);
    return NULL;
}

void start_server(char *ip_address, int port) {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    pthread_t thread_id[MAX_CLIENTS];

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

    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on %s:%d\n", ip_address, port);

    int i = 0;
    while ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) >= 0) {
        int *client_socket = malloc(sizeof(int));
        *client_socket = new_socket;
        pthread_create(&thread_id[i++], NULL, handle_client, (void *)client_socket);
        if (i >= total_fragments) {
            break;
        }
    }

    for (int j = 0; j < i; j++) {
        pthread_join(thread_id[j], NULL);
    }
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
    total_fragments = atoi(argv[3]);

    start_server(ip_address, port);

    pthread_mutex_lock(&mutex);
    while (fragment_count < total_fragments) {
        pthread_cond_wait(&cond, &mutex);
    }
    pthread_mutex_unlock(&mutex);

    print_encoded_text();

    return 0;
}
