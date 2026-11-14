CC=gcc
CXX=g++
LDFLAGS=-s
LDLIBS=-lcrypto

.PHONY: all default build clean zip

all: build

default: build

build: server client

security.o: security.c security.h
	${CC} -c security.c -o security.o

sec.o: sec.c sec.h security.h constsSecurity.h io.h
	${CC} -c sec.c -o sec.o

server: server.cpp transport.cpp io.cpp sec.o security.o
	${CXX} -o server server.cpp transport.cpp io.cpp sec.o security.o ${LDFLAGS} ${LDLIBS}

client: client.cpp transport.cpp io.cpp sec.o security.o
	${CXX} -o client client.cpp transport.cpp io.cpp sec.o security.o ${LDFLAGS} ${LDLIBS}

clean:
	rm -f server client security.o sec.o .bin *.out *.dSYM

zip: clean
	rm -f project.zip
	mkdir -p project
	cp server.cpp client.cpp transport.cpp io.cpp security.c sec.c transport.h io.h consts.h constsSecurity.h security.h sec.h Makefile project
	zip project1.zip project/*
	rm -rf project