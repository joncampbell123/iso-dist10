############################################################################
## ISO MPEG Audio Subgroup Software Simulation Group (1996)
## ISO 13818-3 MPEG-2 Audio Encoder - Lower Sampling Frequency Extension
##
## $Id: makefile.unix,v 1.1 1996/02/14 05:28:25 rowlands Exp $
##
## Makefile for encoder for vanilla UNIX platform using standard make.
##
## $Log: makefile.unix,v $
## Revision 1.1  1996/02/14 05:28:25  rowlands
## Initial revision
##
############################################################################

# MODE= -O -DUNIX
MODE= -g -DUNIX
#MODE= -DMSC60 -AH -Zi -Gt
OBJ_SUFFIX=.o
EXE_SUFFIX=

PROGRAM	= encode$(EXE_SUFFIX)

all: $(PROGRAM)

$(PROGRAM) : \
	common.c common.h encode.c encoder.h formatBitstream.c \
	formatBitstream.h huffman.c huffman.h ieeefloat.c ieeefloat.h \
	l3bitstream-pvt.h l3bitstream.c l3bitstream.h l3psy.c l3psy.h \
	l3side.h loop-pvt.h loop.c loop.h mdct.c mdct.h musicin.c \
	portableio.c portableio.h psy.c reservoir.c reservoir.h subs.c tonal.c

HEDS =  \
	common.h encoder.h formatBitstream.h huffman.h ieeefloat.h \
	l3bitstream-pvt.h l3bitstream.h l3psy.h l3side.h loop-pvt.h loop.h \
	mdct.h portableio.h reservoir.h

OBJS = \
	common$(OBJ_SUFFIX) \
	encode$(OBJ_SUFFIX) \
	formatBitstream$(OBJ_SUFFIX) \
	huffman$(OBJ_SUFFIX) \
	ieeefloat$(OBJ_SUFFIX) \
	l3bitstream$(OBJ_SUFFIX) \
	l3psy$(OBJ_SUFFIX) \
	loop$(OBJ_SUFFIX) \
	mdct$(OBJ_SUFFIX) \
	musicin$(OBJ_SUFFIX) \
	portableio$(OBJ_SUFFIX) \
	psy$(OBJ_SUFFIX) \
	reservoir$(OBJ_SUFFIX) \
	subs$(OBJ_SUFFIX) \
	tonal$(OBJ_SUFFIX)

LFLAG = 

CC    = gcc
LN    = gcc
LNEXTRA=-lm
#CC    = cl
#LN    = cl

.c$(OBJ_SUFFIX): $(HEDS)
	$(CC) -c $(MODE) $*.c

all: $(PROGRAM)

default: all


$(PROGRAM) : $(OBJS) $(HEDS)
	$(LN) -o $(PROGRAM) $(MODE) $(LFLAG) $(OBJS) $(LNEXTRA)

clean:
	rm -f $(OBJS) $(PROGRAM)

veryclean:
	rm -f $(PROGRAM) $(OBJS)
