Dear Jon,

Attached is a modified version of the coding program in tar'ed, compressed and uuencoded form.

The modified program has the following capabilities:

- Encodes and decodes Layer II LSF, in addition to MPEG-1
- Decodes Layer I LSF
- Sometimes encodes Layer I LSF, but has a logical problem somewhere in the psychoacoustics which causes segmentation faults. It is probably a consistency problems in the tables. I have solved the problem long time ago for L2 MPEG-1, but for now I will leave it as is due to the deadline.

The addition of MPEG-2 LSF capability required addition of:
- A new bit allocation table for Layer II, 'alloc_4'.
- 12 new tables for psychoacoustic model 1, with suffixes 4, 5 and 6.
- The relevant bit and sampling rates in 'common.c'.

In 'decode.c' function SubBandSynthesis() there was a rounding problem when converting from floating point to integer. The casting in C truncates against zero for positive as well as negative numbers. Someone has already solved the problem but the change was commented out.

No sampling rate conversion is made.

MPEG-1 or MPEG-2 LSF is selected according to the sampling rate of the input file.

A bug was fixed in the function new_ext() in 'common.c'.

The program runs on SGI Indigo under IRIX 5.3 and under DOS 5.0 Turbo C.

Best regards,


Soeren
