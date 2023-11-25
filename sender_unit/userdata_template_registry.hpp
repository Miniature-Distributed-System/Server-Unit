#ifndef TEMPL_STRUCT_H
#define TEMPL_STRUCT_H
#include <string>
#include <list>
#include <semaphore.h>
#include "../include/flag.h"

#ifndef JSON_H
#define JSON_H
#include "../lib/nlohmann/json-schema.hpp"
using json = nlohmann::json;
#endif

struct UserDataTemplateStruct {
    std::string templateName;
    std::uint8_t algoType;
    std::string *data;
    UserDataTemplateStruct(){}
    UserDataTemplateStruct(std::string templateName,std::uint8_t algoType, std::string *data){
        this->templateName = templateName;
        this->data = data;
        this->algoType = algoType;
    }
};

class UserDataTemplate {
        std::list<UserDataTemplateStruct> uDataTemplateList;
        std::list<json>* uDataTemplateJsonList;
        sem_t uDataTemplateListLock;
        Flag dataUpdated;
    public:
        UserDataTemplate();
        int update(std::list<UserDataTemplateStruct> templateStruct);
        std::list<UserDataTemplateStruct> get();
        std::list<json> toJson();
        void resetFlag();
        bool getUpdateStatus();
        int listSize(){ return uDataTemplateList.size();};
        bool isMatchingFound(std::string templateId);
};

extern UserDataTemplate globalUserDataTemplateRegistry;

#endif