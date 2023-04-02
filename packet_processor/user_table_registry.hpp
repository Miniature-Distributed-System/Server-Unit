#ifndef USER_TABLE_H
#define USER_TABLE_H
#include <list>
#include <string>
#include "../include/task.hpp"
#include "../worker_node/worker.hpp"

struct UserTableState {
    std::string userTableName;
    UserTaskStatus taskStatus;
    Worker *worker;
    UserTableState(std::string userTableName, UserTaskStatus taskStatus){
        this->userTableName = userTableName;
        this->taskStatus = taskStatus;
    }
};

class UserTableRegistry {
        std::list<UserTableState*> userTableRegistryList;
    public:
        std::list<std::string> pushList(std::list<std::string> packetList);
        int popList(std::string tableName);
        bool findMatchInList(std::string tableName);
        bool assignWorker(std::string tableName, Worker *worker);
        long long int getWorkerUid(std::string tableName);
        int updateStatus(std::string tableName, UserTaskStatus status);
};

extern UserTableRegistry globalUserTableRegistry;

#endif