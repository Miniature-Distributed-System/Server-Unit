#include "packet_constructor.hpp"

json PacketConstructor::create(ServerPacketCodes code, std::uint64_t workerUid, json packet = json({}))
{
    packet["head"] = code;
    packet["id"] = workerUid;
}