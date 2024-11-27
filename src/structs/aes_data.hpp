#ifndef STRUCTS_AES_H
#define STRUCTS_AES_H

#include <stdint.h>
#include <vector>

struct AESData {
    uint8_t key[32];

    public:
    static AESData fromVector(std::vector<uint8_t> vector);
    std::vector<uint8_t> toVector();
};

#endif