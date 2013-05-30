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

static const char *connection_handle_header_line(connection_t *self, char *header_line) {
    if (!self->had_first_header_line) {
        char *end_of_method = strchr(header_line, ' ');
        if (end_of_method) {
            char *url = end_of_method+1;
            char *end_of_url = strchr(url, ' ');
            end_of_method[0] = 0;
            if (end_of_url) {
                end_of_url[0] = 0;
                strcpy(self->url, url);
                self->had_first_header_line = 1;
                if (!strcmp("GET",header_line)) {
                    if (url[0] == '/' && !strstr(url, "..")) {
                        struct stat st;
                        char filename[2048];
                        strcpy(filename, "./files");
                        strcat(filename, url);
                        if (!stat(filename, &st)) {
                            if (S_ISREG(st.st_mode)) {
                                self->file_size = st.st_size;
                                self->fp = fopen(filename, "rb");
                                if (!self->fp) {
                                    /* failed to open file */
                                    return HTTP_CODE_403_FORBIDDEN;
                                } else {
                                    self->can_write = 1;
                                    fprintf(stderr, "µfs: %s -> %s\n", filename, inet_ntoa(self->addr.sin_addr));
                                    return NULL;
                                }
                            } else {
                                /* not a regular file */
                                return HTTP_CODE_404_NOT_FOUND;
                            }
                        } else {
                            /* not a file or directory, stat failed */
                            return HTTP_CODE_404_NOT_FOUND;
                        }
                    } else {
                        /* url contained ".." or missed the leading slash */
                        return HTTP_CODE_403_FORBIDDEN;
                    }
                } else {
                    /* POST, HEAD, etc. */
                    return HTTP_CODE_405_METHOD_NOT_ALLOWED;
                }
            } else {
                /* url does not end, the space before "HTTP/1.1" was not found*/
                return HTTP_CODE_400_BAD_REQUEST;
            }
        } else {
            /* method does not end, the space before url was not found*/
            return HTTP_CODE_400_BAD_REQUEST;
        }
    } else {
        const char *range_start = "Range: bytes=";
        if (!strncmp(header_line, range_start,strlen(range_start))) {
            long offset = 0;
            header_line = header_line + strlen(range_start);
            while (header_line[0] >= '0' && header_line[0] <= '9') {
                offset*=10;
                offset+=header_line[0]-'0';
                header_line++;
            }
            fseek(self->fp, offset, SEEK_SET);
            self->file_offset = offset;
        }
        return NULL;
    }
}

void connection_open(connection_t *self, socket_t sock, struct sockaddr_in addr) {
    self->is_connected = 1;
    self->addr = addr;
    self->sock = sock;
    self->buffer[0] = 0;
    self->buffer_length = 0;
    /* 15 seconds timeout until end of header */
    self->timeout = time(NULL)+15;
    self->had_first_header_line = 0;
    self->fp = NULL;
    self->file_size = 0;
    self->file_offset = 0;
    self->can_read = 1;
    self->can_write = 0;
}

void connection_close(connection_t *self) {
    if (self->is_connected) {
        close(self->sock);
        self->is_connected = 0;
        if (self->fp) {
            fclose(self->fp);
            self->fp = NULL;
        }
    }
}

void connection_on_write(connection_t *self) {
    if (self->is_connected && self->fp) {
        int bytes_sent = send(self->sock, self->buffer, self->buffer_length, MSG_NOSIGNAL);
        if (bytes_sent > 0) {
            self->timeout = time(NULL)+2;
            if (bytes_sent < self->buffer_length) {
                memmove(self->buffer, self->buffer+bytes_sent, self->buffer_length+1-bytes_sent);
                self->buffer_length -= bytes_sent;
            } else if (!feof(self->fp)) {
                self->buffer_length = fread(self->buffer, 1, 1024, self->fp);
            } else {
                self->buffer_length = 0;
            }
        }
        if ((feof(self->fp) && self->buffer_length == 0) || ferror(self->fp)) {
            connection_close(self);
        }
    }
}

void connection_on_read(connection_t *self, char *buffer, int buffer_length) {
    while (buffer_length > 0 && self->can_read && self->is_connected) {
        char *end_of_header;
        int can_read = 1023-self->buffer_length;
        if (buffer_length < can_read) {
            can_read = buffer_length;
        }
        memcpy(self->buffer+self->buffer_length, buffer, can_read);
        self->buffer_length += can_read;
        buffer += can_read;
        buffer_length -= can_read;
        self->buffer[self->buffer_length] = 0;
        end_of_header = strstr(self->buffer, "\r\n\r\n");
        if (!end_of_header) {
            if (self->buffer_length == 1023) {
                const char *response = HTTP_CODE_431_REQUEST_HEADER_FIELDS_TOO_LARGE;
                send(self->sock, response, strlen(response), MSG_NOSIGNAL);
                connection_close(self);
            } else {
                /* Header not finished, but neither timed out nor reached maximum length, so we'll just wait */
            }
        } else {
            char *header_line = self->buffer;
            end_of_header[2] = 0;
            while (header_line && *header_line && self->is_connected) {
                const char *response;
                char *next_header_line = strstr(header_line, "\r\n");
                if (next_header_line) {
                    next_header_line[0] = 0;
                    next_header_line+=2;
                }
                response = connection_handle_header_line(self, header_line);
                if (response) {
                    send(self->sock, response, strlen(response), MSG_NOSIGNAL);
                    connection_close(self);
                }
                header_line = next_header_line;
            }
            if (self->is_connected) {
                self->can_read = 0;
                if (!self->file_offset) {
                    sprintf(self->buffer, "HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent-Length: %ld\r\nConnection: close\r\n\r\n", get_mime_type_for_file_name(self->url), self->file_size);
                } else {
                    sprintf(self->buffer, "HTTP/1.1 206 Partial Content\r\nContent-Type: %s\r\nContent-Length: %ld\r\nContent-Range: %ld-%ld/%ld\r\nConnection: close\r\n\r\n", get_mime_type_for_file_name(self->url), self->file_size-self->file_offset, self->file_offset, self->file_size-1, self->file_size);
                }
                self->buffer_length = strlen(self->buffer);
                self->timeout = time(NULL)+5;
            }
        }
    }
}

void connection_on_timeout(connection_t *self) {
    const char *response = HTTP_CODE_408_REQUEST_TIMEOUT;
    send(self->sock, response, strlen(response), MSG_NOSIGNAL);
    connection_close(self);
}
