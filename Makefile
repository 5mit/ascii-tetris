CC=gcc

LIBS = -lncurses -ltinfo

tetris.o: tetris.c
	$(CC) -o tetris tetris.c $(LIBS)

