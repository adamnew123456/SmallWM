CC=/usr/bin/clang
CFLAGS=-g
CXX=/usr/bin/clang++
CXXFLAGS=-g -I/usr/include/i386-linux-gnu/c++/4.8 --std=c++11 -Itest -Iinih -Isrc
LINKERFLAGS=-lXrandr -lX11
BINS=bin/test_configparse

all: ${BINS}

doc:
	doxygen

obj:
	[ -d obj ] || mkdir obj

obj/ini.o: obj inih/ini.c
	${CC} ${CFLAGS} -c inih/ini.c -o obj/inih.o

obj/configparse.o: obj src/configparse.cpp
	${CXX} ${CXXFLAGS} -c src/configparse.cpp -o obj/configparse.o

obj/clientmanager.o: obj src/clientmanager.cpp
	${CXX} ${CXXFLAGS} -c src/clientmanager.cpp -o obj/clientmanager.o

obj/desktops.o: obj src/desktops.cpp
	${CXX} ${CXXFLAGS} -c src/desktops.cpp -o obj/desktops.o

obj/icons.o: obj src/icons.cpp
	${CXX} ${CXXFLAGS} -c src/icons.cpp -o obj/icons.o

obj/layers.o: obj src/layers.cpp
	${CXX} ${CXXFLAGS} -c src/layers.cpp -o obj/layers.o

obj/moveresize.o: obj src/moveresize.cpp
	${CXX} ${CXXFLAGS} -c src/moveresize.cpp -o obj/moveresize.o

obj/utils.o: obj src/utils.cpp
	${CXX} ${CXXFLAGS} -c src/utils.cpp -o obj/utils.o

bin/test_configparse: obj obj/ini.o obj/utils.o obj/configparse.o test/test_configparse.cpp
	${CXX} ${CXXFLAGS} ${LINKERFLAGS} obj/ini.o obj/utils.o obj/configparse.o test/test_configparse.cpp -o bin/test_configparse

clean:
	rm -rf obj doc ${BINS}
