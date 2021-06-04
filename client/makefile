CC=gcc
CFLAGS= -g -Wall -g -I.
# Change the name of source code files 
# student ID #.h (e.g., 2021679.h)
# student ID #.c (e.g., 2021679.c)
DEPS = ./client.h
OBJ = client.o 

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

# Change the name of the output binary file to your student ID #.
client: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)

clean:
	rm -rf *.o client