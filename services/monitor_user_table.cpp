#include <pthread.h>
#include <vector>
#include <list>
#include "../configs.h"
#include "../data_extractor/data_extractor.hpp"
#include "../sink/sink_stack.hpp"
#include "../packet_processor/out_data_registry.hpp"
#include "../include/debug_rp.hpp"
#include "sql_access.hpp"
#include "monitor.hpp"

void *monitorUserTable(void *data)
{
    //TO-DO
    std::string queryTimeStamp = "SELECT MAX(timestamp) as timestamp FROM " + USERDAT_TABLE_NAME + ";" ;
    std::string columnName = "timestamp";
    std::string latestTimeStamp, currentTimeStamp;

    while(monitorStop.isFlagSet())
    {
        if(!globalSenderSink->isSinkFull() || !globalReceiverSink->isSinkFull())
        {
            currentTimeStamp = globalSqlAccess.sqlQueryDb(queryTimeStamp, columnName);
            if(!currentTimeStamp.compare(latestTimeStamp))
            {
                
                DEBUG_MSG(__func__, "current time stamp:", currentTimeStamp, " saved timestamp: ", latestTimeStamp);
                std::string queryTableName = "SELECT " +  USERDAT_DAT_COL_ID + " FROM " + USERDAT_TABLE_NAME + 
                                " WHERE timestamp BETWEEN '" + latestTimeStamp + "' AND '" + currentTimeStamp + "';"; 
                latestTimeStamp = currentTimeStamp;
                std::list<std::string> fileNames = globalSqlAccess.sqlQueryDbList(queryTableName);
                for(auto i = fileNames.begin(); i != fileNames.end(); i++){
                    globalOutDataRegistry.addTable(*i);
                }
                DataExtractor().executeUserTableExtractor(fileNames);
                DEBUG_MSG(__func__, "Finished pulling and parsing data from Database");
            }
        }
        sleep(DB_POLL_THREAD_SLEEP_TIMER);
    }

    return 0;
}

int startMonitorUserTableService()
{
    pthread_t monitor;
    monitorStop.initFlag(false);
    pthread_create(&monitor, NULL, monitorUserTable, NULL);
    DEBUG_MSG(__func__, "user table monitor service created");

    return 0;
}