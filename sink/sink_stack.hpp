#ifndef SINK_STACK_H
#define SINK_STACK_H
#include "prevalidate_json.hpp"
#include <semaphore.h>
#include "../include/flag.h"
#include "../include/task.hpp"
#define MAX_POOL_SIZE 20

struct ExportSinkItem {
    void *dataObject;
    ExportSinkItem();
    ExportSinkItem(void *object){
        dataObject = object;
    }
};

struct SinkItem{
    ExportSinkItem *sinkItem;
    std::uint8_t starveCounter;
    SinkItem *next;
    SinkItem(void *object, std::uint8_t starveCounter){
        next = NULL;
        starveCounter = 0;
        sinkItem = new ExportSinkItem(object);
    };
};

class Sink 
{
    private:
        sem_t sinkLock;
        SinkItem *sinkHead;
        std::uint64_t sinkItemCount;
        std::uint64_t sinkLimit;
        std::string debugPrefix;
    public:
        Sink(std::uint64_t maxSize, std::string debugPrefix);
        int pushObject(void *object, TaskPriority priority);
        int getCurrentSinkSpace();
        bool isSinkFull();
        ExportSinkItem popObject();
};

extern Sink globalReceiverSink;
extern Sink globalSenderSink;

#endif