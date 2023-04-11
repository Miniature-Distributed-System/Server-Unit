#ifndef WORKER_H
#define WORKER_H
#include <list>
#include "../lib/nlohmann/json-schema.hpp"
#include "../include/flag.h"
#include "../packet_processor/out_data_registry.hpp"
#include "../configs.h"
#include <semaphore.h>

using json = nlohmann::json;

//TO-DO: need to implement timeout register which needs to be updated for sent packets
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