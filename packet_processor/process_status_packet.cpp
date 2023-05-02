#include "../include/debug_rp.hpp"
#include "../worker_node/worker_registry.hpp"
#include "../include/packet.hpp"
#include "out_data_registry.hpp"
#include "process_packet.hpp"
#include "packet_constructor.hpp"

using nlohmann::json_schema::json_validator;

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
      }
    },
    "required": [
                 "id"
                 ],
    "type": "object"
}
)"_json;

ProcessStatusPacket::ProcessStatusPacket(json packet)
{
    json_validator validator;
    validator.set_root_schema(packetSchema);

    try{
        auto defaultPatch = validator.validate(packet["body"]);
        DEBUG_MSG(__func__, "packet is a valid status packet");
    }catch (const std::exception &e) {
        DEBUG_ERR(__func__, "is not a valid status packet, ", e.what());
        return;
    }
    
    tableId = packet["body"]["id"];
    workerUid = packet["id"];
    statusCode = packet["head"];
}

void ProcessStatusPacket::execute()
{
    json packet;
    Worker *worker;
    OutDataState *outDataState;
    switch(statusCode)
    {
        case P_RESET:
            worker = globalWorkerRegistry.getWorkerFromUid(workerUid);
            if(!worker){
                DEBUG_ERR(__func__, "worker with workerUid:", workerUid, " not found");
                return;
            }
            break;
        case P_DATA_ACK:
            worker = globalWorkerRegistry.getWorkerFromUid(workerUid);
            if(!worker){
                DEBUG_ERR(__func__, "worker with workerUid:", workerUid, " not found");
                return;
            }
            worker->matchAckablePacket(tableId);
            break;
        case P_INTR_RES:
            globalOutDataRegistry.updateTaskStatus(tableId, DATA_INTER);
            outDataState = globalOutDataRegistry.getOutDataRegistryFromId(tableId);
            if(outDataState){
                 worker->queuePacket(
                    new OutPacket(
                        PacketConstructor().create(SP_INTR_ACK, workerUid), outDataState, false
                    )
                );
                DEBUG_MSG(__func__,"table:",tableId,"intermediate result received");
            }
            break;
        case P_FINAL_RES:
            globalOutDataRegistry.updateTaskStatus(tableId, DATA_FINAL);
            outDataState = globalOutDataRegistry.getOutDataRegistryFromId(tableId);
            if(outDataState){
                 worker->queuePacket(
                    new OutPacket(
                        PacketConstructor().create(SP_FRES_ACK, workerUid), outDataState, false
                    )
                );
                DEBUG_MSG(__func__,"table:",tableId,"final result received");
            }
            break;
        case P_ERR:
            //TO-DO:
        default:
            DEBUG_ERR(__func__,"packet status did not match any known status codes");
    }
}