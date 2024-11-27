#include <enums/request_type.hpp>

std::string RequestTypeHelper::toString(RequestType type) {
    switch (type) {
        case RequestType::encrypt:
            return "encrypt";
        case RequestType::decrypt:
            return "decrypt";
        case RequestType::publickey:
            return "publickey";
    }
}

RequestType RequestTypeHelper::fromString(std::string type) {
    if (type == "encrypt") {
        return RequestType::encrypt;
    } else if (type == "decrypt") {
        return RequestType::decrypt;
    } else if (type == "publickey") {
        return RequestType::publickey;
    }
}