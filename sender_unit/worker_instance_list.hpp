#ifndef WORKER_INST_H
#define WORKER_INST_H
#include <vector>
#include <string>
#include "instance.hpp"

class WorkerInstanceStruct {
        std::string workerUid;
        std::vector<std::string> instanceIdList;
    public:
        WorkerInstanceStruct(std::string workerUid, std::vector<std::string> instanceIdList) : workerUid(workerUid), 
                        instanceIdList(instanceIdList) {};
        void update(std::string instanceId);
        std::string getWorkerUid(){ return workerUid; };
        int getWorkerInstanceListSize() { return instanceIdList.size(); };
};

class WorkerInstanceList {
        std::vector<WorkerInstanceStruct> workerList;
        std::vector<std::string> instanceIdList;
    public:
        void updateInstanceList(std::list<InstanceStruct> instanceList);
        void addWorker(std::string workerUid);
        void updateWorker(std::string workerUid, std::string tableId);
};

extern WorkerInstanceList workerInstanceList;

#endif