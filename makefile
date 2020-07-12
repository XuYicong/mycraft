
EXE_DIR = .
EXE = $(EXE_DIR)/mycraft

SRC= mycraft.c objects.c
# generic build details
CP=     g++
CC=     gcc
COPT=    -g
CFLAGS= -lz

# compile to  object code

COBJ= $(SRC:.c=.o)

.c.o: 
	$(CC) $(COPT) -c -o $@ $< $(CFLAGS)  -Wall -Wpedantic -Warray-bounds



# build executable

$(EXE): $(COBJ) mats.o
	$(CC) $(COBJ) mats.o -lSDL2 -lm -lGL -lGLEW $(CFLAGS) -o $(EXE) 

#mats.o:
#	$(CP) -o mats.o mats.cpp
# clean up compilation

clean:
	rm -f $(COBJ) $(EXE)

# dependencies
mats.o:mats.cpp
