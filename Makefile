CC = gcc
CFLAGS = -Wall -Wstrict-prototypes -g -O2
SRC = crikey.c
OBJ = $(SRC:.c=.o)
LIBS = -L/usr/X11R6/lib -lX11 -lXtst

all: crikey

crikey: $(OBJ)
	$(CC) -o crikey $(OBJ) $(LIBS)

install: crikey
	cp crikey /usr/local/bin

clean:
	rm -f $(OBJ) crikey *~

