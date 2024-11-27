#include <structs/rsa_encrypted_data.hpp>

RSAEncryptedData RSAEncryptedData::fromVector(std::vector<uint8_t> vector) {
    RSAEncryptedData data;
    std::copy(vector.begin(), vector.begin() + 256, data.key);
    return data;
}

std::vector<uint8_t> RSAEncryptedData::toVector() {
    return std::vector<uint8_t>(key, key + sizeof(key));
}