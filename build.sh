#!/bin/bash

noop(){
	:
}

icons(){
	gcc -c icons.c -g
}

event(){
	icons
	gcc -c event.c -g
}

smallwm(){
	event
	gcc smallwm.c event.o icons.o -o smallwm -lX11 -g
}

clean(){
	rm event.o &> /dev/null || noop
	rm icons.o &> /dev/null || noop
	rm smallwm &> /dev/null || noop
	ls | grep "~" | xargs rm
}

help(){
	echo "$1 - The SmallWM Build Script"
	echo "   Arguments:"
	echo "     event - Build the event code"
	echo "     icons - Build the iconification code"
	echo "     smallwm - Build the whole program"
	echo "     clean - Remove all build and temporary files"
}

case "$1" in 
	"event") event ;;
	"icons") icons ;;
	"smallwm") smallwm ;;
	"clean") clean ;;
	*) help ;;
esac
