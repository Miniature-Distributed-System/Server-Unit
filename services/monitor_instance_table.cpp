#include <pthread.h>
#include <list>
#include "../configs.h"
#include "../include/debug_rp.hpp"
#include "../data_extractor/data_extractor.hpp"
#include "sql_access.hpp"
#include "monitor.hpp"

void* monitorInstanceTable(void *data)
{
    //TO-DO
    std::string queryTimeStamp = "SELECT MAX(timestamp) as timestamp FROM " + INSTANCE_TABLE_NAME + ";" ;
    std::string queryBuilder;
    std::string columnName = "timestamp";
    std::string latestTimeStamp, currentTimeStamp;
    std::list<std::string> instanceNameList;

    while(monitorStop.isFlagSet()){
        currentTimeStamp = globalSqlAccess->sqlQueryDb(queryTimeStamp, columnName);
        if(currentTimeStamp.compare(latestTimeStamp)){
            DEBUG_MSG(__func__, "current time stamp:", currentTimeStamp, " saved timestamp: ", latestTimeStamp);
            latestTimeStamp = currentTimeStamp;
            instanceNameList = globalSqlAccess->sqlReadList(INSTANCE_TABLE_NAME, INSTANCE_DAT_COL_ID);
            DataExtractor().executeInstanceExtractor(instanceNameList, HIGH_PRIORITY);
        }
        sleep(DB_POLL_THREAD_SLEEP_TIMER);
    }

    return 0;
}

int startMonitorInstanceTable()
{
    pthread_t monitor;
    monitorStop.initFlag(false);
    pthread_create(&monitor, NULL, monitorInstanceTable, NULL);
    DEBUG_MSG(__func__, "instance table monitor service created");

    return 0;
}