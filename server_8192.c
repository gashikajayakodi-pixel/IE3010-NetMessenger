#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 14192
#define MAX_CLIENTS 32

typedef struct {
    int fd;
    int active;
    char username[32];
} Client;

static Client clients[MAX_CLIENTS];
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

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

static int valid_username(const char *name)
{
    size_t length = strlen(name);
    return length > 0 && length <= 31 &&
        strspn(name,
            "abcdefghijklmnopqrstuvwxyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "0123456789_-") == length;
}

static void *handle_client(void *argument)
{
    Client *client = argument;
    int fd = client->fd;
    char line[256];
    char response[1200];

    for (;;) {
        int result = read_line(fd, line, sizeof(line));

        if (result != 1) {
            if (result == -2)
                send_all(fd,
                    "ERR 006 LINE_TOO_LONG NID:9281\n");
            else if (result == -1)
                perror("recv");
            break;
        }

        int quit = 0;
        pthread_mutex_lock(&lock);

        if (client->username[0] == '\0') {
            if (strncmp(line, "REGISTER ", 9) != 0 ||
                !valid_username(line + 9)) {
                snprintf(response, sizeof(response),
                    "ERR 005 INVALID_REGISTRATION NID:9281\n");
            } else {
                int duplicate = 0;

                for (int i = 0; i < MAX_CLIENTS; ++i) {
                    if (clients[i].active &&
                        strcmp(clients[i].username,
                               line + 9) == 0) {
                        duplicate = 1;
                        break;
                    }
                }

                if (duplicate) {
                    snprintf(response, sizeof(response),
                        "ERR 001 USERNAME_TAKEN NID:9281\n");
                } else {
                    strcpy(client->username, line + 9);
                    printf("Registered: %s\n", client->username);
                    snprintf(response, sizeof(response),
                        "OK REGISTERED %s NID:9281\n",
                        client->username);
                }
            }
        } else if (strcmp(line, "LIST") == 0) {
            strcpy(response, "OK USERS ");
            int first = 1;

            for (int i = 0; i < MAX_CLIENTS; ++i) {
                if (clients[i].active &&
                    clients[i].username[0] != '\0') {
                    if (!first)
                        strcat(response, ",");
                    strcat(response, clients[i].username);
                    first = 0;
                }
            }
            strcat(response, " NID:9281\n");
        } else if (strcmp(line, "QUIT") == 0) {
            strcpy(response, "OK BYE NID:9281\n");
            quit = 1;
        } else {
            strcpy(response,
                "ERR 007 UNKNOWN_COMMAND NID:9281\n");
        }

        pthread_mutex_unlock(&lock);

        if (send_all(fd, response) == -1 || quit)
            break;
    }

    pthread_mutex_lock(&lock);
    printf("Disconnected: %s\n",
           client->username[0] ? client->username : "(unregistered)");
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
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    address.sin_port = htons(PORT);

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

    for (;;) {
        int fd = accept(server_fd, NULL, NULL);
        if (fd == -1) {
            if (errno != EINTR)
                perror("accept");
            continue;
        }

        pthread_mutex_lock(&lock);
        Client *slot = NULL;

        for (int i = 0; i < MAX_CLIENTS; ++i) {
            if (!clients[i].active) {
                slot = &clients[i];
                slot->fd = fd;
                slot->active = 1;
                slot->username[0] = '\0';
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
        int error = pthread_create(&thread, NULL,
                                   handle_client, slot);
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