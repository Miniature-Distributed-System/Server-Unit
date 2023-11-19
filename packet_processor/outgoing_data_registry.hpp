#ifndef USER_TABLE_H
#define USER_TABLE_H
#include <list>
#include <string>
#include "../include/task.hpp"
#include "../include/flag.h"
#include "../worker_node/worker.hpp"

struct OutgoingDataState {
    std::string id;
    UserTaskStatus taskStatus;
    Worker *worker;
    OutgoingDataState(std::string id, UserTaskStatus taskStatus){
        this->id = id;
        this->taskStatus = taskStatus;
    }
};

class OutgoingDataRegistry {
        std::list<OutgoingDataState*> outDataRegistryList;
    public:
        int add(std::string packetList, Worker *worker = NULL);
        int delete(std::string tableName);
        bool findMatchInList(std::string tableName);
        bool assignWorker(std::string tableName, Worker *worker);
        int updateTaskStatus(std::string tableName, UserTaskStatus status);
        std::list<OutgoingDataState*> list();
        OutgoingDataState* getRegistryFromId(std::string id);
};

extern OutgoingDataRegistry globalOutgoingDataRegistry;

#endif