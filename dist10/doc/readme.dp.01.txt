

          INTERNATIONAL ORGANIZATION FOR STANDARDIZATION
           ORGANISATION INTERNATIONALE DE NORMALISATION
                               ISO/IEC JTC1/SC29/WG 11
            CODING OF MOVING PICTURES AND ASSOCIATED AUDIO

								MPEG94/
								Nov. 1994

Source: Davis Pan (Motorola Inc.),
            Chairman of the MPEG/audio ad hoc committee on software
            simulation
Title: Working Draft of MPEG/Audio Technical Report

Disclaimer of Warranty
	These software programs are available to the user without any 
license fee or royalty on an "as is" basis.  ISO disclaims any and all 
warranties,  whether express, implied, or statuary, including any 
implied warranties or merchantability or of fitness for a particular 
purpose.  In no event shall ISO be liable for any incidental, punitive, 
or consequential damages of any kind whatsoever arising from the 
use of these programs.

	This disclaimer of warranty extends to the user of these 
programs and user's customers, employees, agents, transferees, 
successors, and assigns,

	ISO does not represent or warrant that the programs furnished 
hereunder are free of infringement or any third-party patents, 
copyrights or trade secrets.

	The purpose of this software is to provide a tool to help in the 
learning and understanding of the MPEG/audio compression and 
decompression algorithm.  It is not an efficient implementation.



Organization of this Report

	The main body of this report describes the organization and 
use of the software.  The listings of the software, sample makefiles, 
and test bitstreams are contained in the appendices:



Software Limitations

	The software implements levels I and II and psychophysical
auditory models I and II as described in the ISO 3-11171 rev 1
standard.  The input/output audio data may either be headerless raw
16 bit data or alternatively an AIFF formatted file (Audio Interchange
File Format) with certain limitations. (For more information on the
AIFF format see the postscript file /sgi/aiff-c.9.26.91.ps at the
anonymous ftp site FTP.SGI.COM (192.48.153.1)).  The AIFF formatted
file is a convenient way for storing sampling rate of the audio file.

	The encoder and decoder software are configured to output the
coded audio bitstreams as a string of hexadecimal ascii characters. 
For greater compression efficiency,  compile flag, BS_FORMAT, in 
common.h can be switched to configure the bitstream reading and 
writing routines to process raw binary bitstreams.

	This software has been run and verified on a large variety of
computers and operating systems. In particular UNIX, MS_DOS (with
MicroSoft C version 6), Macintosh, AIX Platform (RS6000) and Convex.


Organization of the Code

The MPEG/audio Layer 1 and Layer 2 ** software package consists 
of: 
	21 data files tables
	8 source files (*.c)
	3 definitions files (*.h)
	3 test bitstreams
	* makefiles

** There is a separate release note for the Layer 3 software


Table 1 illustrates how the encoder and decoder is formed from the 
component files.  In this table the definition files are enclosed in 
parenthesis and listed immediately below the primary source file 
which uses them.  The data file names are listed within braces and 
also placed immediately below the source file which uses them.

			Table 1

encoder			common			decoder
files			files			files
----------      ------------    ------------
musicin.c		common.c		musicout.c
encode.c		(common.h)		decode.c
(encoder.h)	 	{alloc_0}  		(decoder.h)
{enwindow}		{alloc_1}		{dewindow}
psy.c, subs.c		{alloc_2}
{absthr_0}		{alloc_3}
{absthr_1}
{absthr_2}
tonal.c
{1cb0}, {1cb1}, {1cb2}
{2cb0}, {2cb1}, {2cb2}
{1th0}, {1th1}, {1th2}
{2th0}, {2th1}, {2th2}

The test files orig.mpg and sine.dec are approximately
129026 and 387072 bytes. (Small variations are due to the blocking
structures imposed by various disk operating systems.)



Software Installation

	Place are the 21 tables (absthr_*, *cb*, *th*, alloc_*)
in a subdirectory called tables. Modify the common.h to reflect
the compiler and machine you are using. (ie. uncomment one of the
defs UNIX, MACINTOSH, MS_DOS, MSC60, AIX or CONVEX). Compile and
link the files using one of the makefiles provided as a guideline.
Note that the compiler may issue warning messages such as

common.c: In function `aiff_read_headers':
common.c:686: warning: multi-character character constant
common.c:687: warning: multi-character character constant
common.c:703: warning: multi-character character constant
common.c:735: warning: multi-character character constant
common.c: In function `aiff_write_headers':
common.c:825: warning: multi-character character constant
common.c:826: warning: multi-character character constant
common.c:827: warning: multi-character character constant

musicin.c:920: warning: static declaration for `usage' follows non-static
musicin.c: In function `aiff_check':
musicin.c:960: warning: multi-character character constant
 
These messages are dependent on the preprocessor directives specified
in the common.h file. 


Usuage

	To run the code type the name of the file followed by a 
carriage return.  The programs will prompt you to input the 
appropriate parameters.  The sound input file for the encoder should 
be sound data, monophonic or stereophonic, sampled at 32, 44.1, or 
48 kHz with 16 bits per sample.  For stereophonic data the left 
channel sample should precede the right channel sample.  The sound 
output file of the decoder will be the same format as the sound input 
file used by the decoder,  except for possible byte order differences 
if the encoder and decoder programs are run on different computer
systems which have different byte ordering conventions.

	Assuming the program musicin produces the MPEG coded file
and musicout decodes the MPEG coded file, the programs can be run
in either commandline mode or user prompt mode.  For example, on a 
UNIX operating system, entering

musicin sine.dec sine.mpg
 
will run the program with the current built in defaults. The output
from the program should appear as below. 

Encoding configuration:
Layer=II   mode=stereo   extn=0   psy model=2
samp frq=44.1 kHz   total bitrate=384 kbps
de-emph=0   c/right=0   orig=0   errprot=0
input file: 'sine.dec'   output file: 'sine.mpg'
using bit allocation table alloc_1
slots/frame = 1253
frac SpF=0.878, tot bitrate=384 kbps, s freq=44.1 kHz
Fractional number of slots, padding required
{   0}absthr[][] sampling frequency index: 1
{   1}{   2}{   3}{   4}{   5}{   6}{   7}...


Alternatively, if you enter

musicin -help

The program will respond with the correct command-line usuage
as shown below and then exit.

musicin: unrec option h
musicin: -l layer must be 1 or 2, not p
usage: musicin                         queries for all arguments, or
       musicin [-l lay][-m mode][-p psy][-s sfrq][-b br][-d emp]
          [-c][-o][-e] inputPCM [outBS]
where
 -l lay   use layer <lay> coding   (dflt    2)
 -m mode  channel mode : s/d/j/m   (dflt    s)
 -p psy   psychoacoustic model 1/2 (dflt    2)
 -s sfrq  input smpl rate in kHz   (dflt 44.1)
 -b br    total bitrate in kbps    (dflt  384)
 -d emp   de-emphasis n/5/c        (dflt    n)
 -c       mark as copyright
 -o       mark as original
 -e       add error protection
 inputPCM input PCM sound file (standard or AIFF)
 outBS    output bit stream of encoded audio (dflt inName+.mpg)



Finally, if you just enter

musicin

Then the program will prompt you for all the coding parameters before
executing.

Enter PCM input file name <required>: sine.dec
>>> PCM input file name is: sine.dec
Enter MPEG encoded output file name <sine.dec.mpg>: sine.mpg
>>> MPEG encoded output file name is: sine.mpg
What is the sampling frequency? <44100>[Hz]: 
>>> Default 44.1 kHz samp freq selected
Which layer do you want to use?
Available: Layer (1), Layer (<2>): 
>>> Using default Layer 2
Which mode do you want?
Available: (<s>)tereo, (j)oint stereo, (d)ual channel, s(i)ngle Channel: 
>>> Using default stereo mode
Which psychoacoustic model do you want to use? <2>: 
>>> Default model 2 selected
What is the total bitrate? <384>[kbps]: 
>>> Using default 384 kbps
What type of de-emphasis should the decoder use?
Available: (<n>)one, (5)0/15 microseconds, (c)citt j.17: 
>>> Using default no de-emphasis
Do you want to set the private bit? (y/<n>): 
>>> Private bit not set
Do you want error protection? (y/<n>): 
>>> Error protection not used
Is the material copyrighted? (y/<n>): 
>>> Material not copyrighted
Is this the original? (y/<n>): 
>>> Material not original
Do you wish to exit (last chance before encoding)? (y/<n>):



The software is functioning properly if the following equations hold:
a. decoded(orig.mpg) == deco.dec
    byte-swapping of deco.dec will be necessary for this 
   equation to hold for little-endian computers
b. encoded(deco.dec) == renc.mpg

    (encode with the default options except for the following:
    48 kHz sampling rate and 256 kbits/sec coded bit rate)

If the bitstream tests fail, make sure that the following 
variable types have at least the precision listed below:

	integer	-	16 or 32bits
	float	-	32 bits
	double	-	64 bits.


Special notes for MSDOS users:

1. The default bitrate option does not work.
2. The input/output filename defaults are not compatible with MSDOS.
3. The large memory model should be used for compilation.


	Appendix  A contains the source code for the MPEG/audio 
software, written in the C programming language.

 

	Appendix B contains sample "makefiles" which can be used to
compile the software.  Before compiling, please examine the file
common.h to determine if any of the "#define" declarations should be
reactivated by removing it from a comment statement.

	Appendix  C contains a minimal bitstream test that can be used 
to verify the proper operation of the MPEG/audio software.  The 
bitstream test consists of three bitstreams:
   orig.mpg	- The original, coded MPEG/audio bitstream
   deco.dec	- The audio data resulting from decoding orig.mpg
   renc.dec	- The encoded MPEG/audio bitstream obtained by 
		   encoding deco.dec



Notes on the Software

	The decoder program has a very crude implementation of 
bitstream synchword detection.  It may not be able to correctly 
decode valid bitstreams which have false synchword patterns in the 
ancillary data portion of the bitstream.




Appendix A MPEG/audio Source Code
(included elsewhere)

Appendix B Sample "makefiles"
(included elsewhere)

Appendix C Test Bitstreams
(included elsewhere)

Appendix D List of Contributors
Bill Aspromonte
Shaun Astarabadi
R. Bittner
Karlheinz Brandenburg
W. Joseph Carter
Jack Chang
Mike Coleman
Johnathan Devine
Ernst Eberlein
Dan Ellis
Peter Farrett
Jean-Georges Fritsch
Vlad Fruchter
Hendrik Fuchs
Bernhard Grill
Amit Gulati
Munsi Haque
Chuck Hsiao
Toshiyuki Ishino
Masahiro Iwadare
Earl Jennings
James Johnston
Leon v.d. Kerkhof
Don Lee
Mike Li
Yu-Tang Lin
Soren Neilsen
Simao F. Campos Neto
Mark Paley
Davis Pan
Tan Ah Peng
Kevin Peterson
Juan Pineda
Ernst F. Schroeder
Peter Siebert
Jens Spille
Sam Stewart
Al Tabayoyon
Kathy Wang
Franz-Otto Witte
Douglas Wong
