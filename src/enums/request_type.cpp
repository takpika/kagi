#include <enums/request_type.hpp>

std::string RequestTypeHelper::toString(RequestType type) {
    switch (type) {
        case RequestType::aes256encrypt:
            return "aes256encrypt";
        case RequestType::aes256decrypt:
            return "aes256decrypt";
        case RequestType::publickey:
            return "publickey";
        case RequestType::unknown:
            return "";
    }
}

RequestType RequestTypeHelper::fromString(std::string type) {
    if (type == "aes256encrypt") {
        return RequestType::aes256encrypt;
    } else if (type == "aes256decrypt") {
        return RequestType::aes256decrypt;
    } else if (type == "publickey") {
        return RequestType::publickey;
    }
    return RequestType::unknown;
}