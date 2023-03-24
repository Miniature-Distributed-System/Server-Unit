#ifndef PKT_VALID_H
#define PKT_VALID_H
#include "process_packet.hpp"

class PacketValidator{
    bool isValid = false;
    json packet;
    public:
        PacketValidator(json packet){
            this->packet = packet;
        };
        void validatePacket();
        bool isDataPacket(){
            return isValid;
        }
        json getPacket(){
            return packet;
        }
};
#endif