#ifndef USER_TABLE_H
#define USER_TABLE_H
#include <list>
#include <string>
#include "../include/task.hpp"
#include "../include/flag.h"
#include "../worker_node/worker.hpp"

struct OutDataState {
    std::string id;
    UserTaskStatus taskStatus;
    Worker *worker;
    Flag packetUpdated;
    OutDataState(std::string id, UserTaskStatus taskStatus){
        this->id = id;
        this->taskStatus = taskStatus;
        packetUpdated.initFlag();
    }
};

class OutDataRegistry {
        std::list<OutDataState*> outDataRegistryList;
    public:
        int addTable(std::string packetList, Worker *worker = NULL);
        int deleteTable(std::string tableName);
        bool findMatchInList(std::string tableName);
        bool assignWorker(std::string tableName, Worker *worker);
        long long int getWorkerUid(std::string tableName);
        int updateTaskStatus(std::string tableName, UserTaskStatus status);
        bool packetCheckInStatus(std::string tableId);
        void packetCheckOut(std::string tableId);
        std::list<OutDataState*> getOutDataRegistryList();
};

extern OutDataRegistry globalOutDataRegistry;

#endif