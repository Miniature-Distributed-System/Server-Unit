#include "userdata_template_registry.hpp"
#include "worker_instance_list.hpp"
#include "../include/debug_rp.hpp"
#include "../include/logger.hpp"

UserDataTemplate::UserDataTemplate()
{
    sem_init(&uDataTemplateListLock, 0, 1);
    uDataTemplateJsonList = new std::list<json>;
    dataUpdated.initFlag();
}

/*
  update: This adds the UserDataTemplateStruct DS to the list
  The method goes through all structs and compares if a instance of this already exists
  then the don't add that entry else add the entry. itemPresent flag is set if it already
  exists in the list which causes the DS to be skipped.
  It generates the json for the data in DS. This data is exported to worker
*/
int UserDataTemplate::update(std::list<UserDataTemplateStruct> templateStruct)
{
    Flag itemPresent;

    if(templateStruct.size() == 0){
        Log().error(__func__, "instance list is empty");
        return EXIT_FAILURE;
    }

    sem_wait(&uDataTemplateListLock);
    // Discard instances that don't exist 
    for(auto i = uDataTemplateList.begin(); i != uDataTemplateList.end(); i++){
        UserDataTemplateStruct instanceListItem = *i;
        itemPresent.initFlag(true);
        for(auto j = templateStruct.begin(); j != templateStruct.end(); j++){
            // If true then instance already exists in list no need to remake it
            if(!(*j).templateName.compare(instanceListItem.templateName)){
                itemPresent.setFlag();
                break;
            }
        }
        if(!itemPresent.isFlagSet()) uDataTemplateList.erase(i--);
    }

    // This loop adds new instance items which aren't in the list
    for(auto i = templateStruct.begin(); i != templateStruct.end(); i++){
        itemPresent.initFlag(false);
        for(auto j = uDataTemplateList.begin(); j != uDataTemplateList.end(); j++){
            if(!(*i).templateName.compare((*j).templateName)){
                itemPresent.setFlag();
                break;
            }
        }
        if(!itemPresent.isFlagSet()){
            uDataTemplateList.push_back(*i);
        }
    }

    // Generate json for the whole updated list
    uDataTemplateJsonList->clear();
    for(auto i = uDataTemplateList.begin(); i != uDataTemplateList.end(); i++){
        UserDataTemplateStruct instance = *i;
        json packet;

        Log().info(__func__,"pushing instance ID: ", instance.templateName, " into list");
        packet["body"]["instanceid"] = instance.templateName;
        packet["body"]["algotype"] = instance.algoType;
        packet["body"]["data"] = *instance.data;
        
        uDataTemplateJsonList->push_back(packet);
    }
    // Set flag so the sender core can update workers in pool with new instance data
    dataUpdated.setFlag();
    workerInstanceList.updateInstanceList(uDataTemplateList);
    sem_post(&uDataTemplateListLock);

    Log().info(__func__, "successfully updated instance list with depth:", uDataTemplateJsonList->size());

    return 0;
}

/*
  get: This fetches the copy of user DataTemplateList
*/
std::list<UserDataTemplateStruct> UserDataTemplate::get()
{
    return uDataTemplateList;
}

/*
  toJson: Takes each entry of list and converts them into json and returns a json list
*/
std::list<json> UserDataTemplate::toJson()
{
    std::list<json> outJsonList;
    for(auto i = uDataTemplateJsonList->begin(); i != uDataTemplateJsonList->end(); i++)
        outJsonList.push_back(*i);
    return outJsonList;
}

/*
  resetFlag: It resets the data updated flag which tells if list was recently updated or not
*/
void UserDataTemplate::resetFlag()
{
    dataUpdated.resetFlag();
}

/*
  resetFlag: It gets the data updated flag which tells if list was recently updated or not
*/
bool UserDataTemplate::getUpdateStatus()
{
    return dataUpdated.isFlagSet();
}

/*
  isMatchingFound: Is used to check if the template ID exists in the list already.
  Usually used at the fringes where the packets are verfied to see if what worker
  sent is valid or not aka the result it gave isnt duplicate or data associated with
  it is no more present. This prevents us from doing any invalid access.
*/
bool UserDataTemplate::isMatchingFound(std::string templateId)
{
    sem_wait(&uDataTemplateListLock);
    for(auto i = uDataTemplateList.begin(); i != uDataTemplateList.end(); i++){
        if(!templateId.compare((*i).templateName)){
            sem_post(&uDataTemplateListLock);
            return true;
        }
    }
    sem_post(&uDataTemplateListLock);
    
    return false;
}
