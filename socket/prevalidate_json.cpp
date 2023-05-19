#include "../include/debug_rp.hpp"
#include "../include/packet.hpp"
#include "../include/logger.hpp"
#include "prevalidate_json.hpp"

static json packetSchema = R"(
{
    "$schema": "http://json-schema.org/draft-07/schema#",
    "title": "packet prevalidator",
    "properties": {
      "head": {
          "description": "packet status",
          "type": "number"
      },
      "id": {
          "description": "CU identification number",
          "type": "string"
      }
    },
    "required": [
                 "head",
                 "id"
                 ],
    "type": "object"
}
)"_json;

JsonPrevalidator::JsonPrevalidator(std::string out)
{
    try{
        this->packet = json::parse(out.c_str());
    }catch (std::exception &e){
        Log().error(__func__, "packet corrupt: ", out);
    }
}

bool JsonPrevalidator::validateJson()
{
    json_validator validator;
    validator.set_root_schema(packetSchema);

    try
    {
        auto defaultPatch = validator.validate(packet);
        Log().info(__func__, "packet validation success!");
        return true;
    }
    catch (const std::exception &e)
    {
        Log().debug(__func__, "packet invalid, ", e.what(), " packet", packet);
        return false;
    }
}

bool JsonPrevalidator::checkQuickSendBit()
{
    int headerBits = packet["head"];
    if (headerBits & P_QSEND)
        return true;
    return false;
}

json JsonPrevalidator::getJson()
{
    return packet;
}