#ifndef PROCESS_PKT_H
#define PROCESS_PKT_H

#include "../include/packet.hpp"
#include "../services/sql_access.hpp"
#include "../include/flag.h"

#ifndef JSON_H
#define JSON_H
#include "../lib/nlohmann/json-schema.hpp"
using json = nlohmann::json;
#endif

class ProcessDataPacket{
        std::uint8_t pDataType;
        std::string tableId;
        std::string data;
        std::string workerUid;
        int packetStausCode;
        SqlAccess *sqlAccess;
        enum packetDataType {
            INTERMEDIATE_RESULT,
            FINAL_RESULT,
            ERROR_DATA
        };
    public:
        ProcessDataPacket(json packet);
        void execute();
        void detectDataType();
        int createCsvFromData();
        int pushCsvToDb();
        void pushDataToDb();
};

class ProcessStatusPacket{
        int statusCode;
        std::string tableId;
        std::string workerUid;
        Flag statsPresent;
        double queueTime;
        std::string vectorString;
        int threadCount;
    public:
        ProcessStatusPacket(json packet);
        ServerPacketCodes getPacketStatusCode();
        void packetStatusParse();
};

#endif