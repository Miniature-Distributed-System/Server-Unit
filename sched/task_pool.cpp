#include "task_pool.hpp"
#include "task_scheduler.hpp"
#include "../include/debug_rp.hpp"
#include "../include/logger.hpp"

TaskPool::TaskPool()
{
    sem_init(&taskPoolLock, 0, 1);
    taskPoolCount = 0;
    headNode = NULL;
    Log().info(__func__,"inited TaskPool!");
}

int TaskPool::addTask(taskStruct *task, TaskPriority loadType)
{
    taskPoolNode *node, *tailNode;
    
    if(taskPoolCount >= MAX_TASK_POOL_SIZE)
    {
        Log().info(__func__,"Task Queue is full");
        return -1;
    }
    
    sem_wait(&taskPoolLock);
    if(!headNode){
        headNode = new taskPoolNode();
        headNode->taskItem = task;
        headNode->taskType = loadType;
        taskPoolCount++;
    } else {
        node = headNode;
        while(node){
            if(node->next == NULL){
                tailNode = new taskPoolNode();
                tailNode->taskItem = task;
                tailNode->taskType = loadType;
                node->next = tailNode;
                taskPoolCount++;
                sem_post(&taskPoolLock);
                Log().info(__func__, "New node added at tail: ", taskPoolCount);
                pthread_cond_signal(&cond);
                return 0;
            }
            node = node->next;
        }
    }

    sem_post(&taskPoolLock);
    Log().info(__func__, "New node added at tail: ", taskPoolCount);
    pthread_cond_signal(&cond);
    return 0;
}

taskPoolNode TaskPool::popTask()
{
    taskPoolNode task, *temp;

    if(taskPoolCount <= 0){
        Log().info(__func__, "The task pool is empty");
        return taskPoolNode();
    }
    sem_wait(&taskPoolLock);
    temp = headNode;
    headNode = temp->next;
    task.copyObject(temp);
    delete temp;
    taskPoolCount--;
    sem_post(&taskPoolLock);
    Log().info(__func__, "Popped item, total remaining items in queue: ",taskPoolCount);
    return task;
}