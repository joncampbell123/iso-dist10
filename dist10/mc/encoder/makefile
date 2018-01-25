###############################################################################
## Copyright (c) 1995 MPEG/audio software simulation group
## All Rights Reserved
## $Id: makefile 1.6 1996/02/12 07:13:35 rowlands Exp $
##
## MPEG/audio coding/decoding software, work in progress
##   NOT for public distribution until verified and approved by the
##   MPEG/audio committee.
##
## $Log: makefile $
## Revision 1.6  1996/02/12 07:13:35  rowlands
## Release following Munich meeting
##
## Revision 1.5  1996/01/20 17:13:10  rowlands
## Moved Merged into main trunk
##
## Revision 1.4.3.2  1996/01/20  17:10:43  rowlands
## Added make "test" to encode decoded compliance bitstreams in
## test directory. Also added make "veryclean" to remove all but source.
##
## Revision 1.4.3.1  1996/01/19  04:40:26  rowlands
## Merged
##
## Revision 1.4  1995/06/16  06:29:49  rowlands
## Moved merged IRT/TI into main revision control trunk.
##
## Revision 1.3.2.1  1995/06/16  06:27:03  rowlands
## Added file dyncross.c
##
## Revision 1.3.2.0  1995/06/16  04:50:21  rowlands
## Created branch for IRT
##
## Revision 1.3.1.4  1995/06/16  03:08:10  rowlands
## Added make clean
##
## Revision 1.3.1.0  1995/06/12  00:02:16  rowlands
## Created branch for TI
##
## Revision 1.3  1995/04/06  05:48:03  rowlands
## Added file lingual.c
##
## Revision 1.2  1995/04/06  05:42:26  rowlands
## Added RCS header and multi-machine support
##
###############################################################################

###############################
# Compiler flags for UNIX
CC		= cc
#CFLAGS		= -DUNIX -g2 -DAugmentation_7ch -DPrintCRCDebug -DPrintBitDebug -DPRINTOUT -DDEBUG
CFLAGS		= -DUNIX -g2 -DAugmentation_7ch
#CFLAGS		= -DUNIX -mips2 -O2
LFLAGS		= -lm
EXE_SUFFIX	=
OBJ_SUFFIX	= .o

###############################
# Compiler flags for DOS MSC6.0
#CC		= cl
#CFLAGS		= -Zi -Gt -AH -DMSDOS -DMS_DOS
#LFLAGS		= -link /stack:0x8000
#EXE_SUFFIX	= .exe
#OBJ_SUFFIX	= .obj

.c$(OBJ_SUFFIX):
	$(CC) -c $(CFLAGS) $<

ENCOBJS = encode$(OBJ_SUFFIX) common$(OBJ_SUFFIX) musicin$(OBJ_SUFFIX)\
	subs$(OBJ_SUFFIX) psy$(OBJ_SUFFIX) tonal$(OBJ_SUFFIX)\
	predisto$(OBJ_SUFFIX) lingual$(OBJ_SUFFIX) dyn_cross$(OBJ_SUFFIX)\
	lfe$(OBJ_SUFFIX)

musicin$(EXE_SUFFIX) : $(ENCOBJS)
	$(CC) $(CFLAGS) -o $@ $(ENCOBJS) $(LFLAGS)

$(ENCOBJS): common.h encoder.h makefile

clean:
	rm -f $(ENCOBJS) musicin$(EXE_SUFFIX)
