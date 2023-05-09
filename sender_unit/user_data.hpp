#ifndef USER_DATA_H
#define USER_DATA_H

#include <string>
#include "../include/task.hpp"

#ifndef JSON_H
#define JSON_H
#include "../lib/nlohmann/json-schema.hpp"
using json = nlohmann::json;
#endif


class UserDataTable {
    public:
        std::string userTable;
        TaskPriority priority;
        std::string instanceName;
        std::string *data;
        UserDataTable(){}
        UserDataTable(std::string userTable, TaskPriority priority, std::string instanceTable, std::string *data);
        json toJson();
        void destruct();
};

#endif