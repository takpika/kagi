#ifndef STRUCT_REQUEST_MODEL_H
#define STRUCT_REQUEST_MODEL_H

#include <nlohmann/json.hpp>
#include <enums/request_type.hpp>
#include <vector>
#include <string>

struct RequestModel {
    std::string id;
    RequestType type;
    std::vector<uint8_t> data;

    public:
    nlohmann::json toJson();
    static RequestModel fromJson(nlohmann::json json);
};

#endif