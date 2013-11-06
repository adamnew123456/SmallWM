client.o: client.c
	gcc -c client.c -g

event.o: event.c
	gcc -c event.c -g

smallwm: smallwm.c smallwm.c client.o event.o
	gcc smallwm.c client.o event.o -o smallwm -lX11 -g

smallwm-release: smallwm.c global.h smallwm.c client.o event.o
	gcc smallwm.c client.o event.o -o smallwm-release -lX11 -O3

xephyr-test: smallwm
	Xephyr :20 &
	sleep 5
	gdb -ex "set environment DISPLAY = :20" smallwm

reformat:
	indent -linux *.c *.h 
	rm *~

clean:
	rm -f *.o smallwm smallwm-release *~
