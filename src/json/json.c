#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "json.h"
#include "json_lexer.h"

#define BUFFER_SIZE 255

struct JSONNode JSON_String(char* value) {
    struct JSONNode node = { .type = STRING };
    node.as.string.value = strdup(value);
    return node;
}

struct JSONNode JSON_Number(double value) {
    struct JSONNode node = { .type = NUMBER };
    node.as.number.value = value;
    return node;
}

struct JSONNode JSON_Object(struct JSONObjectPair* pairs, int size) {
    struct JSONObjectPair* pairs_copy = malloc(sizeof(struct JSONObjectPair) * size);
    memcpy(pairs_copy, pairs, sizeof(struct JSONObjectPair) * size);

    struct JSONNode node = { .type = OBJECT };
    node.as.object.pairs = pairs;
    node.as.object.size = size;
    return node;
}

struct JSONNode JSON_Array(struct JSONNode* elements, int size) {
    struct JSONNode node = { .type = ARRAY };
    node.as.array.elements = elements;
    node.as.array.size = size;
    return node;
}

struct JSONNode JSON_Boolean(bool value) {
    struct JSONNode node = { .type = BOOLEAN };
    node.as.boolean.value = value;
    return node;
}

struct JSONNode JSON_Null() {
    struct JSONNode node = { .type = NULL_TYPE };
    return node;
}

char* JSONArray_serialize(struct JSONArray* array) {
    char buffer[BUFFER_SIZE];
    sprintf(buffer, "[");
    for (int i = 0; i < array->size; i++) {
        char serialized[BUFFER_SIZE];
        int length = JSONNode_serialize(serialized, sizeof(serialized), &array->elements[i]);
        if (length == -1) {
            return NULL;
        }
        strcat(buffer, serialized);
        if (i < array->size - 1) {
            strcat(buffer, ",");
        }
    }
    strcat(buffer, "]");
    return strdup(buffer);
}

char* JSONObject_serialize(struct JSONObject* object) {
    char buffer[BUFFER_SIZE];
    sprintf(buffer, "{");
    for (int i = 0; i < object->size; i++) {
        char* key = object->pairs[i].key;
        char serialized[BUFFER_SIZE];
        int length = JSONNode_serialize(serialized, sizeof(serialized), object->pairs[i].value);
        if (length == -1) {
            return NULL;
        }
        sprintf(buffer, "%s\"%s\":%s", buffer, key, serialized);
        if (i < object->size - 1) {
            strcat(buffer, ",");
        }
    }
    strcat(buffer, "}");
    return strdup(buffer);
}

char* JSONString_serialize(struct JSONString* string) {
    char buffer[BUFFER_SIZE];
    sprintf(buffer, "\"%s\"", string->value);
    return strdup(buffer);
}

char* JSONNumber_serialize(struct JSONNumber* number) {
    char buffer[BUFFER_SIZE];
    sprintf(buffer, "%f", number->value);
    return strdup(buffer);
}

char* JSONBoolean_serialize(struct JSONBool* boolean) {
    char buffer[BUFFER_SIZE];
    sprintf(buffer, "%s", boolean->value ? "true" : "false");
    return strdup(buffer);
}

char* JSONNull_serialize() {
    char buffer[BUFFER_SIZE];
    sprintf(buffer, "null");
    return strdup(buffer);
}

int JSONNode_serialize(char* buffer, int buffer_length, struct JSONNode* node) {
    char* json_buffer;
    switch (node->type) {
        case OBJECT:
            json_buffer = JSONObject_serialize(&node->as.object);
            break;
        case ARRAY:
            json_buffer = JSONArray_serialize(&node->as.array);
            break;
        case STRING:
            json_buffer = JSONString_serialize(&node->as.string);
            break;
        case NUMBER:
            json_buffer = JSONNumber_serialize(&node->as.number);
            break;
        case BOOLEAN:
            json_buffer = JSONBoolean_serialize(&node->as.boolean);
            break;
        case NULL_TYPE:
            json_buffer = JSONNull_serialize();
            break;
    }

    int length = strlen(json_buffer) + 1; // +1 for the null terminator
    if (buffer_length < length) {
        free(json_buffer);
        return -1;
    }

    memcpy(buffer, json_buffer, length);
    free(json_buffer);

    return 0;
}
