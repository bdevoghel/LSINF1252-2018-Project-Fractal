# TODO

gcc main.c -o fract -lpthread


/*

CC = gcc
CFLAGS = -g -Wall -W -I$(HOME)/local/include -std=c99
LDFLAGS = -lm -L/usr/local/lib -L$(HOME)/local/lib -lSDL -lcunit -pthread
LIBRAIRIES = libfractal/libfractal.a stack/libstack.a threads/libthread.a
all: fractal

testing: tests
	@echo 'Launching tests'
	@./tests

fractal: main.o $(LIBRAIRIES)
	@echo 'Making exec'
	@$(CC) -o fractal main.o $(LIBRAIRIES) $(CFLAGS) $(LDFLAGS)

main.o: main.c
	@echo 'Building main'
	@$(CC) -c -o main.o main.c $(CFLAGS)

tests: test/tests.o $(LIBRAIRIES)
	@echo 'Making tests'
	@$(CC) -o tests test/tests.o $(LIBRAIRIES) $(CFLAGS) $(LDFLAGS)

test/tests.o: test/tests.c
	@echo 'Building tests'
	@$(CC) -c -o test/tests.o test/tests.c $(CFLAGS)

libfractal/libfractal.a:
	@echo 'Building fractal lib'
	@cd libfractal && $(MAKE)

stack/libstack.a:
	@echo 'Building stack lib'
	@cd stack && $(MAKE)

threads/libthread.a:
	@echo 'Building thread lib'
	@cd threads && $(MAKE)

.PHONY: clean

clean:
	@echo 'Removing files'
	@rm -rf fractal tests *.o libfractal/*.o libfractal/*.a test/*.o *.bmp stack/*.o stack/*.a threads/*.o threads/*.a stdin.txt


*/