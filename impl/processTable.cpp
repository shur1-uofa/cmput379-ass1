#include "../headers/processTable.hpp"


void ProcessTable::addEntry( pid_t pid, std::string command ) {
    PCB pcb;
    pcb.pid = pid; 
    pcb.status = RUNNING;
    pcb.command = command;
    this->processTable.push_back( pcb );
    this->recent_pid = pid;
}

void ProcessTable::removeEntry( pid_t pid ) {
    this->processTable.remove_if( [pid](PCB pcb) {
        return pcb.pid == pid; 
    });
}

void ProcessTable::changeStatus( pid_t pid, ProcessStatus status ) {
    for (auto& pcb : this->processTable) {
        if ( pcb.pid == pid ) {
            pcb.status = status;
            break;
        }
    }
    return; 
}

pid_t ProcessTable::getRecentPID() {
    return recent_pid;
}

std::list<PCB>::iterator ProcessTable::begin() {
    return this->processTable.begin();
}

std::list<PCB>::iterator ProcessTable::end() {
    return this->processTable.end();
}

int ProcessTable::size() {
    return this->processTable.size();
}