/**********************************************************************
 * ISO MPEG Audio Subgroup Software Simulation Group (1996)
 * ISO 13818-3 MPEG-2 Audio Multichannel Encoder
 *
 * $Id: lingual.c 1.7 1996/02/12 07:13:35 rowlands Exp $
 *
 * $Log: lingual.c $
 * Revision 1.7  1996/02/12 07:13:35  rowlands
 * Release following Munich meeting
 *
 * Revision 1.4.2.1  1995/11/06  04:19:12  rowlands
 * Received from Uwe Felderhoff (IRT)
 *
 * Revision 1.5  1995/08/14  08:03:03  tenkate
 * ML-LSF added Warner ten Kate 7/8/95 (Philips)
 * change alloc and sblimit into alloc_ml and sblimit_ml where appropriate
 * ml_sb_sample_swap() ml_sb_sample_shift() and pick_scale_ml_2() added.
 **********************************************************************/

#define VERY_FAST_FILTER  1	/* JMZ */

#define LOWPASS 20

#include "common.h"
#include "encoder.h"

/************************************************************************/
/*
/* read_samples()
/*
/* PURPOSE:  reads the PCM samples from a file to the buffer
/*
/*  SEMANTICS:
/* Reads #samples_read# number of shorts from #musicin# filepointer
/* into #sample_buffer[]#.  Returns the number of samples read.
/*
/************************************************************************/

unsigned long read_samples_ml(FILE *musicin, long int *sample_buffer, long unsigned int num_samples, long unsigned int frame_size, int *byte_per_sample, int *aiff)
{
unsigned long samples_read;
static unsigned long samples_to_read;
static char init = TRUE;
short pcm_sample_buffer[8064];        /*for correct reading of pcm-data*/
int i;

if (init) 
{
	samples_to_read = num_samples;
	init = FALSE;
}
if (samples_to_read >= frame_size)
	samples_read = frame_size;
else
	samples_read = samples_to_read;
if((*aiff==1) &&(*byte_per_sample !=2))
{
if ((samples_read = fread(sample_buffer, *byte_per_sample, (int)samples_read, musicin)) == 0)
	if (verbosity >= 2) printf("Hit end of audio data\n");
}
else
{
if ((samples_read = fread(pcm_sample_buffer, sizeof(short), (int)samples_read, musicin)) == 0)
	if (verbosity >= 2) printf("Hit end of audio data\n");
for(i = 0; i < samples_read; ++i) /* replace 5760 by 'samples_read' WtK 7/8/95 */
	sample_buffer[i] = pcm_sample_buffer[i];
}

samples_to_read -= samples_read;
if (samples_read < frame_size && samples_read > 0) 
{
	if (verbosity >= 2) printf("Insufficient PCM input for one frame - fillout with zeros\n");
	for (; samples_read < frame_size; sample_buffer[samples_read++] = 0);
	samples_to_read = 0;
}
return(samples_read);
}

 
/************************************************************************/
/*
/* get_audio_ml()
/*
/* PURPOSE:  reads a frame of audio data from a file to the buffer,
/*   aligns the data for future processing, and separates the
/*   left and right channels
/*
/*  SEMANTICS:
/* Calls read_samples() to read a frame of audio data from filepointer
/* #musicin# to #insampl[]#.  The data is shifted to make sure the data
/* is centered for the 1024pt window to be used by the psychoacoustic model,
/* and to compensate for the 256 sample delay from the filter bank. For
/* stereo, the channels are also demultiplexed into #buffer[0][]# and
/* #buffer[1][]#
/*
/************************************************************************/
unsigned long
get_audio_ml(
	FILE *musicin_ml,
	double (*buffer)[1152],
	long unsigned int num_samples,
	IFF_AIFF *aiff_ptr,
	frame_params *fr_ps,
	int *aiff,
	int *byte_per_sample,
	double (*buffer_matr)[1152]
) {
int  j, ch;
long insamp[8064];
unsigned long samples_read;
int  n_ml_ch = fr_ps->header->multiling_ch;
	
	samples_read = read_samples_ml(musicin_ml, insamp, num_samples, (unsigned long) 1152*n_ml_ch, byte_per_sample, aiff);
	for(j=0; j<1152; j++) 
	  for (ch=0;ch<n_ml_ch;ch++)
	    buffer_matr[7+ch][j] = buffer[7+ch][j] = insamp[(n_ml_ch*j)+ch]; /*WtK 7/8/95 */

return(samples_read);
}

/************************************************************************
/*
/* I_encode_scale  (Layer I)
/* II_encode_scale (Layer II)
/*
/* PURPOSE:The encoded scalar factor information is arranged and
/* queued into the output fifo to be transmitted.
/*
/* For Layer II, the three scale factors associated with
/* a given subband and channel are transmitted in accordance
/* with the scfsi, which is transmitted first.
/*
/************************************************************************/
 
 
void II_sample_encoding_ml(unsigned int (*sbband)[3][12][32], unsigned int (*bit_alloc)[32], frame_params *fr_ps, Bit_stream_struc *bs)
{
   unsigned int temp;
   unsigned int i,j,k,s,x,y;
   int n_ml_ch       = fr_ps->header->multiling_ch;
   int lsf           = fr_ps->header->multiling_fs;
   int sblimit_ml     = fr_ps->sblimit_ml;
   al_table *alloc_ml = fr_ps->alloc_ml;
   
for (s=0;s<3;s++)
  for (j=0;j<((lsf==1)?6:12);j+=3)
    for (i=0;i<sblimit_ml;i++)
      for (k = 7; k < 7+n_ml_ch; k++)
	if (bit_alloc[k][i]) 
	{
		if ((*alloc_ml)[i][bit_alloc[k][i]].group == 3) 
		{
			for (x = 0; x < 3; x++)
			putbits(bs,sbband[k][s][j+x][i],
                                    (*alloc_ml)[i][bit_alloc[k][i]].bits);
		}
		else 
		{
			y =(*alloc_ml)[i][bit_alloc[k][i]].steps;
			temp = 	sbband[k][s][j][i] +
				sbband[k][s][j+1][i] * y +
				sbband[k][s][j+2][i] * y * y;
			putbits(bs,temp,(*alloc_ml)[i][bit_alloc[k][i]].bits);
		}
	}
}

void II_encode_bit_alloc_ml(unsigned int (*bit_alloc)[32], frame_params *fr_ps, Bit_stream_struc *bs)
{
   int i,k;
   int n_ml_ch       = fr_ps->header->multiling_ch;
   int sblimit_ml     = fr_ps->sblimit_ml;
   al_table *alloc_ml = fr_ps->alloc_ml;
   
  for (i=0;i<sblimit_ml;i++)
  {
	for(k = 7; k < 7+n_ml_ch; ++k)
	{        
		putbits(bs, bit_alloc[k][i], (*alloc_ml)[i][0].bits);
	}
  }	
}


void ml_sb_sample_swap (int ch0, int ch1, double subsample[14][3][12][SBLIMIT])
/* Function is called if MultiLingual LSF applies.                   */
/* It organizes subband samples from 3 sub frames of 12 samples each */
/* into 6 sub frames of 6 samples each. Subframes 3, 4 and 5 are at  */
/* sample indices 6..11 in subframes 0,1,2 respectively.             */
/* WtK 7/8/95                                                        */
{
int    ch,sb,ss;
double hlp[6];

for (ch=ch0;ch<ch1;ch++)
  for (sb=0;sb<SBLIMIT;sb++)
    for (ss=0;ss<6;ss++) {
      hlp[ss]                    = subsample[ch][2][  ss][sb];
      subsample[ch][2][  ss][sb] = subsample[ch][1][  ss][sb];
      subsample[ch][1][  ss][sb] = subsample[ch][0][6+ss][sb];
      subsample[ch][0][6+ss][sb] = subsample[ch][1][6+ss][sb];
      subsample[ch][1][6+ss][sb] = hlp[ss];
    }
}

void ml_sb_sample_shift (int ch0, int ch1,double subsample[14][3][12][SBLIMIT])
/* In case of MultiLingual LSF this function is called.             */
/* It shifts the second part in the sub frames into the first part. */
/* The first part is shifted into the second part to be used by     */
/* pick_scale_ml_2()                                                */
/* WtK 7/8/95                                                       */
{
int    ch,sb,p,ss;
double hlp[6];

for (ch=ch0;ch<ch1;ch++)
  for (sb=0;sb<SBLIMIT;sb++)
    for (p=0;p<2;p++)
      for (ss=0;ss<6;ss++) {
        hlp[ss]                    = subsample[ch][p][  ss][sb];
	subsample[ch][p][ss][sb]   = subsample[ch][p][ss+6][sb];
	subsample[ch][p][ss+6][sb] = hlp[ss];
      }
}

void pick_scale_ml_2(frame_params *fr_ps, double subsample[14][3][12][SBLIMIT], double (*max_sc)[32])
/* pick largest max_sc of odd and even half of frame in case of LSF ML. */
/* This improves the psychoacoustic result:                             */
/* The masked threshold is calculated over 2 LSF frames; consequently,  */
/* the signal level should also be determined over those 2 frames in    */
/* order to obtain a fair estimate of the SMR.                          */
/* WtK , 7/8/95                                                         */
{
  int k,i,p,j;
  int maxi;
  double maxs,mods;
  int n_ml_ch   = fr_ps->header->multiling_ch;
  int sblimit_ml = fr_ps->sblimit_ml;
  int ml_fs     = fr_ps->header->multiling_fs;

 if ( (n_ml_ch>0) && (ml_fs==1) ) {
    for (k=7; k<7+n_ml_ch; k++) {
       for (i=0;i<sblimit_ml;i++) {
	   maxs = subsample[k][0][0][i]; if (maxs<0) maxs = -maxs;
	   for (p=0;p<3;p++) for (j=6;j<12;j++) {
              mods = subsample[k][p][j][i]; if (mods<0) mods = -mods;
	      if (mods>maxs) maxs = mods;
	   }
           for (j=SCALE_RANGE-1,maxi=0;j>=0;j--)
	    if (maxs < multiple[j]) {
	      maxi = j;
	      break;
	   }
	   if (multiple[maxi]>max_sc[k][i]) max_sc[k][i] = multiple[maxi];
       }
    }
 }
}
