CFLAGS = -O2

all:	lifesrcdumb lifesrc

lifesrcdumb:	search.o interact.o dumbtty.o
	$(CC) $(CFLAGS) -o lifesrcdumb search.o interact.o dumbtty.o

lifesrc:	search.o interact.o cursestty.o
	$(CC) $(CFLAGS) -o lifesrc search.o interact.o cursestty.o -L/usr/local/lib -lcurses -ltermcap
