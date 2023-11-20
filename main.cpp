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
#include "sender_unit/worker_instance_list.hpp"
#include "packet_processor/outgoing_data_registry.hpp"
#include "worker_node/worker_registry.hpp"
#include "sender_unit/sender_core.hpp"
#include "include/debug_rp.hpp"
#include "configs.h"

Flag monitorStop;
Sink *globalReceiverSink;
Sink *globalSenderSink;
WorkerRegistry globalWorkerRegistry;
OutgoingDataRegistry globalOutgoingDataRegistry;
SqlAccess *globalSqlAccess;
TaskPool *globalTaskPool;
SenderCoreData *senderCoreData;
SenderCore senderCore;
Timeout *packetTimeout;
UserDataTemplate globalUserDataTemplateRegistry;
WorkerInstanceList workerInstanceList;

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
    std::cout << "\033[1;97;49mServer Initilization\033[0m";
    std::cout << "--------------------------------------------------------" << std::endl;
    std::cout << "\n\e[33mEnter IP address: \033[0m";
    std::cin >> address;
    std::cout << "\n\e[33mEnter Port number: \033[0m";
    std::cin >> port;
    std::cout << "\n\e[33mEnter number of socket threads: \033[0m";
    std::cin >> socketThreads;
    std::cout << "\n\e[33mEnter number of scheduler threads: \033[0m";
    std::cin >> maxThreads;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cout << "\n\e[33mEnter server output directory full path: \033[0m";
    std::cin.clear();
    getline(std::cin, dir);
    if(!dir.empty()){
        HOME_DIR = dir;
    } else std::cout << "\nUsing default output directory";
    
    std::cout << "\n\e[33mEnter database name: \033[0m";
    std::cin.clear();
    getline(std::cin, name);
    std::cout << "\n\e[33mEnter database username: \033[0m";
    std::cin.clear();
    getline(std::cin, username);
    std::cout << "\n\e[33mEnter database password:\033[0m";
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