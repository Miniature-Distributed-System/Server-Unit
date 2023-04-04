#include "out_data_registry.hpp"
#include "../include/flag.h"
#include "../include/debug_rp.hpp"

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
            outDataRegistryList.remove(*i);
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

int OutDataRegistry::updateTaskStatus(std::string dataTableName, UserTaskStatus status)
{
    OutDataState *outDataState;
    for(auto i = outDataRegistryList.begin(); i != outDataRegistryList.end(); i++){
        outDataState = *i;
        if(outDataState->id == dataTableName){
            DEBUG_MSG(__func__, "updating status for table name: ", dataTableName, " with state: ", status);
            outDataState->taskStatus = status;
            if(status == DATA_SENT || status == DATA_READY)
                outDataState->worker->checkIn();
            return true;
        }
    }

    DEBUG_ERR(__func__, "found no match for table name:", dataTableName, " failed to update status");
    return false;
}

bool OutDataRegistry::assignWorker(std::string tableName, Worker *worker)
{
    for(auto i = outDataRegistryList.begin(); i != outDataRegistryList.end(); i++){
        if((*i)->id == tableName){
            (*i)->worker = worker;
            DEBUG_MSG(__func__, "assigned worker", worker->getWorkerUID(), " with tableId", tableName);
            return true;
        }
    }

    DEBUG_ERR(__func__, "found no match for table name:", tableName);
    return false;
}

long long int OutDataRegistry::getWorkerUid(std::string tableName)
{
    for(auto i = outDataRegistryList.begin(); i != outDataRegistryList.end(); i++){
        if((*i)->id == tableName){
            return (*i)->worker->getWorkerUID();
        }
    }

    DEBUG_ERR(__func__, "found no match for table name:", tableName, " get Worker UID");
    return -1;
}