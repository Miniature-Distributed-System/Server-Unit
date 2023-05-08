#ifndef TIMEOUT_H
#define TIMEOUT_H
#include <semaphore.h>
#include "../worker_node/worker.hpp"

class Timeout{
        std::list<Worker*> timedoutWorkerRegistry;
        std::list<OutPacket*> timedPackets;
        sem_t timeoutListLock;
    public:
        Timeout(){
            sem_init(&timeoutListLock, 0, 1);
        };
        bool isWorkerRegistered(Worker*);
        void addPacket(OutPacket *outPacket);
        void popPacket(OutPacket *outPacket);
        void execute();
};

extern Timeout *packetTimeout;

#endif