run: compile_shell
	./shell379.exe

compile_shell: compile_parser compile_processTable
	g++ shell379.cpp commandParser.o processTable.o -o shell379.exe 

compile_parser:
	g++ -c ./impl/commandParser.cpp -o commandParser.o -Wall 

compile_processTable:
	g++ -c ./impl/processTable.cpp -o processTable.o -Wall


test_parser: compile_parser
	g++ ./tests/parser_driver.cpp commandParser.o -o parser_driver.exe -Wall
	./parser_driver.exe

test_process_table: compile_processTable
	g++ ./tests/process_table_driver.cpp processTable.o -o processTable_driver.exe -Wall
	./processTable_driver.exe

test:
	make test_parser
	make test_process_table

compress:
	tar -caf ass1.tar.gz ./impl ./headers shell379.cpp
	
clean:
	rm *.o 
	rm *.exe