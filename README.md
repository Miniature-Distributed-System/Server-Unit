# Server

This unit is responsible for communicating with the worker nodes and scheduling tasks for the workers. The server is core and needs to be always running. It monitors the database
for any changes done by the WebApp aka User. Server takes the data added by user and forwards it to the worker and waits for the result from the worker.

## Working

- Step 1:
    - The server polls the database of the webapp for any changes in User or Template table.
    - The User table contains the data to be processed and the Template table contains the rules for processing this data.
    - Once server finds new data the Data that was recently added is pulled using primary key.
    - This is sent to Data extractor.
- Step 2:
   - The Data extractor works on the data and if its Template it creates Tempate struture and if its User data it creates the required data structure.
   - For User data it registers this data to be processed in the registry and starts tracking this data via registry and pushes it into sender sink.
   - For template data the a new template struct is constructed and added into list and `globalUserDataTemplateRegistry` is updated.
 - Step 3:
    - The scheduler picks up User data from the sink and does worker best match and queues it against the choosen worker.
    - If it was template data then all workers connected are updated with this new information.
  - Step 4:
     - When the worker reconnects it is sent this queued data and the registry is updated and timer is set.
     - If worker acknowledges within timeout the registry is updated into next status or its resent.
  - Step 5:
     - When worker sends the result of user data the user data result packet is queued initially in Receiver sink.
     - The Packet processor then preprocesses/validates the packet.
     - The information is extracted and Registry is updated into final state and discarded once done.
     - The WebApp database is updated with result.
