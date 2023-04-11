#include "../include/debug_rp.hpp"
#include "sql_access.hpp"

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
        con = driver->connect(host, username, password);
        con->setSchema(database);
        DEBUG_MSG(__func__, "sql initillized successfully");
        inited.setFlag();
    }catch(sql::SQLException &e){
        DEBUG_ERR(__func__, e.what());
        DEBUG_ERR(__func__, "sql Error code: ", e.getErrorCode());
        inited.resetFlag();
    }
}

std::string* SqlAccess::sqlQueryDb(std::string queryStatement, std::string columnName = "")
{
    std::string result;
    sqlExecutor = conn->prepareStatement(queryStatement);
    sqlExecutor->executeQuery();
    while(res->next())
        result = result + res->getString(columnName.empty() ? 1 : columnName);
    return result;
}

int SqlAccess::sqlQueryDbGetInt(std::string queryStatement, std::string columnName = "")
{
    int result;
    sqlExecutor = conn->prepareStatement(queryStatement);
    sqlExecutor->executeQuery();
    while(res->next())
        result = res->getInt(columnName.empty() ? 1 : columnName);
    return result;
}

int SqlAccess::sqlWriteBlob(std::string fileLoc, std::string columnName, std::string tableID)
{
    std::string result;
    std::string query = "UPDATE " + dataTable + " SET " + columnName + " = LOAD_FILE('" + fileLoc +"') WHERE tableID="
                    + tableID + ";";
    sqlExecutor = conn->prepareStatement(queryStatement);
    sqlExecutor->executeQuery();
    while(res->next())
        result = result + res->getString(columnName);
    return result;
}

std::list<std::string> SqlAccess::sqlReadList(std::string tableID, std::string columnName)
{
    int j = 0;
    std::list<std::string> resultList;
    std::string query = "SELECT " + columnName + " FROM " + tableID +";";
    sqlExecutor = conn->prepareStatement(queryStatement);
    sqlExecutor->executeQuery();
    while(res->next()){
        resultList.push_back(res->getString(columnName));
    }

    return resultList;
}