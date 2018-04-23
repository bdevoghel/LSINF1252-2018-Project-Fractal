CC=gcc
AR=ar
CFLAGS=-g -Wall -W -I/usr/include/SDL
LDFLAGS=
SRC=stack.c
OBJ=$(SRC:.c=.o)
LIBFILE=libstack.a

all: $(SRC) $(LIBFILE)

$(LIBFILE): $(OBJ)
	$(AR) r $@ $(OBJ)

*.o:
	$(CC) -o $@ -c $< $(CFLAGS)
