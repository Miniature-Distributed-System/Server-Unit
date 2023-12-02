#include <list>
#include "../include/process.hpp"
#include "../include/debug_rp.hpp"
#include "../sched/task_pool.hpp"
#include "../sink/sink_stack.hpp"
#include "../lib/nlohmann/json-schema.hpp"
#include "../worker_node/worker_registry.hpp"
#include "../data_extractor/data_extractor.hpp"
#include "../packet_processor/outgoing_data_registry.hpp"
#include "../packet_processor/packet_constructor.hpp"
#include "../include/logger.hpp"
#include "sender_core.hpp"
#include "userdata_template_registry.hpp"
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


/* findIdealWorker(): Finds the ideal worker from list of workers to send packet.
 * Desc: This method takes the top most task's priority and the list workers. The method iterates over the list of
 * workers to find the best worker using their stats. The workers is checked if its ready for transmission first, then
 * the worker individual points are calculated.
 * The avgQueueTime takes the queuing time of worker and multiplies it with the taskMapVector elements. The
 * taskMapVector vector contains the number of tasks currently present in the queue of worker segrigated by priority.
 * taskMapVector:- 1 1 3 3
 * taskPriority:- 2 (Medium priority)
 * avgTime = 1 * avgQueueTime + 1 * avgQueueTime + 3 * avgQueueTime;
 * 
 * Like shown above it conciders how long will the next packet take if it were to be queued in the particular worker
 * This avgTime is divided by number of threads available in the worker higher the count more task will be distributed
 * and quicker the queue will free up.
 * Once tempPoints are calculated we compare it with our saved totalWorkerPoints to find who gets next packet.
*/
Worker* findIdealWorker(TaskPriority taskPriority = DEFAULT_PRIORITY)
{
    std::list<Worker*> workerList = globalWorkerRegistry.getWorkerList();
    Worker *worker, *resWorker;
    int finalworkerQueueSize = 0;
    std::string finalWorkerUid;
    double finalWorkerPoints;
    Flag firstElement;

    firstElement.initFlag(false);

    for(auto i = workerList.begin(); i != workerList.end(); i++)
    {
        worker = *i;
        if(worker == NULL){
            Log().error(__func__, "Worker not found");
            continue;
        }
        if (!worker->isWorkerReady()) {
            //Log().senderCoreInfo(__func__, "worker:", worker->getWorkerUID(), " not ready for user data");
        } else {
            
            WorkerStats workerStats = worker->getWorkerStats();
            double tempPoints, avgTime = 0;
            
            double avgQueueTime = workerStats.getAvgQueueTime();
            int totalThreads = workerStats.getTotalAvilableThreads();
            std::vector<int> taskMapVector = workerStats.getTaskQueueVector();
            // This means the stats is not yet initlized for whatever reason
            if(taskMapVector.size() == 0)
                continue;
            for(int i = taskPriority; i >= 0; i--){
                if(taskMapVector[i])
                    avgTime += avgQueueTime * taskMapVector[i];
            }
            // Lower points is better
            tempPoints = avgTime ? (avgTime / totalThreads) : 0;
            Log().senderCoreInfo(__func__, "tempPoints: ", tempPoints, " worker:", worker->getWorkerUID(),
                 " Points to match:", finalWorkerPoints);
            if(!firstElement.isFlagSet()){
                firstElement.setFlag();
                goto assign;
            }
            if(tempPoints <= finalWorkerPoints){
                if(tempPoints == finalWorkerPoints){
                    /* If both selected worker and worker chosen now have same points compare their on server queue 
                     * sizes to determine who gets selected for next packet transmission. */
                    if(worker->getQueueSize() > finalworkerQueueSize) continue;
                }
assign:
                finalworkerQueueSize = worker->getQueueSize();
                finalWorkerUid = worker->getWorkerUID();
                finalWorkerPoints = tempPoints;
            }
        } 
    }

    if(finalWorkerUid.empty()){
        //Log().error(__func__, "all workers are full");
        return NULL;
    }

    resWorker = globalWorkerRegistry.getWorkerFromUid(finalWorkerUid);
    Log().senderCoreInfo(__func__, "worker with emptiest queue: ", finalWorkerUid);

    return resWorker;
}

void pushUserDataToWorkerQueue()
{
    Worker *worker;
    ExportSinkItem sinkItem;
    UserDataTable* userData;
    json packet;

    if(globalSenderSink->isSinkEmpty()){
        return;
    }

    worker = findIdealWorker(globalSenderSink->getTopTaskPriority());
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
            , globalOutgoingDataRegistry.getRegistryFromId(userData->userTable), true)
        );
    globalOutgoingDataRegistry.assignWorker(userData->userTable, worker);
    globalOutgoingDataRegistry.updateTaskStatus(userData->userTable, DATA_READY);
    Log().senderCoreInfo(__func__, "sender sink packet pushed to worker:", worker->getWorkerUID());
}

void pushInstanceToWorkerQueue(std::list<std::string> *workerList)
{
    std::string workerUID;
    Worker* worker;
    std::string tableId;
    std::list<json> instanceJson = globalUserDataTemplateRegistry.toJson();

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
        for(auto j = instanceJson.begin(); j != instanceJson.end(); j++)
        {
            tableId = (*j)["body"]["instanceid"];
            Log().senderCoreInfo(__func__, "queuing packet with instance ID:", tableId);
            globalOutgoingDataRegistry.add(tableId, worker);
            worker->queuePacket(
                new OutPacket(
                    PacketConstructor().create(SP_DATA_SENT, worker->getWorkerUID(), *j)
                    , globalOutgoingDataRegistry.getRegistryFromId(tableId), true)
                );
            globalOutgoingDataRegistry.assignWorker(tableId, worker);
            globalOutgoingDataRegistry.updateTaskStatus(tableId, DATA_READY);
            // Track worker for the instance data
            workerInstanceList.addWorker(worker->getWorkerUID());
        }
        Log().senderCoreInfo(__func__, "queued packet into worker UID: ", workerUID);
        workerList->erase(i--);
    }

    Log().senderCoreInfo(__func__, "cleared pending workerList");
}

void updateAllWorkerInstances(std::list<std::string> *workerList)
{
    std::list<Worker*> workers;

    workers = globalWorkerRegistry.getWorkerList();
    // Only Add workers which are not registered in the list
    Log().senderCoreInfo(__func__, "worker list size:", workers.size());
    for(auto i = workers.begin(); i != workers.end(); i++){
        Worker* worker = *i;
        Flag workerPresent;
        workerPresent.initFlag(false);
        // Don't add already registered workers to list
        for(auto j = workerList->begin(); j != workerList->end(); j++){
            if((*j).compare(worker->getWorkerUID())){
                workerPresent.setFlag();
                break;
            }
        }
        if(workerPresent.isFlagSet()) continue;
        workerList->push_back(worker->getWorkerUID());
    }
    pushInstanceToWorkerQueue(workerList);
    globalUserDataTemplateRegistry.resetFlag();
    Log().senderCoreInfo(__func__, "all workers instances updated");
}

// TO-DO:- Needs rework
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

    //Check for any pending immigrant packets that needs to be sent to worker
    if(!senderData->isPendingPacketsListEmpty()){
        rc = pushPendingPackets(senderData->getPendingPacketsList());
        if(rc){
            Log().senderCoreInfo(__func__, "all worker queues are filled waiting to free-up or no worker present");
            return JOB_PENDING;
        }
        Log().senderCoreInfo(__func__, "pending packets pushed to queue");
    }

    //Check if instance data was updated
    if(globalUserDataTemplateRegistry.getUpdateStatus())
        updateAllWorkerInstances(senderData->getWorkerList());

    //Update all new workers registered in the sender list
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