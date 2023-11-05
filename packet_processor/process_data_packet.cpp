#include <fstream>
#include "../include/debug_rp.hpp"
#include "../include/logger.hpp"
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

void ProcessDataPacket::detectDataType()
{
    if(packetStausCode & P_INTR_RES){
        pDataType = INTERMEDIATE_RESULT;
    }else if(packetStausCode & P_FINAL_RES){
        pDataType = FINAL_RESULT;
    }else{
        pDataType = ERROR_DATA;
        Log().debug(__func__,"Packet does not match any Types");
    }
}

int ProcessDataPacket::createCsvFromData()
{
    std::ofstream outFile;
    
    if(data.length() == 0){
        Log().debug(__func__, "Data feild is empty");
        return -1;
    }

    if(pDataType == INTERMEDIATE_RESULT)
        outFile.open(HOME_DIR + INTER_RESULT_DATA_DIR + tableId + ".csv", std::ios::out | std::ios::trunc);
    else
        outFile.open(HOME_DIR + FINAL_RESULT_DATA_DIR + tableId + ".csv", std::ios::out | std::ios::trunc);
    outFile << data;
    outFile.close();

    Log().pktProcessorInfo(__func__, "created csv file for table: ", tableId, " successfully");
    return 0;
}

void ProcessDataPacket::pushDataToDb()
{
    sqlAccess = new SqlAccess(DATABASE_URL, DATABASE_USERNAME, DATABASE_PASSWORD, DATABASE_NAME, 
                    USERDAT_TABLE_NAME);
    sqlAccess->initialize();
    Log().pktProcessorInfo(__func__, "pushing ", tableId, " final results into database");
    sqlAccess->sqlWriteString(data, USERDAT_RES_COL_ID, USERDAT_ALIASNAME_COL_ID, tableId);
    delete sqlAccess;
}

void ProcessDataPacket::execute()
{
    if(!globalOutDataRegistry.findMatchInList(tableId)){
        Log().error(__func__, "no table with tableID:", tableId, " found.");
        return;
    } 

    detectDataType();
    if(createCsvFromData()){
        Log().error(__func__, "Aborting packet data processing");
        return;
    }

    if(pDataType != INTERMEDIATE_RESULT){
        pushDataToDb();
    }
    Log().pktProcessorInfo(__func__, "Finished processing ", tableId," data");
}