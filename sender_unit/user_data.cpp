#include "user_data.hpp"

UserDataTable::UserDataTable(std::string userTableName, TaskPriority priority, std::string instanceTableName, 
                std::string *data)
{
    this->userTable = userTable;
    this->priority = priority;
    this->instanceName = instanceTableName;
    this->data = data;
}

json UserDataTable::toJson()
{
    json packet;
    packet["body"]["tableid"] = userTable;
    packet["body"]["priority"] = priority;
    packet["body"]["instancetype"] = instanceName;
    packet["body"]["data"] = *data;

    return packet;
}