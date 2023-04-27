#include "instance.hpp"
#include "../include/debug_rp.hpp"

Instance::Instance()
{
    sem_init(&instanceListLock, 0, 1);
    dataUpdated.initFlag();
}

int Instance::update(std::list<InstanceStruct*> instance)
{
    InstanceStruct *instanceStruct;

    if(instance.size() == 0){
        DEBUG_ERR(__func__, "instance list has 0 items");
        return 1;
    }
    sem_wait(&instanceListLock);
    for(auto i = instanceList.begin(); i != instanceList.end(); i++){
        instanceStruct = *i;
        delete instanceStruct;
    }
    for(auto i = instance.begin(); i != instance.end(); i++){
        instanceList.push_back(*i);
    }
    dataUpdated.setFlag();
    sem_post(&instanceListLock);

    DEBUG_MSG(__func__, "successfully updated list");

    return 0;
}

std::list<InstanceStruct*> Instance::getInstance()
{
    return instanceList;
}

std::list<json>* Instance::toJson()
{
    InstanceStruct* instance;
    std::list<json>* instancePacket = new std::list<json>;
    for(auto i = instanceList.begin(); i != instanceList.end(); i++)
    {
        instance = *i;
        json packet;
        packet["body"]["instanceid"] = instance->instanceName;
        packet["body"]["algotype"] = instance->algoType;
        packet["body"]["data"] = *instance->data;

        instancePacket->push_back(packet);
    }

    return instancePacket;
}

void Instance::resetFlag()
{
    dataUpdated.resetFlag();
}

bool Instance::getUpdateStatus()
{
    return dataUpdated.isFlagSet();
}