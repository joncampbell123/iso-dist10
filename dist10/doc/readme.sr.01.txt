READ ME to ISO/MPEG2 Audio coding shareware

07/15/94 Susanne Ritscher, IRT Munich
         Tel: +49 89 32399 458
         Fax: +49 89 32399 351


The content of the main directory is:

read.me		- this file
encoder		- directory containing encoder sources as follows:
		  common.c, common.h, encode.c, encoder.h, musicin.c
		  predisto.c, psy.c, subs.c, tonal.c, makefile
		  tables/
		  1cb0, 1cb1, 1cb2, 1th0, 1th1, 1th2
		  2cb0, 2cb1, 2cb2, 2th0, 2th1, 2th2
		  absthr_0, absthr_1, absthr_2
		  alloc_0, alloc_1, alloc_2, alloc_3
		  dewindow, enwindow

decoder		- directory containing decoder sources as follows:
		  common.c, common.h, decode.c, decoder.h, musicout.c
		  tables/
		  1cb0, 1cb1, 1cb2, 1th0, 1th1, 1th2
		  2cb0, 2cb1, 2cb2, 2th0, 2th1, 2th2
		  absthr_0, absthr_1, absthr_2
		  alloc_0, alloc_1, alloc_2, alloc_3
		  dewindow



This shareware works only under Unix.
The Makefile is done on a Silicon Graphics Iris Indigo.

Features of the multichannel-coding are:

- Layer II
- 3/2 configuration
- matrix mode with center and surround at -3 dB level
- common bit pool
- transmission channel-switching
- pre-quantisation
- extension bitstream

The program can be started via command line or interactive.

For any questions concerning the encoder please contact:

	 Susanne Ritscher, IRT Munich
         Tel:   +49 89 32399 458
         Fax:   +49 89 32399 351
         email: ritscher@irt.de


For any questions concerning the decoder please contact:

         Ralf Schwalbe, Telekom FTZ Berlin
	 Tel:   +49 30 6708 2406
         Fax:   +49 30 6774 539
         email: Schwalbe@audio.fz.telekom.de
