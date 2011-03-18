#!/bin/bash

noop(){
	:
}

event(){
	gcc -c event.c -g
}

smallwm(){
	event
	gcc smallwm.c event.o -o smallwm -lX11 -g
}

clean(){
	rm event.o || noop
	rm smallwm || noop
	ls | grep "~" | xargs rm
}

help(){
	echo "$1 - The SmallWM Build Script"
	echo "   Arguments:"
	echo "     event - Build the event code"
	echo "     smallwm - Build the whole program"
	echo "     clean - Remove all build and temporary files"
}

case "$1" in 
	"event") event ;;
	"smallwm") smallwm ;;
	"clean") clean ;;
	*) help ;;
esac
