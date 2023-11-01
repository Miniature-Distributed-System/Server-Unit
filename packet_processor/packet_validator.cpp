#include "../include/process.hpp"
#include "../include/task.hpp"
#include "../include/debug_rp.hpp"
#include "../sched/task_pool.hpp"
#include "../sink/sink_stack.hpp"
#include "../include/logger.hpp"
#include "packet_validator.hpp"

static json packetSchema = R"(
{
    "$schema": "http://json-schema.org/draft-07/schema#",
    "title": "packet validator",
    "properties": {
      "id": {
          "description": "table ID",
          "type": "string"
      },
      "priority": {
          "description": "CU identification number",
          "type": "number",
          "minimum": 0,
          "maximum": 3
      },
      "data": {
          "description": "Data of packet",
          "type": "string"
      }
    },
    "required": [
                 "id",
                 "priority",
                 "data"
                 ],
    "type": "object"
}
)"_json;

void PacketValidator::validatePacket()
{
    json_validator validator;
    validator.set_root_schema(packetSchema);

    try{
        auto defaultPatch = validator.validate(packet["body"]);
        Log().pktProcessorInfo(__func__, "packet is a valid data packet");
    }catch (const std::exception &e) {
        Log().debug(__func__, "is not a valid data packet, ", e.what());
        isValid = false;
        return;
    }
    isValid = true;

}

JobStatus start_validator(void *data)
{
    JsonExport *jsonExportObject = (JsonExport*)globalReceiverSink->popObject().dataObject;
    if(!jsonExportObject)
        return JOB_DONE;
    PacketValidator *packet = new PacketValidator(jsonExportObject->packet);
    jsonExportObject->destruct();
    packet->validatePacket();

    if(packet->isDataPacket()){
        ProcessDataPacket processData(packet->getPacket());
        processData.execute();
    } 
    ProcessStatusPacket processStatusPacket(packet->getPacket());
    processStatusPacket.packetStatusParse();

    return JOB_PENDING;
}

JobStatus pause_valdiator(void *data){
    return JOB_PENDING;
}

JobStatus end_valdidator(void *data, JobStatus status){
    return JOB_DONE;
}

struct process *valdiatorProcess = new process{
    .start_proc = start_validator,
    .pause_proc = pause_valdiator,
    .end_proc = end_valdidator
};

int init_validator()
{
    globalTaskPool->addTask(new taskStruct(valdiatorProcess, NULL), MEDIUM_PRIORITY);
    return 0;
}