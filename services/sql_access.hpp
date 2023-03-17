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
        SqlAccess(std::string url, std::string username, std::string password, std::string database, 
        std::string dataTable) : url{url}, password{password}, database{database}, dataTable{dataTable};
        void initlize();
        std::string* sqlQueryDb(std::string, std::string);
        int sqlWriteBlob(std::string fileLoc, std::string columnName, std::string tableID);
};

SqlAccess globalSqlAccess; 

#endif