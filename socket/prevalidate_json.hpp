#ifndef H_PREVAL_JSON
#define H_PREVAL_JSON
#include "../lib/nlohmann/json-schema.hpp"

using nlohmann::json_schema::json_validator;
using json = nlohmann::json;

class JsonPrevalidator {
    json packet;
    public:
        JsonPrevalidator(std::string);
        bool validateJson();
        bool checkQuickSendBit();
};
#endif