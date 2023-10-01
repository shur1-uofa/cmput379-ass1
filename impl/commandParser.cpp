#include "../headers/commandParser.hpp"

CommandParser::Content CommandParser::parseLine(std::string line) {
    Content content;
    std::vector<std::string> tokens = CommandParser::splitToTokens(line);
    content = CommandParser::parseTokens(tokens);
    return content;
}

std::vector<std::string> CommandParser::splitToTokens(std::string line) {
    line += ' ';
    std::vector<std::string> tokens;
    // Split string by space characters 
    for( size_t start = 0, end = line.find(" ", start); end != std::string::npos;) {
        tokens.push_back( line.substr(start, end-start) );

        start = end+1;
        end = line.find(" ", start); 
    }
    return tokens; 
}

CommandParser::Content CommandParser::parseTokens(std::vector<std::string> tokens) {
    CommandParser::Content content;
    content.cmd = tokens[0];
    for( size_t i = 1; i < tokens.size(); ++i ) {
        std::string token = tokens[i];
        if ( token[0] == '<' ) {
            content.inputFile = token.substr(1);
        }
        else if (token[0] == '>') {
            content.outputFile = token.substr(1); 
        }
        else if (token == "&") {
            content.BACKGROUND_FLAG = true;
        }
        else {
            content.args.push_back( token );
        }
    }
    std::cout << std::endl;
    return content;
}