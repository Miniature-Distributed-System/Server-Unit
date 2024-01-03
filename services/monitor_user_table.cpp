#include <pthread.h>
#include <vector>
#include <list>
#include "../configs.h"
#include "../data_extractor/data_extractor.hpp"
#include "../sink/sink_stack.hpp"
#include "../packet_processor/outgoing_data_registry.hpp"
#include "../include/debug_rp.hpp"
#include "../include/logger.hpp"
#include "sql_access.hpp"
#include "monitor.hpp"

/*
  monitorUserTable: monitors the SQL Server Database for any newly added User data records.
  It uses the inital timestamp as 2022-05-01 00:00:00 and checks any new data after that. if
  new data found then latest timestamp is updated. It extracts the user data table record 
  names that have been added. It calls the User data extractors to do further extraction and
  triggering next stages. It sleeps after every poll SQL Server can get overwhelmed 
  especially when there are two monitors working on a low end system.
*/
void *monitorUserTable(void *data)
{
    //TO-DO
    std::string queryTimeStamp = "SELECT MAX(timestamp) as timestamp FROM " + USERDAT_TABLE_NAME + ";" ;
    std::string columnName = "timestamp";
    std::string latestTimeStamp = "2022-05-01 00:00:00", currentTimeStamp;
    Log().info(__func__, "started monitoring user data table...");
    SqlAccess *sqlAccess = new SqlAccess(DATABASE_URL, DATABASE_USERNAME, DATABASE_PASSWORD, DATABASE_NAME, 
                    USERDAT_TABLE_NAME);
    sqlAccess->initialize();

    while(!monitorStop.isFlagSet())
    {
        if(!globalSenderSink->isSinkFull() || !globalReceiverSink->isSinkFull())
        {
            currentTimeStamp = sqlAccess->sqlQueryDb(queryTimeStamp, columnName);
            if(currentTimeStamp.compare(latestTimeStamp))
            {
                
                Log().info(__func__, "current time stamp:", currentTimeStamp, " saved timestamp: ", latestTimeStamp);
                std::string queryTableName = "SELECT " +  USERDAT_DAT_COL_ID + " FROM " + USERDAT_TABLE_NAME + 
                                " WHERE timestamp > '" + latestTimeStamp + "' AND timestamp <= CURRENT_TIMESTAMP;"; 
                latestTimeStamp = currentTimeStamp;
                std::list<std::string> fileNames = sqlAccess->sqlQueryDbList(queryTableName);
                DataExtractor().executeUserTableExtractor(fileNames, sqlAccess);
                Log().info(__func__, "Finished pulling and parsing data from Database");
            }
        }
        sleep(DB_POLL_THREAD_SLEEP_TIMER);
    }

    delete sqlAccess;
    return 0;
}

int startMonitorUserTableService()
{
    pthread_t monitor;
    monitorStop.initFlag(false);
    pthread_create(&monitor, NULL, monitorUserTable, NULL);
    Log().info(__func__, "user table monitor service created");

    return 0;
}
