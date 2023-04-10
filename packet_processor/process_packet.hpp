#ifndef PROCESS_PKT_H
#define PROCESS_PKT_H
#include "../lib/nlohmann/json-schema.hpp"

using nlohmann::json_schema::json_validator;
using json = nlohmann::json;


class ProcessDataPacket{
        std::uint8_t pDataType;
        std::string tableId;
        std::string data;
        std::uint64_t workerUid;
        int packetStausCode;
        enum packetDataType {
            INTERMEDIATE_RESULT,
            FINAL_RESULT,
            ERROR_DATA
        };
    public:
        ProcessDataPacket(json packet);
        std::string getExtension();
        void execute();
        void detectDataType();
        int createCsvFromData();
        int pushCsvToDb();
        void cleanUp();
};

class ProcessStatusPacket{
        int statusCode;
        std::string tableId;
        std::uint64_t workerUid;
    public:
        ProcessStatusPacket(json packet);
        ServerPacketCodes getPacketStatusCode();
        void execute();
};

#endif