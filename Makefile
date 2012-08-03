CFLAGS=-Wall -g -O3

all: object.o hashmap.o 
	gcc $(CFLAGS) test.c hashmap.o object.o -ltalloc -o test
 
test: hashmap.o object.o

object.o: object.h
hashmap.o: hashmap.h

.PHONY: clean all
clean:
	rm *.o *.so

