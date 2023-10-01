#ifndef COMMAND_PARSER_HPP
#define COMMAND_PARSER_HPP

#include <string>
#include <vector>
#include <iostream>

#define MAX_ARGS 7 // Max number of arguments to a command
#define MAX_LENGTH 20 // Max # of characters in an argument

// Assumes valid command form; no error checking 
class CommandParser 
{
    public: 
    /*
        Represents the output of parsing a command line
    */
    struct Content 
    {
        std::string cmd; 
        std::vector<std::string> args = std::vector<std::string>(); 
        std::string inputFile = "";
        std::string outputFile = "";
        bool BACKGROUND_FLAG = false;
    };

    /*
        Parses given command line, and returns parsed content
    */
    static Content parseLine(std::string line);

    private:
    /*
        Splits command line input into space-seperated tokens
    */
    static std::vector<std::string> splitToTokens(std::string line);

    /*
        Parses the tokens to command line content
    */
    static Content parseTokens(std::vector<std::string> tokens);
};



#endif 