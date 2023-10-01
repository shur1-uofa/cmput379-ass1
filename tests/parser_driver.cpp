/*
https://github.com/eecs280staff/unit_test_framework/tree/main
Lightweight unit testing framework for C++
*/

#include "../unit_test_framework.hpp"
#include "../headers/commandParser.hpp"


TEST(parse_single_command) {
    std::string line = "command"; 
    CommandParser::Content content = CommandParser::parseLine(line);
    ASSERT_EQUAL( content.cmd, "command" );
    ASSERT_EQUAL( content.args.size(), 0 );
    ASSERT_EQUAL( content.inputFile, "" );
    ASSERT_EQUAL( content.outputFile, "" );
    ASSERT_FALSE( content.BACKGROUND_FLAG );
}

TEST(parse_single_command_with_single_arg) {
    std::string line = "cat input"; 
    CommandParser::Content content = CommandParser::parseLine(line);
    ASSERT_EQUAL( content.cmd, "cat" );
    ASSERT_EQUAL( content.args.size(), 1 );
    ASSERT_EQUAL( content.args[0], "input" );
    ASSERT_EQUAL( content.inputFile, "" );
    ASSERT_EQUAL( content.outputFile, "" );
    ASSERT_FALSE( content.BACKGROUND_FLAG );
}

TEST(parse_single_command_with_single_arg_with_input_redirection) {
    std::string line = "time runner <input"; 
    CommandParser::Content content = CommandParser::parseLine(line);
    ASSERT_EQUAL( content.cmd, "time" );
    ASSERT_EQUAL( content.args.size(), 1 );
    ASSERT_EQUAL( content.args[0], "runner" );
    ASSERT_EQUAL( content.inputFile, "input" );
    ASSERT_EQUAL( content.outputFile, "" );
    ASSERT_FALSE( content.BACKGROUND_FLAG );
}

TEST(parse_single_command_with_single_arg_with_output_redirection) {
    std::string line = "time runner >output"; 
    CommandParser::Content content = CommandParser::parseLine(line);
    ASSERT_EQUAL( content.cmd, "time" );
    ASSERT_EQUAL( content.args.size(), 1 );
    ASSERT_EQUAL( content.args[0], "runner" );
    ASSERT_EQUAL( content.inputFile, "" );
    ASSERT_EQUAL( content.outputFile, "output" );
    ASSERT_FALSE( content.BACKGROUND_FLAG );
}

TEST(parse_single_command_with_single_arg_with_input_and_output_redirection) {
    std::string line = "time runner <input >output"; 
    CommandParser::Content content = CommandParser::parseLine(line);
    ASSERT_EQUAL( content.cmd, "time" );
    ASSERT_EQUAL( content.args.size(), 1 );
    ASSERT_EQUAL( content.args[0], "runner" );
    ASSERT_EQUAL( content.inputFile, "input" );
    ASSERT_EQUAL( content.outputFile, "output" );
    ASSERT_FALSE( content.BACKGROUND_FLAG );
}

TEST(parse_single_command_in_background) {
    std::string line = "time &"; 
    CommandParser::Content content = CommandParser::parseLine(line);
    ASSERT_EQUAL( content.cmd, "time" );
    ASSERT_EQUAL( content.args.size(), 0 );
    ASSERT_EQUAL( content.inputFile, "" );
    ASSERT_EQUAL( content.outputFile, "" );
    ASSERT_TRUE( content.BACKGROUND_FLAG );
}

TEST(parse_single_command_with_input_and_output_redirection_in_background) {
    std::string line = "runner <input >output &"; 
    CommandParser::Content content = CommandParser::parseLine(line);
    ASSERT_EQUAL( content.cmd, "runner" );
    ASSERT_EQUAL( content.args.size(), 0 );
    ASSERT_EQUAL( content.inputFile, "input" );
    ASSERT_EQUAL( content.outputFile, "output" );
    ASSERT_TRUE( content.BACKGROUND_FLAG );
}

TEST_MAIN();