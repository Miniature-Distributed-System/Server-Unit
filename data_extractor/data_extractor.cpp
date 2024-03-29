#include <fstream>
#include <sstream>
#include "../sink/sink_stack.hpp"
#include "../include/debug_rp.hpp"
#include "../sender_unit/userdata_template_registry.hpp"
#include "../sender_unit/user_data.hpp"
#include "../packet_processor/outgoing_data_registry.hpp"
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

/*
  getFileData: This Method fetches the data blob from the file as is for the provided filename.
  The fileName has full name with extension. The isInstance specifies if its a Template data or 
  User data as they are stored seperately in different directories. If file is not found then
  the Error is logged and returns NULL string.
*/
std::string* DataExtractor::getFileData(std::string fileName, bool isInstance)
{
    std::string *resultData, extractedData;
    std::ostringstream buf; 
    std::fstream dataFile;
    std::string dataDirectoryPath;
    
    if(isInstance)
        dataDirectoryPath = HOME_DIR + TEMPLATE_FILE_DATA_DIR + fileName;
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

/*
  executeInstanceExtractor: It extracts the Template table data for the given list of Template Names.
  The Template/Rule names serves as Primary key using which we can pull the Template Data associated 
  with it. The Monitor gives a list of added/modified Rules and it pulls all data for all these names.
  The Sql Access is used to access the SQL Server Database and access the data.
  If in any case record for the Name supplied doesnt exist the error is logged and continues with rest
  of the names.
  UserDataTemplateStruct DS is created for the pulled data and queued into InstanceList.
  UserDataTemplateRegistry is updated and Upper layers handle the rest.
*/
int DataExtractor::executeInstanceExtractor(std::list<std::string> idList, SqlAccess *sqlAccess)
{
    UserDataTemplateStruct instanceStruct;
    std::list<UserDataTemplateStruct> instanceList;
    int j = 0;

    for(auto i = idList.begin(); i != idList.end(); i++,j++)
    {
        std::string curInstanceName = *i, *resultData;
        if(curInstanceName.empty()) {
            Log().info(__func__, "instance id: ", j, " is empty");
            continue;
        }
        
        std::string csvFileNameQuery = "SELECT " + TEMPLATE_FILE_COL_ID + " FROM " + TEMPLATE_TABLE_NAME + " WHERE "
                        + TEMPLATE_NAME_COL_ID + "='"+ curInstanceName +"';";
        std::string csvFileName = sqlAccess->sqlQueryDb(csvFileNameQuery, TEMPLATE_FILE_COL_ID);
        resultData = getFileData(csvFileName, true);
        if(!resultData){
            Log().error(__func__, "fetching ", csvFileName, " failed for ", curInstanceName, " in ", TEMPLATE_TABLE_NAME);
            continue;
        }

        std::string algoTypeQuery = "SELECT " + TEMPLATE_ALGO_COL_ID + " FROM " + TEMPLATE_TABLE_NAME + " WHERE "
                        + TEMPLATE_NAME_COL_ID + "='"+ curInstanceName +"';";
        std::uint8_t algoType = sqlAccess->sqlQueryDbGetInt(algoTypeQuery, TEMPLATE_ALGO_COL_ID);
        instanceStruct = UserDataTemplateStruct(curInstanceName, algoType, resultData);
        instanceList.push_back(instanceStruct);
    }

    Log().info(__func__, "instance data extraction done, total records:", instanceList.size());
    return globalUserDataTemplateRegistry.update(instanceList);
}

/*
  executeUserTableExtractor: It extracts the User Data table data for the given list of User Data Names.
  The User Data names serves as Primary key using which we can pull the User Data associated with it. 
  The Monitor gives a list of added/modified User data and it pulls all data for all these names. These 
  user data are data that needs processing using the associated rules and algorithm. The Sql Access is 
  used to access the SQL Server Database and access the data. If in any case record for the Name supplied 
  doesnt exist the error is logged and continues with rest of the names.
  
  UserDataTable DS is created for the pulled data and queued into SenderSink.
  OutgoingDataRegistry is updated and Upper layers handle the rest.
*/
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
        globalOutgoingDataRegistry.add(userRecordName);
    }
    Log().info(__func__, "pushed user tables from DB to sender stack");
    return 0;
}
