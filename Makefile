# to make just io tests: make io_tests

FLAGS = -lcriterion -Wall
CC = gcc
EXES = tests
FILES = code/io_system.c code/file_system.c tests.c

all:
	$(CC) -o tests  $(FILES) $(FLAGS)
clean:
	rm $(EXES)

io_tests:
	$(CC) -o tests unit_tests/tests.c $(FLAGS)

