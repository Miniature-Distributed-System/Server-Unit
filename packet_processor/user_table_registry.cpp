#include "user_table_registry.hpp"
#include "../include/flag.h"
#include "../include/debug_rp.hpp"

std::list<std::string> OutDataRegistry::pushList(std::list<std::string> userTableList)
{
    std::list<std::string> resultList;
    OutDataState *userTable = NULL;
    Flag resultStatus;
    for(auto j = userTableList.begin(); j != userTableList.end(); j++){
        resultStatus.initFlag();
        for(auto i = userTableRegistryList.begin(); i != userTableRegistryList.end(); i++){
            if(*j == (*i)->userTableName){
                resultStatus.setFlag();
                break;
            }
        }
        if(!resultStatus.isFlagSet()){
            DEBUG_MSG(__func__, "New table :", *j, " added to list");
            userTable = new OutDataState(*j, DATA_QUEUED);
            resultList.push_back(*j);
        }
    }
    
    return resultList;
}

int OutDataRegistry::popList(std::string userTableName)
{
    for(auto i = userTableRegistryList.begin(); i != userTableRegistryList.end(); i++){
        if((*i)->userTableName == userTableName){
            DEBUG_MSG(__func__, "table: ", userTableName, " successfully popped from list");
            userTableRegistryList.remove(*i);
            delete (*i);
            return 0;
        }
    }

    DEBUG_ERR(__func__, "Popping table: ", userTableName, " failed as not found in list");
    return -1;
}

bool OutDataRegistry::findMatchInList(std::string userTableName)
{
    for(auto i = userTableRegistryList.begin(); i != userTableRegistryList.end(); i++){
        if((*i)->userTableName == userTableName){
            DEBUG_MSG(__func__, "found match for table name: ", userTableName);
            return true;
        }
    }

    DEBUG_ERR(__func__, "found no match for table name:", userTableName);
    return false;
}

int OutDataRegistry::updateTaskStatus(std::string userTableName, UserTaskStatus status)
{
    for(auto i = userTableRegistryList.begin(); i != userTableRegistryList.end(); i++){
        if((*i)->userTableName == userTableName){
            DEBUG_MSG(__func__, "updating status for table name: ", userTableName, " with state: ", status);
            (*i)->taskStatus = status;
            return true;
        }
    }

    DEBUG_ERR(__func__, "found no match for table name:", userTableName, " failed to update status");
    return false;
}

bool OutDataRegistry::assignWorker(std::string tableName, Worker *worker)
{
    for(auto i = userTableRegistryList.begin(); i != userTableRegistryList.end(); i++){
        if((*i)->userTableName == tableName){
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
    for(auto i = userTableRegistryList.begin(); i != userTableRegistryList.end(); i++){
        if((*i)->userTableName == tableName){
            return (*i)->worker->getWorkerUID();
        }
    }

    DEBUG_ERR(__func__, "found no match for table name:", tableName, " get Worker UID");
    return -1;
}