#include "packet_constructor.hpp"

json PacketConstructor::create(ServerPacketCodes code, std::string workerUid, json packet)
{
    packet["head"] = code;
    packet["id"] = workerUid;

    return packet;
}