#ifndef CONNECTION_H
#define CONNECTION_H

struct connection_s {
    socket_t sock;
    struct sockaddr_in addr;
    int is_connected;
    time_t timeout;
    int can_write;
    int can_read;

    char buffer[1024];
    int buffer_length;
    int had_first_header_line;
    char url[1024];
    FILE *fp;
    off_t file_size;
    off_t file_offset;
};

typedef struct connection_s connection_t;

void connection_open(connection_t *self, socket_t sock, struct sockaddr_in addr);
void connection_on_read(connection_t *self, char *buffer, int buffer_length);
void connection_on_write(connection_t *self);
void connection_on_timeout(connection_t *self);
void connection_close(connection_t *self);

#endif
