#include <iostream>
#include <sockets/unix_socket.hpp>
#include <structs/request_model.hpp>
#include <crypt/crypt.hpp>

bool handleEncrypt(std::vector<uint8_t> data, std::vector<uint8_t> &encryptedData) {
    AESData key = AESData::fromVector(data);
    std::cout << "Encrypting key" << std::endl;
    RSAEncryptedData encryptedKey;
    if (!encryptKey(key, encryptedKey)) {
        return false;
    }
    encryptedData = encryptedKey.toVector();
    return true;
}

bool handleDecrypt(std::vector<uint8_t> data, std::vector<uint8_t> &decryptedData) {
    RSAEncryptedData key = RSAEncryptedData::fromVector(data);
    std::cout << "Decrypting key" << std::endl;
    AESData decryptedKey;
    if (!decryptKey(key, decryptedKey)) {
        return false;
    }
    decryptedData = decryptedKey.toVector();
    return true;
}

nlohmann::json handleRequest(nlohmann::json request) {
    RequestModel model = RequestModel::fromJson(request);
    std::cout << "Received request: " << model.id << std::endl;
    if (model.type == RequestType::aes256encrypt) {
        std::vector<uint8_t> encryptedKey;
        if (!handleEncrypt(model.data, encryptedKey)) {
            return nlohmann::json{{"error", "encryption failed"}};
        }
        RequestModel response;
        response.id = model.id;
        response.type = RequestType::aes256encrypt;
        response.data = encryptedKey;
        return response.toJson();
    }
    if (model.type == RequestType::aes256decrypt) {
        std::vector<uint8_t> decryptedKey;
        if (!handleDecrypt(model.data, decryptedKey)) {
            return nlohmann::json{{"error", "decryption failed"}};
        }
        RequestModel response;
        response.id = model.id;
        response.type = RequestType::aes256decrypt;
        response.data = decryptedKey;
        return response.toJson();
    }
    return nlohmann::json{{"error", "unknown request type"}};
}

int main() {
    UnixSocket socket;
    if (!socket.setupSocket()) {
        return 1;
    }
    socket.serveForever(handleRequest);
    return 0;
}