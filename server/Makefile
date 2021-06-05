CC=gcc
CFLAGS= -Wall -g -I.
DEPS = server.h
OBJ = server.o
OPT = -lpthread -lrt

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(OPT) $(CFLAGS)

server: $(OBJ)
	gcc -o $@ $^ $(OPT) $(CFLAGS)

clean:
	rm -rf *.o server
