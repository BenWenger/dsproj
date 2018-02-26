CC=g++
CFLAGS=-I -O2 -std=c++11
DEPS = job.h scheduler.h treelist.h treelist.hpp treelist_iterators.hpp types.h

%.o: %.cpp $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

scheduler: main.o scheduler.o 
	$(CC) -o scheduler main.o scheduler.o $(CFLAGS)
	
tester: treelist_tester.o
	$(CC) -o tester treelist_tester.o $(CFLAGS)
	
