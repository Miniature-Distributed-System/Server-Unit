#include "sink_stack.hpp"
#include "../include/debug_rp.hpp"

enum starveLimit
{
    PRIOR_1_STRVLMT = 4,
    PRIOR_2_STRVLMT = 6,
    PRIOR_3_STRVLMT = 8
};

//Return priority based on how straved the process is
int getStraveLimit(int prior)
{
    switch(prior){
        case 1: return PRIOR_1_STRVLMT;
        case 2: return PRIOR_2_STRVLMT;
        case 3: return PRIOR_3_STRVLMT;
        default: return 0;
    }
}

int Sink::pushObject(void *object, std::uint8_t priority)
{
    SinkItem *currentSinkItem, *sinkIterator, *prevSinkItem;
    std::uint8_t curPacketPriority;

    if(sinkItemCount >= MAX_POOL_SIZE){
        DEBUG_MSG(__func__, "Max pool size reached cannot push anymore");
        return -1;
    }
    sem_wait(&sinkLock);
    if(sinkHead->next == NULL){
        currentSinkItem = new SinkItem();
        currentSinkItem->dataObject = object;
        // currentSinkItem->packet = packet;
        sinkHead->next = currentSinkItem;
    }else{
        prevSinkItem = currentSinkItem;
        sinkIterator = prevSinkItem->next;

        while(sinkIterator){
            curPacketPriority = priority;
            if(curPacketPriority > sinkIterator->priority){
                //Item is already starved beyond its threshold then it won't be skipped
                if(sinkIterator->starveCounter > getStraveLimit(sinkIterator->priority)){
                    SinkItem* temp = sinkIterator->next;
                    sinkIterator->next = currentSinkItem;
                    currentSinkItem->next = temp;
                    goto end;
                }
            } else if(sinkIterator->next == NULL){
                sinkIterator->next = currentSinkItem;
                goto end;
            }
            prevSinkItem = sinkIterator;
            sinkIterator = sinkIterator->next;
        }
    }
    DEBUG_ERR(__func__, "Insertion failed for some reason");
    return -1;
end:
    sinkItemCount++;
    sem_post(&sinkLock);
    DEBUG_MSG(__func__, "Item added to sink");
    return 0;
}

void* Sink::popObject(){
    SinkItem *topSinkItem, *nextTopSinkItem;
    void* object;
    
    if(sinkItemCount <= 0){
        DEBUG_MSG(__func__, "Empty no more Items");
        return NULL;
    }
    sem_wait(&sinkLock);
    topSinkItem = sinkHead->next;
    sinkHead->next = topSinkItem->next;
    object = topSinkItem->dataObject;
    delete topSinkItem;
    sem_post(&sinkLock);
    return object;
}
