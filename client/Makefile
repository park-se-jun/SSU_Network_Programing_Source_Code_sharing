CC = gcc
CFLAGS = -Wall -g -I.
DEPS = client.h
OBJ = client.o
OPT = -lpthread

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(OPT) $(CFLAGS)

client: $(OBJ)
	gcc -o $@ $^ $(OPT) $(CFLAGS)

clean:
	rm -rf *.o client
