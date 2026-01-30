#define DEBUG_LOG_ENABLED 1
#define TRACE_LOG_ENABLED 1

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include "greetd.h"
#include "log.h"

int main() {
    char* greetd_sock = getenv("GREETD_SOCK");
    if (greetd_sock == NULL) {
        fprintf(stderr, "GREETD_SOCK is not set\n");
        return 1;
    } else {
        TRACE_LOG("GREETD_SOCK is set to %s\n", greetd_sock);
    }

    struct Greetd greetd = { 0 };
    int ok = greetd_init(&greetd, greetd_sock);
    if (ok == -1) {
        fprintf(stderr, "Failed to initialize greetd: %s\n", greetd_error_string(ok));
        return 1;
    } else {
        DEBUG_LOG("Greetd initialized successfully\n");
    }

    ok = greetd_create_session(&greetd, "rastsislau");
    if (ok == -1) {
        fprintf(stderr, "Failed to create session: %s\n", greetd_error_string(ok));
        return 1;
    } else {
        DEBUG_LOG("Session created successfully\n");
    }

    return 0;
}
