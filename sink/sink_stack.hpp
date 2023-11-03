#ifndef SINK_STACK_H
#define SINK_STACK_H
#include "../socket/prevalidate_json.hpp"
#include <semaphore.h>
#include <list>
#include "../include/flag.h"
#include "../include/task.hpp"
#define MAX_POOL_SIZE 20

struct ExportSinkItem {
    void *dataObject;
    TaskPriority taskPriority;
    ExportSinkItem(){
        dataObject = NULL;
    }
    ExportSinkItem(void *object, TaskPriority taskPriority) : dataObject(object), taskPriority(taskPriority){}
};

class SinkItem{
    public:
        ExportSinkItem sinkItem;
        std::uint8_t starveCounter;
        SinkItem(void *object, std::uint8_t starveCounter, TaskPriority taskPriority){
            starveCounter = 0;
            sinkItem = ExportSinkItem(object, taskPriority);
        };
};

class Sink 
{
    private:
        std::list<SinkItem*> *sinkItemList;
        sem_t sinkLock;
        std::uint64_t sinkLimit;
        std::string debugPrefix;
    public:
        Sink();
        Sink(std::uint64_t maxSize, std::string debugPrefix);
        int pushObject(void *object, TaskPriority priority);
        int getCurrentSinkSpace();
        bool isSinkFull();
        ExportSinkItem popObject();
        TaskPriority getTopTaskPriority();
        bool isSinkEmpty();
};

extern Sink *globalReceiverSink;
extern Sink *globalSenderSink;

#endif