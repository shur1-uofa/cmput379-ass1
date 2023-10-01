/*
https://github.com/eecs280staff/unit_test_framework/tree/main
Lightweight unit testing framework for C++
*/

#include "../unit_test_framework.hpp"
#include "../headers/processTable.hpp"


TEST(size_with_only_adding) {
    ProcessTable table; 
    table.addEntry( 1, "test command 1");
    table.addEntry( 2, "test command 2");
    table.addEntry( 3, "test command 3");
    ASSERT_EQUAL( table.size(), 3 );
}

TEST(size_with_adding_and_removing) {
    ProcessTable table; 
    table.addEntry( 1, "test command 1");
    table.addEntry( 2, "test command 2");
    table.addEntry( 3, "test command 3");
    table.removeEntry( 2 );
    table.addEntry( 4, "test command 4");
    table.removeEntry( 1 );
    ASSERT_EQUAL( table.size(), 2 );
}

TEST(add_one_entry) {
    ProcessTable table; 
    table.addEntry( 1, "test command 1");
    ASSERT_EQUAL( table.size(), 1 );
    for (auto pcb : table ) {
        ASSERT_EQUAL( pcb.pid, 1 );
        ASSERT_EQUAL( pcb.status, ProcessStatus::RUNNING );
        ASSERT_EQUAL( pcb.command, "test command 1" );
    }
}

TEST(remove_all_but_one_entries) {
    ProcessTable table; 
    table.addEntry( 1, "test command 1");
    table.addEntry( 2, "test command 2");
    table.removeEntry( 1 );
    table.addEntry( 3, "test command 3");
    table.removeEntry( 2 );
    table.addEntry( 4, "test command 4");
    table.removeEntry( 3 );
    ASSERT_EQUAL( table.size(), 1 );
    for (auto pcb : table ) {
        ASSERT_EQUAL( pcb.pid, 4 );
    }
}

TEST(stop_process_in_process_table_with_multiple_entries) {
    ProcessTable table; 
    table.addEntry( 1, "test command 1");
    table.addEntry( 2, "test command 2");
    table.addEntry( 3, "test command 3");
    table.addEntry( 4, "test command 3");
    table.changeStatus( 4, ProcessStatus::STOPPED );
    ASSERT_EQUAL( table.size(), 4 );
    for (auto pcb : table ) {
        if ( pcb.pid == 4 ) {
            ASSERT_EQUAL( pcb.status, ProcessStatus::STOPPED );
        }
        else {
            ASSERT_EQUAL( pcb.status, ProcessStatus::RUNNING );
        }
    }
}


TEST_MAIN();