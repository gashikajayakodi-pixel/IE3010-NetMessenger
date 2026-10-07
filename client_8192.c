#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <poll.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 14192
#define BUFFER_SIZE 4096

static int send_all(int fd, const char *text)
{
    size_t length = strlen(text);
    size_t sent = 0;

    while (sent < length) {
        ssize_t n = send(fd, text + sent,
                         length - sent, MSG_NOSIGNAL);

        if (n < 0) {
            if (errno == EINTR)
                continue;
            return -1;
        }

        if (n == 0)
            return -1;

        sent += (size_t)n;
    }

    return 0;
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
        fprintf(stderr, "Invalid server address\n");
        close(fd);
        return EXIT_FAILURE;
    }

    if (connect(fd, (struct sockaddr *)&server,
                sizeof(server)) == -1) {
        perror("connect");
        close(fd);
        return EXIT_FAILURE;
    }

    struct pollfd events[2] = {
        { .fd = STDIN_FILENO, .events = POLLIN },
        { .fd = fd, .events = POLLIN }
    };

    char command[BUFFER_SIZE];
    char response[BUFFER_SIZE];
    size_t command_used = 0;
    size_t response_used = 0;
    int discard_command = 0;
    int status = EXIT_SUCCESS;
    int running = 1;

    printf("Connected. Commands: REGISTER <name>, LIST, "
           "BCAST <message>, QUIT\n");
    printf("> ");
    fflush(stdout);

    while (running) {
        int ready = poll(events, 2, -1);

        if (ready == -1) {
            if (errno == EINTR)
                continue;

            perror("poll");
            status = EXIT_FAILURE;
            break;
        }

        if (events[1].revents & (POLLIN | POLLHUP | POLLERR)) {
            char incoming[1024];
            ssize_t n = recv(fd, incoming, sizeof(incoming), 0);

            if (n <= 0) {
                if (n < 0 && errno == EINTR)
                    continue;

                if (n < 0) {
                    perror("recv");
                    status = EXIT_FAILURE;
                } else {
                    printf("\nServer connection closed.\n");
                }

                break;
            }

            for (ssize_t i = 0; i < n; ++i) {
                char ch = incoming[i];

                if (ch == '\n') {
                    response[response_used] = '\0';
                    printf("\n%s\n", response);

                    if (strcmp(response,
                               "OK BYE NID:9281") == 0) {
                        running = 0;
                    } else if (events[0].fd != -1) {
                        printf("> ");
                    }

                    fflush(stdout);
                    response_used = 0;
                } else {
                    if (response_used >= sizeof(response) - 1) {
                        fprintf(stderr, "\nResponse too long\n");
                        status = EXIT_FAILURE;
                        running = 0;
                        break;
                    }

                    response[response_used++] = ch;
                }
            }
        }

        if (!running)
            break;

        if (events[0].revents & (POLLIN | POLLHUP)) {
            char ch;
            ssize_t n = read(STDIN_FILENO, &ch, 1);

            if (n < 0) {
                if (errno == EINTR)
                    continue;

                perror("stdin");
                status = EXIT_FAILURE;
                break;
            }

            if (n == 0) {
                shutdown(fd, SHUT_WR);
                events[0].fd = -1;
                continue;
            }

            if (ch == '\n') {
                if (discard_command) {
                    fprintf(stderr, "Command too long\n");
                    printf("> ");
                    fflush(stdout);
                } else if (command_used > 0) {
                    command[command_used++] = '\n';
                    command[command_used] = '\0';

                    if (send_all(fd, command) == -1) {
                        perror("send");
                        status = EXIT_FAILURE;
                        break;
                    }

                    if (strcmp(command, "QUIT\n") == 0)
                        events[0].fd = -1;
                } else {
                    printf("> ");
                    fflush(stdout);
                }

                command_used = 0;
                discard_command = 0;
            } else if (!discard_command) {
                if (command_used >= sizeof(command) - 2) {
                    discard_command = 1;
                } else {
                    command[command_used++] = ch;
                }
            }
        }
    }

    close(fd);
    return status;
}