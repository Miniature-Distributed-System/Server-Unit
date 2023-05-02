#include "../include/debug_rp.hpp"
#include "sql_access.hpp"

using namespace sql;

SqlAccess::SqlAccess(){}

SqlAccess::SqlAccess(std::string url, std::string username, std::string password, std::string database, 
        std::string dataTable) 
{
    this->url = url;
    this->username = username;
    this->password = password;
    this->database = database;
    this->dataTable = dataTable;
}

void SqlAccess::initialize()
{
    try{
        driver = get_driver_instance();
        conn = driver->connect(url, username, password);
        conn->setSchema(database);
        DEBUG_MSG(__func__, "sql initialized successfully");
        inited.setFlag();
    }catch(sql::SQLException &e){
        DEBUG_ERR(__func__, e.what());
        DEBUG_ERR(__func__, "sql Error code: ", e.getErrorCode());
        inited.resetFlag();
    }
}

std::string SqlAccess::sqlQueryDb(std::string queryStatement, std::string columnName)
{
    std::string result;
    
    try{
        sqlExecutor = conn->prepareStatement(queryStatement);
        res = sqlExecutor->executeQuery();
        while(res->next()){
            if(columnName.empty())
                result = result + res->getString(1);
            else
                result = result + res->getString(columnName);
        }
    }catch(sql::SQLException &e){
        DEBUG_ERR(__func__, "sql error: ", e.what());
        return "";
    }

    return result;
}

std::list<std::string> SqlAccess::sqlQueryDbList(std::string queryStatement, std::string columnName)
{
    std::list<std::string> result;
    
    try{
        sqlExecutor = conn->prepareStatement(queryStatement);
        res = sqlExecutor->executeQuery();
        while(res->next()){
            if(columnName.empty())
                result.push_back(res->getString(1));
            else
                result.push_back(res->getString(columnName));
        }
    }catch(sql::SQLException &e){
        DEBUG_ERR(__func__, "sql error: ", e.what());
    }

    return result;
}

int SqlAccess::sqlQueryDbGetInt(std::string queryStatement, std::string columnName)
{
    int result;

    try{
        sqlExecutor = conn->prepareStatement(queryStatement);
        res = sqlExecutor->executeQuery();
        while(res->next()){
            if(columnName.empty())
                result = res->getInt(1);
            else
                result = res->getInt(columnName);
        }
    }catch(sql::SQLException &e){
        DEBUG_ERR(__func__, "sql error: ", e.what());
        return -1;
    }
    
    return result;
}

int SqlAccess::sqlWriteBlob(std::string fileLoc, std::string columnName, std::string rowIdentifierName, 
                std::string rowIdentifierValue)
{
    try{
        std::string query = "UPDATE " + dataTable + " SET " + columnName + " = LOAD_FILE('" + fileLoc +"') WHERE "+
        rowIdentifierName + "='"+ rowIdentifierValue + "';";
        sqlExecutor = conn->prepareStatement(query);
        res = sqlExecutor->executeQuery();
    }catch(sql::SQLException &e){
        DEBUG_ERR(__func__, "sql error: ", e.what());
        return -1;
    }

    return 0;
}
{
    try{
        std::string query = "UPDATE " + dataTable + " SET " + columnName + " = LOAD_FILE('" + fileLoc +"') WHERE tableID="
                        + tableID + ";";
        sqlExecutor = conn->prepareStatement(query);
        res = sqlExecutor->executeQuery();
    }catch(sql::SQLException &e){
        DEBUG_ERR(__func__, "sql error: ", e.what());
        return -1;
    }

    return 0;
}

std::list<std::string> SqlAccess::sqlReadList(std::string tableID, std::string columnName)
{
    std::list<std::string> resultList;

    try{
        std::string query = "SELECT " + columnName + " FROM " + tableID +";";
        sqlExecutor = conn->prepareStatement(query);
        res = sqlExecutor->executeQuery();
        while(res->next()){
            resultList.push_back(res->getString(columnName));
        }
    }catch(sql::SQLException &e){
        DEBUG_ERR(__func__, "sql error: ", e.what());
    }

    return resultList;
}