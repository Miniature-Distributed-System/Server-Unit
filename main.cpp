#include <iostream>
#include "socket/socket.hpp"
#include "sink/sink_stack.hpp"
#include "sched/task_pool.hpp"
#include "sched/task_scheduler.hpp"
#include "sched/timeout.hpp"
#include "services/sql_access.hpp"
#include "services/monitor.hpp"
#include "services/timeout_timer.hpp"
#include "sender_unit/instance.hpp"
#include "packet_processor/out_data_registry.hpp"
#include "worker_node/worker_registry.hpp"
#include "sender_unit/sender_core.hpp"
#include "include/debug_rp.hpp"
#include "configs.h"

Flag monitorStop;
Sink *globalReceiverSink;
Sink *globalSenderSink;
WorkerRegistry globalWorkerRegistry;
OutDataRegistry globalOutDataRegistry;
SqlAccess *globalSqlAccess;
TaskPool *globalTaskPool;
SenderCoreData *senderCoreData;
SenderCore senderCore;
Timeout *packetTimeout;
Instance globalInstanceRegistery;

int main()
{
    std::string address;
    int port, socketThreads;
    int sinkMaxSize = 20;
    int timeoutTimerSleepSec = 5;
    int maxThreads = 2;
    std::string dir, username, password, name;

    monitorStop.initFlag();

    std::cout << "--------------------------------------------------------";
    std::cout << "Server Initilization";
    std::cout << "--------------------------------------------------------" << std::endl;
    std::cout << "\nEnter IP address: ";
    std::cin >> address;
    std::cout << "\nEnter Port number: ";
    std::cin >> port;
    std::cout << "\nEnter number of socket threads: ";
    std::cin >> socketThreads;
    std::cout << "\nEnter number of scheduler threads: ";
    std::cin >> maxThreads;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cout << "\nEnter server output directory full path: ";
    std::cin.clear();
    getline(std::cin, dir);
    if(!dir.empty()){
        HOME_DIR = dir;
    } else std::cout << "\nUsing default output directory";
    
    std::cout << "\nEnter database name: ";
    std::cin.clear();
    getline(std::cin, name);
    std::cout << "\nEnter database username: ";
    std::cin.clear();
    getline(std::cin, username);
    std::cout << "\nEnter database password:";
    std::cin.clear();
    getline(std::cin, password);
    if(username.empty() || password.empty() || name.empty()){
        std::cout << "\nUsing default database username/password";
    } else {
        DATABASE_NAME = name;
        DATABASE_USERNAME = username;
        DATABASE_PASSWORD = password;
    }

    std::cout << "--------------------------------SERVER STARTING--------------------------------";
    std::cout << "-----------------------------Logs Starts From Here-----------------------------\n\n";

    globalReceiverSink = new Sink(sinkMaxSize, "ReceiverSink");
    globalSenderSink = new Sink(sinkMaxSize, "SenderSink");
    DEBUG_MSG(__func__, "initilized sender/receiver sinks");

    DEBUG_MSG(__func__, "initilized registry");

    globalSqlAccess = new SqlAccess(DATABASE_URL, DATABASE_USERNAME, DATABASE_PASSWORD, DATABASE_NAME, USERDAT_TABLE_NAME);
    globalSqlAccess->initialize();
    DEBUG_MSG(__func__, "Initilized SQL accesss libs");

    globalTaskPool = new TaskPool();
    DEBUG_MSG(__func__, "Task Pool initilized!");
    init_sched(maxThreads);
    DEBUG_MSG(__func__, "Scheduler Up and Running!");

    senderCoreData = new SenderCoreData();
    senderCore.run();
    DEBUG_MSG(__func__, "Sender core Up and Running!");

    packetTimeout = new Timeout();
    startMonitorInstanceTable();
    startMonitorUserTableService();
    init_timer(timeoutTimerSleepSec);
    DEBUG_MSG(__func__, "Monitor services Up and Running");

    startSocket(address, port, socketThreads);
    DEBUG_MSG(__func__, "Socket up and running");

    DEBUG_MSG(__func__, "All components Up and Running");
    std::cout << "--------------------------------------------------------";
    while(1);

    return 0;
}