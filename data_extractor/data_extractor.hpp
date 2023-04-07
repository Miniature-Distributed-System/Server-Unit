#ifndef DAT_EXTR_H
#define DAT_EXTR_H
#include <list>
#include <string>
#include "../include/task.hpp"

class DataExtractor { 
    public:
        static std::string* getFileData(std::string fileName, bool isInstance);
        static int executeInstanceExtractor(std::list<std::string> instanceNameList, TaskPriority priority);
        static int executeUserTableExtractor(std::list<std::string> userTableName);
};

#endif