#ifndef USER_DATA_H
#define USER_DATA_H

#include <string>
#include "../lib/nlohmann/json-schema.hpp"
#include "../include/task.hpp"

using nlohmann::json_schema::json_validator;
using json = nlohmann::json;

struct UserDataTable {
    std::string userTable;
    TaskPriority priority;
    std::string instanceName;
    std::string *data;
    UserDataTable(std::string userTable, TaskPriority priority, std::string instanceTable, std::string *data);
    json toJson();
};

#endif