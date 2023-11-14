#include "out_data_registry.hpp"
#include "../include/flag.h"
#include "../include/debug_rp.hpp"
#include "../services/sql_access.hpp"
#include "../include/logger.hpp"

int OutgoingDataRegistry::add(std::string dataTableName, Worker *worker)
{
    OutgoingDataState *outData = NULL;
    Flag resultStatus;

    for(auto i = outDataRegistryList.begin(); i != outDataRegistryList.end(); i++){
        if(dataTableName == (*i)->id){
            resultStatus.setFlag();
            break;
        }
    }

    if(!resultStatus.isFlagSet()){
        Log().info(__func__, "New table :", dataTableName, " added to list");
        outData = new OutgoingDataState(dataTableName, DATA_QUEUED);
        if(worker)
            outData->worker = worker;
        outDataRegistryList.push_back(outData);
    }
    
    return 0;
}

int OutgoingDataRegistry::delete(std::string dataTableName)
{
    for(auto i = outDataRegistryList.begin(); i != outDataRegistryList.end(); i++){
        if((*i)->id == dataTableName){
            Log().info(__func__, "table: ", dataTableName, " successfully popped from list");
            outDataRegistryList.erase(i--);
            delete (*i);
            return 0;
        }
    }

    Log().debug(__func__, "Popping table: ", dataTableName, " failed as not found in list");
    return -1;
}

bool OutgoingDataRegistry::findMatchInList(std::string dataTableName)
{
    for(auto i = outDataRegistryList.begin(); i != outDataRegistryList.end(); i++){
        if((*i)->id == dataTableName){
            Log().info(__func__, "found match for table name: ", dataTableName);
            return true;
        }
    }

    Log().error(__func__, "found no match for table name:", dataTableName);
    return false;
}

bool OutgoingDataRegistry::assignWorker(std::string id, Worker *worker)
{
    OutgoingDataState *outDataState;
    for(auto i = outDataRegistryList.begin(); i != outDataRegistryList.end(); i++){
        outDataState = *i;
        if(outDataState->id == id){
            Log().info(__func__, "updating status for table name: ", id);
            outDataState->worker = worker;
            return true;
        }
    }

    Log().error(__func__, "found no match for table name:", id, " failed to update status");
    return false;
}

std::string userStatusEnumToString(UserTaskStatus status)
{
    switch(status){
        case DATA_QUEUED: return "Data has been queued";
        case DATA_READY: return "Searching for free workers";
        case DATA_SENT: return "Data has been sent to worker";
        case DATA_INTER: return "Intermediate data received successfully";
        case DATA_FINAL: return "Success";
        case DATA_ERROR: return "Error in uploaded data";
        default: "";
    }
}

void updateStatusInDb(std::string result, std::string dataTableName)
{
    SqlAccess *sqlAccess = new SqlAccess(DATABASE_URL, DATABASE_USERNAME, DATABASE_PASSWORD, DATABASE_NAME, 
                            USERDAT_TABLE_NAME);
    sqlAccess->initialize();
    sqlAccess->sqlWriteString(result, USERDAT_STATUS_COL_ID, USERDAT_ALIASNAME_COL_ID, dataTableName);
    Log().info(__func__, "updating status for table name: ", dataTableName, " with state: ", result);
    delete sqlAccess;
}

int OutgoingDataRegistry::updateTaskStatus(std::string dataTableName, UserTaskStatus status)
{
    OutgoingDataState *outDataState;
    for(auto i = outDataRegistryList.begin(); i != outDataRegistryList.end(); i++){
        outDataState = *i;
        if(outDataState->id == dataTableName){
            if(status < outDataState->taskStatus)
                return true;
            std::string result = userStatusEnumToString(status);
            if(!result.empty())
                updateStatusInDb(result, dataTableName);
            else Log().error(__func__, "unknown status code:", status);
            outDataState->taskStatus = status;
            return true;
        }
    }

    Log().error(__func__, "found no match for table name:", dataTableName, " failed to update status");
    return false;
}

std::list<OutgoingDataState*> OutgoingDataRegistry::list()
{
    return outDataRegistryList;
}

OutgoingDataState* OutgoingDataRegistry::getRegistryFromId(std::string id)
{
    OutgoingDataState *outDataState;
    for(auto i = outDataRegistryList.begin(); i != outDataRegistryList.end(); i++){
        outDataState = *i;
        if(outDataState->id == id){
            return *i;
        }
    }

    Log().error(__func__, "found no match for table name:", id, " failed to update status");
    return NULL;    
}