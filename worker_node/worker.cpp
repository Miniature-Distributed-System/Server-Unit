#include "../include/debug_rp.hpp"
#include "../sched/timeout.hpp"
#include "../packet_processor/out_data_registry.hpp"
#include "../include/logger.hpp"
#include "worker.hpp"

OutPacket:: OutPacket(json packet, OutDataState* outData, bool ackable){
    this->packet = packet;
    this->outData = outData;
    this->ackable = ackable;
    status.initFlag();
}

void OutPacket::checkIn()
{
    outData->worker->checkIn();
    status.setFlag();
}

void OutPacket::checkOut()
{
    status.resetFlag();
}

bool OutPacket::isCheckedIn()
{
    return status.isFlagSet();
}

bool OutPacket::isAckable()
{
    return ackable;
}

OutDataState* OutPacket::getOutDataState()
{
    return outData;
}

Worker::Worker(){}

Worker::Worker(std::string workerUID)
{
    this->workerUID = workerUID;
    attendance.initFlag(true);
    ackPacketPop.initFlag(false);
    quickSendMode.initFlag(false);
    workerReady.initFlag(false);
    sem_init(&workerLock, 0, 1);
}

void Worker::checkIn()
{
    Log().info(__func__,"Worker-",workerUID, ": attendence marked");  
    attendance.setFlag();
}

void Worker::checkOut()
{
    Log().info(__func__,"Worker-",workerUID, ": attendence marked");  
    attendance.resetFlag();
}

bool Worker::isCheckedIn()
{
    return attendance.isFlagSet();
}

std::string Worker::getWorkerUID()
{  
    return workerUID;
}

int Worker::queuePacket(OutPacket* packet)
{
    if(senderQueue.size() > WORKER_QUEUE_SIZE){
        //Log().info(__func__,"worker-", workerUID,": queue Full size:", senderQueue.size());
        return 1;
    }
    sem_wait(&workerLock);
    senderQueue.push_back(packet);
    sem_post(&workerLock);
    Log().info(__func__, "worker-", workerUID, ": pushed packet to queue, size:", senderQueue.size());
    return 0;
}

json Worker::getQueuedPacket()
{
    OutPacket* outPacket = NULL;

    sem_wait(&workerLock);
    if(ackPacketPop.isFlagSet()){
        Log().info(__func__,"worker-", workerUID,": re-sending non-acked packet to worker");
        ackPacketPop.resetFlag();
        sem_post(&workerLock);
        if(ackPendingQueue.size() > 0)
            return ackPendingQueue.front()->packet;
        else 
            Log().error(__func__, "No ackable packets present flag should have be off");
    }

    if(senderQueue.size() > 0){
        outPacket = senderQueue.front();
        if(outPacket->isAckable()){
            //Check if ackQueue is full if it is then we cant send any more ackable packets
            if(ackPendingQueue.size() > WORKER_QUEUE_SIZE / 2){
                // Only send non ackable packets
                for(auto i = senderQueue.begin(); i != senderQueue.end(); i++){
                    if(!(*i)->isAckable()){
                        outPacket = (*i);
                        senderQueue.erase(i--);
                        Log().info(__func__,"worker-", workerUID,": sending non-ackable packet to worker");
                        sem_post(&workerLock);
                        return (*i)->packet;
                    }
                }
                sem_post(&workerLock);
                return json({});
            } else {
                //Add packet into timeout counter
                packetTimeout->addPacket(outPacket);
                ackPendingQueue.push_back(outPacket);
            }
        }
    } else {
        sem_post(&workerLock);
        return json({});
    }
    Log().info(__func__, "popped from queue size:", senderQueue.size());
    senderQueue.pop_front();
    sem_post(&workerLock);
    return outPacket->packet;
}

int Worker::getQueueSize()
{
    return WORKER_QUEUE_SIZE * (1/2) - (senderQueue.size() + ackPendingQueue.size());
}

bool Worker::matchAckablePacket(std::string id)
{
    sem_wait(&workerLock);
    for(auto i = ackPendingQueue.begin(); i != ackPendingQueue.end(); i++){
        OutPacket *outPacket = (*i);
        if(!outPacket){
            Log().error(__func__, "NULL queue item found");
            ackPendingQueue.erase(i--);
            continue;
        }
        if(outPacket->getOutDataState()->id == id){
            // Pull received packet out from timer
            packetTimeout->popPacket(outPacket);
            delete outPacket;
            ackPendingQueue.erase(i--);
            Log().info(__func__,"worker:", workerUID,"- packet acknowledged");
            sem_post(&workerLock);
            return true;
        }
    }

    sem_post(&workerLock);
    Log().error(__func__,"worker-", workerUID,": no such packet found!");
    return false;
}

std::list<OutPacket*> Worker::shutDown()
{
    std::list<OutPacket*> outPacket;
    sem_wait(&workerLock);
    for(auto i = ackPendingQueue.begin(); i != ackPendingQueue.end(); i++)
    {
        outPacket.push_back(*i);
    }
    for(auto i = senderQueue.begin(); i != senderQueue.end(); i++)
    {
        outPacket.push_back(*i);
    }

    Log().info(__func__,"worker-",workerUID, ": shut down complete");
    sem_post(&workerLock);
    sem_destroy(&workerLock);
    return outPacket;
}

void Worker::pushToFront(OutPacket* outPacket)
{
    sem_wait(&workerLock);
    auto removed = std::remove(ackPendingQueue.begin(), ackPendingQueue.end(), outPacket);
    ackPendingQueue.erase(removed, ackPendingQueue.end());
    ackPendingQueue.push_front(outPacket);
    ackPacketPop.setFlag();
    sem_post(&workerLock);
}

void Worker::setQuickSendMode()
{
    Log().info(__func__, "worker:", workerUID, " is in quick send mode");
    quickSendMode.setFlag();
}

void Worker::resetQuickSendMode()
{
    if(quickSendMode.isFlagSet()){
        Log().info(__func__, "worker:", workerUID, " has exited quick send mode");
        quickSendMode.resetFlag();
    }
}

bool Worker::isQuickSendMode()
{
    return quickSendMode.isFlagSet();
}

void Worker::setWorkerReady()
{
    workerReady.setFlag();
}

void Worker::resetWorkerReady()
{
    workerReady.resetFlag();
}

bool Worker::isWorkerReady()
{
    return workerReady.isFlagSet();
}