CC:=cc
CFLAGS:=-Wall -Wextra -pedantic -std=c17 -g

ncimag: main.c termio.h util.h common.h termio.o util.o
	$(CC) $(CFLAGS) main.c termio.o util.o -o $@

termio.o: termio.c termio.h common.h
	$(CC) $(CFLAGS) -c termio.c 

util.o: util.c util.h common.h
	$(CC) $(CFLAGS) -c util.c

clean: 
	rm -f *.o ncimag

run: ncimag
	./ncimag

debug: ncimag
	gdb ./ncimag

