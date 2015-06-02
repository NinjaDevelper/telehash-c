CFLAGS+=-g  -ggdb3 -Wall -Wextra -Wno-unused-parameter -DDEBUG
INCLUDE+=-Itelehash-c/unix -Itelehash-c/include -Itelehash-c/include/lib -Icxx
LDFLAGS += telehash-c/libtelehash.a
TEST_LDFLAGS= -Llibtap -ltap 
TEST_CPPFLAGS=-fprofile-arcs -ftest-coverage  -Ilibtap

all: test

test: cxx/StorjTelehash.o tests/test.o
	cd telehash-c;make
	cd libtap;make
	$(CXX) $(INCLUDE) ${CFLAGS}  ${TEST_CPPFLAGS}  -o $@ $^  ${LDFLAGS}   ${TEST_LDFLAGS} -std=c++11 -pthread 

.cpp.o:
	$(CXX) $(INCLUDE) ${CFLAGS}  ${TEST_CPPFLAGS}  -o $@ -c $^  -std=c++11

clean:
	rm -f *.o
	rm -f cxx/*.o
	rm -f tests/*.o
	rm -f bin/*
