CPP=g++
CFLAGS+=-g -Wall -Wextra -Wno-unused-parameter -DDEBUG
INCLUDE+=-Itelehash-c/unix -Itelehash-c/include -Itelehash-c/include/lib -Istorjtelehash
LDFLAGS += telehash-c/libtelehash.a
TEST_LDFLAGS= -Llibtap -ltap 
TEST_CPPFLAGS=-fprofile-arcs -ftest-coverage  -Ilibtap -g -ggdb3

test: storjtelehash/StorjTelehash.cpp tests/test.c
	cd telehash-c;make
	cd libtap;make
	$(CPP) $(INCLUDE) ${CFLAGS}  ${TEST_CPPFLAGS} -o $@ $^  ${LDFLAGS}   ${TEST_LDFLAGS} -std=c++11 -pthread

python: ${FILES} storjtelehash/telehashbinder_python.cpp storjtelehash/StorjTelehash.cpp
	cd telehash-c;make
	python setup.py build_ext -i

clean:
	rm -f *.o
	rm -f bin/*
