#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT 14192

static int send_all(int fd, const char *text)
{
    size_t length = strlen(text);
    size_t sent = 0;

    while (sent < length) {
        ssize_t count = send(fd, text + sent,
                             length - sent, MSG_NOSIGNAL);

        if (count < 0 && errno == EINTR)
            continue;

        if (count <= 0)
            return -1;

        sent += (size_t)count;
    }

    return 0;
}

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

    const char *command = "REGISTER student\n";
    printf("Request: %s", command);

    if (send_all(socket_fd, command) == -1) {
        perror("send");
        close(socket_fd);
        return EXIT_FAILURE;
    }

    char response[256];
    size_t used = 0;

    while (used < sizeof(response) - 1) {
        char ch;
        ssize_t count = recv(socket_fd, &ch, 1, 0);

        if (count < 0 && errno == EINTR)
            continue;

        if (count <= 0) {
            if (count < 0)
                perror("recv");
            else
                fprintf(stderr, "Server closed before full response\n");

            close(socket_fd);
            return EXIT_FAILURE;
        }

        if (ch == '\n') {
            response[used] = '\0';
            printf("Response: %s\n", response);
            close(socket_fd);
            return EXIT_SUCCESS;
        }

        response[used++] = ch;
    }

    fprintf(stderr, "Response too long\n");
    close(socket_fd);
    return EXIT_FAILURE;
}