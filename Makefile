CC=gcc
LD=gcc
CFLAGS=-I. -ggdb -std=c99 -Wall
LDFLAGS= -lm
pngPlay: pngPlay.o pixutils.o lodepng.o bmp.o
	$(LD) -o pngPlay  pngPlay.o pixutils.o lodepng.o bmp.o $(LDFLAGS)

clean:
	rm *.o
	rm pngPlay
