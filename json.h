#ifndef JSON_H
#define JSON_H

enum JSONNodeType {
    OBJECT,
    ARRAY,
    STRING,
    NUMBER
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

struct JSONNode {
    enum JSONNodeType type;
    union {
        struct JSONObject object;
        struct JSONArray array;
        struct JSONString string;
        struct JSONNumber number;
    } as;
};

struct JSONNode JSON_String(char* value);
struct JSONNode JSON_Number(double value);
struct JSONNode JSON_Object(struct JSONObjectPair* pairs, int size);
struct JSONNode JSON_Array(struct JSONNode* elements, int size);

int JSONNode_serialize(char* buffer, int buffer_length, struct JSONNode* node);
// char* JSONString_serialize(struct JSONString* string);
// char* JSONNumber_serialize(struct JSONNumber* number);
// char* JSONObject_serialize(struct JSONObject* object);
// char* JSONArray_serialize(struct JSONArray* array);

#endif