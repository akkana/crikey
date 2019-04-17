CC = gcc
OPTS = -O2
CFLAGS = -Wall -Wstrict-prototypes -g $(OPTS)
SRC = crikey.c
OBJ = $(SRC:.c=.o)
X11LIBS = /usr/X11R6/lib
LIBS = -L$(X11LIBS) -lX11 -lXtst -lXext
PREFIX = /usr/local
BINDIR = $(PREFIX)/bin
DESTDIR =

all: crikey

crikey: $(OBJ)
	$(CC) -o crikey $(OBJ) $(LIBS)

install: crikey
	mkdir -p $(DESTDIR)/$(BINDIR)
	cp crikey $(DESTDIR)/$(BINDIR)

uninstall:
	rm -f $(DESTDIR)/$(BINDIR)/crikey

clean:
	rm -f $(OBJ) crikey *~

