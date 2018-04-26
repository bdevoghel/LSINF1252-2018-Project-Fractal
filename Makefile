# code basé essentiellement de https://gl.developpez.com/tutoriel/outil/makefile/

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
all: $(EXEC)

libfractal/libfractal.a:
	echo 'Building libfractal'
	@cd libfractal && $(MAKE)

libstack/libstack.a:
	echo 'Building libstack'
	@cd libstack && $(MAKE)

$(EXEC): $(OBJ) #$(LIBRAIRIES)
	echo 'Making executable'
	@$(CC) -o $@ $^ $(LDFLAGS) #$(LIBRAIRIES) $(CFLAGS)

# pour main (et tout autre .c à construire à partir d'un .o)
%.o: %.c
	echo 'Building main'
	@$(CC) -o $@ -c $< $(CFLAGS)

# dépendances qui seront systématiquement reconstruites
.PHONY: clean mrproper
	
# permet de supprimer tous les fichiers intermédiaires
clean:
	echo 'Cleaning files'
	@rm -rf $(EXEC) *.o libfractal/*.o libfractal/*.a libstack/*.o libstack/*.a *.bmp

# supprime tout ce qui peut être régénéré et permet une reconstruction complète du projet
mrproper: clean
	@rm -rf $(EXEC)