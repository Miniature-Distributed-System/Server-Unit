#ifndef SENDER_CORE_H
#define SENDER_CORE_H
#include <list>
#include "../worker_node/worker.hpp"

class SenderCoreData {
        std::list<std::uint64_t>* newWorkerList;
        std::list<OutPacket*>* pendingPacketsList;
    public:
        SenderCoreData();
        void addWorker(std::uint64_t workerUid);
        void addPackets(OutPacket* outPacket);
        bool isNewWorkerListEmpty();
        bool isPendingPacketsListEmpty();
        std::list<std::uint64_t>* getWorkerList();
        std::list<OutPacket*>* getPendingPacketsList();
};

class SenderCore {
    public:
        int run();
};

extern SenderCoreData* senderCoreData;

#endif