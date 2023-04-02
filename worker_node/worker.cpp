#include "../include/debug_rp.hpp"
#include "worker.hpp"

Worker::Worker(std::uint64_t workerUID)
{
    this->workerUID = workerUID;
    attendance.initFlag(true);
    sem_init(&workerLock, 0, 0);
}

void Worker::markAttendance()
{
    DEBUG_MSG(__func__,"Worker-",workerUID, ": attendence marked");  
    attendance.setFlag();
}

bool Worker::getAttendance()
{
    return attendance.isFlagSet();
}

std::uint64_t Worker::getWorkerUID()
{  
    return workerUID;
}

int Worker::queuePacket(OutPacket* packet)
{
    if(senderQueue.size() > WORKER_QUEUE_SIZE){
        DEBUG_MSG(__func__, "max limit reached");
        return 1;
    }
    sem_wait(&workerLock);
    senderQueue.push_back(packet);
    sem_post(&workerLock);
    DEBUG_MSG(__func__, "worker-", workerUID, ": pushed packet to queue");
    return 0;
}

json Worker::getQueuedPacket()
{
    OutPacket* outPacket = senderQueue.front();
    if(outPacket->ackable){
        if(ackPendingQueue.size() > WORKER_QUEUE_SIZE / 2){
            //TO-DO: need to add timeout indicating resend packet and wait
            while(1){
                for(auto i = senderQueue.begin(); i != senderQueue.end(); i++){
                    if(!(*i)->ackable){
                        outPacket = (*i);
                        senderQueue.erase(i);
                        return (*i)->packet;
                    }
                }
            }
        } else {
            ackPendingQueue.push_back(outPacket);
        }
    }
    senderQueue.pop_front();
    return outPacket->packet;
}

int Worker::getQueueSize()
{
    return WORKER_QUEUE_SIZE - senderQueue.size();
}

bool Worker::matchAckablePacket(std::string id)
{
    for(auto i = ackPendingQueue.begin(); i != ackPendingQueue.end(); i++){
        if((*i)->id == id){
            ackPendingQueue.erase(i);
            DEBUG_MSG(__func__, "packet acked");
            return true;
        }
    }

    DEBUG_ERR(__func__, "no such packet found");
    return false;
}
