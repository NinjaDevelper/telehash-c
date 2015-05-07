CPP=g++
CFLAGS+=-g -Wall -Wextra -Wno-unused-parameter -DDEBUG
INCLUDE+=-Itelehash-c/unix -Itelehash-c/include -Itelehash-c/include/lib -IMessagingTelehash
LDFLAGS += telehash-c/libtelehash.a
TEST_LDFLAGS= -Llibtap -ltap 
TEST_CPPFLAGS=-fprofile-arcs -ftest-coverage  -Ilibtap -g -ggdb3

test: MessagingTelehash/MessagingTelehash.cpp tests/test.c
	cd telehash-c;make
	cd libtap;make
	$(CPP) $(INCLUDE) ${CFLAGS}  ${TEST_CPPFLAGS} -o $@ $^  ${LDFLAGS}   ${TEST_LDFLAGS} -std=c++11 -pthread

python: ${FILES} MessagingTelehash/MessagingTelehash_python.cpp
	python setup.py build_ext -i

clean:
	rm -f *.o
	rm -f bin/*
