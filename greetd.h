#ifndef GREETD_H
#define GREETD_H

#define GREETD_ERROR_SOCKET_CREATION -1
#define GREETD_ERROR_SOCKET_CONNECTION -2
#define GREETD_ERROR_MESSAGE_SERIALIZATION -3
#define GREETD_ERROR_SOCKET_WRITE -4

struct Greetd {
    char* greetd_sock;
    int client_fd;
};

char* greetd_error_string(int error);
int greetd_init(struct Greetd* greetd, char* greetd_sock);
int greetd_create_session(struct Greetd* greetd, char* username);

#endif
