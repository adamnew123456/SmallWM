all: table-test

clean:
	rm -f table-test string_to_long-test

table-test: ../table.c table-test.c
	gcc -g ../table.c table-test.c -o table-test

string_to_long-test: ../util.c string_to_long-test.c
	gcc -g ../util.c string_to_long-test.c -o string_to_long-test
