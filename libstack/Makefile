CC=gcc
AR=ar
CFLAGS=-g -Wall -W
LDFLAGS=
SRC=stack.c
OBJ=$(SRC:.c=.o)
LIBFILE=libstack.a

all: $(SRC) $(LIBFILE)
	@echo 'Making libstack'

$(LIBFILE): $(OBJ)
	$(AR) r $@ $(OBJ)

*.o:
	$(CC) -o $@ -c $< $(CFLAGS)
