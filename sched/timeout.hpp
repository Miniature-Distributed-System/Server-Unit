#ifndef TIMEOUT_H
#define TIMEOUT_H
#include "../worker_node/worker.hpp"

class Timeout{
        std::list<Worker*> timedoutWorkerRegistry;
        std::list<OutPacket*> timedPackets;
    public:
        bool isWorkerRegistered(Worker*);
        void addPacket(OutPacket *outPacket);
        void popPacket(OutPacket *outPacket);
        void execute();
};

extern Timeout *packetTimeout;

#endif