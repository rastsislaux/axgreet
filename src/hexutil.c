#include <stdlib.h>
#include <string.h>
#include "hexutil.h"

void hexdump(char* buffer, int buffer_length, char* data, int length) {
    static const char hexchars[] = "0123456789abcdef";
    const unsigned char* bytes = (const unsigned char*)data;
    char* hex = malloc(length * 3 + 1);
    for (int i = 0; i < length; i++) {
        unsigned char value = bytes[i];
        int offset = i * 3;
        hex[offset] = hexchars[value >> 4];
        hex[offset + 1] = hexchars[value & 0x0F];
        hex[offset + 2] = ' ';
    }
    hex[length * 3] = '\0';

    int result_length = length * 3 + 1;

    int clamped_length;
    if (buffer_length > result_length) {
        clamped_length = result_length;
    } else {
        clamped_length = buffer_length;
        buffer[clamped_length] = '\0';
    }

    memcpy(buffer, hex, clamped_length);
    free(hex);
}
