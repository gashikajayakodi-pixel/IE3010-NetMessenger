#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT 14192

int main(void)
{
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        perror("socket");
        return EXIT_FAILURE;
    }

    struct sockaddr_in server = {0};
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1",
                  &server.sin_addr) != 1) {
        fprintf(stderr, "Invalid server address\n");
        close(socket_fd);
        return EXIT_FAILURE;
    }

    if (connect(socket_fd, (struct sockaddr *)&server,
                sizeof(server)) == -1) {
        perror("connect");
        close(socket_fd);
        return EXIT_FAILURE;
    }

    printf("Connected to server on port %d\n", PORT);
    close(socket_fd);
    return EXIT_SUCCESS;
}