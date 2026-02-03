#ifndef GREETD_H
#define GREETD_H

#include "json/json.h"

#define GREETD_ERROR_SOCKET_CREATION -1
#define GREETD_ERROR_SOCKET_CONNECTION -2
#define GREETD_ERROR_SOCKET_CLOSE -3

#define greetd_send(greetd, command) _Generic((command), \
    struct Greetd_CreateSession*: _greetd_create_session, \
    struct Greetd_StartSession*: _greetd_start_session, \
    struct Greetd_PostAuthMessageResponse*: _greetd_post_auth_message_response, \
    struct Greetd_CancelSession*: _greetd_cancel_session \
)(greetd, command)

struct Greetd {
    char* greetd_sock;
    int client_fd;
};

// ====== Commands ======

struct Greetd_CreateSession {
    char* username;
};

struct Greetd_StartSession {
    int cmd_count;
    char** cmd;
    int env_count;
    char** env;
};

struct Greetd_CancelSession {
};

struct Greetd_PostAuthMessageResponse {
    char* response;
};

// ====== Enums ======

enum Greetd_MessageType {
    GREETD_MESSAGE_TYPE_SUCCESS,
    GREETD_MESSAGE_TYPE_ERROR,
    GREETD_MESSAGE_TYPE_AUTH_MESSAGE
};

enum Greetd_ErrorType {
    GREETD_ERROR_TYPE_ERROR,
    GREETD_ERROR_TYPE_AUTH_ERROR,
    
    // This kind of error comes from client code, not from greetd itself.
    GREETD_ERROR_TYPE_CLIENT_ERROR,
    GREETD_ERROR_TYPE_UNKNOWN,
};

enum Greetd_AuthenticationMessageType {
    GREETD_AUTHENTICATION_MESSAGE_TYPE_SECRET,
    GREETD_AUTHENTICATION_MESSAGE_TYPE_VISIBLE,
    GREETD_AUTHENTICATION_MESSAGE_TYPE_INFO,
    GREETD_AUTHENTICATION_MESSAGE_TYPE_ERROR,

    // This kind of message type comes from client code, not from greetd itself.
    GREETD_AUTHENTICATION_MESSAGE_TYPE_UNKNOWN,
};

// ====== Response model ======

struct Greetd_Result_Error {
    enum Greetd_ErrorType error_type;
    char* description;
};

struct Greetd_Result_AuthMessage {
    enum Greetd_AuthenticationMessageType auth_message_type;
    char* auth_message;
};

struct Greetd_Response {
    enum Greetd_MessageType message_type;
    union {
        struct Greetd_Result_Error error;
        struct Greetd_Result_AuthMessage auth_message;
    } as;
};

// ====== Functions ======

int greetd_init(struct Greetd* greetd, char* greetd_sock);
int greetd_close(struct Greetd* greetd);

struct Greetd_Response _greetd_create_session(struct Greetd* greetd, struct Greetd_CreateSession* command);
struct Greetd_Response _greetd_cancel_session(struct Greetd* greetd, struct Greetd_CancelSession* command);
struct Greetd_Response _greetd_start_session(struct Greetd* greetd, struct Greetd_StartSession* command);
struct Greetd_Response _greetd_post_auth_message_response(struct Greetd* greetd, struct Greetd_PostAuthMessageResponse* command);
struct Greetd_Response _greetd_get_response(struct Greetd* greetd, struct JSONNode* out);

char* greetd_error_string(int error);
void greetd_response_free(struct Greetd_Response* response);

#endif
