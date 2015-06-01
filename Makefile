CPP=g++
CFLAGS+=-g  -ggdb3 -Wall -Wextra -Wno-unused-parameter -DDEBUG -fstack-check
INCLUDE+=-Itelehash-c/unix -Itelehash-c/include -Itelehash-c/include/lib -Icxx
LDFLAGS += telehash-c/libtelehash.a
TEST_LDFLAGS= -Llibtap -ltap 
TEST_CPPFLAGS=-fprofile-arcs -ftest-coverage  -Ilibtap

test: cxx/StorjTelehash.cpp tests/test.c
	cd telehash-c;make
	cd libtap;make
	$(CPP) $(INCLUDE) ${CFLAGS}  ${TEST_CPPFLAGS}  -o $@ $^  ${LDFLAGS}   ${TEST_LDFLAGS} -std=c++11 -pthread 

clean:
	rm -f *.o
	rm -f bin/*
