#include "instance.hpp"
#include "worker_instance_list.hpp"
#include "../include/debug_rp.hpp"

Instance::Instance()
{
    sem_init(&instanceListLock, 0, 1);
    instanceJsonList = new std::list<json>;
    dataUpdated.initFlag();
}

int Instance::update(std::list<InstanceStruct> instance)
{
    InstanceStruct instanceStruct;
    Flag itemPresent;

    if(instance.size() == 0){
        DEBUG_ERR(__func__, "instance list is empty");
        return EXIT_FAILURE;
    }

    sem_wait(&instanceListLock);
    // Discard instances that don't exist 
    for(auto i = instanceList.begin(); i != instanceList.end(); i++){
        InstanceStruct instanceListItem = *i;
        itemPresent.initFlag(true);
        for(auto j = instance.begin(); j != instance.end(); j++){
            // If true then instance already exists in list no need to remake it
            if(!(*j).instanceName.compare(instanceListItem.instanceName)){
                itemPresent.setFlag();
                break;
            }
        }
        if(!itemPresent.isFlagSet()) instanceList.erase(i--);
    }

    // This loop adds new instance items which aren't in the list
    for(auto i = instance.begin(); i != instance.end(); i++){
        itemPresent.initFlag(false);
        for(auto j = instanceList.begin(); j != instanceList.end(); j++){
            if(!(*i).instanceName.compare((*j).instanceName)){
                itemPresent.setFlag();
                break;
            }
        }
        if(!itemPresent.isFlagSet()){
            instanceList.push_back(*i);
        }
    }

    // Generate json for the whole updated list
    instanceJsonList->clear();
    for(auto i = instanceList.begin(); i != instanceList.end(); i++){
        InstanceStruct instance = *i;
        json packet;

        DEBUG_MSG(__func__,"pushing instance ID: ", instance.instanceName, " into list");
        packet["body"]["instanceid"] = instance.instanceName;
        packet["body"]["algotype"] = instance.algoType;
        packet["body"]["data"] = *instance.data;
        
        instanceJsonList->push_back(packet);
    }
    // Set flag so the sender core can update workers in pool with new instance data
    dataUpdated.setFlag();
    workerInstanceList.updateInstanceList(instanceList);
    sem_post(&instanceListLock);

    DEBUG_MSG(__func__, "successfully updated instance list with depth:", instanceJsonList->size());

    return 0;
}

std::list<InstanceStruct> Instance::getInstance()
{
    return instanceList;
}

std::list<json> Instance::toJson()
{
    std::list<json> outJsonList;
    for(auto i = instanceJsonList->begin(); i != instanceJsonList->end(); i++)
        outJsonList.push_back(*i);
    return outJsonList;
}

void Instance::resetFlag()
{
    dataUpdated.resetFlag();
}

bool Instance::getUpdateStatus()
{
    return dataUpdated.isFlagSet();
}