CC = gcc
CFLAGS = -Wall -Wextra -pedantic -O2
LIBS = -lncurses -lm
OBJS = main.o 

jogo: $(OBJS)
	$(CC) $(CFLAGS) -o jogo $(OBJS) $(LIBS)

main.o: main.c mapa.h 
	$(CC) $(CFLAGS) -c main.c

clean:
	rm -f *.o jogo