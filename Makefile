client.o: client.c
	gcc -c client.c -g

event.o: event.c
	gcc -c event.c -g

smallwm: client.o event.o
	gcc smallwm.c event.o client.o -o smallwm -lX11 -g

smallwm-release: client.o event.o
	gcc smallwm.c event.o client.o -o smallwm-release -lX11 -O3

xephyr-test: smallwm
	Xephyr :20 &
	sleep 5
	DISPLAY=":20" gdb smallwm

clean:
	rm -f *.o smallwm smallwm-release *~
