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

int Worker::queuePacket(json packet)
{
    if(senderQueue.size() > workerQueueMaxSize){
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
    json packet = senderQueue.front();
    senderQueue.pop_front();
    return packet;
}

