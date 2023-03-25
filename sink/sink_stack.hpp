#ifndef SINK_STACK_H
#define SINK_STACK_H
#include "prevalidate_json.hpp"
#include <semaphore.h>
#include "../include/flag.h"
#include "../include/task.hpp"
#define MAX_POOL_SIZE 20

struct ExportSinkItem {
    void *dataObject;
    Flag isUserData;
    ExportSinkItem(void *object, Flag isUserData){
        dataObject = object;
        this->isUserData = isUserData;
    }
};

struct SinkItem{
    ExportSinkItem *sinkItem;
    std::uint8_t starveCounter;
    SinkItem *next;
    SinkItem(void *object, Flag isUserData, std::uint8_t starveCounter){
        next = NULL;
        starveCounter = 0;
        sinkItem = new ExportSinkItem(object, isUserData);
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
        int pushObject(void *object, TaskPriority priority, bool isUserData = false);
        int getCurrentSinkSpace();
        bool isSinkFull();
        ExportSinkItem* popObject();
};

extern Sink globalReceiverSink;
extern Sink globalSenderSink;

#endif