#ifndef MONITOR_H
#define MONITOR_H

#include "../include/flag.h"

extern Flag monitorStop;
int startMonitorUserTableService(void);
int startMonitorInstanceTable(void);
#endif