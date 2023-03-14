CC:=cc
CFLAGS:=-Wall -Wextra -pedantic -std=c17 -g -ljpeg -lwebp

OBJECTS:=termio.o util.o lodepng/lodepng.o
HEADERS:=termio.h util.h common.h lodepng/lodepng.h

ncimag: main.c $(HEADERS) $(OBJECTS)
	$(CC) $(CFLAGS) main.c $(OBJECTS) -o $@

test: test.c $(HEADERS) $(OBJECTS)
	$(CC) $(CFLAGS) test.c $(OBJECTS) -o $@

#png support
lodepng/lodepng.o: lodepng/lodepng.c lodepng/lodepng.h
	$(CC) $(CFLAGS) -c lodepng/lodepng.c -o $@

termio.o: termio.c termio.h common.h
	$(CC) $(CFLAGS) -c termio.c -o $@

util.o: util.c util.h termio.h common.h lodepng/lodepng.h
	$(CC) $(CFLAGS) -c util.c -o $@

clean: 
	rm -f *.o ncimag

run: ncimag
	./ncimag

debug: ncimag
	gdb ./ncimag

