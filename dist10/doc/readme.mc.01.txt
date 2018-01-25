Notes on the layer3 port:

During August and September 1995 I ported a 64 kbit/48kHz version of the
layer3 encoder software to the new version of the layer1 and 2 public
code. I mainly added the support for other sampling frequencies and
bitrates, and all the layer3 bitstream code. I did not attempt to fix any
of the "features" of the musicin code it is based upon.

Contributors to the earlier version apparently include Masahiro Iwadare,
Toshiyuki Ishino, Seymour Shlein, and Davis Pan. Seymour Shlein helped
test this version.

This code should produce good bitstreams, but there are still several
problems remaining. Some of them are:

1: I did not look into the timing synchronization of the psychoacoustic
   model with the hybrid filter and the loops code. I think
   that there might be trouble there.

2: There is no support for joint-stereo. This requires a change to the
   program flow which I did not have time to complete.

3: The scalefactor code in the loops module needs work. In particular,
   scalefac_scale is never set to one.

4: The code never sets subblock_gain.

5: Not all scalefactor partitioning schemes are used in the LSF mode.

I think it is safe to say that the audio quality of this coder is not up to
the potential for layer3. The sound quality is especially bad for
non-stationary signals. As with the rest of the public code, no attempts
have been made to improve the efficiency of this code, since its primary
purpose is to help you understand the standard.

Mike Coleman <mc@fivebats.com> 1995/10/16
