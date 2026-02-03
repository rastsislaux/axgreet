#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include "greetd.h"
#include "log.h"
#include <termios.h>
#include "json/json.h"

#define NOT_IMPLEMENTED_YET do { \
    ERROR_LOG(__FILE__ ":%d: Not implemented yet\n", __LINE__); \
    exit(1); \
} while (0)

#define MAX_ATTEMPTS 3

// ====== Private function declarations ======

void __secure_fgets(char* buffer, int size);

// ====== Main function ======

int main(int argc, char** argv) {
    char* greetd_sock = getenv("GREETD_SOCK");
    if (greetd_sock == NULL) {
        fprintf(stderr, "GREETD_SOCK is not set\n");
        return 1;
    } else {
        TRACE_LOG("GREETD_SOCK is set to %s\n", greetd_sock);
    }

    struct Greetd greetd = { 0 };
    int ok = greetd_init(&greetd, greetd_sock);
    if (ok != 0) {
        ERROR_LOG("Failed to initialize greetd: %s", greetd_error_string(ok));
        return 1;
    } else {
        DEBUG_LOG("Greetd initialized successfully\n");
    }

    char username_buffer[255] = { 0 };
    printf("Username: ");
    char* username = fgets(username_buffer, sizeof(username_buffer), stdin);
    if (username == NULL) {
        ERROR_LOG("Failed to read username\n");
        exit(1);
    }
    username[strcspn(username, "\n")] = '\0';

    struct Greetd_CreateSession create_session = { .username = username };
    struct Greetd_Response response = greetd_send(&greetd, &create_session);

    int retries = 0;
    do {
        if (retries++ >= MAX_ATTEMPTS) {
            ERROR_LOG("Max attempts reached\n");
            greetd_send(&greetd, (struct Greetd_CancelSession*) NULL);
            greetd_close(&greetd);
            // TODO: This is a hack for now.
            // I'm not planning to leave this thingy a console app,
            // so this will be replaced with a proper solution later.
            execv(argv[0], argv);
        }

        if (response.message_type == GREETD_MESSAGE_TYPE_ERROR) {
            // For some reason cancelling session returns a error,
            // but ignoring it seems to be the only way to retry.
            greetd_send(&greetd, (struct Greetd_CancelSession*) NULL);

            if (response.as.error.error_type == GREETD_ERROR_TYPE_AUTH_ERROR) {
                printf("Authentication failed: %s\n", response.as.error.description);
                response = greetd_send(&greetd, &create_session);
            } else {
                ERROR_LOG("Failed to create session: %s\n", response.as.error.description);
                exit(1);
            }
        }

        char secret_buffer[255] = { 0 };
        switch (response.as.auth_message.auth_message_type) {
            case GREETD_AUTHENTICATION_MESSAGE_TYPE_SECRET:
                printf("%s", response.as.auth_message.auth_message);
                __secure_fgets(secret_buffer, sizeof(secret_buffer));
                break;
            case GREETD_AUTHENTICATION_MESSAGE_TYPE_VISIBLE:
                printf("%s", response.as.auth_message.auth_message);
                fgets(secret_buffer, sizeof(secret_buffer), stdin);
                secret_buffer[strcspn(secret_buffer, "\n")] = '\0';
                break;
            case GREETD_AUTHENTICATION_MESSAGE_TYPE_INFO:
                printf("%s\n", response.as.auth_message.auth_message);
                break;
            case GREETD_AUTHENTICATION_MESSAGE_TYPE_ERROR:
                printf("ERROR: %s\n", response.as.auth_message.auth_message);
                break;
            case GREETD_AUTHENTICATION_MESSAGE_TYPE_UNKNOWN:
                ERROR_LOG("Unknown authentication message type: %s\n", response.as.auth_message.auth_message);
                exit(1);
        }

        struct Greetd_PostAuthMessageResponse post_auth_message_response = { .response = secret_buffer };
        response = greetd_send(&greetd, &post_auth_message_response);
    } while (response.message_type != GREETD_MESSAGE_TYPE_SUCCESS);

    if (response.message_type == GREETD_MESSAGE_TYPE_ERROR) {
        ERROR_LOG("Failed to post authentication message response: %s\n", response.as.error.description);
        exit(1);
    }

    if (response.message_type != GREETD_MESSAGE_TYPE_SUCCESS) {
        ERROR_LOG("Unexpected response type: %d\n", response.message_type);
        exit(1);
    }

    struct Greetd_StartSession start_session = {
        .cmd_count = 1, .cmd = (char*[]){ "/bin/sh" },
        .env_count = 0, .env = NULL
    };
    response = greetd_send(&greetd, &start_session);

    if (response.message_type == GREETD_MESSAGE_TYPE_ERROR) {
        ERROR_LOG("Failed to start session: %s\n", response.as.error.description);
        exit(1);
    }

    if (response.message_type != GREETD_MESSAGE_TYPE_SUCCESS) {
        ERROR_LOG("Unexpected response type: %d\n", response.message_type);
        exit(1);
    }
    greetd_response_free(&response);
    
    ok = greetd_close(&greetd);
    if (ok != 0) {
        ERROR_LOG("Failed to close greetd: %s\n", greetd_error_string(ok));
        exit(1);
    } else {
        DEBUG_LOG("Greetd closed successfully\n");
    }

    return 0;
}

// ====== Private functions ======

void __secure_fgets(char* buffer, int size) {
    struct termios oldt, newt;    
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;

    newt.c_lflag &= ~(ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    if (fgets(buffer, size, stdin)) {
        buffer[strcspn(buffer, "\n")] = '\0';
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    printf("\n");
}
