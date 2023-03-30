#ifndef PROCESS_PKT_H
#define PROCESS_PKT_H
#include "../lib/nlohmann/json-schema.hpp"

using nlohmann::json_schema::json_validator;
using json = nlohmann::json;


class ProcessDataPacket{
        std::uint8_t pDataType;
        enum packetDataType {
            INTERMEDIATE_RESULT,
            FINAL_RESULT,
            ERROR_DATA
        };
        json packet;
    public:
        ProcessDataPacket(json packet){
            this->packet = packet;
        };
        std::string getExtension();
        void execute();
        void detectDataType();
        int createCsvFromData();
        int pushCsvToDb();
};

class ProcessStatusPacket{
        json packet;
    public:
        ProcessStatusPacket(json packet){
            this->packet = packet;
        };
        void execute();
};

#endif