#include <fstream>
#include "../services/sql_access.hpp"
#include "../include/debug_rp.hpp"
#include "../worker_node/worker_registry.hpp"
#include "../configs.h"
#include "process_packet.hpp"
#include "out_data_registry.hpp"

ProcessDataPacket::ProcessDataPacket(json packet){
    tableId = packet["body"]["id"];
    data = packet["body"]["data"];
    packetStausCode = packet["head"];
    workerUid = packet["id"];
}

std::string ProcessDataPacket::getExtension()
{
    std::string extension;

    if(pDataType == INTERMEDIATE_RESULT)
        extension = "_INTR";
    else extension = "_RES";

    return extension;
}

void ProcessDataPacket::detectDataType()
{
    if(packetStausCode & P_INTR_RES){
        pDataType = INTERMEDIATE_RESULT;
    }else if(packetStausCode & P_FINAL_RES){
        pDataType = FINAL_RESULT;
    }else{
        pDataType = ERROR_DATA;
        DEBUG_ERR(__func__,"Packet does not match any Types");
    }
}

int ProcessDataPacket::createCsvFromData()
{
    std::ofstream outFile;
    
    if(data.length() == 0){
        DEBUG_ERR(__func__, "Data feild is empty");
        return -1;
    }

    outFile.open("temp/" + tableId + getExtension() +".csv", std::ios::out | std::ios::trunc);
    outFile << data;
    outFile.close();

    DEBUG_MSG(__func__, "created csv file successfully");
    return 0;
}

int ProcessDataPacket::pushCsvToDb()
{
    DEBUG_MSG(__func__, "pushing ", tableId, " results into database");
    globalSqlAccess.sqlWriteBlob(HOME_DIR + "temp/" + tableId + getExtension() + ".csv", USERDAT_DAT_COL_ID, tableId);
    return 0;
}

void ProcessDataPacket::cleanUp()
{
    std::filesystem::remove("temp/"+ tableId + getExtension() + ".csv");
}

void ProcessDataPacket::execute()
{
    if(!globalOutDataRegistry.findMatchInList(tableId)){
        DEBUG_ERR(__func__, "no table with tableID:", tableId, " found.");
        return;
    } 

    detectDataType();
    if(createCsvFromData()){
        DEBUG_ERR(__func__, "Aborting packet data processing");
    } else {
        pushCsvToDb();
        Worker *worker = globalWorkerRegistry.getWorkerFromUid(workerUid);
        if(!worker){
            DEBUG_ERR(__func__, "worker with wokerUid:", workerUid + 0, " not found");
            return;
        }
        cleanUp();
    }
}