#ifndef SQL_ACCESS_H
#define SQL_ACCESS_H
#include <string>
#include <stdlib.h>
#include <iostream>
#include "mysql_connection.h"
#include "../include/debug_rp.hpp"
#include "../include/flag.h"

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <list>

class SqlAccess{
        std::string url;
        std::string username;
        std::string password;
        std::string database;
        std::string dataTable;

        sql::Driver *driver;
        sql::Connection *conn;
        sql::Statement *stmt;
        sql::ResultSet *res;
        sql::PreparedStatement *sqlExecutor;

        Flag inited;
    public:
        SqlAccess();
        SqlAccess(std::string url, std::string username, std::string password, std::string database, 
            std::string dataTable);
        void initialize();
        std::string sqlQueryDb(std::string queryString, std::string columnName = "");
        std::list<std::string> sqlQueryDbList(std::string queryString, std::string columnName = "");
        int sqlQueryDbGetInt(std::string, std::string columnName = "");
        int sqlWriteBlob(std::string fileLoc, std::string columnName, std::string rowIdentifierName, 
                std::string rowIdentifierValue);
        std::list<std::string> sqlReadList(std::string tableID, std::string columnName);
};

extern SqlAccess *globalSqlAccess; 

#endif