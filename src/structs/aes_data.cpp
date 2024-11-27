#include <structs/aes_data.hpp>

AESData AESData::fromVector(std::vector<uint8_t> vector) {
    AESData data;
    std::copy(vector.begin(), vector.begin() + 32, data.key);
    return data;
}

std::vector<uint8_t> AESData::toVector() {
    return std::vector<uint8_t>(key, key + sizeof(key));
}