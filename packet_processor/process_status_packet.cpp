#include "../include/debug_rp.hpp"
#include "../worker_node/worker_registry.hpp"
#include "../sender_unit/instance.hpp"
#include "../sender_unit/worker_instance_list.hpp"
#include "../include/packet.hpp"
#include "../include/logger.hpp"
#include "out_data_registry.hpp"
#include "process_packet.hpp"
#include "packet_constructor.hpp"

using nlohmann::json_schema::json_validator;

static json packetIdSchema = R"(
{
    "$schema": "http://json-schema.org/draft-07/schema#",
    "title": "packet validator",
    "properties": {
      "id": {
          "description": "table ID",
          "type": "string"
      }
    },
    "required": [
                 "id"
                 ],
    "type": "object"
}
)"_json;

static json packetStatsSchema = R"(
{
    "$schema": "http://json-schema.org/draft-07/schema#",
    "title": "packet validator",
    "properties": {
      "avgQueueTime": {
          "description": "Average queue time",
          "type": "number"
      },
      "taskQueue": {
          "description": "Task queue vector in string form",
          "type": "string"
      },
      "totalAvailableThreads": {
          "description": "Total alloted threads for this worker",
          "type": "number"
      }
    },
    "required": [
                 "avgQueueTime",
                 "taskQueue",
                 "totalAvailableThreads"
                 ],
    "type": "object"
}
)"_json;

ProcessStatusPacket::ProcessStatusPacket(json packet)
{
    json_validator validator;
    validator.set_root_schema(packetIdSchema);

    try{
        auto defaultPatch = validator.validate(packet);
        Log().pktProcessorInfo(__func__, "packet has valid status");
    }catch (const std::exception &e) {
        Log().debug(__func__, "is not a valid status packet, ", e.what());
        return;
    }
    try{
        tableId = packet["body"]["id"];
    } catch (const std::exception &e) {
        tableId = "";
    }
    workerUid = packet["id"];
    statusCode = packet["head"];

    validator.set_root_schema(packetStatsSchema);
    
    try{
        auto defaultPatch = validator.validate(packet["stats"]);
        Log().pktProcessorInfo(__func__, "packet has valid stats header");
    }catch (const std::exception &e) {
        Log().debug(__func__, "does not have a valid stats header, ", e.what());
        statsPresent.initFlag(false);
        return;
    }
    statsPresent.initFlag(true);
    queueTime = packet["stats"]["avgQueueTime"];
    vectorString = packet["stats"]["taskQueue"];
    threadCount = packet["stats"]["totalAvailableThreads"];
}

std::vector<int> adv_tokenizer(std::string s, char del)
{
    std::stringstream ss(s);
    std::string word;
    std::vector<int> result;
    while (!ss.eof()) {
        std::getline(ss, word, del);
        result.push_back(std::stoi(word));
    }

    return result;
}

void ProcessStatusPacket::packetStatusParse()
{
    json packet;
    Worker *worker;
    OutDataState *outDataState;
    
    if(P_QSEND & statusCode){
        statusCode &= ~(P_QSEND);
    }

    worker = globalWorkerRegistry.getWorkerFromUid(workerUid);
    if(!worker){
        Log().error(__func__, "worker with workerUid:", workerUid, " not found");
        return;
    }

    switch(statusCode)
    {
        case P_RESET:
            if(globalInstanceRegistery.isInstanceId(tableId)){
                // Update tracker about the table in processing
                workerInstanceList.updateWorker(worker->getWorkerUID(), tableId);
            }
            break;
        case P_DATA_ACK:
            if(worker->matchAckablePacket(tableId)){
                if(globalInstanceRegistery.isInstanceId(tableId)){
                    // Update tracker about the received instance table
                    workerInstanceList.updateWorker(worker->getWorkerUID(), tableId);
                }
                Log().pktProcessorInfo(__func__, "packet with ID: ", tableId, " was acknowledged");
            } else Log().error(__func__, "packet with ID: ", tableId, " was not found in worker list");
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
                Log().pktProcessorInfo(__func__,"table:",tableId,"intermediate result received");
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
                Log().pktProcessorInfo(__func__,"table:",tableId,"final result received");
            }
            break;
        case P_ERR:
            //TO-DO:
        default:
            Log().debug(__func__,"packet status did not match any known status codes");
    }

    if(statsPresent.isFlagSet()){
        if(worker){
            WorkerStats workerStats = WorkerStats(threadCount, adv_tokenizer(vectorString, ','), queueTime);
            worker->setWorkerStats(workerStats);
            Log().pktProcessorInfo(__func__, "updated worker:", worker->getWorkerUID());
        } else Log().pktProcessorInfo(__func__, "Skipping stats update for worker:", worker->getWorkerUID());
    }

    Log().pktProcessorInfo(__func__, "Finished processing ", tableId," status");
}