[ comments on fixes to lower sampling frequencies
  encoder by Seymour Shlien, to appear in dist09. Jon ]

Here is a log of the bugs I have found and
corrected as well as those I would like to see
corrected.  -- Seymour

 

Problem: The scalefac's in layer 3 are always 0
for all scale factor bands.

It was noticed that the parameter ratio in
passed to iteration_loop was always the same.
The array ratio is computed by the psychoacoustic
model L3psycho_anal.

In L3psycho_anal (l3psyc.c) it was noticed that
all the variables (arrays) used to compute ratio
(en,thm,eb,thr) were not changing. In fact no
energy was coming out of the fft at line 479.
In fact savebuf was all 0's. In fact syncsize
was 0 preventing savebuf to be filled and
used in the fft. In fact the code for initializing
syncsize was #ifdef out in lines 159 to 176 of
l3psyc.c

When I put the code back in, the program crashed
at line 169 where the function read_absthr(absthr,
 sfreq_idx) was called. It crashed because the
array absthr was not allocated, because  line
152 /* absthr = (FLOAT *) mem_alloc(..) */
was commented out.



Once the psychoacoustic model was reactivated
a whole new set of problems appeared. I guess
there was a reason for deactivating the psychoacoustic
model. The following output appeared.

encode -l 3 -p 2 /ldisk/bascl_4.raw
Encoding configuration:
Algorithm=MPEG-1
Layer=III   mode=stereo   extn=0   psy model=2
samp frq=44.1 kHz   total bitrate=320 kbps
de-emph=0   c/right=0   orig=0   errprot=off
input file: '/ldisk/bascl_4.raw'   output file: '/ldisk/bascl_4.raw.mpg'
slots/frame = 1044
frac SpF=0.898, tot bitrate=320 kbps, s freq=44.1 kHz
Fractional number of slots, padding required
{   0}absthr[][] sampling frequency index: 1
{   1}{   2}Sent 8348 bits = 1043 slots plus 4
{   3}Sent 8348 bits = 1043 slots plus 4
{   4}Sent 8348 bits = 1043 slots plus 4
{   5}Sent 8348 bits = 1043 slots plus 4
{   6}Sent 8348 bits = 1043 slots plus 4
{   7}Sent 8348 bits = 1043 slots plus 4

First the bitstream formatter started to complain
that bitstream produced did not end on a byte boundary.

Secondly, when running the decoder there was an inconsistency
on where the last main_data_block ended and the next
main_data_block began as shown below:

rigel% decode /ldisk/bascl_4.raw.mpg junk
Input file = '/ldisk/bascl_4.raw.mpg'  output file = 'junk'
the bit stream file /ldisk/bascl_4.raw.mpg is a BINARY file
HDR: s=FFF, id=1, l=3, ep=off, br=E, sf=0, pd=1, pr=0, m=0, js=0, c=0, o=0, e=0
alg.=MPEG-1, layer=III, tot bitrate=320, sfrq=44.1
mode=stereo, sblim=32, jsbd=32, ch=2
{   0}{   1}{   2}{   3}Not enough main data to decode frame 3.  Frame discarded.
{   4}{   5}Not enough main data to decode frame 5.  Frame discarded.
{   6}{   7}Not enough main data to decode frame 7.  Frame discarded.
{   8}{   9}Not enough main data to decode frame 9.  Frame discarded.
{  10}{  11}Not enough main data to decode frame 11.  Frame discarded.
{  12}{  13}Not enough main data to decode frame 13.  Frame discarded.
{  14}{  15}Not enough main data to decode frame 15.  Frame discarded.

Third, the value of part2_3_length in the header
did not seem to be correct. Even when the number of
big values was zero and all the spectral intensity
levels were zero, part2_3_length was still large.
(It was probably not correct before the changes.)
Part2_3_length is supposed to bethe number of
main_data bits used to encode the scalefactors and Huffman
code data. It does NOT include stuffing bits.
Part2_3_length is used to determine the end of the
count1 region where quadruple Huffman codes are used.
Stuffing bits are supposed to be put at the end of
the main_data, not interspersed inside the granules
and channels.

Finally, the code for selecting the correct global_gain
and scalefactors was still not working.

In summary the encoder was now worse than ever.

Examining the code and running some simulations, I
beleive that in loop.c

(1) The starting value for quantizerStepSize returned
by quantanf_init, is too large so the outer_loop 
inner_loop code does not operate properly. The value
should be small and negative so that initially more
bits than are allowed are used to encode the data.
The inner_loop then starts increasing the value of
quantizerStepSize until the desired bitrate is achieved
for the particular set of scalefacs chosen by outer_loop. 


(2) The main_data reservoir is being misused for something
as the conflict between where the last maindata block area
ends and the new one starts has something to do with
the maindata end parameter in the header block. 

(3) When scalefactors are actually transmitted,
(ie scalefac_compress not 0)  allowance for there space
 may not be done properly causing the interation_loop
software to allocate non integer number of bytes which
the bitformatter does not like.  

(4) part2_3_length is probably being used properly.




I observed that the function part2_length() in the file loop.c
which computes the number of bits to encode the scale factors,
has an  error for the default case. There
are 21 scale factor bands not 24 scale factor bands.
The corrected lines are shown below.

   600              if ( (gr == 0) || (si->scfsi[ch][0] == 0) )
   601                  bits += (6 * slen1);
   602  
   603              if ( (gr == 0) || (si->scfsi[ch][1] == 0) )
   604                  /* bits += (6 * slen1);  This is wrong SS 19-12-96 */
   605                  bits += (5 * slen1);
   606  
   607              if ( (gr == 0) || (si->scfsi[ch][2] == 0) )
   608                  /* bits += (6 * slen1);  This is wrong SS 19-12-96 */
   609                  bits += (5 * slen2);
   610  
   611              if ( (gr == 0) || (si->scfsi[ch][3] == 0) )
   612                  /* bits += (6 * slen1);  This is wrong SS 19-12-96 */
   613                  bits += (5 * slen2);


Miraculously, after placing these changes two problems were eliminated.
The bitstream formatter stopped sending its warning messages and the
decoder was able to decode the file without error messages. Incredible !


The iteration_loop code was still not working very well as a result 
the scalefactors were still not properly used. A simple fix is to 
change the return value from quantanf_init to something lower.

In loop.c, changing lines 307-308 provided a more reasonable starting
value for the optimization resulting in the usuage of the scalefactors,
but the encoder runs even slower.  


   299          sfm = exp( sum1 / 576.0 ) / (sum2 / 576.0);
   300          tp = nint( system_const * log(sfm) );
   301          if ( tp < minlimit )
   302              tp = minlimit;
   303  #ifdef DEBUG
   304          printf(" quantanf = %d (quantanf_init)\n",tp );
   305  #endif
   306      }
   307  /*    return(tp); SS 19-12-96 */
   308        return(tp-60.0);
   309  }


The optimization procedure described in the standard is computationally
slow and this is the main bottle neck in the encoder. My
code for speeding up the process was not implemented in this distribution
despite the comments in the documentation.

I have re-implemented the binary search procedure into loop.c,
however, I am not too sure whether it runs faster. I shall need
to do more testing. It is probably running more accurately.

I observed that ResvFrameEnd() in reservoir.c adds and includes
stuffingBits into gi->part2_3_length. I have not changed the code
to fix this problem since it is more major surgery.
 
It is quite difficult to find things in the loop.c sources. I
have edited it to improve its appearance and make all the functions
stand out more clearly.



There is an unresolved problem with the outer_loop/inner_loop 
algorithm in loop.c. Though the code is in agreement with the
specification in the normative section of the 11172-3 ITU standard, 
it does not seem to operate properly all the time in particular
when the desired bit rate is low. Essentially, amp_scalefac_bands
function raises the scale factors in order to lower the quantization
noise in particular scale factor bands, but when inner_loop is
called to bring back the bit rate to the specified value
all of this work is undone. Frequently when the algorithm terminates,
the noise in the particular scalefac bands was not reduced by
much or even increased and more noise was introduced in the
other scalefac bands due to the additional overhead of transmitting
the scalefactors. As a result this code does not seem to
accomplish much in improving the overall quality of the encoded
signal. 

It is left as a research topic to find a more efficient algorithm. 
(I have left some debugging code enclosed by #ifdef PERFORM
in loop.c in order to investigate this problem.)    
   

Seymour Shlien
Communications Research Centre
Ottawa, Ontario, Canada
K2H 8S2

voice (613) 998-2551

e-mail seymour.shlien@crc.doc.ca
