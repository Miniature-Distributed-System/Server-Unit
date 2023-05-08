#include "../include/task.hpp"
#include "../include/debug_rp.hpp"
#include "../packet_processor/out_data_registry.hpp"
#include "../worker_node/worker_registry.hpp"
#include "../sender_unit/sender_core.hpp"
#include "timeout.hpp"

bool Timeout::isWorkerRegistered(Worker* worker)
{
    for(auto i = timedoutWorkerRegistry.begin(); i != timedoutWorkerRegistry.end(); i++){
        if(worker == *i){
            DEBUG_MSG(__func__, "worker:",worker->getWorkerUID()," timed out!");
            timedoutWorkerRegistry.erase(i--);
            return true;
        }
    }

    timedoutWorkerRegistry.push_back(worker);
    DEBUG_MSG(__func__, "worker first timeout strike");
    return false;
}

void Timeout::addPacket(OutPacket* outPacket)
{
    sem_wait(&timeoutListLock);
    timedPackets.push_back(outPacket);
    sem_post(&timeoutListLock);
}

void Timeout::popPacket(OutPacket* outPacket)
{
    sem_wait(&timeoutListLock);
    auto removed = std::remove(timedPackets.begin(), timedPackets.end(), outPacket);
    timedPackets.erase(removed, timedPackets.end());
    sem_post(&timeoutListLock);
}

void Timeout::execute()
{
    std::list<OutPacket*> outPackets;

    sem_wait(&timeoutListLock);
    for(auto i = timedPackets.begin(); i != timedPackets.end(); i++)
    {
        OutPacket *outPacket = (*i);
        OutDataState *outDataState = outPacket->getOutDataState();
        // Worker checked in before timeout
        if(outPacket->isCheckedIn()){
            if(outDataState->taskStatus == DATA_FINAL){
                popPacket(outPacket);
                // Worker packet dealloc
                delete outPacket;
                // Table data dealloc
                globalOutDataRegistry.deleteTable(outDataState->id);
            } else {
                // Reset timer for this packet
                outPacket->checkOut();
            }
        } else {
            // Worker not checked in even agfter timeout has occured
            if(!outDataState->worker->isCheckedIn()){
                if(isWorkerRegistered(outDataState->worker)){
                    // Worker is timedout so remove it from list, de-alloc, reassign its resources and data
                    DEBUG_MSG(__func__, "worker timeout ID:", outDataState->worker->getWorkerUID());
                    outPackets = globalWorkerRegistry.deleteWorker(outDataState->worker);
                    for(auto i = outPackets.begin(); i != outPackets.end(); i++){
                        senderCoreData->addPackets(*i);
                    }
                    DEBUG_MSG(__func__, "worker packets have been sent for re-assignment");
                    return;
                }
            }
            // the non-acked packet is next in line to be re-sent to worker when queried
            outDataState->worker->pushToFront(outPacket);
        }
    }
    sem_post(&timeoutListLock);
}