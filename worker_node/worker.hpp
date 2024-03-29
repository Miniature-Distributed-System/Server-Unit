#ifndef WORKER_H
#define WORKER_H
#include <list>
#include "../include/flag.h"
#include "../configs.h"
#include <semaphore.h>
#include "worker_stats.hpp"

#ifndef JSON_H
#define JSON_H
#include "../lib/nlohmann/json-schema.hpp"
using json = nlohmann::json;
#endif

class OutgoingDataState;

struct OutPacket {
    private:
        OutgoingDataState* outData;
        bool ackable;
        Flag status;
    public:
        json packet;
        OutPacket(json packet, OutgoingDataState* outData, bool ackable);
        void checkIn();
        void checkOut();
        bool isCheckedIn();
        bool isAckable();
        OutgoingDataState* getOutDataState();
};

class Worker {
        std::string workerUID;
        std::list<OutPacket*> senderQueue;
        std::list<OutPacket*> ackPendingQueue;
        Flag attendance;
        Flag ackPacketPop;
        Flag quickSendMode;
        Flag workerReady;
        sem_t workerLock;
        WorkerStats workerStats;

    public:
        Worker();
        Worker(std::string workerUID);
        void checkIn();
        void checkOut();
        bool isCheckedIn();
        int queuePacket(OutPacket*);
        json getQueuedPacket();
        int getQueueSize();
        std::string getWorkerUID();
        bool matchAckablePacket(std::string id);
        void pushToFront(OutPacket *outPacket);
        std::list<OutPacket*> shutDown();
        void setQuickSendMode();
        void resetQuickSendMode();
        bool isQuickSendMode();
        void setWorkerReady();
        bool isWorkerReady();
        void resetWorkerReady();
        WorkerStats getWorkerStats(){ return workerStats;};
        void setWorkerStats(WorkerStats workerStats){ this->workerStats = workerStats;};
};
#endif