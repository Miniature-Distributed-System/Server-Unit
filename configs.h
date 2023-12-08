#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#define MAX_THREADS 2
#define DB_POLL_THREAD_SLEEP_TIMER 2
#define WORKER_QUEUE_SIZE 20

extern std::string HOME_DIR;
extern std::string DATABASE_NAME;

extern std::string DATABASE_URL;
extern std::string DATABASE_USERNAME;
extern std::string DATABASE_PASSWORD;

extern std::string TEMPLATE_FILE_DATA_DIR;
extern std::string USER_FILE_DATA_DIR;
extern std::string FINAL_RESULT_DATA_DIR;
extern std::string INTER_RESULT_DATA_DIR;

extern std::string USERDAT_TABLE_NAME;
extern std::string USERDAT_DAT_COL_ID;
extern std::string USERDAT_RES_COL_ID;
extern std::string USERDAT_INTER_COL_ID;
extern std::string USERDAT_ALGO_COL_ID;
extern std::string USERDAT_DAT_PRIORITY_COL_ID;
extern std::string USERDAT_ALIASNAME_COL_ID;
extern std::string USERDAT_STATUS_COL_ID;

extern std::string TEMPLATE_TABLE_NAME;
extern std::string TEMPLATE_NAME_COL_ID;
extern std::string TEMPLATE_ALGO_COL_ID;
extern std::string TEMPLATE_FILE_COL_ID;

#endif