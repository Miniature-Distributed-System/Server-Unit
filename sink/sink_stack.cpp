#include "sink_stack.hpp"
#include "../include/debug_rp.hpp"

//TO-DO:
//These are randomly choosen values no actual research has been done on this
enum starveLimit
{
    PRIOR_1_STRVLMT = 6, // High priority process are default very starved
    PRIOR_2_STRVLMT = 4,
    PRIOR_3_STRVLMT = 0 // Lowest priority least starved
};

// Calculate and return the starvation value based on data items priority
int calculateStarveCounter(TaskPriority priority)
{
    switch(priority){
        case HIGH_PRIORITY: return PRIOR_1_STRVLMT;
        case MEDIUM_PRIORITY: return PRIOR_2_STRVLMT;
        default: return PRIOR_3_STRVLMT;
    }
}

Sink::Sink(){}

Sink::Sink(std::uint64_t maxSinkSize, std::string debugPrefix){
    sinkHead = new SinkItem(NULL, 0);
    sinkItemCount = 0;
    sinkLimit = maxSinkSize;
    this->debugPrefix = debugPrefix + ": ";
    sem_init(&sinkLock, 0, 0);
    DEBUG_MSG(__func__, debugPrefix, " sink initlilized");
}

int Sink::pushObject(void *object, TaskPriority priority)
{
    SinkItem *currentSinkItem, *sinkIterator, *prevSinkItem;
    std::uint8_t curDataStarvation;

    if(sinkItemCount >= MAX_POOL_SIZE){
        DEBUG_MSG(__func__, debugPrefix, "Max pool size reached cannot push anymore");
        return -1;
    }

    sem_wait(&sinkLock);
    if(sinkHead->next == NULL){
        currentSinkItem = new SinkItem(object, calculateStarveCounter(priority));
        sinkHead->next = currentSinkItem;
        DEBUG_MSG(__func__, debugPrefix, "first data element created");
    } else {
        prevSinkItem = sinkHead;
        sinkIterator = sinkHead->next;
        curDataStarvation = calculateStarveCounter(priority);

        while(sinkIterator){
            //Check who is more hungry current Data or data in List
            if(curDataStarvation > sinkIterator->starveCounter){
                prevSinkItem->next = currentSinkItem;
                currentSinkItem->next = sinkIterator;
                goto end;
            } else if(sinkIterator->next == NULL){
                sinkIterator->next = currentSinkItem;
                goto end;
            }
            prevSinkItem = sinkIterator;
            sinkIterator = sinkIterator->next;
        }
    }
    DEBUG_ERR(__func__, debugPrefix, "Insertion failed for some reason");
    return -1;
end:
    sinkItemCount++;
    sem_post(&sinkLock);
    DEBUG_MSG(__func__, debugPrefix, "Item added to sink");
    return 0;
}

ExportSinkItem Sink::popObject(){
    SinkItem *topSinkItem, *nextTopSinkItem;
    ExportSinkItem exportSinkItem;
    
    if(sinkItemCount <= 0){
        DEBUG_MSG(__func__, debugPrefix, "Empty no more Items");
        return exportSinkItem;
    }

    sem_wait(&sinkLock);
    topSinkItem = sinkHead->next;
    sinkHead->next = topSinkItem->next;
    exportSinkItem.dataObject = topSinkItem->sinkItem->dataObject;
    delete topSinkItem;
    sem_post(&sinkLock);
    DEBUG_MSG(__func__, debugPrefix, "topmost sink item popped");
    return exportSinkItem;
}

int Sink::getCurrentSinkSpace()
{
    return sinkItemCount;
}

bool Sink::isSinkFull()
{
    if(sinkItemCount > sinkLimit)
        return true;
    return false;
}
