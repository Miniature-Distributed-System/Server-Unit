#include "sink_stack.hpp"
#include "../include/debug_rp.hpp"
#include "../include/logger.hpp"

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
    sinkItemList = new std::list<SinkItem*>;
    sinkLimit = maxSinkSize;
    this->debugPrefix = debugPrefix + ": ";
    sem_init(&sinkLock, 0, 1);
    Log().info(__func__, debugPrefix, " sink initlilized");
}

int Sink::pushObject(void *object, TaskPriority priority)
{
    if(sinkItemList->size() >= MAX_POOL_SIZE || !object){
        Log().info(__func__, debugPrefix, "Max pool size reached cannot push anymore");
        return -1;
    }

    sem_wait(&sinkLock);
    if(sinkItemList->size() == 0){
        sinkItemList->push_back(new SinkItem(object, calculateStarveCounter(priority), priority));
        goto end;
    } else {
        for(auto i = sinkItemList->begin(); i != sinkItemList->end(); i++){
            SinkItem *sinkItem = (*i);
            if(sinkItem->starveCounter < calculateStarveCounter(priority)){
                sinkItemList->insert(--i, new SinkItem(object, calculateStarveCounter(priority), priority));
                goto end;
            }
        } sinkItemList->push_back(new SinkItem(object, calculateStarveCounter(priority), priority));
    }
end:
    sem_post(&sinkLock);
    Log().info(__func__, debugPrefix, "Item added to sink, itemCount:",sinkItemList->size());
    return 0;
}

ExportSinkItem Sink::popObject(){
    SinkItem *topSinkItem, *nextTopSinkItem;
    ExportSinkItem exportSinkItem;
    
    if(sinkItemList->size() == 0){
        Log().info(__func__, debugPrefix, "Empty no more Items");
        return exportSinkItem;
    }

    sem_wait(&sinkLock);
    topSinkItem = sinkItemList->front();
    exportSinkItem = topSinkItem->sinkItem;
    delete topSinkItem;
    sinkItemList->pop_front();
    sem_post(&sinkLock);
    Log().info(__func__, debugPrefix, "topmost sink item popped itemCount:",sinkItemList->size());
    return exportSinkItem;
}

int Sink::getCurrentSinkSpace()
{
    return sinkItemList->size();
}

bool Sink::isSinkFull()
{
    if(sinkItemList->size() > sinkLimit)
        return true;
    return false;
}

bool Sink::isSinkEmpty()
{
   if(sinkItemList->size() == 0)
        return true;
    return false; 
}

TaskPriority Sink::getTopTaskPriority()
{
    SinkItem *topSinkItem;
    topSinkItem = sinkItemList->front();
    return topSinkItem->sinkItem.taskPriority;
}