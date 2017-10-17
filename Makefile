# to make just io tests: make io_tests

FLAGS = -lcriterion -Wall
CC = gcc
EXES = tests bitmaptests
TXT_FILES = saves/non_existent_file.txt
FILES = code/io_system.c code/file_system.c tests.c

all:
	$(CC) -o tests  $(FILES) $(FLAGS)
clean:
	rm $(EXES) $(TXT_FILES)
io_tests:
	$(CC) -o tests unit_tests/tests.c $(FLAGS)

bitmap_tests:
	$(CC) -o bitmaptests bitmap_tests.c code/io_system.c $(FLAGS)

