#
# Makefile for Actor Byte-Code Machine
#

#CFLAGS=	-ansi -pedantic
#CFLAGS=	-ansi -pedantic -Wall
#CFLAGS=	-O3 -DNDEBUG
#CFLAGS=	-std=c99 -m32 -O2 -Wall
#CFLAGS=	-m16 -O2 -Wall
#CFLAGS=	-m32 -O2 -Wall
#CFLAGS=	-O2 -Wall
#CFLAGS=	-g -Wall -DLOG_ALL
#CFLAGS=	-Wall -DMAIN -DLOG_ALL
CFLAGS=	-DMAIN -Wall

LIB=	libabc.a
LOBJS=	bose.o equiv.o log.o
LHDRS=	bose.h equiv.h log.h

LIBS=	$(LIB)

PROGS=	abcm
JUNK=	*.exe *.stackdump core *~

all: $(LIB) $(PROGS)

clean:
	rm -f $(LIB) $(PROGS) $(JUNK) *.o

test: $(PROGS)
	rm -f $(JUNK)
	./abcm

$(LIB): $(LOBJS)
	$(AR) rv $(LIB) $(LOBJS)

$(LOBJS): $(LHDRS)

abcm: abcm.o $(LIB) test.o test.h abcm.h Makefile
	$(CC) $(CFLAGS) -o $@ abcm.o $(LIBS) test.o

.c.o:
	$(CC) $(CFLAGS) -c $<
