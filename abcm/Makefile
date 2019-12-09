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
CFLAGS=	-g -DMAIN -Wall
#CFLAGS=	-DMAIN -Wall

LIB=	libabc.a
LOBJS=	sponsor.o event.o runtime.o string.o array.o object.o pool.o equiv.o print.o bose.o device.o log.o
LHDRS=	sponsor.h event.h runtime.h string.h array.h object.h pool.h equiv.h print.h bose.h device.h log.h

MAIN=	abcm
MOBJS=	abcm.o program.o test.o
MHDRS=	abcm.h program.h test.h
LIBS=	$(LIB)

JUNK=	*.exe *.stackdump core *~

all: $(LIB) $(MAIN)

clean:
	rm -f $(LIB) $(MAIN) $(JUNK) *.o

test: $(MAIN)
	rm -f $(JUNK)
	./abcm

$(LIB): $(LOBJS)
	$(AR) rv $(LIB) $(LOBJS)

$(LOBJS): $(LHDRS)

$(MAIN): $(MOBJS) $(LIB) Makefile
	$(CC) $(CFLAGS) -o $@ $(MOBJS) $(LIBS)

$(MOBJS): $(MHDRS)

.c.o:
	$(CC) $(CFLAGS) -c $<
