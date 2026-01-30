#include "json_lexer.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#define BUFFER_SIZE 255

void JSONLexer_init(struct JSONLexer* lexer, char* buffer) {
    lexer->buffer = buffer;
    lexer->position = 0;
    lexer->buffer_length = strlen(buffer);
}

bool JSONLexer_is_whitespace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

struct JSONToken JSONLexer_next(struct JSONLexer* lexer) {
    while (JSONLexer_is_whitespace(lexer->buffer[lexer->position])) {
        lexer->position++;
    }

    if (lexer->buffer[lexer->position] == '{') {
        struct JSONToken token = { .type = JSON_OBJECT_START };
        lexer->position++;
        return token;
    }

    if (lexer->buffer[lexer->position] == '}') {
        struct JSONToken token = { .type = JSON_OBJECT_END };
        lexer->position++;
        return token;
    }

    if (lexer->buffer[lexer->position] == '[') {
        struct JSONToken token = { .type = JSON_ARRAY_START };
        lexer->position++;
        return token;
    }

    if (lexer->buffer[lexer->position] == ']') {
        struct JSONToken token = { .type = JSON_ARRAY_END };
        lexer->position++;
        return token;
    }

    if (lexer->buffer[lexer->position] == '"') {
        lexer->position++;
        char* value = malloc(BUFFER_SIZE);
        int i = 0;
        while (lexer->buffer[lexer->position] != '"') {
            value[i] = lexer->buffer[lexer->position];
            i++;
            lexer->position++;

            if (lexer->position >= lexer->buffer_length) {
                free(value);
                struct JSONToken token = { .type = JSON_ERROR };
                return token;
            }
        }
        lexer->position++;
        struct JSONToken token = { .type = JSON_STRING, .value.string = value };
        return token;
    }

    if (lexer->buffer[lexer->position] == 't') {
        char word[5] = { 0 };
        memcpy(word, &lexer->buffer[lexer->position], 4);
        if (strcmp(word, "true") == 0) {
            lexer->position += 4;
            struct JSONToken token = { .type = JSON_TRUE };
            return token;
        }
    }

    if (lexer->buffer[lexer->position] == 'f') {
        char word[6] = { 0 };
        memcpy(word, &lexer->buffer[lexer->position], 5);
        if (strcmp(word, "false") == 0) {
            lexer->position += 5;
            struct JSONToken token = { .type = JSON_FALSE };
            return token;
        }
    }

    if (lexer->buffer[lexer->position] == 'n') {
        char word[5] = { 0 };
        memcpy(word, &lexer->buffer[lexer->position], 4);
        if (strcmp(word, "null") == 0) {
            lexer->position += 4;
            struct JSONToken token = { .type = JSON_NULL };
            return token;
        }
    }

    if (lexer->buffer[lexer->position] == ':') {
        struct JSONToken token = { .type = JSON_COLON };
        lexer->position++;
        return token;
    }

    if (lexer->buffer[lexer->position] == ',') {
        struct JSONToken token = { .type = JSON_COMMA };
        lexer->position++;
        return token;
    }

    if (lexer->buffer[lexer->position] == '\0') {
        struct JSONToken token = { .type = JSON_EOF };
        return token;
    }

    if (isdigit(lexer->buffer[lexer->position]) || lexer->buffer[lexer->position] == '-') {
        char* value = malloc(BUFFER_SIZE);
        for (int i = 0; isdigit(lexer->buffer[lexer->position]) || lexer->buffer[lexer->position] == '-'; i++, lexer->position++) {
            value[i] = lexer->buffer[lexer->position];
        }
        value[lexer->position] = '\0';
        double number = atof(value);
        free(value);
        struct JSONToken token = { .type = JSON_NUMBER, .value.number = number };
        return token;
    }

    struct JSONToken token = { .type = JSON_ERROR };
    return token;
}

struct JSONToken JSONLexer_peek(struct JSONLexer* lexer) {
    int position = lexer->position;
    struct JSONToken token = JSONLexer_next(lexer);
    lexer->position = position;
    return token;
}

void JSONLexer_free_token(struct JSONToken token) {
    if (token.type == JSON_STRING) {
        free(token.value.string);
    }
}

void JSONLexer_debug_print_token(struct JSONToken token) {
    switch (token.type) {
        case JSON_OBJECT_START:
            printf("[ObjectStart]");
            break;
        case JSON_OBJECT_END:
            printf("[ObjectEnd]");
            break;
        case JSON_ARRAY_START:
            printf("[ArrayStart]");
            break;
        case JSON_ARRAY_END:
            printf("[ArrayEnd]");
            break;
        case JSON_STRING:
            printf("[String, value=%s]", token.value.string);
            break;
        case JSON_NUMBER:
            printf("[Number, value=%f]", token.value.number);
            break;
        case JSON_TRUE:
            printf("[True]");
            break;
        case JSON_FALSE:
            printf("[False]");
            break;
        case JSON_NULL:
            printf("[Null]");
            break;
        case JSON_COMMA:
            printf("[Comma]");
            break;
        case JSON_COLON:
            printf("[Colon]");
            break;
        case JSON_EOF:
            printf("[EOF]");
            break;
        case JSON_ERROR:
            printf("[Error]");
            break;
    }
}