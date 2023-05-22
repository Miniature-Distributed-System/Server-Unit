#include "worker_stats.hpp"

WorkerStats::WorkerStats()
{
    totalAvilableThreads = 0;
    avgQueueTime = 0;
}

WorkerStats::WorkerStats(int totalAvilableThreads, std::vector<int> taskQueue, double avgQueueTime) : 
                totalAvilableThreads(totalAvilableThreads), taskQueue(taskQueue), avgQueueTime(avgQueueTime) {}

