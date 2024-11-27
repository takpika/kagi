#include <structs/request_model.hpp>
#include <crypt/base64.hpp>
#include <crypt/gzip.hpp>

nlohmann::json RequestModel::toJson() {
    nlohmann::json json;
    json["id"] = id;
    json["type"] = RequestTypeHelper::toString(type);
    json["data"] = base64Encode(data);
    return json;
}

RequestModel RequestModel::fromJson(nlohmann::json json) {
    RequestModel model;
    model.id = json["id"];
    model.type = RequestTypeHelper::fromString(json["type"]);
    model.data = base64Decode(json["data"]);
    return model;
}