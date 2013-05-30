#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>

typedef int socket_t;

#include "mime.h"
#include "http.h"
#include "connection.h"

#define MAX_CONNECTIONS 100


static int set_up_server_socket(int port, socket_t *sock_p) {
    struct sockaddr_in server_addr;
    socket_t sock;

    sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0) {
        perror("µfs: error in socket()");
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr))) {
        perror("µfs: error in bind()");
        close(sock);
        return -2;
    }

    if (listen(sock, 5)) {
        perror("µfs: error in listen()");
        close(sock);
        return -3;
    }

    *sock_p = sock;
    return 0;
}

static int set_socket_should_block(socket_t sock, int should_block ) {
    int flags = fcntl(sock, F_GETFL, 0);
    if (should_block) {
        flags &= ~O_NONBLOCK;
    } else {
        flags |= O_NONBLOCK;
    }
    return fcntl(sock, F_SETFL, flags);
}

static int server_run(unsigned short port) {
    socket_t serversock;
    int num_connections = MAX_CONNECTIONS;
    connection_t connections[MAX_CONNECTIONS];

    memset(connections, 0, sizeof(connections));

    if (set_up_server_socket(port, &serversock)) {
        return 1;
    }
    if (set_socket_should_block(serversock, 0)) {
        close(serversock);
        return 2;
    }
    printf("Press enter to exit...\n");
    while (1) {
        int num_readyfds;
        int highfd;
        fd_set readfds;
        fd_set writefds;
        int num_fds = 2;
        struct timeval timeout;
        timeout.tv_sec = 2;
        timeout.tv_usec = 0;
        FD_ZERO(&readfds);
        FD_ZERO(&writefds);
        FD_SET(STDIN_FILENO, &readfds);
        highfd = STDIN_FILENO;
        FD_SET(serversock, &readfds);
        highfd = (serversock > highfd) ? serversock : highfd;
        {
            int i;
            for (i = 0; i < num_connections; i++) {
                if (connections[i].is_connected) {
                    FD_SET(connections[i].sock, &readfds);
                    highfd = (connections[i].sock > highfd) ? connections[i].sock : highfd;
                    num_fds++;
                    if (connections[i].can_write) {
                        FD_SET(connections[i].sock, &writefds);
                        highfd = (connections[i].sock > highfd) ? connections[i].sock : highfd;
                        num_fds++;
                    }
                }
            }
        }
        num_readyfds = select(highfd+1, &readfds, &writefds, NULL, &timeout);
        if (num_readyfds < 0) {
            perror("µfs (select)");
            break;
        } else if (num_readyfds == 0) {
        } else if (FD_ISSET(STDIN_FILENO, &readfds)) {
            getchar();
            break;
        } else if (FD_ISSET(serversock, &readfds)) {
            struct sockaddr_in client_addr;
            socklen_t len = sizeof(struct sockaddr_in);
            socket_t sock = accept(serversock, (struct sockaddr *)&client_addr, &len);
            if (sock >= 0) {
                int i;
                if (set_socket_should_block(sock, 0)) {
                    close(sock);
                } else {
                    for (i = 0; i < num_connections && connections[i].is_connected; i++);
                    if (i < num_connections) {
                        connection_open(&connections[i], sock, client_addr);
                    } else {
                        const char *response = HTTP_CODE_503_SERVICE_UNAVAILABLE;
                        send(sock, response, strlen(response), MSG_NOSIGNAL);
                        fprintf(stderr, "µfs: no free connections\n");
                        close(sock);
                    }
                }
            }
        } else {
            int i;
            for (i = 0; i < num_connections; i++) {
                if (connections[i].is_connected) {
                    if (FD_ISSET(connections[i].sock, &readfds)) {
                        char buffer[1024];
                        int bytes_read = recv(connections[i].sock, buffer, 1024, MSG_NOSIGNAL | MSG_DONTWAIT);
                        if (bytes_read == 0 || (bytes_read == -1 && errno != EAGAIN) || (bytes_read == -1 && errno != EWOULDBLOCK)) {
                            connection_close(&connections[i]);
                        } else if (bytes_read > 0 && connections[i].can_read) {
                            connection_on_read(&connections[i], buffer, bytes_read);
                        }
                    } else if (FD_ISSET(connections[i].sock, &writefds)) {
                        if (connections[i].is_connected && connections[i].can_write) {
                            connection_on_write(&connections[i]);
                        }
                    }
                }
            }
        }
        {
            int i;
            time_t current_time = time(NULL);
            for (i = 0; i < num_connections; i++) {
                if (connections[i].is_connected) {
                    if (connections[i].timeout < current_time) {
                        connection_on_timeout(&connections[i]);
                    }
                }
            }
        }
    }
    close(serversock);
    {
        int i;
        for (i = 0; i < num_connections; i++) {
            if (connections[i].is_connected) {
                connection_close(&connections[i]);
            }
        }
    }
    return 0;
}

int main(void) {
    unsigned short port = 1234;
    server_run(port);
    return 0;
}
