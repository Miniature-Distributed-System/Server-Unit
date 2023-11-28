# Monitor Services
- These services are responsible for maintaining connection with SQL Server data base and provide core data.
- Note that these services are very important and failure in this service can cause the server to lose its connection to SQL DB and break fundamental functionality of the system.
  
## User Data Service

- Location: `services/monitor_user_table.cpp`
- This service monitors the user data table in the SQL Server Database that is updated by the webapp when the user adds/deletes any record for processing.
- The `monitorUserTable` method is a thread that gets launched by the Main Monitor process.
- This above method periodically polls the SQL Table for any changes in the `user_data` (subject to change) in the SQL Server Database.
- The `timestamp` column is checked with the servers own updated timestamp if the latest timestamp in the `user_data` table is larger than that of recorded by the server then
the server initiates the Data Extractor and calls the `executeUserTableExtractor` method for extracting and creating a data structure for all the latest user data entered in
the table.
- It pulls the names of all the 'New' user data in the table and passes the name to `executeUserTableExtractor` method for further processing by `DataExtractor`.
- Once the control returns the thread goes to sleep for specfic duration defined by `DB_POLL_THREAD_SLEEP_TIMER` in `configs.h`.

## Template Service 

Important Note: Renaming underway `instance` is being renamed to `template` as it makes more sense. The user data has a 'Template' with rules for processing user data which conform to that
template which is more meaninful than the word 'Instance'. Therfore i shall be referring to all instance words as template.
If this change has reflected everywhere this note needs to be removed.
- Location: `services/monitor_instance_table.cpp`
- This service monitors the the User data template table in the SQL Server Database that gets updated by the product owners/admins.
- The `monitorInstanceTable` is a poll tread that gets initiated by the Main Monitor process (has same funtioanlity as the above service).
- This above method periodically polls the SQL Table for any changes in the `instance` (subject to change) in the SQL Server Database.
- The `timestamp` column is checked with the servers own updated timestamp if the latest timestamp in the `instance` table is larger than that of recorded by the server then
the server initiates the Data Extractor and calls the `executeInstanceExtractor` method for extracting and creating a data structure for all the latest Template entered in
the table.
- It pulls the names of all the 'New/Modified' Templates in the table and passes the name to `executeInstanceExtractor` method for further processing by `DataExtractor`.
- Once the control returns the thread goes to sleep for specfic duration defined by `DB_POLL_THREAD_SLEEP_TIMER` in `configs.h`.

