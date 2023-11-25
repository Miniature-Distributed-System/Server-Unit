#include "worker_instance_list.hpp"
#include "../worker_node/worker_registry.hpp"
#include "../include/debug_rp.hpp"
#include "../include/logger.hpp"

void WorkerInstanceStruct::update(std::string templateId)
{
    for(auto i = templateIdList.begin(); i != templateIdList.end(); i++){
        if(!templateId.compare(*i)){
            templateIdList.erase(i);
            return;
        }
    }
}

void WorkerInstanceList::updateInstanceList(std::list<UserDataTemplateStruct> templateList)
{
    templateIdList.clear();
    for(auto i = templateList.begin(); i != templateList.end(); i++){
        templateIdList.push_back((*i).templateName);
    }
}

void WorkerInstanceList::addWorker(std::string workerUid)
{
    Worker *worker = globalWorkerRegistry.getWorkerFromUid(workerUid);
    if(!worker){
        Log().error(__func__, "cannot find worker with worker UID:", workerUid);
        return;
    }
    worker->resetWorkerReady();
    WorkerInstanceStruct workerInstanceStruct(workerUid, templateIdList);
    workerList.push_back(workerInstanceStruct);
    Log().info(__func__, "worker:", workerUid, " added to tracking list");
}

void WorkerInstanceList::updateWorker(std::string workerUid, std::string templateId)
{
    for(auto i = workerList.begin(); i != workerList.end(); i++){
        if(!workerUid.compare((*i).getWorkerUid())){
            (*i).update(templateId);
            // All instance packets were acknowledged
            if((*i).getWorkerInstanceListSize() == 0){
                Worker *worker = globalWorkerRegistry.getWorkerFromUid((*i).getWorkerUid());
                worker->setWorkerReady();
                workerList.erase(i);
                Log().info(__func__, "worker:", workerUid, " removed from tracking list");
            }
            return;
        }
    }
}