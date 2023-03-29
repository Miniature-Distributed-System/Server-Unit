#include <random>
#include "../include/debug_rp.hpp"
#include "worker_registry.hpp"

int WorkerRegistry::generateWorkerUid()
{
    Flag exit, failed;
    std::uint64_t computeNodeId;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distr(1000, 9999);
    Worker *worker;

    exit.resetFlag();
    failed.initFlag();

    while(!exit.isFlagSet()){
        computeNodeId = distr(gen);
        //Both dead and current worker list should not have this unique value regustered in them
        for(auto iter = currentWorkerList.begin(); iter != currentWorkerList.end(); iter++){
            if((*iter)->getWorkerUID() == computeNodeId){
                failed.resetFlag();
                break;
            }
        }
        for(auto iter = deadWorkerList.begin(); iter != deadWorkerList.end(); iter++){
            if(*iter == computeNodeId){
                failed.resetFlag();
                break;
            }
        }
        if(failed.isFlagSet())
            exit.setFlag();
    }

    worker = new Worker(computeNodeId);
    currentWorkerList.push_front(worker);
    newWorker.setFlag();

    DEBUG_MSG(__func__, "compute unit assigned ID: ", computeNodeId);
    return computeNodeId;
}

void WorkerRegistry::deleteWorker(Worker *worker)
{
    std::uint64_t computeNodeId = worker->getWorkerUID();
    currentWorkerList.remove(worker);
    delete worker;
    deadWorkerList.push_front(computeNodeId);
    DEBUG_MSG(__func__, "compute unit with ID", computeNodeId, " has been removed from active list");
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