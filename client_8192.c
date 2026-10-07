#include <sys/stat.h>
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

/*
 * Return 1 if sent, 0 for a local input/file error,
 * and -1 for a broken connection.
 */
static int send_file_command(int fd, const char *command)
{
    char target[32];
    char filename[101];
    char extra;

    if (sscanf(command + 9, "%31s %100s %c",
               target, filename, &extra) != 2) {
        fprintf(stderr,
                "Usage: SENDFILE <target> <filename>\n");
        return 0;
    }

    if (strchr(filename, '/') != NULL ||
        strchr(filename, '\\') != NULL) {
        fprintf(stderr,
                "Use a filename in the current directory.\n");
        return 0;
    }

    FILE *file = fopen(filename, "rb");

    if (file == NULL) {
        perror("open file");
        return 0;
    }

    struct stat information;

    if (fstat(fileno(file), &information) == -1) {
        perror("file size");
        fclose(file);
        return 0;
    }

    if (!S_ISREG(information.st_mode) ||
        information.st_size < 0 ||
        information.st_size > 1024 * 1024) {
        fprintf(stderr,
                "Choose a regular file no larger than 1 MiB.\n");
        fclose(file);
        return 0;
    }

    size_t size = (size_t)information.st_size;
    unsigned char *data = malloc(size == 0 ? 1 : size);

    if (data == NULL) {
        perror("malloc");
        fclose(file);
        return 0;
    }

    /* Read before sending the header to avoid an incomplete upload. */
    if (fread(data, 1, size, file) != size) {
        fprintf(stderr, "Could not read the complete file.\n");
        free(data);
        fclose(file);
        return 0;
    }

    fclose(file);

    char header[256];
    snprintf(header, sizeof(header),
             "SENDFILE %s %s %zu\n", target, filename, size);

    if (send_all(fd, header) == -1) {
        free(data);
        return -1;
    }

    size_t sent = 0;

    while (sent < size) {
        ssize_t n = send(fd, data + sent,
                         size - sent, MSG_NOSIGNAL);

        if (n < 0 && errno == EINTR)
            continue;

        if (n <= 0) {
            free(data);
            return -1;
        }

        sent += (size_t)n;
    }

    free(data);
    printf("Uploaded %s (%zu bytes).\n", filename, size);
    return 1;
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
    size_t response_used = 0;    FILE *incoming_file = NULL;
    unsigned long long incoming_remaining = 0;
    char incoming_path[256] = {0};
    int incoming_write_failed = 0;
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
                if (incoming_remaining > 0) {
                    if (incoming_file != NULL &&
                        !incoming_write_failed) {
                        if (fputc((unsigned char)ch,
                                  incoming_file) == EOF)
                            incoming_write_failed = 1;
                    }

                    --incoming_remaining;

                    if (incoming_remaining == 0) {
                        if (incoming_file != NULL) {
                            if (fclose(incoming_file) == EOF)
                                incoming_write_failed = 1;
                            incoming_file = NULL;
                        }

                        if (incoming_write_failed) {
                            fprintf(stderr,
                                    "\nCould not save received file.\n");
                            if (incoming_path[0] != '\0')
                                unlink(incoming_path);
                        } else {
                            printf("\nReceived file saved: %s\n",
                                   incoming_path);
                        }

                        printf("> ");
                        fflush(stdout);
                    }

                    continue;
                }

                if (ch == '\n') {
                    response[response_used] = '\0';
                    printf("\n%s\n", response);
                    if (strncmp(response, "MSG FILE ", 9) == 0) {
                        char sender[32];
                        char filename[101];
                        unsigned long long size;
                        char extra;
                        const char *safe_chars =
                            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                            "abcdefghijklmnopqrstuvwxyz"
                            "0123456789_.-";

                        int fields = sscanf(
                            response + 9, "%31s %100s %llu %c",
                            sender, filename, &size, &extra);

                        if (fields != 3 ||
                            size > 1024ULL * 1024ULL ||
                            strspn(sender, safe_chars) !=
                                strlen(sender) ||
                            strspn(filename, safe_chars) !=
                                strlen(filename)) {
                            fprintf(stderr,
                                    "Invalid incoming file header.\n");
                            status = EXIT_FAILURE;
                            running = 0;
                            break;
                        }

                        incoming_remaining = size;
                        incoming_write_failed = 0;
                        incoming_path[0] = '\0';
                        incoming_file = NULL;

                        if (mkdir("./received", 0700) == -1 &&
                            errno != EEXIST) {
                            incoming_write_failed = 1;
                        } else {
                            snprintf(incoming_path,
                                     sizeof(incoming_path),
                                     "./received/%s_%s_XXXXXX",
                                     sender, filename);

                            int file_fd = mkstemp(incoming_path);

                            if (file_fd == -1) {
                                incoming_write_failed = 1;
                                incoming_path[0] = '\0';
                            } else {
                                incoming_file = fdopen(file_fd, "wb");

                                if (incoming_file == NULL) {
                                    close(file_fd);
                                    unlink(incoming_path);
                                    incoming_path[0] = '\0';
                                    incoming_write_failed = 1;
                                }
                            }
                        }

                        if (size == 0) {
                            if (incoming_file != NULL) {
                                if (fclose(incoming_file) == EOF)
                                    incoming_write_failed = 1;
                                incoming_file = NULL;
                            }

                            if (incoming_write_failed) {
                                fprintf(stderr,
                                        "Could not save empty file.\n");
                                if (incoming_path[0] != '\0')
                                    unlink(incoming_path);
                            } else {
                                printf("Received file saved: %s\n",
                                       incoming_path);
                            }
                        }
                    }

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

                                        int send_result;

                    if (strncmp(command, "SENDFILE ", 9) == 0) {
                        send_result = send_file_command(fd, command);

                        if (send_result == 0) {
                            printf("> ");
                            fflush(stdout);
                        }
                    } else {
                        send_result = send_all(fd, command);
                    }

                    if (send_result == -1) {
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
    if (incoming_remaining > 0) {
        fprintf(stderr, "File transfer interrupted.\n");

        if (incoming_file != NULL) {
            fclose(incoming_file);
            incoming_file = NULL;
        }

        if (incoming_path[0] != '\0')
            unlink(incoming_path);

        status = EXIT_FAILURE;
    }

    close(fd);
    return status;
}