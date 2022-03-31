# dwm - dynamic window manager
# See LICENSE file for copyright and license details.

include config.mk

SRC = main.c
OBJ = ${SRC:.c=.o}

all: test

.c.o:
	${CC} -c ${CFLAGS} $<

${OBJ}: config.mk

test: ${OBJ}
	${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	rm -f test ${OBJ} 

.PHONY: all
