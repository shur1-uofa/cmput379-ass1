#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <iostream>
#include <vector>
#include <string>

#include "headers/commandParser.hpp"
#include "headers/processTable.hpp"

#define LINE_LENGTH 100 // Max # of characters in an input line

// TODO: error handling for all codes



bool Running = true;
double completedUserTime = 0.0;
double completedSystemTime = 0.0;

ProcessTable PROCESS_TABLE;
int processes = 0; 
// Update PCB with SIGCHLD to remove finished children
// On signal, call wait to remove from OS's process table and update our own process table 
// And update completed user times and such if necessary 

void spawnProcess(std::string line, CommandParser::Content cmdLine);
 

int main() {

    while( Running ) {
        std::string line; 
        getline(std::cin, line);
        CommandParser::Content cmdLine = CommandParser::parseLine(line);

        // if & used, run function in background    (don't call wait)
        // if > or < used, I/O redirection          (???)

        // With given input, defer to function call 
        // syscallFunc( &PROCESS_TABLE, &FLAGS, other variables, ... )

        if ( cmdLine.cmd == "jobs" ) {
            /*
            Print process table:
            Consider using getrusage & times
            */
        }
        else if ( cmdLine.cmd == "sleep" ) {
            // sleep syscall    
        }
        else if ( cmdLine.cmd == "suspend" ) {
            // SIGSTOP signal
        }
        else if ( cmdLine.cmd == "wait" ) {
            // wait() system call
        }
        else if ( cmdLine.cmd == "resume" ) {
            // SIGCONT signal
        }
        else if ( cmdLine.cmd == "kill" ) {
            // kill() syscall to send signal or SIGKILL signal or SIGTERM might be more graceful
        }
        else if ( cmdLine.cmd == "exit" ) {
            /*
            wait() syscall with getrusage & times to print total user and system time
            */
        }
        else {
            spawnProcess(line, cmdLine);
        }
    }

    return( 0 );
}



 void spawnProcess(std::string line, CommandParser::Content cmdLine) {
    int pipefd[2];
    pipe(pipefd);
    pid_t pid = fork();
    if ( pid < 0 ) {
        exit(1);
    } 
    else if ( pid == 0 ) {      // parent process 
        close(pipefd[1]);       // close write end 
        char buffer;
        read(pipefd[0], &buffer, 1);        // FIXME: blocking?
        // Wait for child process to end
        if ( !cmdLine.BACKGROUND_FLAG ) {
            waitpid( PROCESS_TABLE.getRecentPID(), NULL, 0 );
        }
    }
    else {                      // child process 
        close(pipefd[0]);       // close read end
        if ( cmdLine.inputFile != "" ) {
            int inputfd = open( cmdLine.inputFile.c_str(), O_RDONLY );
            dup2( inputfd, STDIN_FILENO );
        }
        if ( cmdLine.outputFile != "" ) {
            int outputfd = open( cmdLine.outputFile.c_str(), O_WRONLY );
            dup2( outputfd, STDOUT_FILENO );
        }
        PROCESS_TABLE.addEntry( pid, line );
        write(pipefd[1], (void*) 'K', 1);
        execv( cmdLine.cmd.c_str(), (char**)&cmdLine.args[0]);
    }
}