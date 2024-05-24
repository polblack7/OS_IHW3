#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024


void encode_text(char *input, char *output) {
    
    for (int i = 0; input[i] != '\0'; i++) {
        output[i] = (char)((int)input[i] - 64);
    }
    output[strlen(input)] = '\0';
}

void send_encoded_fragment(char *ip_address, int port, int fragment_index, char *fragment) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char encoded_fragment[BUFFER_SIZE];

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip_address, &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return;
    }

    encode_text(fragment, encoded_fragment);

    
    send(sock, &fragment_index, sizeof(int), 0);

   
    send(sock, encoded_fragment, strlen(encoded_fragment), 0);

    close(sock);
}

int main(int argc, char const *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <IP Address> <Port> <Fragment Index> <Text Fragment>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *ip_address = argv[1];
    int port = atoi(argv[2]);
    int fragment_index = atoi(argv[3]);
    char *text_fragment = argv[4];

    send_encoded_fragment(ip_address, port, fragment_index, text_fragment);

    return 0;
}
