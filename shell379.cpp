#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/times.h>
#include <signal.h>
#include <iostream>
#include <string>

#include "headers/commandParser.hpp"
#include "headers/processTable.hpp"

#define LINE_LENGTH 100 // Max # of characters in an input line

/*
LIST OF BUGS:
I would appreciate it if you could figure it out because I am just done.

- When a child process ends, the console loops infinitely and keeps prints "SHELL:\n"

- Calculation for child process does not work at all. 
Setting the current user time from getrusage and saving it inside PCB and calculating process user time by calculating
the difference between that time and the current user time but just prints 0

There's likely others but the first one has been getting in the way for me to properly test this program.
*/

bool Running = true;
ProcessTable PROCESS_TABLE;


void printJobs();

void printCompletedTimes();

void suspendProcess(pid_t pid);

void waitProcess(pid_t pid);

void resumeProcess(pid_t pid);

void killProcess(pid_t pid);

void exitShell();

void spawnProcess(std::string line, CommandParser::Content cmdLine);

void handleChildTermination(int signum, siginfo_t *info, void *context) 
{
    // Contrary to documentation, info->si_pid does not return the child process id. 
    // #undef si_pid 
    // std::cout << "SIGNAL CALLED FROM CHILD " << (int) info->_sifields._sigchld.si_pid << std::endl;
    pid_t pid = waitpid(-1, NULL, WNOHANG); 
    while ( pid > 0 ) {
        PROCESS_TABLE.removeEntry( pid );
        pid = waitpid(-1, NULL, WNOHANG);
    }
}

int main() {

    struct sigaction childTerminator;
    childTerminator.sa_sigaction = handleChildTermination;
    sigemptyset( &childTerminator.sa_mask );
    childTerminator.sa_flags = SA_NOCLDSTOP || SA_SIGINFO;
    sigaction( SIGCHLD, &childTerminator, NULL);

    while( Running ) {
        std::string line; 
        std::cout << "SHELL379: ";
        getline(std::cin, line);
        CommandParser::Content cmdLine = CommandParser::parseLine(line);

        // TODO: implement & and <, > operator for system commands 
        if ( cmdLine.cmd == "jobs" ) {
            printJobs();
        }
        // TODO:
        // else if ( cmdLine.cmd == "sleep" ) {
        //     sleep( stoi(cmdLine.args[0]) );  
        // }
        else if ( cmdLine.cmd == "suspend" ) {
            suspendProcess( stoi(cmdLine.args[0]) );
        }
        else if ( cmdLine.cmd == "wait" ) {
            waitProcess( stoi(cmdLine.args[0]) );
        }
        else if ( cmdLine.cmd == "resume" ) {
            resumeProcess( stoi(cmdLine.args[0]) );
        }
        else if ( cmdLine.cmd == "kill" ) {
            killProcess( stoi(cmdLine.args[0]) );
        }
        else if ( cmdLine.cmd == "exit" ) {
            exitShell();
        }
        else if ( cmdLine.cmd != "" ) {
            spawnProcess(line, cmdLine);
        }
        else {
            // FIXME: WHY DOES IT LOOP INFINITELY?????
        }
    }

    return( 0 );
}


void printJobs() {
    std::cout << "Running processes:\n";
    std::cout << " #    PID S SEC COMMAND\n";
    int index = 0;
    struct rusage usage; 
    if ( getrusage( RUSAGE_SELF, &usage ) < 0 ) {
        perror("Failed to get current time");
    } 
    for (const auto pcb : PROCESS_TABLE) {
        std::cout << " " << index++ << ": " << pcb.pid << " ";
        if ( pcb.status == ProcessStatus::RUNNING ) {
            std::cout << 'R';
        } 
        else {
            std::cout << 'S';
        }
        // There might be some rounding error since I am not counting micro seconds but I don't care 
        // FIXME: But of course, it doesn't work anyways because why not! 
        std::cout << ' ' << usage.ru_utime.tv_sec - pcb.createdTime.tv_sec << ' ';
        std::cout << pcb.command << std::endl;
    }
    printCompletedTimes();
}

void printCompletedTimes() {
    struct rusage usage; 
    if ( getrusage( RUSAGE_CHILDREN, &usage ) < 0) {
        perror("Failed to obtain completed process time information");
    } 
    else {
        std::cout << "Completed processes:\n";
        std::cout << "User time = " << usage.ru_utime.tv_sec << " seconds\n";
        std::cout << "Sys time = " << usage.ru_stime.tv_sec << " seconds\n";
    }
}

void spawnProcess(std::string line, CommandParser::Content cmdLine) {

    pid_t pid = fork();

    if ( pid < 0 ) {
        exit(1);
    } 
    // Parent process 
    else if ( pid > 0 ) {
        struct rusage usage;
        timeval currentTime; 
        if ( getrusage( RUSAGE_SELF, &usage) < 0 ) {
            perror("Couldn't access current time");
        }
        else {
            currentTime = usage.ru_utime;
        }
        PROCESS_TABLE.addEntry( pid, line, currentTime );
        if ( !cmdLine.BACKGROUND_FLAG ) {
            waitProcess( pid );
        }
    }
    // Child process
    else {
        if ( cmdLine.inputFile != "" ) {
            int inputfd = open( cmdLine.inputFile.c_str(), O_RDONLY );
            dup2( inputfd, STDIN_FILENO );
        }
        if ( cmdLine.outputFile != "" ) {
            int outputfd = open( cmdLine.outputFile.c_str(), O_WRONLY );
            dup2( outputfd, STDOUT_FILENO );
        }
        
        // Run command 
        // C is pain 
        char* cargs[ cmdLine.args.size() + 2];
        cargs[0] = (char*) cmdLine.cmd.c_str();
        size_t i = 0;
        for (; i < cmdLine.args.size(); ++i) {
            cargs[i+1] = (char*) cmdLine.args[i].c_str();
        }
        cargs[i+1] = NULL;

        if( execvp( cmdLine.cmd.c_str(), cargs ) < 0 ) {
            perror("Could not spawn process");
        }
        _exit(0);
    }
}

void suspendProcess(pid_t pid) {
    if ( kill(pid, SIGSTOP) < 0 ) {
        perror("Suspending process has failed");
    }
    else {
        PROCESS_TABLE.changeStatus(pid, ProcessStatus::STOPPED);
    }
}

void resumeProcess(pid_t pid) {
    if ( kill(pid, SIGCONT) < 0 ) {
        perror("Resuming process has failed");
    }
    else {
        PROCESS_TABLE.changeStatus(pid, ProcessStatus::RUNNING);
    }
}

void killProcess(pid_t pid) {
    // SIGTERM might be more graceful, but we will opt for violence
    if ( kill(pid, SIGKILL) < 0 ) {
        perror("Killing process has failed");
    }
}

void waitProcess(pid_t pid) {
    if ( waitpid( pid, NULL, 0 ) < 0 ) {
        perror("Waiting for process has failed");
    }
}

void exitShell() { 
    while( PROCESS_TABLE.size() > 0 ) {
        wait(NULL);
    }
   printCompletedTimes();
   exit(0);
}   

