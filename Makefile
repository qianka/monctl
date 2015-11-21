include config.mk

SRC = $(wildcard monctl.c util/*.c)
OBJ = $(SRC:.c=.o)

all: options monctl

options:
	@echo monctl build options
	@echo "CFLAGS = ${CFLAGS}"
	@echo "LDLAGS = ${LDFLAGS}"
	@echo "CC = ${CC}"

config.h:
	cp config.def.h config.h

%.o : %.c
	@echo CC -o $@
	@${CC} -c ${CFLAGS} -o $@ $<

${OBJ}: config.h config.mk

monctl: ${OBJ}
	@echo CC -o monctl
	@${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	@echo cleaning
	@rm -f monctl ${OBJ}

.PHONY: all options clean
