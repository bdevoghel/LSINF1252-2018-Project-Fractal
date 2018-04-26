# https://gl.developpez.com/tutoriel/outil/makefile/

CC = gcc
AR=ar
CFLAGS = -g -Wall -W
LDFLAGS = -lm -L/usr/local/lib -lSDL -lpthread
LIBRAIRIES = libfractal/libfractal.a libstack/libstack.a
all: main

libfractal/libfractal.a:
	@echo 'Building libfractal'
	@cd libfractal && $(MAKE)

libstack/libstack.a:
	@echo 'Building libstack'
	@cd libstack && $(MAKE)

fractal: main.o $(LIBRAIRIES)
	@echo 'Making main'
	@$(CC) -o fractal main.o $(LIBRAIRIES) $(CFLAGS) $(LDFLAGS)

main.o: main.c
	@echo 'Building main'
	@$(CC) -c -o main.o main.c $(CFLAGS)

clean:
	@echo 'Removing files'
	@rm -rf fractal *.o libfractal/*.o libfractal/*.a *.bmp libstack/*.o libstack/*.a
