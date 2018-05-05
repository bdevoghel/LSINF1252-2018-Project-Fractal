# code basé essentiellement de https://gl.developpez.com/tutoriel/outil/makefile/
# TODO : compiler les tests

# compilateur utilisé
CC = gcc
# options de compilation
CFLAGS = -g -Wall -W
# options de l'édition de liens
LDFLAGS = -lm -L/usr/local/lib -lSDL -lpthread
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

libfractal/libfractal.a:
	@echo 'Building libfractal'
	@cd libfractal && $(MAKE)

libstack/libstack.a:
	@echo 'Building libstack'
	@cd libstack && $(MAKE)

main: $(OBJ) $(LIBRAIRIES)
	@echo 'Making executable'
	@$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS) $(LIBRAIRIES) # TODO utiliser gdb.c

# pour main (et tout autre .c à construire à partir d'un .o)
%.o: %.c
	@echo 'Building main'
	@$(CC) -o $@ -c $< $(CFLAGS)

# dépendances qui seront systématiquement reconstruites
.PHONY: build clean rebuild
	
# permet de supprimer tous les fichiers intermédiaires
clean:
	@echo 'Cleaning previously made files'
	@rm -vf $(EXEC) *.o libfractal/*.o libstack/*.o $(LIBRAIRIES) *.bmp

# supprime tout et reconstruit le projet
rebuild: clean build
