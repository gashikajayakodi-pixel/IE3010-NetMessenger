
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>

#define PORT 14192
#define MAX_CLIENTS 32
#define LINE_SIZE 4096
#define NAME_SIZE 32

typedef struct {
    int fd;
    int active;
    char username[NAME_SIZE];
        unsigned char room_membership[16];
} Client;

static Client clients[MAX_CLIENTS];
static char room_names[16][NAME_SIZE];
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

/* Caller holds lock, so writes to each socket cannot interleave. */
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

/* A failed partial write makes this connection unusable. */
static void deliver(Client *client, const char *text)
{
    if (send_all(client->fd, text) == -1)
        shutdown(client->fd, SHUT_RDWR);
}

static int read_line(int fd, char *line, size_t capacity)
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
            if (used > 0 && line[used - 1] == '\r')
                --used;

            line[used] = '\0';
            return 1;
        }

        if (ch == '\0')
            return -2;

        line[used++] = ch;
    }

    return -2;
}

static int valid_username(const char *name)
{
    size_t length = strlen(name);

    if (length == 0 || length >= NAME_SIZE)
        return 0;

    for (size_t i = 0; i < length; ++i) {
        unsigned char ch = (unsigned char)name[i];

        if (!isalnum(ch) && ch != '_' && ch != '-')
            return 0;
    }

    return 1;
}

/* Registry and socket writes are protected by lock. */
static void notify_others(Client *sender, const char *message)
{
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        Client *target = &clients[i];

        if (target != sender &&
            target->active &&
            target->username[0] != '\0') {
            deliver(target, message);
        }
    }
}

static void *handle_client(void *argument)
{
    Client *client = argument;
    int fd = client->fd;
    char line[LINE_SIZE];
    char response[LINE_SIZE + 128];

    for (;;) {
        int result = read_line(fd, line, sizeof(line));

        if (result != 1) {
            if (result == -2) {
                pthread_mutex_lock(&lock);
                deliver(client,
                        "ERR 006 LINE_TOO_LONG NID:9281\n");
                pthread_mutex_unlock(&lock);
            } else if (result == -1) {
                perror("recv");
            }

            break;
        }

        int quit = 0;
        pthread_mutex_lock(&lock);

        if (client->username[0] == '\0') {
            if (strncmp(line, "REGISTER ", 9) != 0 ||
                !valid_username(line + 9)) {
                deliver(client,
                        "ERR 005 INVALID_REGISTRATION NID:9281\n");
            } else {
                const char *name = line + 9;
                int duplicate = 0;

                for (int i = 0; i < MAX_CLIENTS; ++i) {
                    if (clients[i].active &&
                        strcmp(clients[i].username, name) == 0) {
                        duplicate = 1;
                        break;
                    }
                }

                if (duplicate) {
                    deliver(client,
                            "ERR 001 USERNAME_TAKEN NID:9281\n");
                } else {
                    strcpy(client->username, name);

                    snprintf(response, sizeof(response),
                             "OK REGISTERED %s NID:9281\n", name);
                    deliver(client, response);

                    snprintf(response, sizeof(response),
                             "MSG JOIN %s\n", name);
                    notify_others(client, response);

                    printf("Registered: %s\n", name);
                    fflush(stdout);
                }
            }
        } else if (strcmp(line, "LIST") == 0) {
            size_t used = (size_t)snprintf(
                response, sizeof(response), "OK USERS ");
            int first = 1;

            for (int i = 0; i < MAX_CLIENTS; ++i) {
                if (clients[i].active &&
                    clients[i].username[0] != '\0') {
                    int written = snprintf(
                        response + used, sizeof(response) - used,
                        "%s%s", first ? "" : ",",
                        clients[i].username);

                    used += (size_t)written;
                    first = 0;
                }
            }

            snprintf(response + used, sizeof(response) - used,
                     " NID:9281\n");
            deliver(client, response);
        } else if (strncmp(line, "BCAST ", 6) == 0 &&
                   line[6] != '\0') {
            snprintf(response, sizeof(response),
                     "MSG BCAST %s %s\n",
                     client->username, line + 6);

            notify_others(client, response);
            deliver(client, "OK SENT NID:9281\n");

            printf("Broadcast from %s: %s\n",
                   client->username, line + 6);
            fflush(stdout);
                } else if (strncmp(line, "PMSG ", 5) == 0) {
            char *name = line + 5;
            char *message = strchr(name, ' ');

            if (message == NULL || message[1] == '\0') {
                deliver(client,
                        "ERR 009 INVALID_MESSAGE NID:9281\n");
            } else {
                *message = '\0';
                ++message;

                Client *target = NULL;

                for (int i = 0; i < MAX_CLIENTS; ++i) {
                    if (clients[i].active &&
                        clients[i].username[0] != '\0' &&
                        strcmp(clients[i].username, name) == 0) {
                        target = &clients[i];
                        break;
                    }
                }

                if (target == NULL) {
                    deliver(client,
                            "ERR 002 USER_NOT_FOUND NID:9281\n");
                } else {
                    snprintf(response, sizeof(response),
                             "MSG PRIV %s %s\n",
                             client->username, message);

                    deliver(target, response);
                    deliver(client, "OK SENT NID:9281\n");
                }
                        }
        } else if (strncmp(line, "JOIN ", 5) == 0) {
            const char *name = line + 5;
            int room = -1;
            int empty = -1;

            for (int i = 0; i < 16; ++i) {
                if (strcmp(room_names[i], name) == 0)
                    room = i;
                if (room_names[i][0] == '\0' && empty == -1)
                    empty = i;
            }

            if (!valid_username(name)) {
                deliver(client,
                        "ERR 010 INVALID_ROOM NID:9281\n");
            } else if (room == -1 && empty == -1) {
                deliver(client,
                        "ERR 011 ROOM_LIMIT NID:9281\n");
            } else {
                if (room == -1) {
                    room = empty;
                    strcpy(room_names[room], name);
                }

                client->room_membership[room] = 1;
                snprintf(response, sizeof(response),
                         "OK JOINED %s NID:9281\n", name);
                deliver(client, response);
            }
        } else if (strncmp(line, "LEAVE ", 6) == 0) {
            const char *name = line + 6;
            int room = -1;

            for (int i = 0; i < 16; ++i) {
                if (room_names[i][0] != '\0' &&
                    strcmp(room_names[i], name) == 0) {
                    room = i;
                    break;
                }
            }

            if (room == -1 || !client->room_membership[room]) {
                deliver(client,
                        "ERR 003 ROOM_NOT_FOUND NID:9281\n");
            } else {
                client->room_membership[room] = 0;
                snprintf(response, sizeof(response),
                         "OK LEFT %s NID:9281\n", name);
                deliver(client, response);
            }
        } else if (strcmp(line, "ROOMS") == 0) {
            size_t used = (size_t)snprintf(
                response, sizeof(response), "OK ROOMS ");
            int first = 1;

            for (int i = 0; i < 16; ++i) {
                if (room_names[i][0] != '\0') {
                    int written = snprintf(
                        response + used, sizeof(response) - used,
                        "%s%s", first ? "" : ",", room_names[i]);
                    used += (size_t)written;
                    first = 0;
                }
            }

            snprintf(response + used, sizeof(response) - used,
                     " NID:9281\n");
            deliver(client, response);
        } else if (strncmp(line, "RMSG ", 5) == 0) {
            char *name = line + 5;
            char *message = strchr(name, ' ');

            if (message == NULL || message[1] == '\0') {
                deliver(client,
                        "ERR 009 INVALID_MESSAGE NID:9281\n");
            } else {
                *message = '\0';
                ++message;
                int room = -1;

                for (int i = 0; i < 16; ++i) {
                    if (room_names[i][0] != '\0' &&
                        strcmp(room_names[i], name) == 0) {
                        room = i;
                        break;
                    }
                }

                if (room == -1 ||
                    !client->room_membership[room]) {
                    deliver(client,
                            "ERR 003 ROOM_NOT_FOUND NID:9281\n");
                } else {
                    snprintf(response, sizeof(response),
                             "MSG ROOM %s %s %s\n",
                             name, client->username, message);

                    for (int i = 0; i < MAX_CLIENTS; ++i) {
                        if (&clients[i] != client &&
                            clients[i].active &&
                            clients[i].username[0] != '\0' &&
                            clients[i].room_membership[room]) {
                            deliver(&clients[i], response);
                        }
                    }

                    deliver(client, "OK SENT NID:9281\n");
                }
            }
        } else if (strcmp(line, "QUIT") == 0) {
            deliver(client, "OK BYE NID:9281\n");
            quit = 1;
        } else {
            deliver(client,
                    "ERR 007 UNKNOWN_COMMAND NID:9281\n");
        }

        pthread_mutex_unlock(&lock);

        if (quit)
            break;
    }

    pthread_mutex_lock(&lock);

    if (client->username[0] != '\0') {
        snprintf(response, sizeof(response),
                 "MSG LEAVE %s\n", client->username);

        notify_others(client, response);
        printf("Disconnected: %s\n", client->username);
        fflush(stdout);
    }

    close(fd);
    client->username[0] = '\0';
    client->active = 0;

    pthread_mutex_unlock(&lock);
    return NULL;
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
    address.sin_port = htons(PORT);
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (bind(server_fd, (struct sockaddr *)&address,
             sizeof(address)) == -1) {
        perror("bind");
        close(server_fd);
        return EXIT_FAILURE;
    }

    if (listen(server_fd, MAX_CLIENTS) == -1) {
        perror("listen");
        close(server_fd);
        return EXIT_FAILURE;
    }

    printf("Server listening on 127.0.0.1:%d\n", PORT);
    fflush(stdout);

    for (;;) {
        int fd = accept(server_fd, NULL, NULL);

        if (fd == -1) {
            if (errno == EINTR)
                continue;

            perror("accept");
            continue;
        }

        /* Bound how long a slow receiver can block a send. */
        struct timeval timeout = { .tv_sec = 2, .tv_usec = 0 };

        if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO,
                       &timeout, sizeof(timeout)) == -1) {
            perror("send timeout");
            close(fd);
            continue;
        }

        pthread_mutex_lock(&lock);
        Client *slot = NULL;

        for (int i = 0; i < MAX_CLIENTS; ++i) {
            if (!clients[i].active) {
                slot = &clients[i];
                slot->fd = fd;
                slot->username[0] = '\0';
                slot->active = 1;
                memset(slot->room_membership, 0, sizeof(slot->room_membership));
                break;
            }
        }

        pthread_mutex_unlock(&lock);

        if (slot == NULL) {
            send_all(fd, "ERR 008 SERVER_FULL NID:9281\n");
            close(fd);
            continue;
        }

        pthread_t thread;
        int error = pthread_create(
            &thread, NULL, handle_client, slot);

        if (error != 0) {
            fprintf(stderr, "pthread_create: %s\n",
                    strerror(error));

            pthread_mutex_lock(&lock);
            close(fd);
            slot->active = 0;
            pthread_mutex_unlock(&lock);
        } else {
            pthread_detach(thread);
        }
    }
}