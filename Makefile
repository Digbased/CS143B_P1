FLAGS = -lcriterion -Wall -g -DCMAKE_BUILD_TYPE=Debug
WINDOWS_FLAGS = -I../criterion_build/Criterion/src/entry -I../criterion_build/Criterion/include -L../criterion_build/build -Wall
CC = gcc
EXES = main_test

SOURCE_FILES = code/io_system.c code/file_system.c
TEST_FILES = code/io_system.c code/file_system.c main_test.c

all:
	$(CC) -o main_test  $(TEST_FILES) $(FLAGS)
clean:
	rm $(EXES)
bitmap_tests_windows:
	$(CC) -o bitmaptests_windows bitmap_tests.c code/io_system.c $(WINDOWS_FLAGS)
