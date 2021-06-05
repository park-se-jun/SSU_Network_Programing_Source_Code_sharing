CC=gcc
CFLAGS= -Wall -g -I.
DEPS = client.h
OBJ = client.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

client: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)

clean:
	rm -rf *.o client
