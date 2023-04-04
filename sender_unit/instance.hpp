#ifndef INST_STRUCT_H
#define INST_STRUCT_H
#include <string>
#include <list>
#include <semaphore.h>
#include "../lib/nlohmann/json-schema.hpp"

using nlohmann::json_schema::json_validator;
using json = nlohmann::json;

struct InstanceStruct {
    std::string instanceName;
    std::uint8_t algoType;
    std::string *data;
    InstanceStruct(std::string instanceName,std::uint8_t algoType, std::string *data){
        this->instanceName = instanceName;
        this->data = data;
        this->algoType = algoType;
    }
};

class Instance {
        std::list<InstanceStruct*> instanceList;
        sem_t instanceListLock;
    public:
        Instance();
        int update(std::list<InstanceStruct*> instance);
        std::list<InstanceStruct*> getInstance();
        std::list<json>* toJson();
};

extern Instance globalInstanceRegistery;

#endif