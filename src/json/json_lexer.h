#ifndef JSON_LEXER_H
#define JSON_LEXER_H

#include <stdbool.h>

struct JSONToken {
    enum {
        JSON_OBJECT_START,
        JSON_OBJECT_END,
        JSON_ARRAY_START,
        JSON_ARRAY_END,
        JSON_STRING,
        JSON_NUMBER,
        JSON_TRUE,
        JSON_FALSE,
        JSON_NULL,
        JSON_COMMA,
        JSON_COLON,
        JSON_EOF,
        JSON_ERROR
    } type;
    
    union {
        char* string;
        double number;
        bool boolean;
    } value;
};

struct JSONLexer {
    char* buffer;
    int buffer_length;
    int position;
};

void JSONLexer_init(struct JSONLexer* lexer, char* buffer);

struct JSONToken JSONLexer_next(struct JSONLexer* lexer);
struct JSONToken JSONLexer_peek(struct JSONLexer* lexer);
void JSONLexer_free_token(struct JSONToken token);

#endif
