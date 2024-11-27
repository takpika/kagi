#ifndef ENUM_REQUEST_TYPE_H
#define ENUM_REQUEST_TYPE_H

#include <string>

enum class RequestType {
    encrypt,
    decrypt,
    publickey
};

class RequestTypeHelper {
    public:
    static std::string toString(RequestType type);
    static RequestType fromString(std::string type);
};

#endif