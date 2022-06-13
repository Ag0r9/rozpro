SOURCES=$(wildcard *.c)
HEADERS=$(SOURCES:.c=.h)
FLAGS=-DDEBUG -g

all: main

main: $(SOURCES) $(HEADERS)
	mpiCC $(SOURCES) $(FLAGS) -o pojedynek

clear: clean

clean:
	rm pojedynek a.out main

run: main
	mpirun -oversubscribe -np 8 ./pojedynek
