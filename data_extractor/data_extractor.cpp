#include <fstream>
#include <sstream>
#include "../sink/sink_stack.hpp"
#include "../include/debug_rp.hpp"
#include "../sender_unit/instance.hpp"
#include "../sender_unit/user_data.hpp"
#include "../packet_processor/out_data_registry.hpp"
#include "../include/logger.hpp"
#include "../configs.h"
#include "data_extractor.hpp"

TaskPriority getTaskPriority(int priority)
{
    switch(priority){
        case 0: return HIGH_PRIORITY;
        case 1: return MEDIUM_PRIORITY;
        case 2:
        default:  return LOW_PRIORITY;
    }
}

std::string* DataExtractor::getFileData(std::string fileName, bool isInstance)
{
    std::string *resultData, extractedData;
    std::ostringstream buf; 
    std::fstream dataFile;
    std::string dataDirectoryPath;
    
    if(isInstance)
        dataDirectoryPath = HOME_DIR + INSTANCE_FILE_DATA_DIR + fileName;
    else
        dataDirectoryPath = HOME_DIR + USER_FILE_DATA_DIR + fileName;

    dataFile.open(dataDirectoryPath, std::ios::in);
    if(dataFile){
        buf << dataFile.rdbuf();
        resultData = new std::string;
        *resultData = buf.str();
        resultData->erase(std::remove(resultData->begin(), resultData->end(), '\r'), resultData->end());

        return resultData;
    }
    Log().error(__func__, "No data was found in ", fileName);
    return NULL;
}

int DataExtractor::executeInstanceExtractor(std::list<std::string> idList, SqlAccess *sqlAccess)
{
    InstanceStruct instanceStruct;
    std::list<InstanceStruct> instanceList;
    int j = 0;

    for(auto i = idList.begin(); i != idList.end(); i++,j++)
    {
        std::string curInstanceName = *i, *resultData;
        if(curInstanceName.empty()) {
            Log().info(__func__, "instance id: ", j, " is empty");
            continue;
        }
        
        std::string csvFileNameQuery = "SELECT " + INSTANCE_FILE_COL_ID + " FROM " + INSTANCE_TABLE_NAME + " WHERE "
                        + INSTANCE_NAME_COL_ID + "='"+ curInstanceName +"';";
        std::string csvFileName = sqlAccess->sqlQueryDb(csvFileNameQuery, INSTANCE_FILE_COL_ID);
        resultData = getFileData(csvFileName, true);
        if(!resultData){
            Log().error(__func__, "fetching ", csvFileName, " failed for ", curInstanceName, " in ", INSTANCE_TABLE_NAME);
            continue;
        }

        std::string algoTypeQuery = "SELECT " + INSTANCE_ALGO_COL_ID + " FROM " + INSTANCE_TABLE_NAME + " WHERE "
                        + INSTANCE_NAME_COL_ID + "='"+ curInstanceName +"';";
        std::uint8_t algoType = sqlAccess->sqlQueryDbGetInt(algoTypeQuery, INSTANCE_ALGO_COL_ID);
        instanceStruct = InstanceStruct(curInstanceName, algoType, resultData);
        instanceList.push_back(instanceStruct);
    }

    Log().info(__func__, "instance data extraction done, total records:", instanceList.size());
    return globalInstanceRegistery.update(instanceList);
}

int DataExtractor::executeUserTableExtractor(std::list<std::string> userTableNameList, SqlAccess *sqlAccess)
{
    std::string csvFileNameQuery;
    std::string tableAlgoIdQuery, recordNameQuery, tablePriorityQuery;
    std::string *fileData;
    
    if(!userTableNameList.size()){
        Log().info(__func__, "No new user data");
        return 0;
    }

    for(auto i = userTableNameList.begin(); i != userTableNameList.end(); i++){
        Log().info(__func__,(*i));
    }

    for(auto i = userTableNameList.begin(); i != userTableNameList.end(); i++){
        std::string userTableName = *i;
        if(userTableName.empty()){
            Log().error(__func__, "Table fetch failed");
            continue;
        }
        Log().info(__func__, "pulling user table file data:", userTableName);
        
        tablePriorityQuery = "SELECT " + USERDAT_DAT_PRIORITY_COL_ID + " FROM " + USERDAT_TABLE_NAME + " WHERE " 
        + USERDAT_DAT_COL_ID + "='" + userTableName + "';";
        tableAlgoIdQuery = "SELECT " + USERDAT_ALGO_COL_ID + " FROM " + USERDAT_TABLE_NAME + " WHERE " 
        + USERDAT_DAT_COL_ID + "='" + userTableName + "';";
        recordNameQuery = "SELECT " + USERDAT_ALIASNAME_COL_ID + " FROM " + USERDAT_TABLE_NAME + " WHERE " 
        + USERDAT_DAT_COL_ID + "='" + userTableName + "';";

        std::string userRecordName = sqlAccess->sqlQueryDb(recordNameQuery);
        int userTablePriority = sqlAccess->sqlQueryDbGetInt(tablePriorityQuery);
        std::string userTableAlgo = sqlAccess->sqlQueryDb(tableAlgoIdQuery);

        fileData = getFileData(userTableName, false);
        Log().info(__func__, "Table Name:",userRecordName, "Instance Name:",  userTableAlgo," priority:" ,userTablePriority," algo:" ,userTableAlgo);
        UserDataTable *userDataTable = new UserDataTable(userTableName.c_str(), getTaskPriority(userTablePriority), userTableAlgo.c_str(), fileData);
        userDataTable->userTable = userRecordName;
        userDataTable->instanceName = userTableAlgo;
        userDataTable->data = fileData;
        globalSenderSink->pushObject(userDataTable, getTaskPriority(userTablePriority));
        globalOutDataRegistry.addTable(userRecordName);
    }
    Log().info(__func__, "pushed user tables from DB to sender stack");
    return 0;
}
