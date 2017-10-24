# to make just io tests: make io_tests

FLAGS = -lcriterion -Wall -g -DCMAKE_BUILD_TYPE=Debug
WINDOWS_FLAGS = -I../criterion_build/Criterion/src/entry -I../criterion_build/Criterion/include -L../criterion_build/build -Wall
CC = gcc
EXES = tests bitmaptests main_test
TXT_FILES = saves/non_existent_file.txt
FILES = code/io_system.c code/file_system.c tests.c

all:
	$(CC) -o tests  $(FILES) $(FLAGS)
clean:
	rm $(EXES) $(TXT_FILES)
io_tests:
	$(CC) -o tests unit_tests/tests.c $(FLAGS)

bitmap_tests:
	$(CC) -o bitmaptests bitmap_tests.c code/io_system.c code/file_system.c $(FLAGS)
bitmap_tests_windows:
	$(CC) -o bitmaptests_windows bitmap_tests.c code/io_system.c $(WINDOWS_FLAGS)
main_test:
	$(CC) -o main_test main_test.c code/io_system.c $(FLAGS)
