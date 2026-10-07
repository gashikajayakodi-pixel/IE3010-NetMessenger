#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 14192

static int read_line(int fd, char *buffer, size_t capacity)
{
    size_t used = 0;

    while (used < capacity - 1) {
        char ch;
        ssize_t count = recv(fd, &ch, 1, 0);

        if (count == 0)
            return 0;

        if (count < 0) {
            if (errno == EINTR)
                continue;
            return -1;
        }

        if (ch == '\n') {
            buffer[used] = '\0';
            return 1;
        }

        buffer[used++] = ch;
    }

    return -2;
}

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
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        return EXIT_FAILURE;
    }

    int reuse = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR,
                   &reuse, sizeof(reuse)) == -1) {
        perror("setsockopt");
        close(server_fd);
        return EXIT_FAILURE;
    }

    struct sockaddr_in address = {0};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address,
             sizeof(address)) == -1) {
        perror("bind");
        close(server_fd);
        return EXIT_FAILURE;
    }

    if (listen(server_fd, 5) == -1) {
        perror("listen");
        close(server_fd);
        return EXIT_FAILURE;
    }

    printf("Server listening on 127.0.0.1:%d\n", PORT);

    for (;;) {
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd == -1) {
            perror("accept");
            continue;
        }

        char line[256];
        char response[256];
        int result = read_line(client_fd, line, sizeof(line));

        if (result == 1) {
            printf("Request: %s\n", line);

            if (strncmp(line, "REGISTER ", 9) == 0 &&
                line[9] != '\0' &&
                strlen(line + 9) <= 31 &&
                strspn(line + 9,
                    "abcdefghijklmnopqrstuvwxyz"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "0123456789_-") == strlen(line + 9)) {
                snprintf(response, sizeof(response),
                         "OK REGISTERED %s NID:9281\n",
                         line + 9);
            } else {
                snprintf(response, sizeof(response),
                         "ERR 005 INVALID_REGISTRATION NID:9281\n");
            }

            if (send_all(client_fd, response) == -1)
                perror("send");
        } else if (result == -2) {
            if (send_all(client_fd,
                "ERR 006 LINE_TOO_LONG NID:9281\n") == -1)
                perror("send");
        } else if (result == -1) {
            perror("recv");
        }

        close(client_fd);
    }
}