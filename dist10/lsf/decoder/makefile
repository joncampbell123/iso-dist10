############################################################################
## ISO MPEG Audio Subgroup Software Simulation Group (1996)
## ISO 13818-3 MPEG-2 Audio Decoder - Lower Sampling Frequency Extension
##
## $Id: makefile.unix,v 1.3 1996/02/14 05:16:29 rowlands Exp $
##
## Makefile for decoder for vanilla UNIX platform using standard make.
##
## $Log: makefile.unix,v $
## Revision 1.3  1996/02/14 05:16:29  rowlands
## Cleanups.
##
## Received from FhG
############################################################################

# MODE= -O -DUNIX
MODE= -g -DUNIX -Wall -Wextra -pedantic
#MODE= -DMSC60 -AH -Zi -Gt
OBJ_SUFFIX=.o
EXE_SUFFIX=

PROGRAMD	= decode$(EXE_SUFFIX)
PROGRAM		= $(PROGRAMD) 

all: $(PROGRAM)

$(PROGRAMD) : \
	decode.c musicout.c common.c common.h decoder.h huffman.h \
	ieeefloat.h portableio.h

HEDS =  common.h decoder.h huffman.h portableio.h ieeefloat.h

OBJSD = \
	decode$(OBJ_SUFFIX) \
	musicout$(OBJ_SUFFIX) \
	common$(OBJ_SUFFIX) \
	huffman$(OBJ_SUFFIX) \
	portableio$(OBJ_SUFFIX) \
	ieeefloat$(OBJ_SUFFIX)

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


$(PROGRAMD) : $(OBJSD) $(HEDS)
	$(LN) -o $(PROGRAMD) $(MODE) $(LFLAG) $(OBJSD) $(LNEXTRA)

clean:
	rm -f $(OBJSD) $(PROGRAM)

veryclean:
	rm -f $(PROGRAM) $(OBJSD) $(PROGRAM)

