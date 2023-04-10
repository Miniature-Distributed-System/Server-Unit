#include "../include/debug_rp.hpp"
#include "../worker_node/worker_registry.hpp"
#include "../include/packet.hpp"
#include "out_data_registry.hpp"
#include "process_packet.hpp"
#include "packet_constructor.hpp"

ProcessStatusPacket::ProcessStatusPacket(json packet)
{
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