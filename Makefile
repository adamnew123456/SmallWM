CC=/usr/bin/clang
CFLAGS=-g
CXX=/usr/bin/clang++
CXXFLAGS=-g -I/usr/include/i386-linux-gnu/c++/4.8 --std=c++11 -Itest -Iinih -Isrc -lX11
BINS=bin/test_configparse

all: ${BINS}

doc:
	doxygen

obj:
	[ -d obj ] || mkdir obj

obj/ini.o: obj inih/ini.c
	${CC} ${CFLAGS} -c inih/ini.c -o obj/ini.o

obj/client.o: obj src/client.cpp
	${CXX} ${CXXFLAGS} -c src/client.cpp -o obj/client.o

obj/configparse.o: obj obj/ini.o obj/utils.o src/configparse.cpp
	${CXX} ${CXXFLAGS} -c src/configparse.cpp -o obj/configparse.o

obj/utils.o: obj src/utils.cpp
	${CXX} ${CXXFLAGS} -c src/utils.cpp -o obj/utils.o

bin/test_configparse: obj obj/ini.o obj/utils.o obj/configparse.o
	${CXX} ${CXXFLAGS} obj/ini.o obj/utils.o obj/configparse.o test/test_configparse.cpp -o bin/test_configparse

clean:
	rm -rf obj doc ${BINS}
