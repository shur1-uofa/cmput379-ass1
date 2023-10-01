#ifndef PROCESS_TABLE_HPP
#define PROCESS_TABLE_HPP

#include <sys/resource.h>
#include <sys/times.h>
#include <string>
#include <list>

#define MAX_PT_ENTRIES 32 // Max entries in the Process Table


enum ProcessStatus {
    RUNNING,
    STOPPED
};


struct PCB {
    pid_t pid;
    ProcessStatus status;
    std::string command;
    timeval createdTime;
};


class ProcessTable {
    private:    
    std::list<PCB> processTable;
    pid_t recent_pid;

    public:
    void addEntry( pid_t pid, std::string command, timeval currentTime );

    void removeEntry( pid_t pid );

    void changeStatus( pid_t pid, ProcessStatus status );

    std::list<PCB>::iterator begin();
    std::list<PCB>::iterator end();

    int size();
};

#endif