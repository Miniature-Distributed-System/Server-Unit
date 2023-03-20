#ifndef WORKER_H
#define WORKER_H
#include <list>
#include "../lib/nlohmann/json-schema.hpp"
#include "../include/flag.h"
using json = nlohmann::json;

class Worker {
        std::uint64_t workerUID;
        std::list<json> senderQueue;
        Flag attendance;
    public:
        Worker(std::uint64_t workerUID);
        void markAttendance();
        bool getAttendance();
        void queuePacket(json);
        json getQueuedPacket();
        std::uint64_t getWorkerUID();
};
#endif