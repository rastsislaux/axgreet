#ifndef JSON_H
#define JSON_H

#include <stdbool.h>

enum JSONNodeType {
    OBJECT,
    ARRAY,
    STRING,
    NUMBER,
    BOOLEAN,
    NULL_TYPE,
};

struct JSONObjectPair {
    char* key;
    struct JSONNode* value;
};

struct JSONObject {
    int size;
    struct JSONObjectPair* pairs;
};

struct JSONArray {
    int size;
    struct JSONNode* elements;
};

struct JSONString {
    char* value;
};

struct JSONNumber {
    double value;
};

struct JSONBool {
    bool value;
};

struct JSONNode {
    enum JSONNodeType type;
    union {
        struct JSONObject object;
        struct JSONArray array;
        struct JSONString string;
        struct JSONNumber number;
        struct JSONBool boolean;
    } as;
};

struct JSONNode JSON_String(char* value);
struct JSONNode JSON_Number(double value);
struct JSONNode JSON_Object(struct JSONObjectPair* pairs, int size);
struct JSONNode JSON_Array(struct JSONNode* elements, int size);
struct JSONNode JSON_Boolean(bool value);
struct JSONNode JSON_Null();

int JSONNode_serialize(char* buffer, int buffer_length, struct JSONNode* node);
struct JSONNode JSONNode_deserialize(char* buffer);

#endif
