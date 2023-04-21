#ifndef INST_STRUCT_H
#define INST_STRUCT_H
#include <string>
#include <list>
#include <semaphore.h>
#include "../include/flag.h"

#ifndef JSON_H
#define JSON_H
#include "../lib/nlohmann/json-schema.hpp"
using json = nlohmann::json;
#endif

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
        Flag dataUpdated;
    public:
        Instance();
        int update(std::list<InstanceStruct*> instance);
        std::list<InstanceStruct*> getInstance();
        std::list<json>* toJson();
        void resetFlag();
        bool getUpdateStatus();
};

extern Instance globalInstanceRegistery;

#endif