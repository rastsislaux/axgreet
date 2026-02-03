#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>

#include "greetd.h"

#define RESPONSE_BUFFER_SIZE 255
#define JSON_ARRAY_BUFFER_SIZE 1024

// ==== Private function declarations ======

int __greetd_write(struct Greetd* greetd, char* buffer);
int __greetd_read(struct Greetd* greetd, char* buffer);
struct Greetd_Response __greetd_get_response(struct Greetd* greetd);

// ====== Public functions implementations ======

char* greetd_error_string(int error) {
    switch (error) {
        case GREETD_ERROR_SOCKET_CREATION:
            return "Failed to create socket";
        case GREETD_ERROR_SOCKET_CONNECTION:
            return "Failed to connect to socket";
        case GREETD_ERROR_SOCKET_CLOSE:
            return "Failed to close socket";
        default:
            return "Unknown error";
    }
}

void greetd_response_free(struct Greetd_Response* response) {
    if (response == NULL) return;
    
    switch (response->message_type) {
        case GREETD_MESSAGE_TYPE_ERROR:
            if (response->as.error.description != NULL) {
                free(response->as.error.description);
                response->as.error.description = NULL;
            }
            break;
        case GREETD_MESSAGE_TYPE_AUTH_MESSAGE:
            if (response->as.auth_message.auth_message != NULL) {
                free(response->as.auth_message.auth_message);
                response->as.auth_message.auth_message = NULL;
            }
            break;
        case GREETD_MESSAGE_TYPE_SUCCESS:
            // No dynamically allocated strings in success response
            break;
    }
}

int greetd_init(struct Greetd* greetd, char* greetd_sock)
{
    greetd->greetd_sock = greetd_sock;
    greetd->client_fd = -1;

    int client_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_fd == -1) {
        return GREETD_ERROR_SOCKET_CREATION;
    }

    struct sockaddr_un addr = { 0 };
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, greetd_sock, sizeof(addr.sun_path) - 1);

    socklen_t addr_len = sizeof(addr.sun_path) + sizeof(addr.sun_family);
    if (connect(client_fd, (struct sockaddr*)&addr, addr_len) == -1) {
        return GREETD_ERROR_SOCKET_CONNECTION;
    }

    greetd->client_fd = client_fd;
    return 0;
}

int greetd_close(struct Greetd* greetd)
{
    if (greetd->client_fd == -1) {
        return GREETD_ERROR_SOCKET_CLOSE;
    }

    int ok = close(greetd->client_fd);
    if (ok == -1) {
        return GREETD_ERROR_SOCKET_CLOSE;
    }

    greetd->client_fd = -1;
    return 0;
}

struct Greetd_Response _greetd_create_session(struct Greetd* greetd, struct Greetd_CreateSession* command)
{
    char buffer[40 + strlen(command->username)];
    snprintf(buffer,
        sizeof(buffer),
        // uck json libs
        "{\"type\":\"create_session\",\"username\":\"%s\"}",
        command->username);

    int ok = __greetd_write(greetd, buffer);
    if (ok == -1) {
        return (struct Greetd_Response){ 
            .message_type = GREETD_MESSAGE_TYPE_ERROR,
            .as.error = {
                .error_type = GREETD_ERROR_TYPE_CLIENT_ERROR,
                .description = "Failed to write to greetd socket"
            }
        };
    }

    return __greetd_get_response(greetd);
}

struct Greetd_Response _greetd_cancel_session(struct Greetd* greetd, struct Greetd_CancelSession* command)
{
    char buffer[] = "{\"type\":\"cancel_session\"}";
    int ok = __greetd_write(greetd, buffer);
    if (ok == -1) {
        return (struct Greetd_Response){ 
            .message_type = GREETD_MESSAGE_TYPE_ERROR,
            .as.error = {
                .error_type = GREETD_ERROR_TYPE_CLIENT_ERROR,
                .description = "Failed to write to greetd socket"
            }
        };
    }

    return __greetd_get_response(greetd);
}

struct Greetd_Response _greetd_post_auth_message_response(struct Greetd* greetd, struct Greetd_PostAuthMessageResponse* command)
{
    char buffer[52 + strlen(command->response)];
    
    if (strlen(command->response) > 0) {
        snprintf(
            buffer,
            sizeof(buffer),
            "{\"type\":\"post_auth_message_response\",\"response\":\"%s\"}",
            command->response);   
    } else {
        snprintf(
            buffer,
            sizeof(buffer),
            "{\"type\":\"post_auth_message_response\"}"
        );
    }
    
    int ok = __greetd_write(greetd, buffer);
    if (ok == -1) {
        return (struct Greetd_Response){ 
            .message_type = GREETD_MESSAGE_TYPE_ERROR,
            .as.error = {
                .error_type = GREETD_ERROR_TYPE_CLIENT_ERROR,
                .description = "Failed to write to greetd socket"
            }
        };
    }

    return __greetd_get_response(greetd);
}

struct Greetd_Response _greetd_start_session(struct Greetd *greetd, struct Greetd_StartSession *command)
{
    char cmd_array_buffer[JSON_ARRAY_BUFFER_SIZE] = { 0 };
    snprintf(cmd_array_buffer, sizeof(cmd_array_buffer), "[");
    for (int i = 0; i < command->cmd_count; i++) {
        strcat(cmd_array_buffer, "\"");
        strcat(cmd_array_buffer, command->cmd[i]);
        strcat(cmd_array_buffer, "\"");
        if (i < command->cmd_count - 1) {
            strcat(cmd_array_buffer, ",");
        }
    }
    strcat(cmd_array_buffer, "]");
    
    char env_array_buffer[JSON_ARRAY_BUFFER_SIZE] = { 0 };
    snprintf(env_array_buffer, sizeof(env_array_buffer), "[");
    for (int i = 0; i < command->env_count; i++) {
        strcat(env_array_buffer, "\"");
        strcat(env_array_buffer, command->env[i]);
        strcat(env_array_buffer, "\"");
        if (i < command->env_count - 1) {
            strcat(env_array_buffer, ",");
        }
    }
    strcat(env_array_buffer, "]");
    
    char buffer[39 + strlen(cmd_array_buffer) + strlen(env_array_buffer)];
    snprintf(
        buffer,
        sizeof(buffer),
        "{\"type\":\"start_session\",\"cmd\":%s,\"env\":%s}",
        cmd_array_buffer,
        env_array_buffer);

    int ok = __greetd_write(greetd, buffer);
    if (ok == -1) {
        return (struct Greetd_Response){ 
            .message_type = GREETD_MESSAGE_TYPE_ERROR,
            .as.error = {
                .error_type = GREETD_ERROR_TYPE_CLIENT_ERROR,
                .description = "Failed to write to greetd socket"
            }
        };
    }

    return __greetd_get_response(greetd);
}

// ====== Private functions ======

struct Greetd_Response __greetd_get_response(struct Greetd* greetd)
{
    char response_buffer[RESPONSE_BUFFER_SIZE];
    int ok = __greetd_read(greetd, response_buffer);
    if (ok == -1) {
        return (struct Greetd_Response){ 
            .message_type = GREETD_MESSAGE_TYPE_ERROR,
            .as.error = {
                .error_type = GREETD_ERROR_TYPE_CLIENT_ERROR,
                .description = "Failed to read from greetd socket"
            }
        };
    }

    struct JSONNode response_node = JSONNode_deserialize(response_buffer);
    if (response_node.type == ERROR) {
        JSONNode_free(&response_node);
        return (struct Greetd_Response){ 
            .message_type = GREETD_MESSAGE_TYPE_ERROR,
            .as.error = {
                .error_type = GREETD_ERROR_TYPE_CLIENT_ERROR,
                .description = "Failed to deserialize response from greetd socket"
            }
        };
    } else if (response_node.type != OBJECT) {
        JSONNode_free(&response_node);
        return (struct Greetd_Response){ 
            .message_type = GREETD_MESSAGE_TYPE_ERROR,
            .as.error = {
                .error_type = GREETD_ERROR_TYPE_CLIENT_ERROR,
                .description = "Response from greetd socket is not a valid JSON object"
            }
        };
    }

    struct JSONNode* message_type = JSONObject_get(response_node.as.object, "type");
    if (message_type == NULL) {
        JSONNode_free(&response_node);
        return (struct Greetd_Response){
            .message_type = GREETD_MESSAGE_TYPE_ERROR,
            .as.error = {
                .error_type = GREETD_ERROR_TYPE_CLIENT_ERROR,
                .description = "Response from greetd socket does not contain a message type"
            }
        };
    } else if (message_type->type != STRING) {
        JSONNode_free(&response_node);
        return (struct Greetd_Response){
            .message_type = GREETD_MESSAGE_TYPE_ERROR,
            .as.error = {
                .error_type = GREETD_ERROR_TYPE_CLIENT_ERROR,
                .description = "Message type from greetd socket is not a string"
            }
        };
    }

    char* message_type_value = message_type->as.string.value;
    if (strcmp(message_type_value, "success") == 0) {
        JSONNode_free(&response_node);
        return (struct Greetd_Response){
            .message_type = GREETD_MESSAGE_TYPE_SUCCESS
        };
    } else if (strcmp(message_type_value, "error") == 0) {
        struct JSONNode* error_type = JSONObject_get(response_node.as.object, "error_type");
        enum Greetd_ErrorType error_type_value;
        if (error_type == NULL || error_type->type != STRING || error_type->as.string.value == NULL) {
            error_type_value = GREETD_ERROR_TYPE_UNKNOWN;
        } else if (strcmp(error_type->as.string.value, "auth_error") == 0) {
            error_type_value = GREETD_ERROR_TYPE_AUTH_ERROR;
        } else if (strcmp(error_type->as.string.value, "error") == 0) {
            error_type_value = GREETD_ERROR_TYPE_ERROR;
        } else {
            error_type_value = GREETD_ERROR_TYPE_UNKNOWN;
        }

        struct JSONNode* description = JSONObject_get(response_node.as.object, "description");
        char* description_value;
        if (description == NULL || description->type != STRING || description->as.string.value == NULL) {
            description_value = strdup("Unknown error");
        } else {
            description_value = strdup(description->as.string.value);
        }

        JSONNode_free(&response_node);
        return (struct Greetd_Response){
            .message_type = GREETD_MESSAGE_TYPE_ERROR,
            .as.error = {
                .error_type = error_type_value,
                .description = description_value
            }
        };
    } else if (strcmp(message_type_value, "auth_message") == 0) {
        struct JSONNode* auth_message_type = JSONObject_get(response_node.as.object, "auth_message_type");
        enum Greetd_AuthenticationMessageType auth_message_type_value;
        if (auth_message_type == NULL || auth_message_type->type != STRING || auth_message_type->as.string.value == NULL) {
            auth_message_type_value = GREETD_AUTHENTICATION_MESSAGE_TYPE_UNKNOWN;
        } else if (strcmp(auth_message_type->as.string.value, "secret") == 0) {
            auth_message_type_value = GREETD_AUTHENTICATION_MESSAGE_TYPE_SECRET;
        } else if (strcmp(auth_message_type->as.string.value, "visible") == 0) {
            auth_message_type_value = GREETD_AUTHENTICATION_MESSAGE_TYPE_VISIBLE;
        } else if (strcmp(auth_message_type->as.string.value, "info") == 0) {
            auth_message_type_value = GREETD_AUTHENTICATION_MESSAGE_TYPE_INFO;
        } else if (strcmp(auth_message_type->as.string.value, "error") == 0) {
            auth_message_type_value = GREETD_AUTHENTICATION_MESSAGE_TYPE_ERROR;
        } else {
            auth_message_type_value = GREETD_AUTHENTICATION_MESSAGE_TYPE_UNKNOWN;
        }

        struct JSONNode* auth_message = JSONObject_get(response_node.as.object, "auth_message");
        char* auth_message_value;
        if (auth_message == NULL || auth_message->type != STRING || auth_message->as.string.value == NULL) {
            auth_message_value = strdup("Unknown authentication message");
        } else {
            auth_message_value = strdup(auth_message->as.string.value);
        }
        
        JSONNode_free(&response_node);
        return (struct Greetd_Response){
            .message_type = GREETD_MESSAGE_TYPE_AUTH_MESSAGE,
            .as.auth_message = {
                .auth_message_type = auth_message_type_value,
                .auth_message = auth_message_value
            }
        };
    }

    JSONNode_free(&response_node);
    return (struct Greetd_Response){
        .message_type = GREETD_MESSAGE_TYPE_ERROR,
        .as.error = {
            .error_type = GREETD_ERROR_TYPE_CLIENT_ERROR,
            .description = "Unknown message type from greetd socket"
        }
    };
}

int __greetd_write(struct Greetd* greetd, char* buffer) {
    int length = strlen(buffer);
    if (write(greetd->client_fd, &length, 4) == -1) {
        return -1;
    }
    if (write(greetd->client_fd, buffer, length) == -1) {
        return -1;
    }
    return 0;
}

int __greetd_read(struct Greetd* greetd, char* buffer) {
    int response_length = 0;
    if (read(greetd->client_fd, &response_length, 4) == -1) {
        return -1;
    }
    if (read(greetd->client_fd, buffer, response_length) == -1) {
        return -1;
    }
    buffer[response_length] = '\0';
    return 0;
}
