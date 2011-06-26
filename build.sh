#!/bin/bash

noop(){
	:
}

client(){
	gcc -c client.c -g
}

event(){
	client
	gcc -c event.c -g
}

smallwm(){
	event
	gcc smallwm.c event.o client.o -o smallwm -lX11 -g
}

release(){
	event
	gcc smallwm.c event.o client.o -o smallwm -lX11 -O3
}

clean(){
	rm *.o &> /dev/null || noop
	rm smallwm &> /dev/null || noop
	ls | grep "~" | xargs rm
}

help(){
	echo "$1 - The SmallWM Build Script"
	echo "   Arguments:"
	echo "     event - Build the event code"
	echo "     client - Build the client code"
	echo "     smallwm - Build the whole program"
	echo "     release - Build an optimized release"
	echo "     clean - Remove all build and temporary files"
}

case "$1" in 
	"event") event ;;
	"client") client ;;
	"smallwm") smallwm ;;
	"release") release ;;
	"clean") clean ;;
	*) help ;;
esac
