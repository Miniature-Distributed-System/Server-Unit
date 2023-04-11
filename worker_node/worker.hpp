#ifndef WORKER_H
#define WORKER_H
#include <list>
#include "../include/flag.h"
#include "../configs.h"
#include <semaphore.h>

#ifndef JSON_H
#define JSON_H
#include "../lib/nlohmann/json-schema.hpp"
using json = nlohmann::json;
#endif

class OutDataState;

struct OutPacket {
    private:
        OutDataState* outData;
        bool ackable;
        Flag status;
    public:
        json packet;
        OutPacket(json packet, OutDataState* outData, bool ackable);
        void checkIn();
        void checkOut();
        bool isCheckedIn();
        bool isAckable();
        OutDataState* getOutDataState();
};

class Worker {
        std::uint64_t workerUID;
        std::list<OutPacket*> senderQueue;
        std::list<OutPacket*> ackPendingQueue;
        Flag attendance;
        Flag ackPacketPop;
        Flag quickSendMode;
        sem_t workerLock;
    public:
        Worker(std::uint64_t workerUID);
        void checkIn();
        void checkOut();
        bool isCheckedIn();
        int queuePacket(OutPacket*);
        json getQueuedPacket();
        int getQueueSize();
        std::uint64_t getWorkerUID();
        bool matchAckablePacket(std::string id);
        void pushToFront(OutPacket *outPacket);
        std::list<OutPacket*> shutDown();
        void setQuickSendMode();
        void resetQuickSendMode();
        bool isQuickSendMode();
};
#endif