CFLAGS=-I/usr/X11R7/include
LDFLAGS=-L/usr/X11R7/lib

all: mainwindow.o
	gcc $(LDFLAGS) $(CFLAGS) -o level-editor mainwindow.o -lXaw -lXmu -lXt -lX11

clean:
	rm -rf mainwindow.o
	rm level-editor
