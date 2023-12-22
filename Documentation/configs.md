# Config

The `configs.cpp` file in the root directory of the server has all the necessary configs for the server. This file is essential for the working of the server unit. it contains
few parameters for database access and default comfigurations. Any new constants that will be used should be declared here.

## Parameters

- `HOME_DIR` : This defines the working directory
- `DATABASE_NAME` : This defines the default database name. This database which will be queried for all operations.
- `DATABASE_URL` : This is the database url for Sql Server access.
- `DATABASE_USERNAME` : This defines the default user name to use in order to access protected SQL database.
- `DATABASE_PASSWORD` : This defines the default password the server uses to access the user profile of SQL Database.
- `TEMPLATE_FILE_DATA_DIR` : This is the template/rules files loctaion.
- `USER_FILE_DATA_DIR` : This is the user file location.
- `FINAL_RESULT_DATA_DIR` : This is the final location where the final results are stored.
- `INTER_RESULT_DATA_DIR` : This is the location for the intermediate/cleaned files location.
- `USERDAT_TABLE_NAME` : This is the user data table name in the SQL server database.
- `USERDAT_DAT_COL_ID` : This is the user data column name which stores the filename to be fecthed from the `USER_FILE_DATA_DIR`.
- `USERDAT_RES_COL_ID` : This is the user results column name in SQL database which contains the final result file name.
- `USERDAT_INTER_COL_ID` : This is the user intermediate column name in SQL database which contains the intermediate file name.
- `USERDAT_ALGO_COL_ID` : This is the user algorithm column name in SQL Database which gives us which algorithm to use for processing the corresponding data.
- `USERDAT_DAT_PRIORITY_COL_ID` : This is the user data priority column name in the SQL Database which gives the priority for processing the file.
- `USERDAT_ALIASNAME_COL_ID` : This is the user data alias column name in the SQL Database. Every user data has a Alias name in the SQL database.(Will be changed soon)
- `USERDAT_STATUS_COL_ID` : This is the user data status column name in SQL Database. This stores the status of processing the user data file is in to the WebApp.
- `TEMPLATE_TABLE_NAME` : This stores the table name of Template/Rules in the SQL Database.
- `TEMPLATE_NAME_COL_ID` : This stores the template rule name column name in the SQL Database. Each rule has a name that is stored in the name column. This is the primary key.
- `TEMPLATE_ALGO_COL_ID` : This stores the algorithm index to use for worker unit will use to identify the Rule.
- `TEMPLATE_FILE_COL_ID` : This is the directory location of the Template/Rule file.
