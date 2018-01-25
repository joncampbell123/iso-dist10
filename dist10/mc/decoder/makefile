###############################################################################
## Copyright (c) 1995 MPEG/audio software simulation group
## All Rights Reserved
## $Id: makefile 1.5 1996/06/18 22:47:29 rowlands Exp $
##
## MPEG/audio coding/decoding software, work in progress
##   NOT for public distribution until verified and approved by the
##   MPEG/audio committee.
##
## $Log: makefile $
## Revision 1.5  1996/06/18 22:47:29  rowlands
## Added extra debug output
##
## Revision 1.4  1996/01/20 17:02:20  rowlands
## Moved merged FTZ_04 and Philips_01 into main trunk
##
## Revision 1.3.3.2  1996/01/20  16:59:40  rowlands
## Added "test" target for running compliance bitstreams. Also
## added "veryclean" target to leave only source.
##
## Revision 1.3.3.1  1995/11/05  22:50:42  rowlands
## Merged FTZ_04 and Philips_01 using #ifdefs
##
## Revision 1.3  1995/06/16  08:44:24  rowlands
## Moved merged IRT/TI into main revision control trunk.
##
## Revision 1.2.1.2  1995/06/16  03:16:47  rowlands
## Added executable to make clean
##
## Revision 1.2.1.0  1995/06/16  02:07:35  rowlands
## Created branch for TI
##
## Revision 1.2  1995/06/14  04:39:32  rowlands
## Added "make clean"
##
## Revision 1.1  1995/04/06  05:43:27  rowlands
## Initial revision
##
###############################################################################

###############################
# Compiler flags for UNIX
CC		= cc
#CFLAGS		= -DUNIX -g -DAugmentation_7ch -DPrintCRCDebug -DPrintBitDebug
CFLAGS		= -DUNIX -g -DAugmentation_7ch
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

DECOBJS = decode$(OBJ_SUFFIX) common$(OBJ_SUFFIX) musicout$(OBJ_SUFFIX)

musicout$(EXE_SUFFIX) : $(DECOBJS)
	$(CC) $(CFLAGS) -o $@ $(DECOBJS) $(LFLAGS)

$(DECOBJS): common.h decoder.h makefile

clean:
	rm -f $(DECOBJS) musicout$(EXE_SUFFIX)
