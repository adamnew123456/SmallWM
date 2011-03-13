#!/bin/bash

event(){
	gcc -c event.c -g
}

smallwm(){
	event
	gcc smallwm.c event.o -o smallwm -lX11 -g
}

clean(){
	rm event.o
	rm smallwm
	ls | grep "~" | xargs rm
}

help(){
	echo "$1 - The SmallWM Build Script"
	echo "   Arguments:"
	echo "     event - Build the event code"
	echo "     smallwm - Build the whole program"
	echo "     clean - Remove all build and temporary files"
}

if [ "$1" = "event" ]; then
	event
elif [ "$1" = "smallwm" ]; then
	smallwm
elif [ "$1" = "clean" ]; then
	clean
else
	help
fi
