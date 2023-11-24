#ifndef WORKER_INST_H
#define WORKER_INST_H
#include <vector>
#include <string>
#include "userdata_template_registry.hpp"

class WorkerInstanceStruct {
        std::string workerUid;
        std::vector<std::string> templateIdList;
    public:
        WorkerInstanceStruct(std::string workerUid, std::vector<std::string> templateIdList) : workerUid(workerUid), 
                        templateIdList(templateIdList) {};
        void update(std::string templateId);
        std::string getWorkerUid(){ return workerUid; };
        int getWorkerInstanceListSize() { return templateIdList.size(); };
};

class WorkerInstanceList {
        std::vector<WorkerInstanceStruct> workerList;
        std::vector<std::string> templateIdList;
    public:
        void updateInstanceList(std::list<UserDataTemplateStruct> instanceList);
        void addWorker(std::string workerUid);
        void updateWorker(std::string workerUid, std::string tableId);
};

extern WorkerInstanceList workerInstanceList;

#endif