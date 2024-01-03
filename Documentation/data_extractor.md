# Data Extractor

This module extracts the table information for the provided keys. They pull values from all fields of the User table/ Template table. It starts the pipeline. It also locates the
files in the predefined directories and pulls the data from the files and creates a basic DS for them.

### `executeInstanceExtractor()`

- This methods extracts the Template/rules for the user data for the Template table(currently called as instance but bound to change soon).
- When the monitor detects a new entry in the Template table it calls this method.
- It extracts the file data, template name/ID and algorithm it corrosponds to and creates a DS `UserDataTemplateStruct`.
- It stores this DS in queue for next stage processing `instanceList`.
- The Registry is updated `globalUserDataTemplateRegistry` which notifies the Worker Units Scheduler.

### `executeUserTableExtractor()`

- This method extracts the User data fields from the User Data table.
- When the monitor detects a new user data entry it invokes this method.
- It extracts the File data, Data name/ID, The Rule it conforms to, Priority and creates a DS using this `UserDataTable`.
- It stores this DS in queue for next stage processing `globalSenderSink`.
- The Registry is updated `globalOutgoingDataRegistry` which notifies the Worker Units Scheduler.

This module isnt handled by the server Task Scheduler. It needs to be made into a job soon and scheduled all memory and processes handling is schedulers job.
