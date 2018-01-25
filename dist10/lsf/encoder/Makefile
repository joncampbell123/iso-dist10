# Generated automatically from Makefile.in by configure.
############################################################################
## ISO MPEG Audio Subgroup Software Simulation Group (1996)
## ISO 13818-3 MPEG-2 Audio Decoder - Lower Sampling Frequency Extension
##
## $Id: Makefile,v 1.2 1996/08/14 23:18:34 rowlands Exp $
##
## Makefile for encoder. Requies GNU make.
##
## $Log: Makefile,v $
## Revision 1.2  1996/08/14 23:18:34  rowlands
## Cleanups.
##
## Revision 1.2  1996/02/14 05:18:05  rowlands
## Cleanups.
##
## Revision 1.1  1996/02/14 04:04:23  rowlands
## Initial revision
##
## Received from Mike Coleman
############################################################################

CC = gcc

c_sources = \
	common.c \
	encode.c \
	formatBitstream.c \
	huffman.c \
	ieeefloat.c \
	l3bitstream.c \
	l3psy.c \
	loop.c \
	mdct.c \
	musicin.c \
	portableio.c \
	psy.c \
	reservoir.c \
	subs.c \
	tonal.c

OBJ = $(c_sources:.c=.o)
DEP = $(c_sources:.c=.d)

NINT_SWITCH = 

CC_SWITCHES = -g -O -DUNIX -DBS_FORMAT=BINARY $(NINT_SWITCH) -DNDEBUG

PGM = encode

LIBS =  -lm


%.o: %.c 
	$(CC) $(CC_SWITCHES) -c $< -o $@

%.d: %.c
	$(SHELL) -ec '$(CC) -M $(CC_SWITCHES) $< | sed '\''s/$*.o/& $@/g'\'' > $@'


$(PGM):	$(OBJ) Makefile
	$(CC) -o $(PGM) $(OBJ) $(LIBS)

clean:
	-rm $(OBJ) $(DEP)

tags: TAGS

TAGS: ${c_sources}
	etags -T ${c_sources}

-include $(DEP)
