all: smallwm-release

client.o: client.c
	gcc -c client.c

event.o: event.c
	gcc -c event.c

icon.o: icon.c
	gcc -c icon.c

table.o: table.c
	gcc -c table.c

util.o: util.c
	gcc -c util.c

wm.o: wm.c
	gcc -c wm.c

smallwm-release: client.o event.o icon.o table.o util.o wm.o
	gcc -O3 client.o event.o icon.o table.o util.o wm.o smallwm.c -o smallwm-release -lX11

client-debug.o: client.c
	gcc -g -c client.c -o client-debug.o

event-debug.o: event.c
	gcc -g -c event.c -o event-debug.o

icon-debug.o: icon.c
	gcc -g -c icon.c -o icon-debug.o

table-debug.o: table.c
	gcc -g -c table.c -o table-debug.o

util-debug.o: util.c
	gcc -g -c util.c -o util-debug.o

wm-debug.o: wm.c
	gcc -g -c wm.c -o wm-debug.o

smallwm-debug: client-debug.o event-debug.o icon-debug.o table-debug.o util-debug.o wm-debug.o
	gcc -g client-debug.o event-debug.o icon-debug.o table-debug.o util-debug.o wm-debug.o smallwm.c -o smallwm-debug -lX11

clean:
	rm -f *.o smallwm-*
