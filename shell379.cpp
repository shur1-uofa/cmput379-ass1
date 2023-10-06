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
LIST OF QUESTIONS:
- getrusage is supposed to print microseconds but it obviously doesn't. Why does it contradict the docs?
- getrusage's seconds is just zero. It doesn't get updated at all.
- getrusage( RUSAGE_CHILDREN ) returns a time 3 times the expected. Why?

- for my sigaction, I chose to not ignore any signals that come through. What are your thoughts?

- for my time calculation, I took the difference of main process user time from rusage( SELF ) to calculate active child process times.
Is this actually accurate or not?   

- For spawning a child process and redirecting input/output, I took advantage of the fact that exec retains I/O streams of the original process.
What do you think about this approach? 

- Do you have any other feedback? I know there are many other ways to complete this but I want to hear
other approaches as well as faults with my approach if there are any. 
*/

bool Running = true;
ProcessTable PROCESS_TABLE;


void printJobs();

void printCompletedTimes();

int differenceInSeconds(timeval now, timeval past);

int inSeconds(timeval now);

void suspendProcess(pid_t pid);

int waitProcess(pid_t pid, int options);

void resumeProcess(pid_t pid);

void killProcess(pid_t pid);

void exitShell();

void spawnProcess(std::string line, CommandParser::Content cmdLine);

void handleChildTermination(int signum, siginfo_t *info, void *context) 
{
    while ( waitProcess(-1, WNOHANG) > 0 );
}

int main() {

    struct sigaction childTerminator;
    childTerminator.sa_sigaction = handleChildTermination;
    sigemptyset( &childTerminator.sa_mask );
    childTerminator.sa_flags = SA_NOCLDSTOP || SA_SIGINFO;
    sigaction( SIGCHLD, &childTerminator, NULL);

    while( Running ) {
        std::string line; 
        /*
        Without the below checks, getline will not fail causing an infinite loop with line being empty. 
        https://man7.org/linux/man-pages/man7/signal.7.html : E_INTR
        According to this, when a blocking syscall or library call is interrupted, that call can fall into an errorful state
        SA_RESTART doesn't work for getline(), likely because it's not UNIX-specific. 
        However, you can check the error code with std::istream.fail() and clear it with clear()

        If I used read() and write() syscalls instead, I can use SA_RESTART signal flag instead.
        Ironically, I would have had an easier time using lower-level APIs than the higher level APIs I am using.  
        */
        if ( std::cin.fail() ) {
            std::cin.clear();
        }
        else {
            std::cout << "SHELL379: ";
        }
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
            waitProcess( stoi(cmdLine.args[0]), 0 );
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
        std::cout << ' ' << differenceInSeconds(usage.ru_utime, pcb.createdTime) << ' ';
        std::cout << pcb.command << std::endl;
    }
    std::cout << std::endl;
    printCompletedTimes();
}

void printCompletedTimes() {
    struct rusage usage; 
    if ( getrusage( RUSAGE_CHILDREN, &usage ) < 0) {
        perror("Failed to obtain completed process time information");
    } 
    else {
        std::cout << "Completed processes:\n";
        std::cout << "User time = " << inSeconds(usage.ru_utime) << " seconds\n";
        std::cout << "Sys time = " << inSeconds(usage.ru_utime) << " seconds\n";
    }
}

int differenceInSeconds(timeval now, timeval past) {
    return (now.tv_sec - past.tv_sec) + (now.tv_usec - past.tv_usec)/100;
}

int inSeconds(timeval time) {
    return time.tv_sec + time.tv_usec/100;
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
            waitProcess( pid, 0 );
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

int waitProcess(pid_t pid, int options) {
    int res = waitpid( pid, NULL, options );
    if ( res > 0 ) {
        PROCESS_TABLE.removeEntry( res );
    }
    return res; 
}

void exitShell() { 
    while( waitProcess(-1, 0) > 0 );
    printCompletedTimes();
    exit(0);
}   

