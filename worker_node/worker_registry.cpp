#include <random>
#include "../include/debug_rp.hpp"
#include "worker_registry.hpp"

WorkerRegistry::WorkerRegistry(){}

std::string WorkerRegistry::generateWorkerUid()
{
    Flag exit, failed;
    std::uint64_t computeNodeId;
    std::string finalComputeNodeId;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distr(1000, 9999);
    Worker *worker;

    exit.resetFlag();
    failed.initFlag();

    while(!exit.isFlagSet()){
        computeNodeId = distr(gen);
        finalComputeNodeId = std::to_string(computeNodeId);
        //Both dead and current worker list should not have this unique value regustered in them
        for(auto iter = currentWorkerList.begin(); iter != currentWorkerList.end(); iter++){
            if((*iter)->getWorkerUID() == finalComputeNodeId){
                failed.resetFlag();
                break;
            }
        }
        for(auto iter = deadWorkerList.begin(); iter != deadWorkerList.end(); iter++){
            if(*iter == finalComputeNodeId){
                failed.resetFlag();
                break;
            }
        }
        if(failed.isFlagSet())
            exit.setFlag();
    }
    
    worker = new Worker(finalComputeNodeId);
    currentWorkerList.push_front(worker);
    newWorker.setFlag();

    DEBUG_MSG(__func__, "compute unit assigned ID: ", finalComputeNodeId);
    return finalComputeNodeId;
}

std::list<OutPacket*> WorkerRegistry::deleteWorker(Worker *worker)
{
    std::list<OutPacket*> outPacket;
    std::string computeNodeId = worker->getWorkerUID();
    
    currentWorkerList.remove(worker);
    outPacket = worker->shutDown();
    delete worker;
    deadWorkerList.push_front(computeNodeId);
    DEBUG_MSG(__func__, "compute unit with ID", computeNodeId, " has been removed from active list");

    return outPacket;
}

bool WorkerRegistry::getNewWorkersStatus()
{
    if(newWorker.isFlagSet())
        return true;
    return false;
}

std::list<Worker*> WorkerRegistry::getWorkerList()
{
    return currentWorkerList;
}

Worker* WorkerRegistry::getWorkerFromUid(std::string workerUid)
{
    Worker* worker;
    for(auto i = currentWorkerList.begin(); i != currentWorkerList.end(); i++)
    {
        worker = *i;
        if(worker != NULL){
            if(worker->getWorkerUID() == workerUid)
                return worker;
        } 
    }
    DEBUG_ERR(__func__, "could not find worker with UID: ", workerUid);
    return NULL;
}