#include <list>
#include "../include/process.hpp"
#include "../include/debug_rp.hpp"
#include "../sched/task_pool.hpp"
#include "../sink/sink_stack.hpp"
#include "../lib/nlohmann/json-schema.hpp"
#include "../worker_node/worker_registry.hpp"
#include "../data_extractor/data_extractor.hpp"
#include "../packet_processor/out_data_registry.hpp"
#include "../packet_processor/packet_constructor.hpp"
#include "../include/logger.hpp"
#include "sender_core.hpp"
#include "instance.hpp"
#include "user_data.hpp"
#include "worker_instance_list.hpp"

using nlohmann::json_schema::json_validator;
using json = nlohmann::json;

SenderCoreData::SenderCoreData()
{
    newWorkerList = new std::list<std::string>;
    pendingPacketsList = new std::list<OutPacket*>;
    Log().senderCoreInfo(__func__, "inited sender core data");
}

void SenderCoreData::addWorker(std::string workerUid)
{
    newWorkerList->push_back(workerUid);
    Log().senderCoreInfo(__func__, "added worker for instance update depth:", newWorkerList->size());
}

void SenderCoreData::addPackets(OutPacket *outData)
{
    pendingPacketsList->push_back(outData);
}

bool SenderCoreData::isNewWorkerListEmpty()
{
    if(newWorkerList->size() <= 0)
        return true;
    return false;
}

bool SenderCoreData::isPendingPacketsListEmpty()
{
    if(pendingPacketsList->size() > 0)
        return false;
    return true;
}

std::list<std::string>* SenderCoreData::getWorkerList()
{
    return newWorkerList;
}

std::list<OutPacket*>* SenderCoreData::getPendingPacketsList()
{
    return pendingPacketsList;
}

Worker* findIdealWorker()
{
    std::list<Worker*> workerList = globalWorkerRegistry.getWorkerList();
    Worker *worker, *resWorker;
    int hSize = 0;
    std::string hWorkerUid = 0;

    for(auto i = workerList.begin(); i != workerList.end(); i++)
    {
        worker = *i;
        if(worker == NULL){
            Log().error(__func__, "Worker not found");
            continue;
        }
            Log().senderCoreInfo(__func__, "tempPoints: ", tempPoints, " worker:", worker->getWorkerUID());
        }
    }

    if(hWorkerUid.empty()){
        //Log().error(__func__, "all workers are full");
        return NULL;
    }

    resWorker = globalWorkerRegistry.getWorkerFromUid(hWorkerUid);
    Log().senderCoreInfo(__func__, "worker with emptiest queue: ", finalWorkerUid);

    return resWorker;
}

void pushUserDataToWorkerQueue()
{
    Worker *worker;
    ExportSinkItem sinkItem;
    UserDataTable* userData;
    json packet;

    worker = findIdealWorker();
    if(worker == NULL){
        return;
    }

    sinkItem = globalSenderSink->popObject();
    userData = (UserDataTable*)sinkItem.dataObject;
    //Log().senderCoreInfo(__func__,"Table Name:",userData->userTable ,"Instance name:", userData->instanceName, " priority: ", userData->priority);
    packet = userData->toJson();
    worker->queuePacket(
        new OutPacket(
            PacketConstructor().create(SP_DATA_SENT, worker->getWorkerUID(), packet)
            , userData->userTable, true)
        );
    globalOutDataRegistry.assignWorker(userData->userTable, worker);
    globalOutDataRegistry.updateTaskStatus(userData->userTable, DATA_READY);
    Log().senderCoreInfo(__func__, "sender sink packet pushed to worker:", worker->getWorkerUID());
}

void pushInstanceToWorkerQueue(std::list<std::string> *workerList)
{
    std::string workerUID;
    Worker* worker;
    std::string tableId;
    std::list<json>* instanceJson = globalInstanceRegistery.toJson();

    Log().senderCoreInfo(__func__, " worker list size:", workerList->size());
    for(auto i = workerList->begin(); i != workerList->end(); i++)
    {
        workerUID = *i;
        worker = globalWorkerRegistry.getWorkerFromUid(workerUID);
        if(worker == NULL){
            Log().debug(__func__, "could not find worker, may have disconnected/timedout");
            workerList->erase(i--);
            continue;
        }
        for(auto j = instanceJson->begin(); j != instanceJson->end(); j++)
        {
            tableId = (*j)["body"]["instanceid"];
            Log().senderCoreInfo(__func__, "queuing packet with instance ID:", tableId);
            globalOutDataRegistry.addTable(tableId, worker);
            worker->queuePacket(
                new OutPacket(
                    PacketConstructor().create(SP_DATA_SENT, worker->getWorkerUID(), *j)
                    , tableId, true)
                );
            globalOutDataRegistry.updateTaskStatus(tableId, DATA_READY);
        Log().senderCoreInfo(__func__, "queued packet into worker UID: ", workerUID);
    Log().senderCoreInfo(__func__, "cleared pending workerList");
    Log().senderCoreInfo(__func__, "worker list size:", workers.size());
        }
        DEBUG_MSG(__func__, "queued packet into worker UID: ", workerUID);
    DEBUG_MSG(__func__, "cleared pending workerList");
    }
    Log().senderCoreInfo(__func__, "all workers instances updated");
}

int pushPendingPackets(std::list<OutPacket*>* outPacketList)
{
    Worker *worker = findIdealWorker();
    
    while(outPacketList->size() > 0){
        worker->queuePacket(outPacketList->front());
        outPacketList->pop_front();
        worker = findIdealWorker();
        if(!worker){
            return -1;
        }
    }

    return 0;
}

JobStatus startSenderCore(void *data)
{
    int rc = 0;
    SenderCoreData *senderData = (SenderCoreData*)data;

    if(!senderData->isPendingPacketsListEmpty()){
        rc = pushPendingPackets(senderData->getPendingPacketsList());
        if(rc){
            Log().senderCoreInfo(__func__, "all worker queues are filled waiting to free-up or no worker present");
            return JOB_PENDING;
        }
        Log().senderCoreInfo(__func__, "pending packets pushed to queue");
    }

    if(!senderData->isNewWorkerListEmpty()){
        pushInstanceToWorkerQueue(senderData->getWorkerList());
    } else {
        //Pull the packet from sender sink to be sent to the ideal worker
        pushUserDataToWorkerQueue();
    }

    return JOB_PENDING;
}
JobStatus pauseSenderCore(void *data)
{
    return JOB_PENDING;
}
JobStatus endSenderCore(void *data, JobStatus status)
{
    Log().senderCoreInfo(__func__, "sender instance is shutting down");
    return JOB_DONE;
}

struct process *senderCoreProcess = new process{
    .start_proc = startSenderCore,
    .pause_proc = pauseSenderCore,
    .end_proc = endSenderCore
};

int SenderCore::run()
{
    globalTaskPool->addTask(new taskStruct(senderCoreProcess, senderCoreData), LOW_PRIORITY);
    Log().senderCoreInfo(__func__,"sender core pushed to task queue");
    return 0;
}