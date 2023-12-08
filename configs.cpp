#include "configs.h"

std::string HOME_DIR = "/home/tejas/temp";
std::string DATABASE_NAME = "web";

std::string DATABASE_URL = "tcp://127.0.0.1:3306";
std::string DATABASE_USERNAME = "mcu";
std::string DATABASE_PASSWORD = "password";

std::string TEMPLATE_FILE_DATA_DIR = "/instance/";
std::string USER_FILE_DATA_DIR = "/user_data/";
std::string FINAL_RESULT_DATA_DIR = "/final_result/";
std::string INTER_RESULT_DATA_DIR = "/inter_result/";

std::string USERDAT_TABLE_NAME = "Home_filelog";
std::string USERDAT_DAT_COL_ID = "file_name";
std::string USERDAT_RES_COL_ID = "final_result";
std::string USERDAT_INTER_COL_ID = "inter_result";
std::string USERDAT_ALGO_COL_ID = "instance_name";
std::string USERDAT_DAT_PRIORITY_COL_ID = "priority";
std::string USERDAT_ALIASNAME_COL_ID = "aliasname";
std::string USERDAT_STATUS_COL_ID = "status";

std::string TEMPLATE_TABLE_NAME = "Home_instances";
std::string TEMPLATE_NAME_COL_ID = "instance_name";
std::string TEMPLATE_ALGO_COL_ID = "algorithm";
std::string TEMPLATE_FILE_COL_ID = "csvfile";