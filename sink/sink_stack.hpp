#ifndef SINK_STACK_H
#define SINK_STACK_H
#include "prevalidate_json.hpp"
#include <semaphore.h>
#define MAX_POOL_SIZE 20

class Sink 
{
    private:
        struct SinkItem{
            void *dataObject;
            std::uint8_t priority;
            std::uint8_t starveCounter;
            SinkItem *next;
            SinkItem(){
                next = NULL;
                starveCounter = 0;
            };
        };
        sem_t sinkLock;
        SinkItem *sinkHead;
        std::uint64_t sinkItemCount;
    public:
        Sink(){
            sinkHead = new SinkItem();
            sinkItemCount = 0;
            sem_init(&sinkLock, 0, 0);
        }
        int pushObject(void *object, std::uint8_t priority);
        int getCurrentSinkSpace() { return sinkItemCount;}
        void* popObject();
};

extern Sink globalReceiverSink;
extern Sink globalSenderSink;

#endif