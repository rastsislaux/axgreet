#include "hexutil.h"

#define DEBUG_LOG_ENABLED 1
#define TRACE_LOG_ENABLED 1

#include "greetd.h"
#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include "json/json.h"

int create_socket() {
    int client_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_fd == -1) {
        fprintf(stderr, "Failed to create socket\n");
        return -1;
    }
    return client_fd;
}

int connect_to_socket(int client_fd,char* greetd_sock) {
    struct sockaddr_un addr = { 0 };
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, greetd_sock, sizeof(addr.sun_path) - 1);

    socklen_t addr_len = sizeof(addr.sun_path) + sizeof(addr.sun_family);
    if (connect(client_fd, (struct sockaddr*)&addr, addr_len) == -1) {
        return -1;
    }
    return client_fd;
}

int greetd_create_session_message(char* buffer) {
    char* message_type = "create_session";
    char* username = "rastsislau";

    struct JSONNode message_type_node = JSON_String(message_type);
    struct JSONNode username_node = JSON_String(username);

    struct JSONObjectPair pairs[] = {
        { "type", &message_type_node },
        { "username", &username_node },
    };
    struct JSONNode object = JSON_Object(pairs, 2);

    char serialized[255];
    int ok = JSONNode_serialize(serialized, sizeof(serialized), &object);

    if (ok == -1) {
        fprintf(stderr, "Failed to serialize object\n");
        return -1;
    }

    int length = strlen(serialized);

    memcpy(buffer, &length, 4);
    memcpy(buffer + 4, serialized, length);
    return length + 4;
}

char* greetd_error_string(int error) {
    switch (error) {
        case GREETD_ERROR_SOCKET_CREATION:
            return "Failed to create socket";
        case GREETD_ERROR_SOCKET_CONNECTION:
            return "Failed to connect to socket";
        case GREETD_ERROR_MESSAGE_SERIALIZATION:
            return "Failed to serialize message";
        case GREETD_ERROR_SOCKET_WRITE:
            return "Failed to write to socket";
        default:
            return "Unknown error";
    }
    return NULL;
}

int greetd_init(struct Greetd* greetd, char* greetd_sock) {
    greetd->greetd_sock = greetd_sock;
    greetd->client_fd = create_socket();
    if (greetd->client_fd == -1) {
        return GREETD_ERROR_SOCKET_CREATION;
    }

    int ok = connect_to_socket(
        greetd->client_fd,
        greetd->greetd_sock
    );
    if (ok == -1) {
        return GREETD_ERROR_SOCKET_CONNECTION;
    }
    return 0;
}

int _greetd_write(struct Greetd* greetd, char* buffer) {
    int length = strlen(buffer);
    if (write(greetd->client_fd, &length, 4) == -1) {
        return GREETD_ERROR_SOCKET_WRITE;
    }
    if (write(greetd->client_fd, buffer, length) == -1) {
        return GREETD_ERROR_SOCKET_WRITE;
    }
    return 0;
}

int _greetd_read(struct Greetd* greetd, char* buffer) {
    int response_length = 0;
    if (read(greetd->client_fd, &response_length, 4) == -1) {
        return GREETD_ERROR_SOCKET_READ;
    }
    if (read(greetd->client_fd, buffer, response_length) == -1) {
        return GREETD_ERROR_SOCKET_READ;
    }
    return 0;
}

int greetd_create_session(struct Greetd* greetd, char* username) {
    struct JSONNode message_type_node = JSON_String("create_session");
    struct JSONNode username_node = JSON_String(username);
    struct JSONNode object = JSON_Object((struct JSONObjectPair[]){
        { "type", &message_type_node },
        { "username", &username_node },
    }, 2);

    char buffer[255];
    int ok = JSONNode_serialize(buffer, sizeof(buffer), &object);
    if (ok == -1) {
        return GREETD_ERROR_MESSAGE_SERIALIZATION;
    }

    if (_greetd_write(greetd, buffer) != 0) {
        return GREETD_ERROR_SOCKET_WRITE;
    }

    if (_greetd_read(greetd, buffer) != 0) {
        return GREETD_ERROR_SOCKET_READ;
    }

    JSONNode_deserialize(buffer);
    if (ok == -1) {
        return GREETD_ERROR_MESSAGE_DESERIALIZATION;
    }

    return 0;
}