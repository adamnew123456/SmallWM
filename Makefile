# C related flags. This project is mostly C++ at this point, but it still uses
# the C API provided by inih.
CC=/usr/bin/clang
CFLAGS=-O3

# C++ related flags. Note that you may need to modify the CXXFLAGS variable to
# get SmallWM to build with Clang++.
CXX=/usr/bin/clang++
CXXFLAGS=-g -IUnitTest++/src -Itest -Iinih -Isrc -Isrc.new -Wold-style-cast --std=c++11
LINKERFLAGS=-lX11 -lXrandr

# Binaries are classified into two groups - ${BINS} includes the main smallwm
# binary only, while ${TESTS} includes all the binaries for the test suite.
BINS=bin/smallwm
TESTS=bin/test-test

OBJS=obj/ini.o obj/clients.o obj/configparse.o obj/clientmanager.o obj/desktops.o obj/events.o obj/icccm.o obj/icons.o obj/layers.o obj/logging.o obj/moveresize.o obj/smallwm.o obj/utils.o

# ${HEADERS} exists mostly to make building Doxygen output more consistent
# since a change in the headers may require the API documentation to be
# re-created.
HEADERS=src/actions.h src/clients.h src/clientmanager.h src/common.h src/configparse.h src/desktops.h src/events.h src/icccm.h src/layers.h src/shared.h src/utils.h src.new/changes.h src.new/client-model.h src.new/desktop-type.h src.new/unique-multimap.h
SRCS=src/clients.cpp src/clientmanager.cpp src/configparse.cpp src/desktops.cpp src/events.cpp src/icccm.cpp src/icons.cpp src/layers.cpp src/moveresize.cpp src/smallwm.cpp src/utils.cpp

all: bin/smallwm

# Used to probe for compiler errors, without linking everything
check: ${OBJS}

doc: ${HEADERS} ${SRCS}
	doxygen

bin:
	[ -d bin ] || mkdir bin

obj:
	[ -d obj ] || mkdir obj

test: ${TESTS}

clean:
	rm -rf bin obj doc

bin/smallwm: bin obj ${OBJS}
	${CXX} ${CXXFLAGS} ${OBJS} ${LINKERFLAGS} -o bin/smallwm

obj/ini.o: obj inih/ini.c
	${CC} ${CFLAGS} -c inih/ini.c -o obj/ini.o

obj/clients.o: obj src/clients.cpp
	${CXX} ${CXXFLAGS} -c src/clients.cpp -o obj/clients.o

obj/configparse.o: obj src/configparse.cpp
	${CXX} ${CXXFLAGS} -c src/configparse.cpp -o obj/configparse.o

obj/clientmanager.o: obj src/clientmanager.cpp
	${CXX} ${CXXFLAGS} -c src/clientmanager.cpp -o obj/clientmanager.o

obj/desktops.o: obj src/desktops.cpp
	${CXX} ${CXXFLAGS} -c src/desktops.cpp -o obj/desktops.o

obj/events.o: obj src/events.cpp
	${CXX} ${CXXFLAGS} -c src/events.cpp -o obj/events.o

obj/icccm.o: obj src/icccm.cpp
	${CXX} ${CXXFLAGS} -c src/icccm.cpp -o obj/icccm.o

obj/icons.o: obj src/icons.cpp
	${CXX} ${CXXFLAGS} -c src/icons.cpp -o obj/icons.o

obj/layers.o: obj src/layers.cpp
	${CXX} ${CXXFLAGS} -c src/layers.cpp -o obj/layers.o

obj/logging.o: obj src/logging.cpp
	${CXX} ${CXXFLAGS} -c src/logging.cpp -o obj/logging.o

obj/moveresize.o: obj src/moveresize.cpp
	${CXX} ${CXXFLAGS} -c src/moveresize.cpp -o obj/moveresize.o

obj/smallwm.o: obj src/smallwm.cpp
	${CXX} ${CXXFLAGS} -c src/smallwm.cpp -o obj/smallwm.o

obj/utils.o: obj src/utils.cpp
	${CXX} ${CXXFLAGS} -c src/utils.cpp -o obj/utils.o

# Getting unit tests to build is a bit awkward. Since I want to avoid
# distributing a static library along with SmallWM, it is necessary to build
# UnitTest++ on-demand from source. Hence, recursive make...
bin/libUnitTest++.a: bin
	export CXX
	$(MAKE) -C UnitTest++ libUnitTest++.a
	cp UnitTest++/libUnitTest++.a bin

# This simply ensure that the test itself builds properly. It doesn't do
# anything, since if it builds, the test is a success.
bin/test-test: bin bin/libUnitTest++.a test/test.cpp
	${CXX} ${CXXFLAGS} test/test.cpp bin/libUnitTest++.a -o bin/test-test

bin/test-configparse: bin/libUnitTest++.a obj/test-configparse.o obj/ini.o obj/configparse.o obj/utils.o
	${CXX} ${CXXFLAGS} obj/test-configparse.o bin/libUnitTest++.a obj/configparse.o obj/ini.o obj/utils.o ${LINKERFLAGS} -o bin/test-configparse

obj/test-configparse.o: obj test/configparse.cpp
	${CXX} ${CXXFLAGS} -c test/configparse.cpp -o obj/test-configparse.o

bin/test-client-model: bin/libUnitTest++.a obj/test-client-model.o
	${CXX} ${CXXFLAGS} obj/test-client-model.o bin/libUnitTest++.a ${LINKER_FLAGS} -o bin/test-client-model

obj/test-client-model.o: obj test/client-model.cpp src.new/changes.h src.new/client-model.h src.new/desktop-type.h src.new/unique-multimap.h
	${CXX} ${CXXFLAGS} -c test/client-model.cpp -o obj/test-client-model.o
