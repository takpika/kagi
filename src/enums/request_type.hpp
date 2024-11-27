#ifndef ENUM_REQUEST_TYPE_H
#define ENUM_REQUEST_TYPE_H

#include <string>

enum class RequestType {
    aes256encrypt,
    aes256decrypt,
    publickey,
    unknown
};

class RequestTypeHelper {
    public:
    static std::string toString(RequestType type);
    static RequestType fromString(std::string type);
};

#endif