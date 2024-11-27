#ifndef STRUCTS_RSA_ENCRYPTED_DATA_H
#define STRUCTS_RSA_ENCRYPTED_DATA_H

#include <stdint.h>
#include <vector>

struct RSAEncryptedData {
    uint8_t key[256];

    public:
    static RSAEncryptedData fromVector(std::vector<uint8_t> vector);
    std::vector<uint8_t> toVector();
};

#endif