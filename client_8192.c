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
        ssize_t n = send(fd, text + sent,
                         length - sent, MSG_NOSIGNAL);
        if (n < 0 && errno == EINTR)
            continue;
        if (n <= 0)
            return -1;
        sent += (size_t)n;
    }
    return 0;
}

static int read_line(int fd, char *buffer, size_t capacity)
{
    size_t used = 0;

    while (used < capacity - 1) {
        char ch;
        ssize_t n = recv(fd, &ch, 1, 0);

        if (n == 0)
            return 0;
        if (n < 0) {
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

int main(void)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        perror("socket");
        return EXIT_FAILURE;
    }

    struct sockaddr_in server = {0};
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1",
                  &server.sin_addr) != 1) {
        fprintf(stderr, "Invalid address\n");
        close(fd);
        return EXIT_FAILURE;
    }

    if (connect(fd, (struct sockaddr *)&server,
                sizeof(server)) == -1) {
        perror("connect");
        close(fd);
        return EXIT_FAILURE;
    }

    printf("Connected. Commands: REGISTER <name>, LIST, QUIT\n");

    char command[256];
    char response[1200];
    int status = EXIT_SUCCESS;

    for (;;) {
        printf("> ");
        fflush(stdout);

        if (fgets(command, sizeof(command), stdin) == NULL)
            break;

        if (strchr(command, '\n') == NULL) {
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF) {
            }
            fprintf(stderr, "Command too long\n");
            continue;
        }

        if (send_all(fd, command) == -1) {
            perror("send");
            status = EXIT_FAILURE;
            break;
        }

        int result = read_line(fd, response, sizeof(response));
        if (result != 1) {
            fprintf(stderr, "Connection closed or response error\n");
            status = EXIT_FAILURE;
            break;
        }

        printf("%s\n", response);

        if (strcmp(response, "OK BYE NID:9281") == 0)
            break;
    }

    close(fd);
    return status;
}