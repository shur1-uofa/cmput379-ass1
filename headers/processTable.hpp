#ifndef PROCESS_TABLE_HPP
#define PROCESS_TABLE_HPP

#include <unistd.h>
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
    // Don't forget time
};


class ProcessTable {
    private:    
    std::list<PCB> processTable;
    pid_t recent_pid;

    public:
    void addEntry( pid_t pid, std::string command );

    void removeEntry( pid_t pid );

    void changeStatus( pid_t pid, ProcessStatus status );

    /*
        Returns the most recently added process's id 
        BUG:
            If the process with recent PID is removed, then the id is stale.
            However, I don't care because it's exclusively used for the parent process to obtain the child process's id
            right away and not any later.  
    */
    pid_t getRecentPID();

    std::list<PCB>::iterator begin();
    std::list<PCB>::iterator end();

    int size();
};

#endif