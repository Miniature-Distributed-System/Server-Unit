#ifndef PKT_CREATOR_H
#define PKT_CREATOR_H

#include "../include/packet.hpp"

#ifndef JSON_H
#define JSON_H
#include "../lib/nlohmann/json-schema.hpp"
using json = nlohmann::json;
#endif

class PacketConstructor {
    public:
        static json create(ServerPacketCodes code, std::string workerUid, json packet = json({}));
};

#endif