all: pong
pong: main.o
	gcc main.o -o pong -lsense -lm
main.o: main.c
	gcc -c main.c
clean: 
	rm -f *.o pong
