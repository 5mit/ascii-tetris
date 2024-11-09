CC=gcc

LIBS = -lncurses

tetris.o: tetris.c
	$(CC) -o tetris tetris.c $(LIBS)

