#ifndef WORKER_REGI_H
#define WORKER_REGI_H
#include <string>
#include <list>
#include "../include/flag.h"
#include "worker.hpp"


class WorkerRegistry{
        std::list<Worker*> currentWorkerList;
        std::list<std::string> deadWorkerList;
        Flag newWorker;
    public:
        WorkerRegistry();
        std::string generateWorkerUid();
        std::list<OutPacket*> deleteWorker(Worker*);
        bool getNewWorkersStatus();
        std::list<Worker*> getWorkerList();
        Worker* getWorkerFromUid(std::string);
};

extern WorkerRegistry globalWorkerRegistry;

#endif