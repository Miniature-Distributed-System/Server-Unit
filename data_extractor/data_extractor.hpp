#ifndef DAT_EXTR_H
#define DAT_EXTR_H
#include <list>
#include <string>
#include "../include/task.hpp"
#include "../services/sql_access.hpp"

class DataExtractor { 
    public:
        static std::string* getFileData(std::string fileName, bool isInstance);
        static int executeInstanceExtractor(std::list<std::string> instanceNameList, SqlAccess *sqlAccess);
        static int executeUserTableExtractor(std::list<std::string> userTableName, SqlAccess *sqlAccess);
};

#endif