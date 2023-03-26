#ifndef WORKER_REGI_H
#define WORKER_REGI_H
#include <string>
#include <list>
#include "../include/flag.h"
#include "worker.hpp"


class WorkerRegistry{
        std::list<Worker*> currentWorkerList;
        std::list<std::uint64_t> deadWorkerList;
        Flag newWorker;
    public:
        WorkerRegistry();
        int getWorkerUid();
        void deleteWorker(Worker*);
        bool getNewWorkersStatus();
        std::list<Worker*> getWorkerList();
};

extern WorkerRegistry globalWorkerRegistry;

#endif