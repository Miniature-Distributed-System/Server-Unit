#ifndef SENDER_CORE_H
#define SENDER_CORE_H
#include <list>
#include "../worker_node/worker.hpp"

class SenderCoreData {
        std::list<std::string>* newWorkerList;
        std::list<OutPacket*>* pendingPacketsList;
    public:
        SenderCoreData();
        void addWorker(std::string workerUid);
        void addPackets(OutPacket* outPacket);
        bool isNewWorkerListEmpty();
        bool isPendingPacketsListEmpty();
        std::list<std::string>* getWorkerList();
        std::list<OutPacket*>* getPendingPacketsList();
};

class SenderCore {
    public:
        int run();
};

extern SenderCoreData* senderCoreData;

#endif