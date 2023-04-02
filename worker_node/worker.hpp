#ifndef WORKER_H
#define WORKER_H
#include <list>
#include "../lib/nlohmann/json-schema.hpp"
#include "../include/flag.h"
#include "../configs.h"
#include <semaphore.h>

using json = nlohmann::json;

//TO-DO: need to implement timeout register which needs to be updated for sent packets
struct OutPacket {
    json packet;
    std::string id;
    bool ackable;
    OutPacket(json packet, std::string tableName, bool ackable){
        this->packet = packet;
        this->id = tableName;
        this->ackable = ackable;
    }
};

class Worker {
        std::uint64_t workerUID;
        std::list<OutPacket*> senderQueue;
        std::list<OutPacket*> ackPendingQueue;
        Flag attendance;
        sem_t workerLock;
    public:
        Worker(std::uint64_t workerUID);
        void markAttendance();
        bool getAttendance();
        int queuePacket(OutPacket*);
        json getQueuedPacket();
        int getQueueSize();
        std::uint64_t getWorkerUID();
        bool matchAckablePacket(std::string id);
};
#endif