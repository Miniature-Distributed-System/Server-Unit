#include "out_data_registry.hpp"
#include "../include/flag.h"
#include "../include/debug_rp.hpp"
#include "../services/sql_access.hpp"

int OutDataRegistry::addTable(std::string dataTableName, Worker *worker)
{
    OutDataState *outData = NULL;
    Flag resultStatus;

    for(auto i = outDataRegistryList.begin(); i != outDataRegistryList.end(); i++){
        if(dataTableName == (*i)->id){
            resultStatus.setFlag();
            break;
        }
    }

    if(!resultStatus.isFlagSet()){
        DEBUG_MSG(__func__, "New table :", dataTableName, " added to list");
        outData = new OutDataState(dataTableName, DATA_QUEUED);
        if(worker)
            outData->worker = worker;
        outDataRegistryList.push_back(outData);
    }
    
    return 0;
}

int OutDataRegistry::deleteTable(std::string dataTableName)
{
    for(auto i = outDataRegistryList.begin(); i != outDataRegistryList.end(); i++){
        if((*i)->id == dataTableName){
            DEBUG_MSG(__func__, "table: ", dataTableName, " successfully popped from list");
            outDataRegistryList.erase(i--);
            delete (*i);
            return 0;
        }
    }

    DEBUG_ERR(__func__, "Popping table: ", dataTableName, " failed as not found in list");
    return -1;
}

bool OutDataRegistry::findMatchInList(std::string dataTableName)
{
    for(auto i = outDataRegistryList.begin(); i != outDataRegistryList.end(); i++){
        if((*i)->id == dataTableName){
            DEBUG_MSG(__func__, "found match for table name: ", dataTableName);
            return true;
        }
    }

    DEBUG_ERR(__func__, "found no match for table name:", dataTableName);
    return false;
}

bool OutDataRegistry::assignWorker(std::string id, Worker *worker)
{
    OutDataState *outDataState;
    for(auto i = outDataRegistryList.begin(); i != outDataRegistryList.end(); i++){
        outDataState = *i;
        if(outDataState->id == id){
            DEBUG_MSG(__func__, "updating status for table name: ", id);
            outDataState->worker = worker;
            return true;
        }
    }

    DEBUG_ERR(__func__, "found no match for table name:", id, " failed to update status");
    return false;
}

std::string userStatusEnumToString(UserTaskStatus status)
{
    switch(status){
        case DATA_QUEUED: return "Data Queued";
        case DATA_READY: return "Data Ready";
        case DATA_SENT: return "Data Sent";
        case DATA_INTER: return "Intermediate Data Received";
        case DATA_FINAL: return "Received Final Data";
        default: "";
    }
}

void updateStatusInDb(std::string result, std::string dataTableName)
{
    SqlAccess *sqlAccess = new SqlAccess(DATABASE_URL, DATABASE_USERNAME, DATABASE_PASSWORD, DATABASE_NAME, 
                            USERDAT_TABLE_NAME);
    sqlAccess->initialize();
    sqlAccess->sqlWriteString(result, USERDAT_STATUS_COL_ID, USERDAT_ALIASNAME_COL_ID, dataTableName);
    delete sqlAccess;
}

int OutDataRegistry::updateTaskStatus(std::string dataTableName, UserTaskStatus status)
{
    OutDataState *outDataState;
    for(auto i = outDataRegistryList.begin(); i != outDataRegistryList.end(); i++){
        outDataState = *i;
        if(outDataState->id == dataTableName){
            DEBUG_MSG(__func__, "updating status for table name: ", dataTableName, " with state: ", status);
            std::string result = userStatusEnumToString(status);
            if(!result.empty())
                updateStatusInDb(result, dataTableName);
            outDataState->taskStatus = status;
            return true;
        }
    }

    DEBUG_ERR(__func__, "found no match for table name:", dataTableName, " failed to update status");
    return false;
}

std::list<OutDataState*> OutDataRegistry::getOutDataRegistryList()
{
    return outDataRegistryList;
}

OutDataState* OutDataRegistry::getOutDataRegistryFromId(std::string id)
{
    OutDataState *outDataState;
    for(auto i = outDataRegistryList.begin(); i != outDataRegistryList.end(); i++){
        outDataState = *i;
        if(outDataState->id == id){
            return *i;
        }
    }

    DEBUG_ERR(__func__, "found no match for table name:", id, " failed to update status");
    return NULL;    
}