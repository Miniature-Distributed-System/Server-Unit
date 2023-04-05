#ifndef PKT_CREATOR_H
#define PKT_CREATOR_H

#include "../lib/nlohmann/json-schema.hpp"
#include "../include/packet.hpp"

using nlohmann::json_schema::json_validator;
using json = nlohmann::json;

class PacketConstructor {
    public:
        static json create(ServerPacketCodes code, std::uint64_t workerUid, json packet = json({}));
};

#endif