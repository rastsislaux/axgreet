#include "json.h"
#include "json_lexer.h"
#include <stdlib.h>
#include <string.h>


struct JSONNode _JSONNode_deserialize(struct JSONLexer* lexer);

// Assumes, that the array is already started.
struct JSONNode JSONNode_deserialize_array(struct JSONLexer* lexer) {
    int size = 0;
    struct JSONNode* elements = malloc(sizeof(struct JSONNode) * 1);

    struct JSONToken last_token;
    while (true) {
        struct JSONToken token = JSONLexer_peek(lexer);
        if (token.type == JSON_ARRAY_END) {
            last_token = JSONLexer_next(lexer);
            break;
        }

        struct JSONNode* element = malloc(sizeof(struct JSONNode));
        *element = _JSONNode_deserialize(lexer);
        elements = realloc(elements, sizeof(struct JSONNode) * (size + 1));
        elements[size++] = *element;

        last_token = JSONLexer_next(lexer);
        if (last_token.type != JSON_COMMA) {
            break;
        }
    }

    if (last_token.type != JSON_ARRAY_END) {
        return JSON_Null();
    }

    return JSON_Array(elements, size);
}

// Assumes, that the object is already started.
struct JSONNode JSONNode_deserialize_object(struct JSONLexer* lexer) {
    struct JSONToken token = JSONLexer_peek(lexer);
    if (token.type == JSON_OBJECT_END) {
        JSONLexer_next(lexer);
        return JSON_Object(NULL, 0);
    }

    int size = 0;
    struct JSONObjectPair* pairs = malloc(sizeof(struct JSONObjectPair) * 1);

    struct JSONToken last_token;
    while (true) {
        struct JSONToken token = JSONLexer_next(lexer);
        char* key;
        if (token.type == JSON_STRING) {
            key = token.value.string;
        } else {
            return JSON_Null();
        }

        token = JSONLexer_next(lexer);
        if (token.type != JSON_COLON) {
            return JSON_Null();
        }

        struct JSONNode* value = malloc(sizeof(struct JSONNode));
        *value = _JSONNode_deserialize(lexer); 

        struct JSONObjectPair pair = { key, value };
        pairs[size] = pair;
        size++;

        token = JSONLexer_next(lexer);
        if (token.type != JSON_COMMA) {
            last_token = token;
            break;
        }

        pairs = realloc(pairs, sizeof(struct JSONObjectPair) * (size + 1));
    }

    if (last_token.type != JSON_OBJECT_END) {
        return JSON_Null();
    }
    
    return JSON_Object(pairs, size);
}

struct JSONNode _JSONNode_deserialize(struct JSONLexer* lexer) {
    while (true) {
        struct JSONToken token = JSONLexer_next(lexer);

        if (token.type == JSON_STRING) {
            struct JSONNode node = JSON_String(strdup(token.value.string));
            return node;
        }

        if (token.type == JSON_NUMBER) {
            struct JSONNode node = JSON_Number(token.value.number);
            return node;
        }

        if (token.type == JSON_TRUE) {
            struct JSONNode node = JSON_Boolean(true);
            return node;
        }

        if (token.type == JSON_FALSE) {
            struct JSONNode node = JSON_Boolean(false);
            return node;
        }

        if (token.type == JSON_NULL) {
            struct JSONNode node = JSON_Null();
            return node;
        }

        if (token.type == JSON_OBJECT_START) {
            struct JSONNode node = JSONNode_deserialize_object(lexer);
            return node;
        }

        if (token.type == JSON_ARRAY_START) {
            struct JSONNode node = JSONNode_deserialize_array(lexer);
            return node;
        }

        if (token.type == JSON_EOF) {
            break;
        }
        if (token.type == JSON_ERROR) {
            return JSON_Null();
        }

        JSONLexer_free_token(token);
    }
    return JSON_Null();
}

struct JSONNode JSONNode_deserialize(char* buffer) {
    struct JSONLexer lexer;
    JSONLexer_init(&lexer, buffer);
    return _JSONNode_deserialize(&lexer);
}
