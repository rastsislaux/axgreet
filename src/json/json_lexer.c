#include "json_lexer.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define BUFFER_SIZE 255

// Uncomment this to print the tokens as they are being parsed.
// This is useful for debugging the lexer.
// #define PRINT_TOKENS

#ifdef PRINT_TOKENS
#include <stdio.h>
#endif

void JSONLexer_init(struct JSONLexer* lexer, char* buffer) {
    lexer->buffer = buffer;
    lexer->position = 0;
    lexer->buffer_length = strlen(buffer);

    #ifdef PRINT_TOKENS
        printf("\n");
    #endif
}

bool JSONLexer_is_whitespace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

#ifdef PRINT_TOKENS
struct JSONToken _JSONLexer_next(struct JSONLexer* lexer)
#endif
#ifndef PRINT_TOKENS
struct JSONToken JSONLexer_next(struct JSONLexer* lexer)
#endif
{
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
        value[i] = '\0';
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

#ifdef PRINT_TOKENS
void __print_token(struct JSONToken token);
struct JSONToken JSONLexer_next(struct JSONLexer* lexer) {
    struct JSONToken token = _JSONLexer_next(lexer);
    __print_token(token);
    printf(", ");
    return token;
}
#endif

struct JSONToken JSONLexer_peek(struct JSONLexer* lexer) {
    int position = lexer->position;
#ifdef PRINT_TOKENS
    struct JSONToken token = _JSONLexer_next(lexer);
#else
    struct JSONToken token = JSONLexer_next(lexer);
#endif
    lexer->position = position;
    return token;
}

void JSONLexer_free_token(struct JSONToken token) {
    if (token.type == JSON_STRING) {
        free(token.value.string);
    }
}

#ifdef PRINT_TOKENS
void __print_token(struct JSONToken token)
{
    switch (token.type) {
        case JSON_OBJECT_START:
            printf("JSON_OBJECT_START");
            break;
        case JSON_OBJECT_END:
            printf("JSON_OBJECT_END");
            break;
        case JSON_ARRAY_START:
            printf("JSON_ARRAY_START");
            break;
        case JSON_ARRAY_END:
            printf("JSON_ARRAY_END");
            break;
        case JSON_STRING:
            printf("JSON_STRING(\"%s\")", token.value.string);
            break;
        case JSON_NUMBER:
            printf("JSON_NUMBER(%g)", token.value.number);
            break;
        case JSON_TRUE:
            printf("JSON_TRUE");
            break;
        case JSON_FALSE:
            printf("JSON_FALSE");
            break;
        case JSON_NULL:
            printf("JSON_NULL");
            break;
        case JSON_COMMA:
            printf("JSON_COMMA");
            break;
        case JSON_COLON:
            printf("JSON_COLON");
            break;
        case JSON_EOF:
            printf("JSON_EOF");
            break;
        case JSON_ERROR:
            printf("JSON_ERROR");
            break;
    }
}
#endif
