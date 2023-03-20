#include "worker.hpp"

Worker::Worker(std::uint64_t workerUID)
{
    this->workerUID = workerUID;
    attendance.initFlag(true);
}

void Worker::markAttendance()
{  
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

void Worker::queuePacket(json packet)
{
    senderQueue.push_back(packet);
}

json Worker::getQueuedPacket()
{
    json packet = senderQueue.front();
    senderQueue.pop_front();
    return packet;
}

