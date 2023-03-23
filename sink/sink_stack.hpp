#ifndef SINK_STACK_H
#define SINK_STACK_H
#include "prevalidate_json.hpp"
#include <semaphore.h>
#include "../include/task.hpp"
#define MAX_POOL_SIZE 20

struct SinkItem{
    void *dataObject;
    TaskPriority priority;
    std::uint8_t starveCounter;
    SinkItem *next;
    SinkItem(){
        next = NULL;
        starveCounter = 0;
    };
};

class Sink 
{
    private:
        sem_t sinkLock;
        SinkItem *sinkHead;
        std::uint64_t sinkItemCount;
        std::uint64_t sinkLimit;
    public:
        Sink(std::uint64_t maxSize);
        int pushObject(void *object, std::uint8_t priority);
        int getCurrentSinkSpace();
        bool isSinkFull();
        void* popObject();
};

extern Sink globalReceiverSink;
extern Sink globalSenderSink;

#endif