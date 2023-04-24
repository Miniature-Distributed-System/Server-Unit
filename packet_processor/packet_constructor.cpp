#include "packet_constructor.hpp"

json PacketConstructor::create(ServerPacketCodes code, std::uint64_t workerUid, json packet)
{
    packet["head"] = code;
    packet["id"] = workerUid;

    return packet;
}