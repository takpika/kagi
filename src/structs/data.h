#ifndef STRUCTS_AES_H
#define STRUCTS_AES_H

#include <stdint.h>

struct AESData {
    uint8_t key[32];
};

struct RSAEncryptedData {
    uint8_t key[256];
};

#endif