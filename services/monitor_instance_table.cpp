#include <pthread.h>
#include <list>
#include "../configs.h"
#include "../include/debug_rp.hpp"
#include "../data_extractor/data_extractor.hpp"
#include "../include/logger.hpp"
#include "sql_access.hpp"
#include "monitor.hpp"

/*
  monitorInstanceTable: monitors the SQL Server Database for any newly added Template records.
  It uses the inital timestamp as 2022-05-01 00:00:00 and checks any new data after that. if
  new data found then latest timestamp is updated. It extracts the user data table record 
  names that have been added. It calls the Template data extractors to do further extraction and
  triggering next stages. It sleeps after every poll SQL Server can get overwhelmed 
  especially when there are two monitors working on a low end system.
*/
void* monitorInstanceTable(void *data)
{
    //TO-DO
    std::string queryTimeStamp = "SELECT MAX(timestamp) as timestamp FROM " + TEMPLATE_TABLE_NAME + ";" ;
    std::string queryBuilder;
    std::string columnName = "timestamp";
    std::string latestTimeStamp, currentTimeStamp;
    std::list<std::string> instanceNameList;
    Log().info(__func__, "started monitoring instance table...");
    SqlAccess *sqlAccess = new SqlAccess(DATABASE_URL, DATABASE_USERNAME, DATABASE_PASSWORD, DATABASE_NAME, 
                    USERDAT_TABLE_NAME);
    sqlAccess->initialize();
    
    while(!monitorStop.isFlagSet()){
        currentTimeStamp = sqlAccess->sqlQueryDb(queryTimeStamp, columnName);
        if(currentTimeStamp.compare(latestTimeStamp)){
            Log().info(__func__, "current time stamp:", currentTimeStamp, " saved timestamp: ", latestTimeStamp);
            latestTimeStamp = currentTimeStamp;
            instanceNameList = sqlAccess->sqlReadList(TEMPLATE_TABLE_NAME, TEMPLATE_NAME_COL_ID);
            DataExtractor().executeInstanceExtractor(instanceNameList, sqlAccess);
        }
        sleep(DB_POLL_THREAD_SLEEP_TIMER);
    }

    delete sqlAccess;
    return 0;
}

int startMonitorInstanceTable()
{
    pthread_t monitor;
    monitorStop.initFlag(false);
    pthread_create(&monitor, NULL, monitorInstanceTable, NULL);
    Log().info(__func__, "instance table monitor service created");

    return 0;
}
