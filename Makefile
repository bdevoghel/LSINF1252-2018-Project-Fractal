# makefile basé essentiellement sur https://gl.developpez.com/tutoriel/outil/makefile/

# compilateur utilisé
CC = gcc
# options de compilation
CFLAGS = -g -Wall -W
# options de l'édition de liens
LDFLAGS = -lm -L/usr/local/lib -lSDL -lpthread -lcunit
# librairies externes à utiliser
LIBRAIRIES = libfractal/libfractal.a libstack/libstack.a
# liste des fichiers sources du projet
SRC = main.c
# liste des fichiers objets
OBJ = $(SRC:.c=.o)
# nom de l'exécutable à générer
EXEC = main

# regroupe dans cette dépendance l'ensemble des exécutables à produire
build: $(EXEC)
lib : libfractal/libfractal.a

tests:
    @echo 'Testing (rebuilding beforehand)'
    @make clean
    @make tests/test
    ./tests/test
    @echo 'Tests complete'

tests/test: tests/*.o $(LIBRAIRIES)
	@$(CC) -o tests/test tests/*.o $(LIBRAIRIES) $(CFLAGS) $(LDFLAGS)

tests/*.o: tests/*.c
	@echo 'Building tests'
	@$(CC) -c -o tests/test.o tests/*.c $(CFLAGS)

libfractal/libfractal.a:
	@echo 'Building libfractal'
	@cd libfractal && $(MAKE)

libstack/libstack.a:
	@echo 'Building libstack'
	@cd libstack && $(MAKE)

$(EXEC): $(OBJ) $(LIBRAIRIES)
	@echo 'Making executable'
	@$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS) $(LIBRAIRIES)

# pour main (et tout autre .c à construire à partir d'un .o)
%.o: %.c
	@echo 'Building main'
	@$(CC) -o $@ -c $< $(CFLAGS)

# dépendances qui seront systématiquement reconstruites
.PHONY: build clean rebuild tests

# permet de supprimer tous les fichiers intermédiaires
clean:
	@echo 'Cleaning previously made files'
	@rm -vf $(EXEC) tests/test tests/*.o *.o libfractal/*.o libstack/*.o $(LIBRAIRIES) *.bmp

# supprime tout et reconstruit le projet
rebuild: clean build
