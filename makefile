
EXE_DIR = .
EXE = $(EXE_DIR)/mycraft

SRC= mycraft.c

# generic build details

CC=     gcc
COPT=    -g
CFLAGS= -lz

# compile to  object code

OBJ= $(SRC:.c=.o)

.c.o:
	$(CC) $(COPT) -c -o $@ $< $(CFLAGS)  -Wall -Wpedantic -Warray-bounds

# build executable

$(EXE): $(OBJ)
	$(CC) $(OBJ) -lSDL2 -lGL -lGLU $(CFLAGS) -o $(EXE) 

# clean up compilation

clean:
	rm -f $(OBJ) $(EXE)

# dependencies

