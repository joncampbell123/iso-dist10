/**********************************************************************
 * ISO MPEG Audio Subgroup Software Simulation Group (1996)
 * ISO 13818-3 MPEG-2 Audio Multichannel Encoder
 *
 * $Id: encode.c 1.9 1996/02/12 07:13:35 rowlands Exp $
 *
 * $Log: encode.c $
 * Revision 1.9  1996/02/12 07:13:35  rowlands
 * Release following Munich meeting
 *
 * Revision 1.6.2.1  1995/11/06  04:19:12  rowlands
 * Received from Uwe Felderhoff (IRT)
 *
 * Revision 1.8  1995/08/14  07:53:28  tenkate
 * ML-LSF added Warner ten Kate 7/8/95 (Philips)
 * variables "alloc" and "sblimit" changed into "alloc_ml" and
 * "sblimit_ml" where appropriate.
 * sample loops adapted to 12 or 6 depending on full or half fs.
 * II_a_bit_allocation separate summing for ML.
 *
 * Revision 1.7  1995/07/31  07:48:55  tenkate
 * addition of phantom coding, in void matricing(), 25/07/95 WtK
 *
 * Revision 1.4.3.1  1995/06/16  08:25:11  rowlands
 * Input from Sang Wook Kim (Samsung AIT)
 *
 * Revision 1.4.2.1  1995/06/16  03:46:42  rowlands
 * Input from Susanne Ritscher (IRT)
 *
 * Revision 1.4.1.6  1995/06/16  02:41:51  rowlands
 * Added dematrix procedure 2, corrected dematrixing values.
 *
 * Added support for dematrix procedure 2, without predistortion.
 * To implement predistortion will require a delay in the encoder
 * to be able to calculate the predistorted filtered surround signal
 * subband samples.
 *
 * Modified procedure matricing() to reuse the compatible stereo
 * channel signals computed by matricing_fft(). This was needed to
 * avoid calculating the low-pass-filtered surround signal in the
 * subband domain. The main function of procedure matricing() now
 * is to weight the multichannels. These channels are unweighted
 * prior to this for the benefit of the psycho model.
 *
 * Corrected weighting values for dematrixing procedures.
 * Deleted matrixing on multilingual channels.
 * In the quantization procedures, made quantizing to zero bits
 * a special case to skip some operations on undefined values.
 *
 **********************************************************************/

/**********************************************************************
 *   date   programmers         comment                               *
 * 3/01/91  Douglas Wong,       start of version 1.1 records          *
 *          Davis Pan                                                 *
 * 3/06/91  Douglas Wong        rename: setup.h to endef.h            *
 *                                      efilter to enfilter           *
 *                                      ewindow to enwindow           *
 *                              integrated "quantizer", "scalefactor",*
 *                              and "transmission" files              *
 *                              update routine "window_subband"       *
 * 3/31/91  Bill Aspromonte     replaced read_filter by               *
 *                              create_an_filter                      *
 * 5/10/91  W. Joseph Carter    Ported to Macintosh and Unix.         *
 *                              Incorporated Jean-Georges Fritsch's   *
 *                              "bitstream.c" package.                *
 *                              Incorporated Bill Aspromonte's        *
 *                              filterbank coefficient matrix         *
 *                              calculation routines and added        *
 *                              roundoff to coincide with specs.      *
 *                              Modified to strictly adhere to        *
 *                              encoded bitstream specs, including    *
 *                              "Berlin changes".                     *
 *                              Modified PCM sound file handling to   *
 *                              process all incoming samples and fill *
 *                              out last encoded frame with zeros     *
 *                              (silence) if needed.                  *
 *                              Located and fixed numerous software   *
 *                              bugs and table data errors.           *
 * 19jun91  dpwe (Aware)        moved "alloc_*" reader to common.c    *
 *                              Globals sblimit, alloc replaced by new*
 *                              struct 'frame_params' passed as arg.  *
 *                              Added JOINT STEREO coding, layers I,II*
 *                              Affects: *_bit_allocation,            *
 *                              subband_quantization, encode_bit_alloc*
 *                              sample_encoding                       *
 * 6/10/91  Earle Jennings      modified II_subband_quantization to   *
 *                              resolve type cast problem for MS_DOS  *
 * 6/11/91  Earle Jennings      modified to avoid overflow on MS_DOS  *
 *                              in routine filter_subband             *
 * 7/10/91  Earle Jennings      port to MsDos from MacIntosh version  *
 * 8/ 8/91  Jens Spille         Change for MS-C6.00                   *
 *10/ 1/91  S.I. Sudharsanan,   Ported to IBM AIX platform.           *
 *          Don H. Lee,                                               *
 *          Peter W. Farrett                                          *
 *10/ 3/91  Don H. Lee          implemented CRC-16 error protection   *
 *                              newly introduced function encode_CRC  *
 *11/ 8/91  Kathy Wang          Documentation of code                 *
 *                              All variablenames are referred to     *
 *                              with surrounding pound (#) signs      *
 * 2/11/92  W. Joseph Carter    Ported new code to Macintosh.  Most   *
 *                              important fixes involved changing     *
 *                              16-bit ints to long or unsigned in    *
 *                              bit alloc routines for quant of 65535 *
 *                              and passing proper function args.     *
 *                              Removed "Other Joint Stereo" option   *
 *                              and made bitrate be total channel     *
 *                              bitrate, irrespective of the mode.    *
 *                              Fixed many small bugs & reorganized.  *
 * 92-08-11 Soren H. Nielsen    Fixed bug: allocation of space in the *
 *                              bitstream for the CRC-word. Fixed     *
 *                              reading of window from file.          *
 * 92-11-06 Soren H. Nielsen	Fixed scalefactor calculation.        *
 **********************************************************************
 *                                                                    *
 *                                                                    *
 *  MPEG/audio Phase 2 coding/decoding multichannel                   *
 *                                                                    *
 *  7/27/93        Susanne Ritscher,  IRT Munich                      *
 *  8/10/93        changed matricing to 7 channels                    *
 *	           added void matricing_fft                           *
 *  8/12/93        added int required_bits,                           *
 *                       int max_alloc                                *
 *                 implemented the new mc_header (third working draft)*
 *  8/13/93        added channel-switching in required_bits and       *
 *                 II_subband_quantisation, II_encode_scale,          *
 *                 II_encode_bit_alloc, II_encode_sample,             *
 *                 encode_info                                        *
 *                 all channels normalized                            *
 *  9/20/93        channel-switching is only performed at a           *
 *                 certain limit of TC_ALLOC dB, which is included    *
 *                 in encoder.h                                       *
 *  1/04/94        get out some rubbish                               *
 *                                                                    *
 * 01/05/94        implemented the Committee Draft header             *
 *                                                                    *
 * 01/12/94        changed matricing procedure according to           *
 *                 Committee Draft                                    *
 *                                                                    *
 *  Version 1.0                                                       *
 *                                                                    *
 *  07/12/94       Susanne Ritscher,  IRT Munich                      *
 *                 Tel: +49 89 32399 458                              *
 *                 Fax: +49 89 32399 415                              *
 *                                                                    *
 *  Version 1.1                                                       *
 *                                                                    *
 *  02/23/95	   Susanne Ritscher,  IRT Munich                      *
 *                 corrected some bugs                                *
 *                 extension bitstream is working                     *
 *                                                                    *
 *  Version 2.0                                                       *
 *                                                                    *
 *  01/28/97       Frans de Bont, Philips Sound & Vision, Eindhoven   *
 *		    - dynamic crosstalk working for all configurations*
 *		    - prediction working for all configurations	      *
 *		    - extension bitstream fixed			      *
 *		    - fully compliant to DIS 13818-3.2                *
 *                                                                    *				      *		    
 **********************************************************************/
/**********************************************************************
 *                                                                    *
 *  06/06/95       Yeon Bae Thomas Kim, Samsung AIT                   *
 *                 ancillary data is working                          *
 *                                                                    *
 *  06/06/95       Sang Wook Kim, Samsung AIT                         *
 *                 corrected some bugs                                *
 *                                                                    *
 **********************************************************************/
#define VERY_FAST_FILTER  1	/* JMZ 08/03/1995 FILTER */

#include "common.h"
#include "encoder.h"
 
/*=======================================================================\
|                                                                       |
| This segment contains all the core routines of the encoder,           |
| except for the psychoacoustic models.                                 |
|                                                                       |
| The user can select either one of the two psychoacoustic              |
| models. Model I is a simple tonal and noise masking threshold         |
| generator, and Model II is a more sophisticated cochlear masking      |
| threshold generator. Model I is recommended for lower complexity      |
| applications whereas Model II gives better subjective quality at low  |
| bit rates.                                                            |
|                                                                       |
| Layers I and II of mono, stereo, and joint stereo modes are supported.|
| Routines associated with a given layer are prefixed by "I_" for layer |
| 1 and "II_" for layer 2.                                              |
\=======================================================================*/
 
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

unsigned long read_samples(FILE *musicin, long int *sample_buffer, long unsigned int num_samples, long unsigned int frame_size, int
 *byte_per_sample, int *aiff)
{
unsigned long samples_read;
static unsigned long samples_to_read;
static char init = TRUE;
short pcm_sample_buffer[9216];        /*for correct reading of pcm-data*/
int i;

   if (init) {
        samples_to_read = num_samples;
        init = FALSE;
        fseek(musicin, 12+18, SEEK_SET);
   }
   if (samples_to_read >= frame_size)
        samples_read = frame_size;
   else
		  samples_read = samples_to_read;

	if((*aiff==1) &&(*byte_per_sample !=2)){
	 
            if ((samples_read =
		  fread(sample_buffer, *byte_per_sample, (int)samples_read, musicin)) == 0)
		  if (verbosity >= 2) printf("Hit end of audio data\n");
	   
	}
	else{
	 if ((samples_read =
		  fread(pcm_sample_buffer, sizeof(short), (int)samples_read, musicin)) == 0)
		  if (verbosity >= 2) printf("Hit end of audio data\n");
            for(i = 0; i < 9216; ++i) sample_buffer[i] = pcm_sample_buffer[i];
        }

   samples_to_read -= samples_read;
   if (samples_read < frame_size && samples_read > 0) {
        if (verbosity >= 2) printf("Insufficient PCM input for one frame - fillout with zeros\n");
        for (; samples_read < frame_size; sample_buffer[samples_read++] = 0);
        samples_to_read = 0;
   }
   return(samples_read);
}

/************************************************************************/
/*
/* get_audio()
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
/* 21/03/1995 JMZ Multimode adaptations
/************************************************************************/
 
unsigned long
get_audio (
	FILE *musicin,
	double (*buffer)[1152],
	long unsigned int num_samples,
	int stereo,
	IFF_AIFF *aiff_ptr,
	int stereomc,
	int stereoaug,
	frame_params *fr_ps,
	int *aiff,
	int *byte_per_sample,
	double (*buffer_matr)[1152]
)
{
    int k, j, i;
    long insamp[9216];
    unsigned long samples_read;
    int lay;
    int lfe;
    layer *info = fr_ps->header;
    
    lay = info->lay;
    lfe = info->lfe;

    if (lay == 1)
    {
	if (stereo == 2)
	{ /* layer 1, stereo */
	    samples_read = read_samples (musicin, insamp, num_samples,
				         (unsigned long) 768, byte_per_sample, aiff);
	    for (j = 0; j < 448; j++) 
	    {
		if (j<64) 
		{
		    buffer[0][j] = buffer[0][j+384];
		    buffer[1][j] = buffer[1][j+384];
		    buffer[2][j] = 0;
		    buffer[3][j] = 0;
		    buffer[4][j] = 0;
		}
		else 
		{
		    buffer[0][j] = insamp[2*j-128];
		    buffer[1][j] = insamp[2*j-127];
		    buffer[2][j] = 0;
		    buffer[3][j] = 0;
		    buffer[4][j] = 0;
		}
	    }
	}
	else 
	{ /* layer 1, mono */
	    samples_read = read_samples (musicin, insamp, num_samples,
				         (unsigned long) 384, byte_per_sample, aiff);
	    for (j = 0; j < 448; j++)
	    {
		if (j < 64) 
		{
		    buffer[0][j] = buffer[0][j+384];
		    buffer[1][j] = 0;
		    buffer[2][j] = 0;
		    buffer[3][j] = 0;
		    buffer[4][j] = 0;
		}
		else 
		{
		    buffer[0][j] = insamp[j-64];
		    buffer[1][j] = 0;
		    buffer[2][j] = 0;
		    buffer[3][j] = 0;
		    buffer[4][j] = 0;
		}
	    }
	}
    }
    else 
    {
	if (*aiff == 1)
	{
	    k = aiff_ptr->numChannels;
	    samples_read = read_samples (musicin, insamp, num_samples,
					     (unsigned long) (k * 1152), byte_per_sample, aiff);

	    for (i = 0; i < k; i++)
		for (j = 0; j < 1152; j++)
		    buffer[i][j] = insamp[k*j+i];
	}
	else 
	{  /* layerII, stereo */
	    if (stereo == 2)
	    {
		samples_read = read_samples (musicin, insamp, num_samples,
			(unsigned long) ((2+lfe)*1152), byte_per_sample, aiff);
		for (j = 0; j < 1152; j++)
		{ /* fixed bug 28.6.93 S.R. */
		    buffer[0][j] = insamp[(2+lfe)*j];
		    buffer[1][j] = insamp[(2+lfe)*j+1];
		    buffer[2][j] = 0;
		    buffer[3+lfe][j] = 0;
		    buffer[4+lfe][j] = 0;
		    if (lfe)
		        buffer[3][j] = insamp[(2+lfe)*j+2]; /* ########### */
		}
	    }
	    else
	    { /* layer 2 (or 3), mono */
		samples_read = read_samples (musicin, insamp, num_samples,
			(unsigned long) 1152, byte_per_sample, aiff);
		for (j = 0; j < 1152; j++)
		{
		    buffer[0][j] = insamp[j];
		    buffer[1][j] = 0;
		    buffer[2][j] = 0;
		    buffer[3][j] = 0;
		    buffer[4][j] = 0;
		}        
	    }
	}
    }

    /*
     * If LFE is not enabled, "buffer" contains:
     *	buffer[0]	L
     *	buffer[1]	R
     *	buffer[2]	C
     *	buffer[3]	Ls
     *	buffer[4]	Rs
#ifdef Augmentation_7ch
     *  optional in 7.1 channel augmentation mode
     *	buffer[5]	Lc
     *	buffer[6]	Rc
#endif
     *
     * If LFE is enabled, "buffer" contains:
     *	buffer[0]	L
     *	buffer[1]	R
     *	buffer[2]	C
     *	buffer[3]	LFE
     *	buffer[4]	Ls
     *	buffer[5]	Rs
#ifdef Augmentation_7ch
     *  optional in 7.1 channel augmentation mode
     *	buffer[6]	Lc
     *	buffer[7]	Rc
#endif
     */

#ifdef Augmentation_7ch
    if (stereoaug == 2)
    	matricing_aug_fft (buffer, buffer_matr, fr_ps);
    else
#endif
	matricing_fft (buffer, buffer_matr, fr_ps);

    /*
     * After matrixing, "buffer_matr" contains:
     *	buffer_matr[0]	Lo
     *	buffer_matr[1]	Ro
     *	buffer_matr[2]	C
     *	buffer_matr[3]	Ls
     *	buffer_matr[4]	Rs
     *	buffer_matr[5]	L
     *	buffer_matr[6]	R
#ifdef Augmentation_7ch
     *  optional in 7.1 channel augmentation mode
     *	buffer_matr[7]	L7
     *	buffer_matr[8]	R7
     *	buffer_matr[9]	C7
     *	buffer_matr[10]	Lc
     *	buffer_matr[11]	Rc
#endif
     */

    return (samples_read);
}

/*************************************************************************
/*
/* matricing()
/*
/* The five-channel signal must be matricied to guarantee
/* the compatibility to the stereo-decoder.
/* There must be something of the surround information in the
/* two front channels. In a five-channel decoder there will
/* be a dematricing.
/* There must be 7 channels for channel switching 8/10/93, SR
/* 
/* Channel 5 and 6 are the not matriced signals L and R
/*
/* If phantom coding is used the high-frequency part of the
/* center signal is matrixed to left and right.
/* 27/07/95 WtK
/*                                                                         */
/***************************************************************************/

void normalizing (double (*sb_sample)[3][12][32], frame_params *fr_ps)
{
    double matrNorm;	/* factor for normalizing JMZ */
    double matrC;	/* weighting factor for C, Phantom C */
    double matrLsRs;	/* weighting factor for Ls, Rs */
    int stereo = fr_ps->stereo;
    int i, j, k, l;
    
    layer *info = fr_ps->header;
  
    switch (info->matrix)
    {
    /* Changed the factors according to International Standard */
      
    case 0:
    case 2: matrNorm	= 1 / (1 + sqrt(2.0));
	    matrC	= 1 / sqrt(2.0);
	    matrLsRs	= 1 / sqrt(2.0);
	    break;
    case 1: matrNorm	= 1 / (1.5 + 0.5*sqrt(2.0));
	    matrC	= 1 / sqrt(2.0);
	    matrLsRs	= 0.5;
	    break;
    case 3: matrNorm	= 1;
	    matrC	= 1;
	    matrLsRs	= 1;
	    break;
    }


    for (i = 0; i < stereo; i++)
    	for (j = 0; j < 3; j++)
	    for (l = 0; l < 12; l++)
	    	for (k = 0; k < SBLIMIT; k ++)
		    sb_sample[i][j][l][k] *= matrNorm;

    for (j = 0; j < 3; ++j)
	for (l = 0; l < 12; l ++)
	    for (k = 0; k < SBLIMIT; k ++)
		if (fr_ps->config == 320)
		{
		    sb_sample[2][j][l][k] = sb_sample[2][j][l][k] * matrNorm * matrC;
		    sb_sample[3][j][l][k] = sb_sample[3][j][l][k] * matrNorm * matrLsRs;
		    sb_sample[4][j][l][k] = sb_sample[4][j][l][k] * matrNorm * matrLsRs;
		}
		else if (fr_ps->config == 310)
		{
		    sb_sample[2][j][l][k] = sb_sample[2][j][l][k] * matrNorm * matrC;
		    sb_sample[3][j][l][k] = sb_sample[3][j][l][k] * matrNorm * matrLsRs;
		}
		else if (fr_ps->config == 220)
		{
		    sb_sample[2][j][l][k] = sb_sample[2][j][l][k] * matrNorm * matrLsRs;
		    sb_sample[3][j][l][k] = sb_sample[3][j][l][k] * matrNorm * matrLsRs;
		}
		else if (fr_ps->config == 300 || fr_ps->config == 302)
		    sb_sample[2][j][l][k] = sb_sample[2][j][l][k] * matrNorm * matrC;
		else if (fr_ps->config == 210)
		    sb_sample[2][j][l][k] = sb_sample[2][j][l][k] * matrNorm * matrLsRs;
}

void matricing (double (*sb_sample)[3][12][32], frame_params *fr_ps)
{
    double matrPC;	/* weighting factor for Phantom C */
    double mono_surround;
    register double val;
    int i, j, k, l;
    
    layer *info = fr_ps->header;
  
    switch (info->matrix)
    {
    /* Changed the factors according to International Standard */      
    case 0:
    case 1:
    case 2: matrPC	= 1;
	    break;
    case 3: matrPC	= 1 / sqrt(2.0);
	    break;
    }


    for (j = 0; j < 3; ++j)
    {
	for (l = 0; l < 12; l ++)
	{
	    for (k = 0; k < SBLIMIT; k ++)
	    {
		sb_sample[5][j][l][k] = sb_sample[0][j][l][k];
		sb_sample[6][j][l][k] = sb_sample[1][j][l][k];

		if (fr_ps->config == 320)
		{
		    /* 960819 FdB changed matricing */
		    if (info->matrix == 0 || info->matrix == 1)
		    {
			    sb_sample[0][j][l][k] = sb_sample[5][j][l][k] + sb_sample[2][j][l][k] + sb_sample[3][j][l][k];
			    sb_sample[1][j][l][k] = sb_sample[6][j][l][k] + sb_sample[2][j][l][k] + sb_sample[4][j][l][k];
		    }
		    else if (info->matrix == 2)
		    {
			    mono_surround = (sb_sample[3][j][l][k] + sb_sample[4][j][l][k]) / 2.0;
			    sb_sample[0][j][l][k] = sb_sample[5][j][l][k] + sb_sample[2][j][l][k] - mono_surround;
			    sb_sample[1][j][l][k] = sb_sample[6][j][l][k] + sb_sample[2][j][l][k] + mono_surround;
		    }
		    else
		    {
			    sb_sample[0][j][l][k] = sb_sample[5][j][l][k];
			    sb_sample[1][j][l][k] = sb_sample[6][j][l][k];
		    }
		}
		else if (fr_ps->config == 310)
		{
		    /* 960819 FdB changed matricing */
		    if (info->matrix == 0 || info->matrix == 1)
		    {
			    sb_sample[0][j][l][k] = sb_sample[5][j][l][k] + sb_sample[2][j][l][k] + sb_sample[3][j][l][k];
			    sb_sample[1][j][l][k] = sb_sample[6][j][l][k] + sb_sample[2][j][l][k] + sb_sample[3][j][l][k];
		    }
		    else if (info->matrix == 2)
		    {
			    sb_sample[0][j][l][k] = sb_sample[5][j][l][k] + sb_sample[2][j][l][k] - sb_sample[3][j][l][k];
			    sb_sample[1][j][l][k] = sb_sample[6][j][l][k] + sb_sample[2][j][l][k] + sb_sample[3][j][l][k];
		    }
		    else
		    {
			    sb_sample[0][j][l][k] = sb_sample[5][j][l][k];
			    sb_sample[1][j][l][k] = sb_sample[6][j][l][k];
		    }
		}
		else if (fr_ps->config == 220)
		{
		    /* 960819 FdB changed matricing */
		    if (info->matrix == 0 || info->matrix == 1)
		    {
			    sb_sample[0][j][l][k] = sb_sample[5][j][l][k] + sb_sample[2][j][l][k];
			    sb_sample[1][j][l][k] = sb_sample[6][j][l][k] + sb_sample[3][j][l][k];
		    }
		    else if (info->matrix == 3)
		    {
			    sb_sample[0][j][l][k] = sb_sample[5][j][l][k];
			    sb_sample[1][j][l][k] = sb_sample[6][j][l][k];
		    }
		}
		else if (fr_ps->config == 300 || fr_ps->config == 302)
		{
		    /* 960819 FdB changed matricing */
		    if (info->matrix == 0 || info->matrix == 1)
		    {
			    sb_sample[0][j][l][k] = sb_sample[5][j][l][k] + sb_sample[2][j][l][k];
			    sb_sample[1][j][l][k] = sb_sample[6][j][l][k] + sb_sample[2][j][l][k];
		    }
		    else if (info->matrix == 3)
		    {
			    sb_sample[0][j][l][k] = sb_sample[5][j][l][k];
			    sb_sample[1][j][l][k] = sb_sample[6][j][l][k];
		    }
		}
		else if (fr_ps->config == 210)
		{
		    /* 960819 FdB changed matricing */
		    if (info->matrix == 0 || info->matrix == 1)
		    {
			    sb_sample[0][j][l][k] = sb_sample[5][j][l][k] + sb_sample[2][j][l][k];
			    sb_sample[1][j][l][k] = sb_sample[6][j][l][k] + sb_sample[2][j][l][k];
		    }
		    else if (info->matrix == 3)
		    {
			    sb_sample[0][j][l][k] = sb_sample[5][j][l][k];
			    sb_sample[1][j][l][k] = sb_sample[6][j][l][k];
		    }
		}
	    }

	    if (info->center == 3)            /* 27/07/95 WtK */
	    {
		if (info->matrix==3)
		    for (k = 12; k < SBLIMIT; k ++)
		    {
			val = matrPC * sb_sample[2][j][l][k];
			sb_sample[0][j][l][k] += val;
			sb_sample[1][j][l][k] += val;
			sb_sample[2][j][l][k] = 0;
		    }
		else 
		    for (k = 12; k < SBLIMIT; k ++)
		    {
			val = matrPC * sb_sample[2][j][l][k];
			sb_sample[5][j][l][k] += val;
			sb_sample[6][j][l][k] += val;
			sb_sample[2][j][l][k] = 0;
		    }
	    }
	}
    }
}

#ifdef Augmentation_7ch
void normalizing_aug (double (*sb_sample)[3][12][32], frame_params *fr_ps)
{
    double norm;	/* factor for normalizing JMZ */
    double c[7];
    int i, j, k, l;
    
    layer *info = fr_ps->header;
  
    for (i = 0; i < 7; i++)
	c[i] = 1.0;

    switch (info->matrix)
    {
    	/* factors according to International Standard */
    	case 0:
    	case 2: c[2] = c[3] = c[4] = 1.0 / sqrt (2.0);	/* weigh factor for C, Ls and Rs */
	    	break;
    	case 1: c[2] = 1.0 / sqrt (2.0);		/* weigh factor for C            */
		c[3] = c[4] = 0.5;			/* weigh factor for Ls, Rs       */
		break;
    }

    if (info->aug_mtx_proc == 0)
    	/* factors according to 7-ch augmentation */
	/* unweigh factor for LC, RC */
	c[5] = c[6] = 0.75;

    /* normalization factor */
    switch (info->matrix * 10 + info->aug_mtx_proc)
    {
    case 00:
    case 20: norm = 1.0 / (1.75 + 1.25 * sqrt (2.0));
	     break;
    case 10: norm = 1.0 / (2.25 + 0.75 * sqrt (2.0));
	     break;
    case 30: norm = 1.0 / 1.75;
	     break;
    case 01:
    case 21: norm = 1.0 / (2.0 + sqrt (2.0));
	     break;
    case 11: norm = 1.0 / (2.5 + 0.5 * sqrt (2.0));
	     break;
    case 31: norm = 0.5;
	     break;
    case 03:
    case 23: norm = 1.0 / (1.0 + sqrt (2.0));
	     break;
    case 13: norm = 1.0 / (1.5 + 0.5 * sqrt (2.0));
	     break;
    case 33: norm = 1.0;
	     break;
    }

    for (i = 0; i < 7; i++)
	c[i] *= norm;

    /* normalizing */
    for (i = 0; i < 7; i++)
	for (j = 0; j < 3; j++)
	    for (k = 0; k < 12; k++)
		for (l = 0; l < SBLIMIT; l++)
		    sb_sample[i][j][k][l] *= c[i];
}

void matricing_aug (double (*sb_sample)[3][12][32], frame_params *fr_ps)
{
    int i, j, k, l;
    
    layer *info = fr_ps->header;
  
    for (j = 0; j < 3; ++j)
	for (l = 0; l < 12; l ++)
	    for (k = 0; k < SBLIMIT; k ++)
	    {
		/* copy L7, R7, C7, Lc and Rc to sb_sample[7 .. 11] */
		sb_sample[11][j][l][k] = sb_sample[6][j][l][k];
		sb_sample[10][j][l][k] = sb_sample[5][j][l][k];
		sb_sample[ 9][j][l][k] = sb_sample[2][j][l][k];
		sb_sample[ 8][j][l][k] = sb_sample[1][j][l][k];
		sb_sample[ 7][j][l][k] = sb_sample[0][j][l][k];

		/* 960819 FdB changed matricing */
		if (info->aug_mtx_proc == 0 || info->aug_mtx_proc == 1)
		{
		    sb_sample[0][j][l][k] += sb_sample[5][j][l][k];
		    sb_sample[1][j][l][k] += sb_sample[6][j][l][k];
                }
		if (info->aug_mtx_proc == 0)
		    sb_sample[2][j][l][k] += (sb_sample[5][j][l][k] + sb_sample[6][j][l][k]) / 3;
	    }
}
#endif

/*************************************************************************
/*
/* matricing_fft()
/*
/* To get the best results in psychoacoustics there must be both,
/* the matriced and the not matriced signal. This matricing
/* may be in full bandwith.
 *
 * If LFE is not enabled, "buffer" contains:
 *	buffer[0]	L
 *	buffer[1]	R
 *	buffer[2]	C
 *	buffer[3]	Ls
 *	buffer[4]	Rs
 *
 * If LFE is enabled, "buffer" contains:
 *	buffer[0]	L
 *	buffer[1]	R
 *	buffer[2]	C
 *	buffer[3]	LFE
 *	buffer[4]	Ls
 *	buffer[5]	Rs
 *
 * This function matrixes the original audio samples to pass to the
 * psychoacoustic model. The model considers the matrixed and non-
 * matrixed versions of the signal, so both are retained here.
 * 
 * On exit, buffer_matr[0] to buffer_matr[6] contain the channels
 * Lo, Ro, C, Ls, Rs, L, R, respectively.
 *
 **************************************************************************/

void matricing_fft (double (*buffer)[1152],
		    double (*buffer_matr)[1152],
		    frame_params *fr_ps)
{
    double matrNorm;	/* factor for normalizing JMZ */
    double matrC;		/* weighting factor for C */
    double matrLsRs;	/* weighting factor for Ls, Rs */
    double mono_surround;
    int i, j, k, l;
    int lfe, lfe_pos;
    double dummy;
    
    layer *info = fr_ps->header;
    lfe = info->lfe;
    lfe_pos = fr_ps->lfe_pos;

    switch(info->matrix)
    {
    case 0:
    case 2: matrNorm	= 1 / (1 + sqrt(2.0));
	    matrC	= 1 / sqrt(2.0);
	    matrLsRs	= 1 / sqrt(2.0);
	    break;
    case 1: matrNorm	= 1 / (1.5 + 0.5*sqrt(2.0));
	    matrC	= 1 / sqrt(2.0);
	    matrLsRs	= 0.5;
	    break;
    case 3: matrNorm	= 1;
	    matrC	= 1;
	    matrLsRs	= 1;
	    break;
    }

    for (i = 0; i < 1152; i++)
    {
	if (lfe && lfe_pos < 3)
	    buffer_matr[2][i]	= buffer[3][i];
	else
	    buffer_matr[2][i]	= buffer[2][i];
	buffer_matr[3][i]	= buffer[3+lfe][i];
	buffer_matr[4][i]	= buffer[4+lfe][i];
	buffer_matr[5][i]	= buffer[0][i];
	buffer_matr[6][i]	= buffer[1][i];
   
	switch (info->matrix)
	{
	case 0:
	case 1: buffer_matr[0][i] = matrNorm*(buffer[0][i] + matrC*buffer_matr[2][i] + matrLsRs*buffer_matr[3][i]);
		buffer_matr[1][i] = matrNorm*(buffer[1][i] + matrC*buffer_matr[2][i] + matrLsRs*buffer_matr[4][i]);
		break;
	case 2: mono_surround = (buffer_matr[3][i] + buffer_matr[4][i]) / 2.0;
		buffer_matr[0][i] = matrNorm*(buffer[0][i] + matrC*buffer_matr[2][i] - matrLsRs * mono_surround);
		buffer_matr[1][i] = matrNorm*(buffer[1][i] + matrC*buffer_matr[2][i] + matrLsRs * mono_surround);
		break;
	case 3: buffer_matr[0][i] = buffer[0][i];
		buffer_matr[1][i] = buffer[1][i];
		break;
	}
   }
}

#ifdef Augmentation_7ch
/*************************************************************************
/*
/* matricing_aug_fft()
/*
/* To get the best results in psychoacoustics there must be both,
/* the matriced and the not matriced signal. This matricing
/* may be in full bandwith.
 *
 * If LFE is not enabled, "buffer" contains:
 *	buffer[0]	L
 *	buffer[1]	R
 *	buffer[2]	C
 *	buffer[3]	Ls
 *	buffer[4]	Rs
 *	buffer[5]	Lc
 *	buffer[6]	Rc
 *
 * If LFE is enabled, "buffer" contains:
 *	buffer[0]	L
 *	buffer[1]	R
 *	buffer[2]	C
 *	buffer[3]	LFE
 *	buffer[4]	Ls
 *	buffer[5]	Rs
 *	buffer[6]	Lc
 *	buffer[7]	Rc
 *
 * This function matrixes the original audio samples to pass to the
 * psychoacoustic model. The model considers the matrixed and non-
 * matrixed versions of the signal, so both are retained here.
 * 
 * On exit, buffer[0] to buffer[5] contain the channels
 * L5, R5, C5, (LFE, ) Ls, Rs, respectively.
 * On exit, buffer_matr[7] to buffer_matr[11] contain the channels
 * L7, R7, C7, Lc and Rc respectively.
 *
 **************************************************************************/

void matricing_aug_fft (double (*buffer)[1152],
			double (*buffer_matr)[1152],
			frame_params *fr_ps)
{
    double matrNorm;	/* factor for normalizing JMZ */
    double matrC;	/* weighting factor for C */
    double matrLR;	/* weighting factor for L, R */
    double matrLsRs;	/* weighting factor for Ls, Rs */
    int i, j, k, l;
    int lfe;
    double dummy;
    
    layer *info = fr_ps->header;
    lfe = info->lfe;
  
    switch (info->aug_mtx_proc)
    {
    case 0:
	  matrNorm	= 1 / 1.75;
	  matrC		= 0.25;
	  matrLR	= 0.75;
	  break;
    case 1:
	  matrNorm	= 0.5;
	  matrC		= 0;
	  matrLR	= 1;
	  break;
    case 3:
	  matrNorm	= 1;
	  matrC		= 0;
	  matrLR	= 0;
	  break;
    }

    for (i = 0; i < 1152; i++)
    {
	buffer_matr[3][i] = buffer[3+lfe][i];	/* Ls */
	buffer_matr[4][i] = buffer[4+lfe][i];	/* Rs */
   
	buffer_matr[7][i] = buffer[0][i];	/* L7 */
	buffer_matr[8][i] = buffer[1][i];	/* R7 */
	buffer_matr[9][i] = buffer[2][i];	/* C7 */
	buffer_matr[10][i] = buffer[5+lfe][i];	/* Lc */
	buffer_matr[11][i] = buffer[6+lfe][i];	/* Rc */

	/* calculate L5, R5, C5 */
	buffer_matr[0][i] = matrNorm * (buffer_matr[7][i] + matrLR * buffer_matr[10][i]);
	buffer_matr[1][i] = matrNorm * (buffer_matr[8][i] + matrLR * buffer_matr[11][i]);
	buffer_matr[2][i] = matrNorm * (buffer_matr[9][i] + matrC * (buffer_matr[10][i] + buffer_matr[11][i]));
    }

    switch (info->matrix)
    {
    case 0:
    case 2:
	  matrNorm	= 1 / (1 + sqrt(2.0));
	  matrC		= 1 / sqrt(2.0);
	  matrLsRs	= 1 / sqrt(2.0);
	  break;
    case 1:
	  matrNorm	= 1 / (1.5 + 0.5*sqrt(2.0));
	  matrC		= 1 / sqrt(2.0);
	  matrLsRs	= 0.5;
	  break;
    case 3:
	  matrNorm	= 1;
	  matrC		= 1;
	  matrLsRs	= 1;
	  break;
    }

    for (i = 0; i < 1152; i++)
    {
	buffer_matr[5][i] = buffer_matr[0][i];
	buffer_matr[6][i] = buffer_matr[1][i];
   
	switch (info->matrix)
	{
	case 0:
	case 1: buffer_matr[0][i] = matrNorm * (buffer_matr[0][i] +
						matrC * buffer_matr[2][i] +
						matrLsRs * buffer_matr[3][i]);
		buffer_matr[1][i] = matrNorm * (buffer_matr[1][i] +
						matrC * buffer_matr[2][i] +
						matrLsRs * buffer_matr[4][i]);
		break;
	case 2: buffer_matr[0][i] = matrNorm * (buffer_matr[0][i] +
						matrC * buffer_matr[2][i] -
						matrLsRs * 0.5 *
						 (buffer_matr[3][i] + buffer_matr[4][i]));
		buffer_matr[1][i] = matrNorm * (buffer_matr[1][i] +
						matrC * buffer_matr[2][i] +
						matrLsRs * 0.5 *
						 (buffer_matr[3][i] + buffer_matr[4][i]));
		break;
	}
    }
}
#endif


/************************************************************************/
/*
/* read_ana_window()
/*
/* PURPOSE:  Reads encoder window file "enwindow" into array #ana_win#
/*
/************************************************************************/
 
void read_ana_window(double *ana_win)
       /*far*/                   
{
    int i,j[4];
    FILE *fp;
    double f[4];
    char t[150];
 
    if (!(fp = OpenTableFile("enwindow") ) ) {
       printf("Please check analysis window table 'enwindow'\n");
       exit(1);
    }
    for (i=0;i<512;i+=4) {
       fgets(t, 80, fp); /* changed from 150, 92-08-11 shn */
       sscanf(t,"C[%d] = %lf C[%d] = %lf C[%d] = %lf C[%d] = %lf\n",
              j, f,j+1,f+1,j+2,f+2,j+3,f+3);
       if (i==j[0]) {
          ana_win[i] = f[0];
          ana_win[i+1] = f[1];
          ana_win[i+2] = f[2];
          ana_win[i+3] = f[3];
       }
       else {
          printf("Check index in analysis window table\n");
          exit(1);
       }
       fgets(t,80,fp); /* changed from 150, 92-08-11 shn */
    }
    fclose(fp);
}

/************************************************************************/
/*
/* window_subband()
/*
/* PURPOSE:  Overlapping window on PCM samples
/*
/* SEMANTICS:
/* 32 16-bit pcm samples are scaled to fractional 2's complement and
/* concatenated to the end of the window buffer #x#. The updated window
/* buffer #x# is then windowed by the analysis window #c# to produce the
/* windowed sample #z#
/*
/************************************************************************/
 
void window_subband (double **buffer, double *z, int k)
{
    typedef double XX[14][HAN_SIZE];	/* 08/03/1995 JMZ Multilingual */
    static XX *x;
    int i, j;
    static off[14]= {0,0,0,0,0,0,0,0,0,0,0,0,0,0};/* 08/03/1995 JMZ Multilingual */
    static char init = 0;
    static double *c;
 
    if (!init)
    {
	c = (double *) mem_alloc (sizeof(double) * HAN_SIZE, "window");
	read_ana_window (c);
	x = (XX *) mem_alloc (sizeof(XX), "x");
	for (i = 0; i < 14; i++)
	   for (j = 0; j < HAN_SIZE; j++)
		 (*x)[i][j] = 0;
	init = 1;
    }
 
    for (i = 0; i < 32; i++)
	(*x)[k][31-i+off[k]] = (double) *(*buffer)++ / SCALE;
    for (i = 0; i < HAN_SIZE; i++)
	z[i] = (*x)[k][(i+off[k])&HAN_SIZE-1] * c[i];
    off[k] += 480; /*offset is modulo (HAN_SIZE-1)*/
    off[k] &= HAN_SIZE-1;
}
 
/************************************************************************/
/*
/* create_ana_filter()
/*
/* PURPOSE:  Calculates the analysis filter bank coefficients
/*
/* SEMANTICS:
/* Calculates the analysis filterbank coefficients and rounds to the
/* 9th decimal place accuracy of the filterbank tables in the ISO
/* document.  The coefficients are stored in #filter#
/*
/************************************************************************/
 
void create_ana_filter(double (*filter)[64])
       /*far*/                     
{
   register int i,k;
 
   for (i = 0; i < 32; i++)
      for (k = 0; k < 64; k++) {
          if ((filter[i][k] = 1e9*cos((double)((2*i+1)*(16-k)*PI64))) >= 0)
             modf(filter[i][k]+0.5, &filter[i][k]);
          else
             modf(filter[i][k]-0.5, &filter[i][k]);
          filter[i][k] *= 1e-9;
   }
}

/************************************************************************/
/*
/* filter_subband()
/*
/* PURPOSE:  Calculates the analysis filter bank coefficients
/*
/* SEMANTICS:
/*      The windowed samples #z# is filtered by the digital filter matrix #m#
/* to produce the subband samples #s#. This done by first selectively
/* picking out values from the windowed samples, and then multiplying
/* them by the filter matrix, producing 32 subband samples.
/*
/************************************************************************/
 
void filter_subband_old(double *z, double *s)
       /*far*/                         
{
   double y[64];
   int i,j, k;
static char init = 0;
   typedef double MM[SBLIMIT][64];
static MM /*far*/ *m;
   double sum1, sum2;
   
#ifdef MS_DOS
   long    SIZE_OF_MM;
   SIZE_OF_MM      = SBLIMIT*64;
   SIZE_OF_MM      *= 8;
   if (!init) {
       m = (MM /*far*/ *) mem_alloc(SIZE_OF_MM, "filter");
       create_ana_filter(*m);
       init = 1;
   }
#else
   if (!init) {
       m = (MM /*far*/ *) mem_alloc(sizeof(MM), "filter");
       create_ana_filter(*m);
       init = 1;
   }
#endif
   /* Window */
   for (i=0; i<64; i++)
   {
      for (k=0, sum1 = 0.0; k<8; k++)
         sum1 += z[i+64*k];
      y[i] = sum1;
   }

   /* Filter */
   for (i=0;i<SBLIMIT;i++)
   {
       for (k=0, sum1=0.0 ;k<64;k++)
          sum1 += (*m)[i][k] * y[k];
       s[i] = sum1;
   }

/*   for (i=0;i<64;i++) for (j=0, y[i] = 0;j<8;j++) y[i] += z[i+64*j];*/
/*   for (i=0;i<SBLIMIT;i++)*/
/*       for (j=0, s[i]= 0;j<64;j++) s[i] += (*m)[i][j] * y[j];*/

}

/************************************************************************/
/* JMZ 08/03/1995 FILTER */

void filter_subband(double *z, double *s)
       /*far*/                         
{
   double y[64];
   int i,j, k;
static char init = 0;
   typedef double MM[SBLIMIT][64];
static MM /*far*/ *m;
   double sum1, sum2;
  
   if (!init) {
       m = (MM /*far*/ *) mem_alloc(sizeof(MM), "filter");
       create_ana_filter(*m);
       init = 1;
   }
   /* Window */
   for (i=0; i<64; i++)
   {
      for (k=0, sum1 = 0.0; k<8; k++)
         sum1 += z[i+64*k];
      y[i] = sum1;
   }

   /* Filter */
#if VERY_FAST_FILTER 
   for (i=0; i<SBLIMIT/2; i++)
   {
       for (k=0, sum1=0.0, sum2=0.0; k<16;)
       {
	sum1 += (*m)[i][k] * (y[k]+y[32-k]); 
	sum2 += (*m)[i][k+1] * (y[k+1]+y[31-k]); 
	sum2 += (*m)[i][k+33] * (y[k+33]-y[63-k]); 
	sum1 += (*m)[i][k+34] * (y[k+34]-y[62-k]); 
	k+=2;
       }
	sum1 += (*m)[i][16]*y[16] - (*m)[i][48]*y[48]; 

	s[i]    = sum1 + sum2;
	s[31-i] = sum1 - sum2;
   }
#else
   for (i=0;i<SBLIMIT;i++)
   {
       for (k=0, sum1=0.0 ;k<64;k++)
          sum1 += (*m)[i][k] * y[k];
       s[i] = sum1;
   }
#endif /*VERY_FAST_FILTER*/
}

/* JMZ 08/03/1995 FILTER */
/************************************************************************/

/************************************************************************/
/*
/* encode_info()
/* encode_infomc1() SR
/* encode_infomc2() SR
/*
/* PURPOSE:  Puts the syncword and header information on the output
/* bitstream.
/*
/************************************************************************/
 
void encode_info (frame_params *fr_ps, Bit_stream_struc *bs)
{
    layer *info = fr_ps->header;

    putbits (bs, 0xfff, 12);                   /* syncword 12 bits */
    put1bit (bs, info->version);               /* ID        1 bit  */
    putbits (bs, 4-info->lay, 2);              /* layer     2 bits */
    put1bit (bs, !info->error_protection);     /* bit set => no err prot */
    putbits (bs, info->bitrate_index, 4);
    putbits (bs, info->sampling_frequency, 2);
    put1bit (bs, info->padding);
    put1bit (bs, info->extension);             /* private_bit */
    putbits (bs, info->mode, 2);
    putbits (bs, info->mode_ext, 2);
    put1bit (bs, info->copyright);
    put1bit (bs, info->original);
    putbits (bs, info->emphasis, 2);
}

void encode_info_mc1 (frame_params *fr_ps, Bit_stream_struc *bs)
{
    layer *info = fr_ps->header;

    put1bit (bs, info->ext_bit_stream_present);
    if(info->ext_bit_stream_present == 1) 
      putbits (bs, info->n_ad_bytes, 8); 
    putbits (bs, info->center, 2); 
    putbits (bs, info->surround, 2);
    put1bit (bs, info->lfe);
    put1bit (bs, info->audio_mix);
    putbits (bs, info->matrix, 2);
    putbits (bs, info->multiling_ch, 3);
    put1bit (bs, info->multiling_fs);
    put1bit (bs, info->multiling_lay);
    put1bit (bs, info->copy_ident_bit);
    put1bit (bs, info->copy_ident_start);
}

void encode_info_mc2 (frame_params *fr_ps, Bit_stream_struc *bs)
{
    layer *info = fr_ps->header;
    int i, j;       

    put1bit (bs, info->tc_sbgr_select);
    put1bit (bs, info->dyn_cross_on);
    put1bit (bs, info->mc_prediction_on);

    /* 960627 FdB tca bits dependent on configuration */
    if (fr_ps->config == 320 || fr_ps->config == 310)
    {
	/* 3 bits for tca's */
	if (info->tc_sbgr_select == 1)
	    putbits (bs, info->tc_allocation, 3);
	else
	    for (i = 0; i < 12; i++)
		putbits (bs, info->tc_alloc[i], 3);
    }
    else if (fr_ps->config == 300 || fr_ps->config == 302 ||
	     fr_ps->config == 220 || fr_ps->config == 210)
    {
	/* 2 bits for tca's */
	if (info->tc_sbgr_select == 1)
	    putbits (bs, info->tc_allocation, 2);
	else
	    for (i = 0; i < 12; i++)
		putbits (bs, info->tc_alloc[i], 2);
    }

    if (info->dyn_cross_on == 1)
    {
	put1bit (bs, info->dyn_cross_LR);
	for (i = 0; i < 12; i++)
	{
	    /* 960627 FdB DynX bits dependent on configuration */
	    if (fr_ps->config == 320)
		/* 3/2 */
		putbits (bs, info->dyn_cross[i], 4);
	    else if (fr_ps->config == 310 || fr_ps->config == 220)
		/* 3/1 and 2/2 */
		putbits (bs, info->dyn_cross[i], 3);
	    else if (fr_ps->config == 300 || fr_ps->config == 302 || fr_ps->config == 210)
		/* 3/0 (+2/0) and 2/1 */
		putbits (bs, info->dyn_cross[i], 1);
  
	    if (info->surround == 3)
		put1bit (bs, info->dyn_second_stereo[i]);
	}	    
    }

    if (info->mc_prediction_on == 1)
    {
	for(i = 0; i < 8; i++)
	{
	    put1bit (bs, info->mc_pred[i]);  
	    if (info->mc_pred[i] == 1)
	    {
		for (j = 0; j < n_pred_coef[info->dyn_cross[i]]; j++)
		    putbits (bs, info->predsi[i][j], 2);
	    }
	}  
    }
}			  
    
#ifdef Augmentation_7ch
void encode_info_aug (frame_params *fr_ps, Bit_stream_struc *bs)
{
    int sbgr;
    layer *info = fr_ps->header;

    putbits (bs, info->aug_mtx_proc,  2);
    put1bit (bs, info->aug_dyn_cross_on);
    put1bit (bs, info->aug_future_ext);
    if (info->aug_mtx_proc == 0)
	for (sbgr = 0; sbgr < 12; sbgr++)
	    putbits (bs, info->aug_tc_alloc[sbgr], 3); 
    else if (info->aug_mtx_proc == 1)
	for (sbgr = 0; sbgr < 12; sbgr++)
	    putbits (bs, info->aug_tc_alloc[sbgr], 2); 
    if (info->aug_dyn_cross_on == 1)
	for (sbgr = 0; sbgr < 12; sbgr++)
	    putbits (bs, info->aug_dyn_cross[sbgr], 5); 
}
#endif

void encode_info_ext1 (frame_params *fr_ps, Bit_stream_struc *bs_ext)
{
    layer *info = fr_ps->header;

    info->ext_sync = 0x7ff;

    putbits (bs_ext, info->ext_sync, 12); 
}

void encode_info_ext2 (frame_params *fr_ps, Bit_stream_struc *bs_ext, unsigned int crc)
{
    layer *info = fr_ps->header;

    putbits (bs_ext, crc, 16);
    putbits (bs_ext, info->ext_length, 11);
    put1bit (bs_ext, info->ext_bit);
}


/************************************************************************/
/*
/* mod()
/*
/* PURPOSE:  Returns the absolute value of its argument
/*
/************************************************************************/
 
double mod(double a)
{
    return (a > 0) ? a : -a;
}
 
/************************************************************************/
/*
/* I_combine_LR    (Layer I)
/* II_combine_LR	 (Layer II)
/*
/* PURPOSE:Combines left and right channels into a mono channel
/*
/* SEMANTICS:  The average of left and right subband samples is put into
/* #joint_sample#
/*
/* Layer I and II differ in frame length and # subbands used
/*
/************************************************************************/
 
void I_combine_LR(double (*sb_sample)[3][12][32], double (*joint_sample)[3][12][32])
       /*far*/                                       
       /*far*/                                          
{   /* make a filtered mono for joint stereo */
    int sb, smp;
 
   for(sb = 0; sb<SBLIMIT; ++sb)
      for(smp = 0; smp<SCALE_BLOCK; ++smp)
		  joint_sample[0][0][smp][sb] = .5 *
                    (sb_sample[0][0][smp][sb] + sb_sample[1][0][smp][sb]);
}
 
void II_combine_LR(double (*sb_sample)[3][12][32], double (*joint_sample)[3][12][32], int sblimit)
       /*far*/                                       
       /*far*/                                          
            
{  /* make a filtered mono for joint stereo */
   int sb, smp, sufr;
 
   for(sb = 0; sb<sblimit; ++sb)
      for(smp = 0; smp<SCALE_BLOCK; ++smp)
         for(sufr = 0; sufr<3; ++sufr)
		joint_sample[0][sufr][smp][sb] = .5 * (sb_sample[0][sufr][smp][sb]
                                          + sb_sample[1][sufr][smp][sb]);
}


/************************************************************************
/*
/* I_scale_factor_calc     (Layer I)
/* II_scale_factor_calc    (Layer II)
/*
/* PURPOSE:For each subband, calculate the scale factor for each set
/* of the 12 (6 in case of lsf ML) subband samples
/*
/* SEMANTICS:  Pick the scalefactor #multiple[]# just larger than the
/* absolute value of the peak subband sample of 12 samples,
/* and store the corresponding scalefactor index in #scalar#.
/*
/* Layer II has three sets of 12 (6 in case of lsf ML) subband samples 
/* for a given subband.
/*
/************************************************************************/
 
void I_scale_factor_calc(double (*sb_sample)[3][12][32], unsigned int (*scalar)[3][32], int stereo)
       /*far*/                                       
                                   
           
{
   int i,j, k;
   double s[SBLIMIT];
 
   for (k=0;k<stereo;k++) {
     for (i=0;i<SBLIMIT;i++)
       for (j=1, s[i] = mod(sb_sample[k][0][0][i]);j<SCALE_BLOCK;j++)
         if (mod(sb_sample[k][0][j][i]) > s[i])
            s[i] = mod(sb_sample[k][0][j][i]);
 
     for (i=0;i<SBLIMIT;i++)
       for (j=SCALE_RANGE-1,scalar[k][0][i]=0;j>=0;j--)
         if (s[i] < multiple[j]) { /* <= changed to <, 1992-11-06 shn */
            scalar[k][0][i] = j;
            break;
         }
   }
}

/******************************** Layer II ******************************/
 
void II_scale_factor_calc (frame_params *fr_ps,
			   double (*sb_sample)[3][12][32],
			   unsigned int (*scalar)[3][32],
			   int sblimit, int l, int m)
/* sblimit has the value of sblimit_ml in case II_scale_factor_calc */
/* is called in a ML channel , 7/8/95 WtK                          */
{
    int    i,j, k,t;
    double s[SBLIMIT];
    int    leng;
   
    leng = SCALE_BLOCK; /* == 12 */
    if (l >= 7 && fr_ps->header->multiling_fs == 1)
	leng /= 2;
  
    for (k = l; k < m; k++)
	for (t = 0; t < 3; t++) 
	{
	    for (i = 0; i < sblimit; i++)
		for (j = 1, s[i] = mod (sb_sample[k][t][0][i]); j < leng; j++)
		    if (mod (sb_sample[k][t][j][i]) > s[i])
			s[i] = mod (sb_sample[k][t][j][i]);
   
	    for (i = 0; i < sblimit; i++)
		for (j = SCALE_RANGE - 1, scalar[k][t][i] = 0; j >= 0; j--)
		    if (s[i] < multiple[j]) 
		    {
			/* <= changed to <, 1992-11-06 shn */
			scalar[k][t][i] = j;
			break;
		    }
	    for (i = sblimit; i < SBLIMIT; i++)
		scalar[k][t][i] = SCALE_RANGE - 1;
	}
} 

/***************************************************************************
/* void II_scale_factor_calc1(sb_sample, scalar, stereo, sblimit)
/*
/* in case of any joint stereo the scalefactor must be computed
/* a second time for the combind samples
/*
/***************************************************************************/

void II_scale_factor_calc1(double (*sb_sample)[3][12][32], unsigned int (*scalar)[3][32], int sblimit, int dim)
       /*far*/                                       
                                   
            
        
{
  int i,j, k,t;
  double s[SBLIMIT];

	for (t=0;t<3;t++) {
	 for (i=0;i<sblimit;i++)
		for (j=1, s[i] = mod(sb_sample[dim][t][0][i]);j<SCALE_BLOCK;j++)
		  if (mod(sb_sample[dim][t][j][i]) > s[i])
				 s[i] = mod(sb_sample[dim][t][j][i]);

  for (i=0;i<sblimit;i++)
	 for (j=SCALE_RANGE-1,scalar[dim][t][i]=0;j>=0;j--)
		if (s[i] < multiple[j]) { /* <= changed to <, 1992-11-06 shn */
			scalar[dim][t][i] = j;
			break;
		}
		for (i=sblimit;i<SBLIMIT;i++) scalar[dim][t][i] = SCALE_RANGE-1;
	 }
}



/************************************************************************
/*
/* pick_scale  (Layer II)
/*
/* PURPOSE:For each subband, puts the smallest scalefactor of the 3
/* associated with a frame into #max_sc#.  This is used
/* used by Psychoacoustic Model I.
/* (I would recommend changin max_sc to min_sc)
/*
/************************************************************************/
 
void pick_scale (unsigned int (*scalar)[3][32],
		 frame_params *fr_ps,
		 double (*max_sc)[32],
		 int cha_sw,
		 int aug_cha_sw,
		 int aiff)         
{
    int i,j,k,l,m;
    int max;
    int stereo  = fr_ps->stereo;
    int stereomc = fr_ps->stereomc;
    int stereoaug = fr_ps->stereoaug;
    int sblimit = fr_ps->sblimit;
    int sblimit_mc = fr_ps->sblimit_mc;
    int sblimit_ml = fr_ps->sblimit_ml;
    int n_ml_ch = fr_ps->header->multiling_ch; /* 08/03/1995 JMZ Multilingual */
   
  
    if (aiff != 1)
    {
       l = 0; m = stereo;
    }
    else
    {   
       l = 0;
       if (stereoaug == 2) m = 12;
       else		   m = 7;
    }
  
    for (k = 0; k < stereo; k++)
    {
	for (i = 0; i < sblimit; max_sc[k][i] = multiple[max], i++)
	    for (j=1, max = scalar[k][0][i];j<3;j++)
	        if (max > scalar[k][j][i])
		    max = scalar[k][j][i];
	for (i = sblimit; i < SBLIMIT;i++)
	    max_sc[k][i] = 1E-20;
    }  
  
    for (k = stereo; k < m; k++)
    {
	for (i = 0; i < sblimit_mc; max_sc[k][i] = multiple[max], i++)
	    for (j=1, max = scalar[k][0][i];j<3;j++)
	        if (max > scalar[k][j][i])
		    max = scalar[k][j][i];
	for (i = sblimit_mc; i < SBLIMIT;i++)
	    max_sc[k][i] = 1E-20;
    }  
  
    if (aiff == 1)
    {
/* OLD 961114 FdB
	if (fr_ps->header->matrix == 3 || cha_sw == 0)
	{
	    fr_ps->header->tc_sbgr_select = 1;
	    for (i = 0; i < 12; i++)
		fr_ps->header->tc_alloc[i] = 0;
	}
	else
	    tc_alloc (fr_ps, max_sc);
*/

	for (i = 0; i < 12; i++)
	    if (cha_sw == -1 && fr_ps->header->matrix != 3)
		switch (fr_ps->config)
		{
		case 320:
			if (fr_ps->header->center == 3 && i >= 10)   /* tc_alloc = 0,3,4,5 */
			{
			    fr_ps->header->tc_alloc[i] = rand () % 4;
			    if (fr_ps->header->tc_alloc[i] > 0)
				fr_ps->header->tc_alloc[i] += 2;
			}
			else
			    fr_ps->header->tc_alloc[i] = rand () % 8;
			break;
		case 310:
			if (fr_ps->header->center == 3 && i >= 10)   /* tc_alloc = 0,3,4 */
			{
			    fr_ps->header->tc_alloc[i] = rand () % 3;
			    if (fr_ps->header->tc_alloc[i] > 0)
				fr_ps->header->tc_alloc[i] += 2;
			}
			else if (fr_ps->header->matrix == 2)
			    fr_ps->header->tc_alloc[i] = rand () % 6;
			else
			    fr_ps->header->tc_alloc[i] = rand () % 5;
			break;
		case 300:
		case 302:
			if (fr_ps->header->center == 3 && i >= 10)   /* tc_alloc = 0 */
			    fr_ps->header->tc_alloc[i] = 0;
			else
			    fr_ps->header->tc_alloc[i] = rand () % 3;
			break;
		case 220:
			fr_ps->header->tc_alloc[i] = rand () % 4;
			break;
		case 210:
			fr_ps->header->tc_alloc[i] = rand () % 3;
			break;
		default:
			break;
		}
	    else if (cha_sw == -2 && fr_ps->header->matrix != 3)
	    	tc_alloc (fr_ps, max_sc);
	    else if (fr_ps->header->matrix == 3)
		fr_ps->header->tc_alloc[i] = 0;
	    else
		fr_ps->header->tc_alloc[i] = cha_sw;

	fr_ps->header->tc_sbgr_select = 1;
	fr_ps->header->tc_allocation = fr_ps->header->tc_alloc[0];
	for (i = 1; i < 12; i++)
	    if (fr_ps->header->tc_alloc[i] != fr_ps->header->tc_alloc[0])
	    	fr_ps->header->tc_sbgr_select = 0;

#ifdef Augmentation_7ch
	for (i = 0; i < 12; i++)
	    if (aug_cha_sw == -1)
		switch (fr_ps->header->aug_mtx_proc)
		{
		case 0: fr_ps->header->aug_tc_alloc[i] = rand () % 8;
			break;
		case 1: fr_ps->header->aug_tc_alloc[i] = rand () % 4;
			break;
		case 3: fr_ps->header->aug_tc_alloc[i] = 0;
			break;
		}
	    else
		fr_ps->header->aug_tc_alloc[i] = aug_cha_sw;
#endif
    }
  
/********************************************************/
/* JMZ 08/03/1995 Multilingual , WtK 07/08/95 */

    if (n_ml_ch > 0)
    {
	for (k = 7; k < 7 + n_ml_ch; k++)
	{
	    for (i = 0; i < sblimit_ml; max_sc[k][i] = multiple[max], i++)
		for (j = 1, max = scalar[k][0][i]; j < 3; j++)
		    if (max > scalar[k][j][i])
			max = scalar[k][j][i];
	  for (i= sblimit_ml; i < SBLIMIT;i++)
	    max_sc[k][i] = 1E-20;
	}
    }

/* JMZ 08/03/1995 Multilingual */
/********************************************************/

}

/***************************************************************************
/*
/* tc_alloc  (Layer II, multichannel)
/*
/* PURPOSE: For each subbandgroup the three transmissionchannels are
/*          determined by taking the channel with the lowest level
/*          according to the tabel tc_allocation in the draft
/*  8/10/93, SR          
/* 
/*           changed to a certain limit of TC_ALLOC which must be stepped
/*           beyond, before there is channel-switching
/*           9/20/93 SR

/* JMZ 08/03/1995 Ajout pour traiter les differentes configurations 
/*		envisagees dans la norme
/**************************************************************************/

void tc_alloc (frame_params *fr_ps, double (*max_sc)[32])
{
    layer 	*info 	= fr_ps->header;	
    int 	center 	= info->center;		
    int 	surround = info->surround;	
    int 	matrix 	= info->matrix;		
    int 	i, l, k;
    int 	min;
    double 	min1;
    double 	min2[7][12];
	
    
    /* 01/03/1995 JMZ Configuration 3/2 */
    if (surround == 2 && center != 0)
    {
	/* if (matrix == 3)		->tc_alloc = 0
	   else if (center == 3)	->tc_alloc = 0,3,4,5
	   else				->tc_alloc = 0,1,2,3,4,5,6,7 */

	if (matrix == 3)
	{
	    for (i = 0; i < 12; i++)
		fr_ps->header->tc_alloc[i] = 0;
	}
	else if (center == 3) /* && matrix != 3 */
	{
	    /* 3/2 Phantom Center coding */
	    for (i = 0; i < 8; i++)
	    {
		if (((20 * log10(max_sc[3][i])) - (20 * log10(max_sc[5][i]))) > TC_ALLOC)
		{
		    if (max_sc[6][i] < max_sc[4][i])
			fr_ps->header->tc_alloc[i] = 5;
		    else
			fr_ps->header->tc_alloc[i] = 3;
		}  
		else if (((20 * log10(max_sc[4][i])) - (20 * log10(max_sc[6][i]))) > TC_ALLOC)
		    fr_ps->header->tc_alloc[i] = 4;
		else
		    fr_ps->header->tc_alloc[i] = 0; 
	    } 
	
	    for (i = 8; i < 12; i++)
	    {
		for (k = 2; k < 7; k++)
		{
		    min2[k][i] = 0.0;	 
		    for (l = (sb_groups[i-1] + 1); l <= sb_groups[i]; l++)
			min2[k][i] += max_sc[k][l];	 
		    min2[k][i] = min2[k][i] / (sb_groups[i] - sb_groups[i-1]); 
		}

		if (((20 * log10(min2[3][i])) - (20 * log10(min2[5][i]))) > TC_ALLOC)
		{
		    if (min2[6][i] < min2[4][i])
			fr_ps->header->tc_alloc[i] = 5;
		    else
			fr_ps->header->tc_alloc[i] = 3;
		}  
		else if (((20 * log10(min2[4][i])) - (20 * log10(min2[6][i]))) > TC_ALLOC)
		    fr_ps->header->tc_alloc[i] = 4;
		else
		    fr_ps->header->tc_alloc[i] = 0;
	    }
	}	
	else
	{
	    /* 3/2 no Phantom Center coding */
	    for (i = 0; i < 8; i++)
	    {
		if (((20 * log10(max_sc[2][i])) - (20 * log10(max_sc[5][i]))) > TC_ALLOC)
		{
		    if (max_sc[6][i] < max_sc[5][i])
			min = 6;
		    else if (max_sc[6][i] == max_sc[5][i])
		    {
			if (max_sc[3][i] <= max_sc[5][i])
			    min = 5;
			else
			    min = 6;	          
		    }
		    else
			min = 5;
		}  
		else if (((20 * log10(max_sc[2][i])) - (20 * log10(max_sc[6][i]))) > TC_ALLOC)
		{
		    min = 6;
		    /* 01/03/1995 JMZ Simplification */
		}  
		else   	  	    
		{
		    min = 2;
		}
	
		switch (min)
		{
		    case 5: if (max_sc[4][i] <= max_sc[6][i])  /* left front,Rs*/
				fr_ps->header->tc_alloc[i] = 1;
			    else
				fr_ps->header->tc_alloc[i] = 7;  /*R*/
			    break;
		    case 6: if (max_sc[3][i] <= max_sc[5][i])  /* right front,Ls*/
				fr_ps->header->tc_alloc[i] = 2;
			    else
				fr_ps->header->tc_alloc[i] = 6;  /* L */
			    break;		    
		    case 2: if (((20 * log10(max_sc[3][i])) - (20 * log10(max_sc[5][i]))) > TC_ALLOC)
			    {
				if  (max_sc[4][i] <= max_sc[6][i])  
				    fr_ps->header->tc_alloc[i] = 3;
			       else
				    fr_ps->header->tc_alloc[i] = 5;
			    }  
			    else
			    {
				if (((20 * log10(max_sc[4][i])) - (20 * log10(max_sc[6][i]))) > TC_ALLOC)  
				    fr_ps->header->tc_alloc[i] = 4;
				else
				    fr_ps->header->tc_alloc[i] = 0;
			    }    
			    break;
		}
	    }

	    for (i = 8; i < 12; i++)    /*taking the average scalefactor of each sb-group*/
	    {
		for (k = 2; k < 7; k++)
		{
		    min2[k][i] = 0.0;	 
		    for (l = (sb_groups[i-1] + 1); l <= sb_groups[i]; l++)
			min2[k][i] += max_sc[k][l];	 
		    min2[k][i] = min2[k][i] / (sb_groups[i] - sb_groups[i-1]); 
		}

		if (((20 * log10(min2[2][i])) - (20 * log10(min2[5][i]))) > TC_ALLOC)
		{
		    if (min2[6][i] < min2[5][i])
			min = 6;
		    else if (min2[6][i] == min2[5][i])
		    {
			if (min2[3][i] <= min2[5][i])
			    min = 5;
			else
			    min = 6;	          
		    }
		    else
			min = 5;
		}  
		else if (((20 * log10(min2[2][i])) - (20 * log10(min2[6][i]))) > TC_ALLOC)
		{
		    min = 6;
		    /* 01/03/1995 JMZ Simplification */
		}  
		else   	  	    
		{
		    min = 2;
		}
	
		switch (min)
		{
		    case 5: if (min2[4][i] <= min2[6][i])  /* left front,Rs*/
				fr_ps->header->tc_alloc[i] = 1;
			    else
				fr_ps->header->tc_alloc[i] = 7;  /*R*/
			    break;
		    case 6: if (min2[3][i] <= min2[5][i])  /* right front,Ls*/
				fr_ps->header->tc_alloc[i] = 2;
			    else
				fr_ps->header->tc_alloc[i] = 6;  /* L */
			    break;
		    case 2: if (((20 * log10(min2[3][i])) - (20 * log10(min2[5][i]))) > TC_ALLOC)
			    {
				if (min2[4][i] <= min2[6][i])  
				    fr_ps->header->tc_alloc[i] = 3;
				else
				    fr_ps->header->tc_alloc[i] = 5;
			    }  
			    else
			    {
				if (((20 * log10(min2[4][i])) - (20 * log10(min2[6][i]))) > TC_ALLOC)  
				    fr_ps->header->tc_alloc[i] = 4;
				else
				    fr_ps->header->tc_alloc[i] = 0;
			    }    
			    break;
		}
	    }
	}
    }

    /* 01/03/1995 JMZ Configuration 3/1 */
    if (surround == 1 && center != 0)
    {
	/* if (matrix == 3)		->tc_alloc = 0
	   else if (center == 3)	->tc_alloc = 0,3,4
	   else if (matrix == 2)	->tc_alloc = 0,1,2,3,4,5
	   else				->tc_alloc = 0,1,2,3,4 */

	if (matrix == 3)
	{
    	    for (i = 0; i < 12; i++)
		fr_ps->header->tc_alloc[i] = 0;
	}
	else if (center == 3) /* && matrix != 3 */
	{
	    /* 3/1 Phantom Center coding */
	    for	(i = 0; i < 8; i++)
	    {
		if (((20 * log10(max_sc[3][i])) - (20 * log10(max_sc[5][i]))) > TC_ALLOC)
		{
		    if (max_sc[6][i] < max_sc[5][i])
			fr_ps->header->tc_alloc[i] = 4;
		    else
			fr_ps->header->tc_alloc[i] = 3;
		}  
		else if (((20 * log10(max_sc[3][i])) - (20 * log10(max_sc[6][i]))) > TC_ALLOC)
		    fr_ps->header->tc_alloc[i] = 4;
		else
		    fr_ps->header->tc_alloc[i] = 0; 
	    } 
	
	    for (i = 8; i < 12; i++)    
	    {
		for (k = 2; k < 7; k++)
		{
		    min2[k][i] = 0.0;	 
		    for (l = (sb_groups[i-1] + 1); l <= sb_groups[i]; l++)
			    min2[k][i] += max_sc[k][l];	 
		    min2[k][i] = min2[k][i] / (sb_groups[i] - sb_groups[i-1]); 
		}
		    
		if (((20 * log10(min2[3][i])) - (20 * log10(min2[5][i]))) > TC_ALLOC)
		{
		    if (min2[6][i] < min2[5][i])
			fr_ps->header->tc_alloc[i] = 4;
		    else 	
			fr_ps->header->tc_alloc[i] = 3;
		}  
		else 	
		    if (((20 * log10(min2[3][i])) - (20 * log10(min2[6][i]))) > TC_ALLOC)
			fr_ps->header->tc_alloc[i] = 4;
		    else   	
			fr_ps->header->tc_alloc[i] = 0; 
	    } 
	}	
	else
/*
		if(matrix==2)
		{    
		}
		else
*/
	{	
	    /* 3/1 no Phantom Center coding */
	    for (i = 0; i < 8; i++)
	    {
		if (((20 * log10(max_sc[2][i])) - (20 * log10(max_sc[5][i]))) > TC_ALLOC)
		{
		    if (max_sc[6][i] < max_sc[5][i])
			min = 6;
		    else
			min = 5;
		}  
		else if (((20 * log10(max_sc[2][i])) - (20 * log10(max_sc[6][i]))) > TC_ALLOC)
		    min = 6;
		else   	  	    
		    min = 2;
	
		switch (min)
		{
		    case 5: fr_ps->header->tc_alloc[i] = 1;
			    break;

		    case 6: fr_ps->header->tc_alloc[i] = 2;
			    break;
		    
		    case 2: if (((20 * log10(max_sc[3][i])) - (20 * log10(max_sc[5][i]))) > TC_ALLOC)
			    {
				if (max_sc[6][i] <= max_sc[5][i])  
				    fr_ps->header->tc_alloc[i] = 4;
				else
				    fr_ps->header->tc_alloc[i] = 3;
			    }  
			    else
			    {
				if (((20 * log10(max_sc[3][i])) - (20 * log10(max_sc[6][i]))) > TC_ALLOC)  
				    fr_ps->header->tc_alloc[i] = 4;
				else
				    fr_ps->header->tc_alloc[i] = 0;
			    }    
			    break;
		}
	    }
 
	    for (i = 8; i < 12; i++)
	    {
		for (k = 2; k < 7; k++)
		{
		    min2[k][i] = 0.0;	 
		    for (l = (sb_groups[i-1] + 1); l <= sb_groups[i]; l++)
			min2[k][i] += max_sc[k][l];	 
		    min2[k][i] = min2[k][i] / (sb_groups[i] - sb_groups[i-1]); 
		}

		if (((20 * log10(min2[2][i])) - (20 * log10(min2[5][i]))) > TC_ALLOC)
		{
		    if (min2[6][i] < min2[5][i])
			min = 6;
		    else
			min = 5;
		}  
		else if (((20 * log10(min2[2][i])) - (20 * log10(min2[6][i]))) > TC_ALLOC)
		    min = 6;
		else   	  	    
		    min = 2;
	
		switch (min)
		{
		    case 5: fr_ps->header->tc_alloc[i] = 1;
			    break;

		    case 6: fr_ps->header->tc_alloc[i] = 2;
			    break;
		    
		    case 2: if (((20 * log10(min2[3][i])) - (20 * log10(min2[5][i]))) > TC_ALLOC)
			    {
				if (min2[6][i] <= min2[5][i])  
				    fr_ps->header->tc_alloc[i] = 4;
				else
				    fr_ps->header->tc_alloc[i] = 3;
			    }  
			    else
			    {
				if (((20 * log10(min2[3][i])) - (20 * log10(min2[6][i]))) > TC_ALLOC)  
				    fr_ps->header->tc_alloc[i] = 4;
				else
				    fr_ps->header->tc_alloc[i] = 0;
			    }    
			    break;
		}
	    }
	}
    }
     
    /* 01/03/1995 JMZ Configuration 3/0 (+2/0) */
    if (center != 0 && (surround == 3 || surround == 0))
    {
	if (matrix == 3 || center == 3)
	{
    	    for (i = 0; i < 12; i++)
		fr_ps->header->tc_alloc[i] = 0;
	}
/* 02/02/97 FdB no matrix == 2 allowed for 3/0
	else if (matrix == 2) 
	{
		for(i = 0; i < 8; i++)
		{
			if(((20 * log10(max_sc[2][i])) - (20 * log10(max_sc[5][i]))) > TC_ALLOC)
			{
				if(max_sc[6][i] < max_sc[5][i])
					fr_ps->header->tc_alloc[i] = 2;
				else 	fr_ps->header->tc_alloc[i] = 1;
			}  
			else 	if(((20 * log10(max_sc[2][i])) - (20 * log10(max_sc[6][i]))) > TC_ALLOC)
					fr_ps->header->tc_alloc[i] = 2;
				else   	fr_ps->header->tc_alloc[i] = 0; 
		} 
		
		for(i = 8; i < 12; i++)   
		for(k = 2; k < 7; k++)
			min2[k][i] = 0.0;	 
 
		for(i = 8; i < 12; i++)    
		{
			for(k = 2; k < 7; k++)
			{
				for(l = (sb_groups[i-1] + 1); l <= sb_groups[i]; l++)
				{
					min2[k][i] += max_sc[k][l];	 
				}
				min2[k][i] = min2[k][i] / (sb_groups[i] - sb_groups[i-1]); 
			}
		
			if(((20 * log10(min2[2][i])) - (20 * log10(min2[5][i]))) > TC_ALLOC)
			{
				if(min2[6][i] < min2[5][i])
					fr_ps->header->tc_alloc[i] = 2;
				else 	fr_ps->header->tc_alloc[i] = 1;
			}  
			else 	if(((20 * log10(min2[2][i])) - (20 * log10(min2[6][i]))) > TC_ALLOC)
					fr_ps->header->tc_alloc[i] = 2;
				else   	fr_ps->header->tc_alloc[i] = 0; 
		} 
	}
*/
	else
	{	
	    for (i = 0; i < 8; i++)
	    {
		if (((20 * log10(max_sc[2][i])) - (20 * log10(max_sc[5][i]))) > TC_ALLOC)
		{
		    if (max_sc[6][i] < max_sc[5][i])
			min = 6;
		    else
			min = 5;
		}  
		else if (((20 * log10(max_sc[2][i])) - (20 * log10(max_sc[6][i]))) > TC_ALLOC)
		    min = 6;
		else   	  	    
		    min = 2;

		switch (min)
		{
		    case 5: fr_ps->header->tc_alloc[i] = 1;
			    break;

		    case 6: fr_ps->header->tc_alloc[i] = 2;
			    break;

		    case 2: fr_ps->header->tc_alloc[i] = 0;
			    break;
		}
	    }

	    for (i = 8; i < 12; i++)
	    {
		for (k = 2; k < 7; k++)
		{
		    min2[k][i] = 0.0;	 
		    for (l = (sb_groups[i-1] + 1); l <= sb_groups[i]; l++)
			min2[k][i] += max_sc[k][l];	 
		    min2[k][i] = min2[k][i] / (sb_groups[i] - sb_groups[i-1]); 
		}

		if (((20 * log10(min2[2][i])) - (20 * log10(min2[5][i]))) > TC_ALLOC)
		{
		    if (min2[6][i] < min2[5][i])
			min = 6;
		    else
			min = 5;
		}  
		else if (((20 * log10(min2[2][i])) - (20 * log10(min2[6][i]))) > TC_ALLOC)
		    min = 6;
		else   	  	    
		    min = 2;
	
		switch (min)
		{
		    case 5: fr_ps->header->tc_alloc[i] = 1;
			    break;

		    case 6: fr_ps->header->tc_alloc[i] = 2;
			    break;

		    case 2: fr_ps->header->tc_alloc[i] = 0;
			    break;
		}
	    }
	}
    }

    /* 01/03/1995 JMZ Configuration 2/2 */
    if (center == 0 && surround == 2)
    {
	if (matrix == 3)
	{
    	    for (i = 0; i < 12; i++)
		fr_ps->header->tc_alloc[i] = 0;
	}
	else	
	{
	    for (i = 0; i < 8; i++)
	    {
		if (((20 * log10(max_sc[2][i])) - (20 * log10(max_sc[5][i]))) > TC_ALLOC)
		{
		    if (max_sc[6][i] < max_sc[3][i])
			fr_ps->header->tc_alloc[i] = 3;
		    else 	
			fr_ps->header->tc_alloc[i] = 2;
		}  
		else if (((20 * log10(max_sc[3][i])) - (20 * log10(max_sc[6][i]))) > TC_ALLOC)
		    fr_ps->header->tc_alloc[i] = 1;
		else   	  	    
		    fr_ps->header->tc_alloc[i] = 0;
	    }

	    for (i = 8; i < 12; i++)
	    {
		for (k = 2; k < 7; k++)
		{
		    min2[k][i] = 0.0;	 
		    for (l = (sb_groups[i-1] + 1); l <= sb_groups[i]; l++)
			min2[k][i] += max_sc[k][l];	 
		    min2[k][i] = min2[k][i] / (sb_groups[i] - sb_groups[i-1]); 
		}

		if (((20 * log10(min2[2][i])) - (20 * log10(min2[5][i]))) > TC_ALLOC)
		{
		    if (min2[6][i] < min2[3][i])
			fr_ps->header->tc_alloc[i] = 3;
		    else 	
			fr_ps->header->tc_alloc[i] = 2;
		}  
		else if (((20 * log10(min2[3][i])) - (20 * log10(min2[6][i]))) > TC_ALLOC)
		    fr_ps->header->tc_alloc[i] = 1;
		else   	  	    
		    fr_ps->header->tc_alloc[i] = 0;
	    }
	}
    }
 
    /* 01/03/1995 JMZ et Configuration 2/1*/
    if (center == 0 && surround == 1)
    {
	if (matrix == 3)
	{
    	    for (i = 0; i < 12; i++)
		fr_ps->header->tc_alloc[i] = 0;
	}
	else	
	{
	    for (i = 0; i < 8; i++)
	    {
		if (((20 * log10(max_sc[2][i])) - (20 * log10(max_sc[5][i]))) > TC_ALLOC)
		{
		    if (max_sc[6][i] < max_sc[5][i])
			min = 6;
		    else
			min = 5;
		}  
		else if (((20 * log10(max_sc[2][i])) - (20 * log10(max_sc[6][i]))) > TC_ALLOC)
		    min = 6;
		else   	  	    
		    min = 2;
	
		switch (min)
		{
		    case 5: fr_ps->header->tc_alloc[i] = 1;
			    break;

		    case 6: fr_ps->header->tc_alloc[i] = 2;
			    break;
		    
		    case 2: fr_ps->header->tc_alloc[i] = 0;
			    break;
		}
	    }

	    for (i = 8; i < 12; i++)
	    {
		for (k = 2; k < 7; k++)
		{
		    min2[k][i] = 0.0;	 
		    for (l = (sb_groups[i-1] + 1); l <= sb_groups[i]; l++)
			min2[k][i] += max_sc[k][l];	 
		    min2[k][i] = min2[k][i] / (sb_groups[i] - sb_groups[i-1]); 
		}

		if (((20 * log10(min2[2][i])) - (20 * log10(min2[5][i]))) > TC_ALLOC)
		{
		    if (min2[6][i] < min2[5][i])
			min = 6;
		    else
			min = 5;
		}  
		else if (((20 * log10(min2[2][i])) - (20 * log10(min2[6][i]))) > TC_ALLOC)
		    min = 6;
		else   	  	    
		    min = 2;
	
		switch (min)
		{
		    case 5: fr_ps->header->tc_alloc[i] = 1;
			    break;

		    case 6: fr_ps->header->tc_alloc[i] = 2;
			    break;
		    
		    case 2: fr_ps->header->tc_alloc[i] = 0;
			    break;
		}
	    }
	}
    }

    if (fr_ps->header->tc_alloc[0] == fr_ps->header->tc_alloc[1] &&
	fr_ps->header->tc_alloc[1] == fr_ps->header->tc_alloc[2] &&
	fr_ps->header->tc_alloc[2] == fr_ps->header->tc_alloc[3] &&
	fr_ps->header->tc_alloc[3] == fr_ps->header->tc_alloc[4] &&
	fr_ps->header->tc_alloc[4] == fr_ps->header->tc_alloc[5] &&
	fr_ps->header->tc_alloc[5] == fr_ps->header->tc_alloc[6] &&
	fr_ps->header->tc_alloc[6] == fr_ps->header->tc_alloc[7] &&
	fr_ps->header->tc_alloc[7] == fr_ps->header->tc_alloc[8] &&
	fr_ps->header->tc_alloc[8] == fr_ps->header->tc_alloc[9] &&
	fr_ps->header->tc_alloc[9] == fr_ps->header->tc_alloc[10] &&
	fr_ps->header->tc_alloc[10] == fr_ps->header->tc_alloc[11])
	{
	    fr_ps->header->tc_sbgr_select = 1;
	    fr_ps->header->tc_allocation = fr_ps->header->tc_alloc[0];
	}
	else
	    fr_ps->header->tc_sbgr_select = 0; /* added 8/20/93,SR*/ 
}


/************************************************************************
/*
/* put_scale   (Layer I)
/*
/* PURPOSE:Sets #max_sc# to the scalefactor index in #scalar.
/* This is used by Psychoacoustic Model I
/*
/************************************************************************/
 
void put_scale(unsigned int (*scalar)[3][32], frame_params *fr_ps, double (*max_sc)[32])
                                   
                    
       /*far*/                    
{
   int i,j,k, max;
   int stereo  = fr_ps->stereo;
   int stereomc = fr_ps->stereomc;
   int sblimit = fr_ps->sblimit;
 
/*	for (k = 0; k < stereo+stereomc+2; k++) 960814 FdB bug for some configurations */
	for (k = 0; k < 7; k++)
	   for (i = 0; i < SBLIMIT; i++)
              max_sc[k][i] = multiple[scalar[k][0][i]];
}
 
/************************************************************************
/*
/* II_transmission_pattern (Layer II only)
/*
/* PURPOSE:For a given subband, determines whether to send 1, 2, or
/* all 3 of the scalefactors, and fills in the scalefactor
/* select information accordingly
/*
/* SEMANTICS:  The subbands and channels are classified based on how much
/* the scalefactors changes over its three values (corresponding
/* to the 3 sets of 12 samples per subband).  The classification
/* will send 1 or 2 scalefactors instead of three if the scalefactors
/* do not change much.  The scalefactor select information,
/* #scfsi#, is filled in accordingly.
/*
/************************************************************************/
 
void II_transmission_pattern (unsigned int (*scalar)[3][32], unsigned int (*scfsi)[32], frame_params *fr_ps)
{
    int stereo  = fr_ps->stereo;
    int stereomc = fr_ps->stereomc;
    int sblimit;
    int sblimit_mc = fr_ps->sblimit_mc;
    int sblimit_ml = fr_ps->sblimit_ml;
    int n_ml_ch = fr_ps->header->multiling_ch;

    int dscf[2];
    int class[2],i,j,k;

static int pattern[5][5] = {0x123, 0x122, 0x122, 0x133, 0x123,
                            0x113, 0x111, 0x111, 0x444, 0x113,
                            0x111, 0x111, 0x111, 0x333, 0x113,
                            0x222, 0x222, 0x222, 0x333, 0x123,
                            0x123, 0x122, 0x122, 0x133, 0x123};
 
    if (stereomc > 0 && fr_ps->sblimit_mc > fr_ps->sblimit)
	sblimit = fr_ps->sblimit_mc;
    else
	sblimit = fr_ps->sblimit;

/*  for (k = 0; k < stereo+stereomc+2; k++) 960814 FdB bug for some configurations */
    for (k = 0; k < 12; k++)
	for (i = 0; i < SBLIMIT; i++) 
	{
	    dscf[0] = (scalar[k][0][i] - scalar[k][1][i]);
	    dscf[1] = (scalar[k][1][i] - scalar[k][2][i]);
	    for (j = 0; j < 2; j++)
	    {
	        if (dscf[j] <= -3)
		    class[j] = 0;
	        else if (dscf[j] > -3 && dscf[j] < 0)
		    class[j] = 1;
		else if (dscf[j] == 0)
		    class[j] = 2;
		else if (dscf[j] > 0 && dscf[j] < 3)
		    class[j] = 3;
		else
		    class[j] = 4;
	    }
	    switch (pattern[class[0]][class[1]]) 
	    {
	      case 0x123 :    scfsi[k][i] = 0;
			      break;
	      case 0x122 :    scfsi[k][i] = 3;
			      scalar[k][2][i] = scalar[k][1][i];
			      break;
	      case 0x133 :    scfsi[k][i] = 3;
			      scalar[k][1][i] = scalar[k][2][i];
			      break;
	      case 0x113 :    scfsi[k][i] = 1;
			      scalar[k][1][i] = scalar[k][0][i];
			      break;
	      case 0x111 :    scfsi[k][i] = 2;
			      scalar[k][1][i] = scalar[k][2][i] = scalar[k][0][i];
			      break;
	      case 0x222 :    scfsi[k][i] = 2;
			      scalar[k][0][i] = scalar[k][2][i] = scalar[k][1][i];
			      break;
	      case 0x333 :    scfsi[k][i] = 2;
			      scalar[k][0][i] = scalar[k][1][i] = scalar[k][2][i];
			      break;
	      case 0x444 :    scfsi[k][i] = 2;
			      if (scalar[k][0][i] > scalar[k][2][i])
				  scalar[k][0][i] = scalar[k][2][i];
			      scalar[k][1][i] = scalar[k][2][i] = scalar[k][0][i];
	
	
	    } /* switch */
	} /* subband */

    if (n_ml_ch > 0)
    {
	for (k = 7; k < 7 + n_ml_ch; k++)
	    for (i = 0; i < sblimit_ml; i++) 
	    {
		dscf[0] = (scalar[k][0][i] - scalar[k][1][i]);
		dscf[1] = (scalar[k][1][i] - scalar[k][2][i]);
		for (j = 0; j < 2; j++)
		{
		    if (dscf[j] <= -3)
			class[j] = 0;
		    else if (dscf[j] > -3 && dscf[j] < 0)
			class[j] = 1;
		    else if (dscf[j] == 0)
			class[j] = 2;
		    else if (dscf[j] > 0 && dscf[j] < 3)
			class[j] = 3;
		    else
			class[j] = 4;
		}
		switch (pattern[class[0]][class[1]]) 
		{
		  case 0x123 :    scfsi[k][i] = 0;
				  break;
		  case 0x122 :    scfsi[k][i] = 3;
				  scalar[k][2][i] = scalar[k][1][i];
				  break;
		  case 0x133 :    scfsi[k][i] = 3;
				  scalar[k][1][i] = scalar[k][2][i];
				  break;
		  case 0x113 :    scfsi[k][i] = 1;
				  scalar[k][1][i] = scalar[k][0][i];
				  break;
		  case 0x111 :    scfsi[k][i] = 2;
				  scalar[k][1][i] = scalar[k][2][i] = scalar[k][0][i];
				  break;
		  case 0x222 :    scfsi[k][i] = 2;
				  scalar[k][0][i] = scalar[k][2][i] = scalar[k][1][i];
				  break;
		  case 0x333 :    scfsi[k][i] = 2;
				  scalar[k][0][i] = scalar[k][1][i] = scalar[k][2][i];
				  break;
		  case 0x444 :    scfsi[k][i] = 2;
				  if (scalar[k][0][i] > scalar[k][2][i])
				      scalar[k][0][i] = scalar[k][2][i];
				  scalar[k][1][i] = scalar[k][2][i] = scalar[k][0][i];
		} /* switch */
	    } /* subband */
    }
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
 
void I_encode_scale(unsigned int (*scalar)[3][32], unsigned int (*bit_alloc)[32], frame_params *fr_ps, Bit_stream_struc *bs)
{
   int stereo  = fr_ps->stereo;
   int sblimit = fr_ps->sblimit;
   int i,j;
 
   for (i=0;i<SBLIMIT;i++) for (j=0;j<stereo;j++)
      if (bit_alloc[j][i]) putbits(bs,scalar[j][0][i],6);
}
 
/***************************** Layer II  ********************************/

void II_encode_scale (
	unsigned int (*bit_alloc)[32],
	unsigned int (*scfsi)[32],
	unsigned int (*scalar)[3][32], /* JMZ 08/03/1995 Multilingual */
	unsigned int lfe_alloc, 
	unsigned int lfe_scalar, 
	frame_params *fr_ps,
	Bit_stream_struc *bs,
	int *l,
	int *z
)
{
   layer *info     = fr_ps->header;
   int center      = info->center;
   int surround    = info->surround;
   int stereo  = fr_ps->stereo;
   int sblimit;
   int lfe     = fr_ps->header->lfe;
   int i,j,k, m, n, pci;
   int pred;

    if (*l == 0)
	sblimit = fr_ps->sblimit;
    else
	sblimit = fr_ps->sblimit_mc;

    for (i = 0; i < sblimit; i++)
    {
	n = sbgrp[i];
	for (m = *l; m < *z; m++)
	{
	    k = transmission_channel (fr_ps, n, m);

	    if (bit_alloc[k][i] && (i < 12 || m != 2 || center != 3))
		putbits (bs, scfsi[k][i], 2);
	}
    }

    pred = 0;
    if (*l == stereo)
    {
	if (fr_ps->header->mc_prediction_on == 1)
	{
	    for (i = 0; i < 8; i++)
	    {
	        if (fr_ps->header->mc_pred[i] == 1)
	        {
		    for (m = 0; m < n_pred_coef[fr_ps->header->dyn_cross[i]]; m++)
		    {
		        if (fr_ps->header->predsi[i][m] != 0)
		        {
		            putbits (bs, fr_ps->header->delay_comp[i][m], 3);
			    pred += 3;
			    for (pci = 0; pci < fr_ps->header->predsi[i][m]; pci++)
			    {
				putbits (bs, fr_ps->header->pred_coef[i][m][pci], 8);
				pred += 8;			      
			    }
		        }    
		    }
	        }
	    }
	}  
    }  
 
    if (*l == stereo && lfe && lfe_alloc)
	putbits (bs, lfe_scalar, 6);

    for (i = 0; i < sblimit; i++)
    {
	n = sbgrp[i];
	for (m = *l; m < *z; m++)
	{
	    k = transmission_channel (fr_ps, n, m);

            if (bit_alloc[k][i] && (i < 12 || m != 2 || center != 3))  
	    switch (scfsi[k][i]) 
	    {
		case 0: for (j = 0; j < 3; j++)
                            putbits (bs, scalar[k][j][i], 6);
			break;
		case 1:
		case 3:	putbits (bs, scalar[k][0][i], 6);
			putbits (bs, scalar[k][2][i], 6);
			break;
		case 2: putbits (bs, scalar[k][0][i], 6);
	    }
	}
    }	
}
 
/*JMZ 24/2/95 Multilingual , WtK 7/8/95*/
void II_encode_scale_ml (unsigned int (*bit_alloc)[32], unsigned int (*scfsi)[32],
			 unsigned int (*scalar)[3][32], frame_params *fr_ps,
			 Bit_stream_struc *bs, int *l, int *z)
{
   int stereo    = fr_ps->stereo;
   int sblimit_ml = fr_ps->sblimit_ml;
   int i,j,k, m, n, pci;

for (i=0;i<sblimit_ml;i++)
  for (m = *l; m < *z; m++)
	{
	k=m;
	if (bit_alloc[k][i])  
		putbits(bs,scfsi[k][i],2);
	}
for (i=0;i<sblimit_ml;i++)
  for (m = *l;m < *z; m++)
	{
	k=m;
	if (bit_alloc[k][i])  
		switch (scfsi[k][i]) 
		{
			case 0: for (j=0;j<3;j++)
				putbits(bs,scalar[k][j][i],6);
				break;
			case 1:
			case 3: putbits(bs,scalar[k][0][i],6);
				putbits(bs,scalar[k][2][i],6);
				break;
			case 2: putbits(bs,scalar[k][0][i],6);
				break;
		}
	}	
}

/*=======================================================================\
|                                                                        |
|      The following routines are done after the masking threshold       |
| has been calculated by the fft analysis routines in the Psychoacoustic |
| model. Using the MNR calculated, the actual number of bits allocated   |
| to each subband is found iteratively.                                  |
|                                                                        |
\=======================================================================*/
 
/************************************************************************
/*
/* I_bits_for_nonoise  (Layer I)
/* II_bits_for_nonoise (Layer II)
/*
/* PURPOSE:Returns the number of bits required to produce a
/* mask-to-noise ratio better or equal to the noise/no_noise threshold.
/*
/* SEMANTICS:
/* bbal = # bits needed for encoding bit allocation
/* bsel = # bits needed for encoding scalefactor select information
/* banc = # bits needed for ancillary data (header info included)
/*
/* For each subband and channel, will add bits until one of the
/* following occurs:
/* - Hit maximum number of bits we can allocate for that subband
/* - MNR is better than or equal to the minimum masking level
/*   (NOISY_MIN_MNR)
/* Then the bits required for scalefactors, scfsi, bit allocation,
/* and the subband samples are tallied (#req_bits#) and returned.
/*
/* (NOISY_MIN_MNR) is the smallest MNR a subband can have before it is
/* counted as 'noisy' by the logic which chooses the number of JS
/* subbands.
/*
/* Joint stereo is supported.
/*
/************************************************************************/

/*static double snr[18] = {0.00, 7.00, 11.00, 16.00, 20.84,
                         25.28, 31.59, 37.75, 43.84,
                         49.89, 55.93, 61.96, 67.98, 74.01,
                         80.03, 86.05, 92.01, 98.01};*/
static double snr[18] = { 0.00,  6.03, 11.80, 15.81, /* 0, 3, 5, 7 */
			 19.03, 23.50, 29.82, 35.99, /* 9,15,31,63 */
			 42.08, 48.13, 54.17, 60.20, /* 127, ...   */
			 66.22, 72.25, 78.27, 84.29, /* 2047, ...  */
			 90.31, 96.33};              /* 16383, ... */

int I_bits_for_nonoise(double (*perm_smr)[32], frame_params *fr_ps)
{
   int i,j,k;
   int stereo  = fr_ps->stereo;
   int sblimit = fr_ps->sblimit;
   int jsbound = fr_ps->jsbound;
   int req_bits = 0;
 
   /* initial b_anc (header) allocation bits */
   req_bits = 32 + 4 * ( (jsbound * stereo) + (SBLIMIT-jsbound) );
 
   for(i=0; i<SBLIMIT; ++i)
     for(j=0; j<((i<jsbound)?stereo:1); ++j) {
       for(k=0;k<14; ++k)
			if( (-perm_smr[j][i] + snr[k]) >= fr_ps->mnr_min)
           break; /* we found enough bits */
         if(stereo == 2 && i >= jsbound)     /* check other JS channel */
           for(;k<14; ++k)
				 if( (-perm_smr[1-j][i] + snr[k]) >= fr_ps->mnr_min) break;
         if(k>0) req_bits += (k+1)*12 + 6*((i>=jsbound)?stereo:1);
   }
   return req_bits;
}
 
/***************************** Layer II  ********************************/
 
int II_bits_for_nonoise(double (*perm_smr)[32], unsigned int (*scfsi)[32], frame_params *fr_ps, int a, int b, int *aiff)
{
    int sb,ch,ba,i;
    int stereo  = fr_ps->stereo;
    int stereomc = fr_ps->stereomc;
    int stereoaug = fr_ps->stereoaug;
    int n_ml_ch = fr_ps->header->multiling_ch;	/* 23/03/1995 JMZ Multilingual */
    int sblimit = fr_ps->sblimit;
    int sblimit_mc = fr_ps->sblimit_mc;
    int sblimit_ml = fr_ps->sblimit_ml;
    int jsbound = fr_ps->jsbound;
    al_table *alloc = fr_ps->alloc;
    al_table *alloc_mc = fr_ps->alloc_mc;
    al_table *alloc_ml = fr_ps->alloc_ml;
    int bbal = 0;
    int berr;		                  /* before: =0 92-08-11 shn */
    int banc = 32; 	                  /* header ISO Layer II */
    int bancmc = 0;                            /* header multichannel = 93, 5.7.93,SR*/
    int bancext = 0;                      /* header multichannel = 93, 5.7.93,SR*/
    int maxAlloc, sel_bits, sc_bits, smp_bits;
    int req_bits = 0;
    int tca_bits, dynx_bits, aug_tca_bits;
    static int sfsPerScfsi[] = { 3,2,1,2 };    /* lookup # sfs per scfsi */

    if (verbosity >= 3) printf("bits_for_nonoise\n");

    if( *aiff == 1)
    { 
	if (fr_ps->header->ext_bit_stream_present == 0)
	    bancmc += 35;      /* mc_header + crc + tc_sbgr_select+ dyn_cross_on + 
				mc_prediction_on  01/05/94,  SR  new! 05/04/94,  SR*/
	else
	{
	    bancmc += 43;			    
	    bancext = 40;
	}

	/* 960627 FdB TCA bits dependent on configuration */
	if (fr_ps->config == 320)
	    tca_bits = 3;
	else if (fr_ps->config == 310)
	    tca_bits = 3;
	else if (fr_ps->config == 220)
	    tca_bits = 2;
	else if (fr_ps->config == 300 || fr_ps->config == 302 || fr_ps->config == 210)
	    tca_bits = 2;
	else
	    tca_bits = 0;
    
	if (fr_ps->header->tc_sbgr_select == 0)
	    bancmc += 12 * tca_bits;
	else
	    bancmc += tca_bits;	

	if (fr_ps->header->dyn_cross_on == 1)
	{
	    /* 960627 FdB DynX dependent on configuration */
	    if (fr_ps->config == 320)
		dynx_bits = 4;
	    else if (fr_ps->config == 310 || fr_ps->config == 220)
		dynx_bits = 3;
	    else if (fr_ps->config == 300 || fr_ps->config == 302 || fr_ps->config == 210)
		dynx_bits = 1;
	    else
		dynx_bits = 0;
    
	    bancmc = 1 + 12 * dynx_bits;
	    if (fr_ps->header->surround == 3)
		bancmc += 12;      /* now with dyn_second_stereo,  17/02/95,  SR*/
	}
    
	if (fr_ps->header->mc_prediction_on == 1)
	{
	    /* mc_pred, predsi, delay_comp, pred_coef */
	    /* bancmc += 8 * (3 + 6 * (3 + 3 * 8)); */
	    for (i = 0; i < 8; i++)
	    {
		bancmc += 3;
		bancmc += n_pred_coef[fr_ps->header->dyn_cross[i]] * 27;
	    }
	}	
    
#ifdef Augmentation_7ch
	if (stereoaug == 2)
	{
	    if (fr_ps->header->aug_mtx_proc == 0)
		aug_tca_bits = 3;
	    else if (fr_ps->header->aug_mtx_proc == 1)
		aug_tca_bits = 2;
	    else
		aug_tca_bits = 0;
    
	    bancmc += 12 * aug_tca_bits;
    
	    if (fr_ps->header->aug_dyn_cross_on == 1)
		bancmc += 12 * 5;
	}
#endif
    }     
    else
	bancmc = 0;

    if (fr_ps->header->error_protection)
	berr=16;
    else
	berr=0; /* added 92-08-11 shn */

    for (sb = 0; sb < jsbound; sb++)
	bbal += (stereo) * (*alloc)[sb][0].bits;
    for (sb = jsbound; sb < sblimit; sb++)
	bbal += (stereo-1) * (*alloc)[sb][0].bits;     	 
    for (sb = 0; sb < sblimit_mc; sb++)
	bbal += (stereomc+stereoaug) * (*alloc_mc)[sb][0].bits;     	 
    for (sb = 0; sb < sblimit_ml; sb++)
	bbal += (n_ml_ch) * (*alloc_ml)[sb][0].bits;     	 

    req_bits = banc + bancmc + bbal + berr;

    for (sb = 0; sb < SBLIMIT; sb++)
	for (ch = 0; ch < ((sb < jsbound)? stereo+stereomc+stereoaug : 1); ch++)
	{
	    if (ch < stereo)
	    {
		alloc = fr_ps->alloc;
		sblimit = fr_ps->sblimit;
	    }
	    else
	    {
		alloc = fr_ps->alloc_mc;
		sblimit = fr_ps->sblimit_mc;
	    }
	    if (sb < sblimit)
	    {
		maxAlloc = (1<<(*alloc)[sb][0].bits)-1;
		sel_bits = sc_bits = smp_bits = 0;
		for (ba = 0; ba < maxAlloc-1; ba++)
		    if ((-perm_smr[ch][sb] + snr[(*alloc)[sb][ba].quant + ((ba>0)?1:0)])
				    >= fr_ps->mnr_min)
			break;
		if (((b - a) >= 1) && (sb >= jsbound))
		{ /* check other JS channels */
		    for (; ba < maxAlloc-1; ba++)
			if ((-perm_smr[1-ch][sb] + snr[(*alloc)[sb][ba].quant + ((ba>0)?1:0)])
				   >= fr_ps->mnr_min)
			    break;
		}
	   
		if (ba > 0) 
		{
		    smp_bits = 12 * ((*alloc)[sb][ba].group * (*alloc)[sb][ba].bits);
		    /* scale factor bits required for subband */
		    sel_bits = 2;
		    sc_bits  = 6 * sfsPerScfsi[scfsi[ch][sb]];
   
		    if (stereo == 2 && sb >= jsbound)
		    {
			/* each new js sb has L+R scfsis*/
			sel_bits += 2;
			sc_bits  += 6 * sfsPerScfsi[scfsi[1-ch][sb]];
		    } 
   
		    req_bits += (smp_bits+sel_bits+sc_bits);
		}
	    }
	}

    if (n_ml_ch > 0)
    {
	/* remaining part from jsbound to sblimit_ml in II_bits_for_indi() , WtK*/
	for (sb = 0; sb < (jsbound < sblimit_ml) ? jsbound:sblimit_ml; sb++)
	{
	    for (ch = 7; ch < 7+n_ml_ch; ch++)  
	    {
		maxAlloc = (1<<(*alloc_ml)[sb][0].bits)-1;
		sel_bits = sc_bits = smp_bits = 0;
		for (ba = 0; ba < maxAlloc-1; ba++)
		    if ((-perm_smr[ch][sb] + snr[(*alloc_ml)[sb][ba].quant + ((ba>0)?1:0)])
				 >= fr_ps->mnr_min)
			break;	       
		if (ba > 0) 
		{
		    if (fr_ps->header->multiling_fs==1)
			smp_bits =  6 * ((*alloc_ml)[sb][ba].group * (*alloc_ml)[sb][ba].bits);
		    else
			smp_bits = 12 * ((*alloc_ml)[sb][ba].group * (*alloc_ml)[sb][ba].bits);

		    /* scale factor bits required for subband */
		    sel_bits = 2;
		    sc_bits  = 6 * sfsPerScfsi[scfsi[ch][sb]];

		    req_bits += (smp_bits+sel_bits+sc_bits);
		}
	    }
	}
    }

    return req_bits;
}


/**********************************************************************/ 
/*now for the independent channels, 8/6/93, SR                        */
/**********************************************************************/
int II_bits_for_indi(double (*perm_smr)[32], unsigned int (*scfsi)[32], frame_params *fr_ps, int *a, int *b, int *aiff)
{
    int sb,ch,ba,i;
    int stereo  = fr_ps->stereo;
    int stereomc = fr_ps->stereomc;
    int stereoaug = fr_ps->stereoaug;
    int n_ml_ch = fr_ps->header->multiling_ch;	/* 23/03/1995 JMZ Multilingual */
    int sblimit = fr_ps->sblimit_mc;
    int sblimit_ml = fr_ps->sblimit_ml;
    int jsbound = fr_ps->jsbound;
    al_table *alloc = fr_ps->alloc_mc;
    al_table *alloc_ml = fr_ps->alloc_ml;
    int req_bits = 0;
    int maxAlloc, sel_bits, sc_bits, smp_bits;
    static int sfsPerScfsi[] = { 3,2,1,2 };    /* lookup # sfs per scfsi */

    for (sb = jsbound; sb < sblimit; sb++)
    {
	for (ch = stereo; ch < stereo+stereomc+stereoaug; ch++) 
	{
	    maxAlloc = (1<<(*alloc)[sb][0].bits)-1;
	    sel_bits = sc_bits = smp_bits = 0;
	    for (ba = 0; ba < maxAlloc-1; ba++)
		if ((-perm_smr[ch][sb] + snr[(*alloc)[sb][ba].quant + ((ba>0)?1:0)])
			 >= fr_ps->mnr_min)
		    break;		/* we found enough bits */

	    if (ba > 0) 
	    {
		smp_bits = 12 * ((*alloc)[sb][ba].group * (*alloc)[sb][ba].bits);
		/* scale factor bits required for subband */
		sel_bits = 2;
		sc_bits  = 6 * sfsPerScfsi[scfsi[ch][sb]];
		req_bits += smp_bits+sel_bits+sc_bits;
	    }
	}
    }

    if (n_ml_ch > 0)
	for (sb = jsbound; sb < sblimit_ml; sb++)
	{
	    for (ch = 7; ch < 7+n_ml_ch; ch++) 
	    {
		maxAlloc = (1<<(*alloc_ml)[sb][0].bits)-1;
		sel_bits = sc_bits = smp_bits = 0;
		for (ba = 0; ba < maxAlloc-1; ba++)
		    if ((-perm_smr[ch][sb] + snr[(*alloc_ml)[sb][ba].quant + ((ba>0)?1:0)])
			  >= fr_ps->mnr_min)
			break;		/* we found enough bits */

		if (ba > 0) 
		{
		    if (fr_ps->header->multiling_fs == 1)
			smp_bits =  6 * ((*alloc_ml)[sb][ba].group * (*alloc_ml)[sb][ba].bits);
		    else
			smp_bits = 12 * ((*alloc_ml)[sb][ba].group * (*alloc_ml)[sb][ba].bits);

		     /* scale factor bits required for subband */
		    sel_bits = 2;
		    sc_bits  = 6 * sfsPerScfsi[scfsi[ch][sb]];

		    req_bits += smp_bits+sel_bits+sc_bits;
		 }
	    }
	}

    return req_bits;
}

	    
/************************************************************************
/*
/* I_main_bit_allocation   (Layer I)
/* II_main_bit_allocation  (Layer II)
/*
/* PURPOSE:For joint stereo mode, determines which of the 4 joint
/* stereo modes is needed.  Then calls *_a_bit_allocation(), which
/* allocates bits for each of the subbands until there are no more bits
/* left, or the MNR is at the noise/no_noise threshold.
/*
/* SEMANTICS:
/*
/* For joint stereo mode, joint stereo is changed to stereo if
/* there are enough bits to encode stereo at or better than the
/* no-noise threshold (fr_ps->mnr_min).  Otherwise, the system
/* iteratively allocates less bits by using joint stereo until one
/* of the following occurs:
/* - there are no more noisy subbands (MNR >= fr_ps->mnr_min)
/* - mode_ext has been reduced to 0, which means that all but the
/*   lowest 4 subbands have been converted from stereo to joint
/*   stereo, and no more subbands may be converted
/*
/*     This function calls *_bits_for_nonoise() and *_a_bit_allocation().
/*
/************************************************************************/
 
void I_main_bit_allocation(double (*perm_smr)[32], unsigned int (*bit_alloc)[32], int *adb, frame_params *fr_ps)
{
   int  noisy_sbs;
   int  mode, mode_ext, lay, i;
   int  rq_db, av_db = *adb;
static  int init = 0;
 
   if(init == 0) {
     /* rearrange snr for layer I */
     snr[2] = snr[3];
     for (i=3;i<16;i++) snr[i] = snr[i+2];
     init = 1;
   }
 
   if((mode = fr_ps->actual_mode) == MPG_MD_JOINT_STEREO) {
     fr_ps->header->mode = MPG_MD_STEREO;
     fr_ps->header->mode_ext = 0;
     fr_ps->jsbound = fr_ps->sblimit;
     if(rq_db = I_bits_for_nonoise(perm_smr, fr_ps) > *adb) {
       fr_ps->header->mode = MPG_MD_JOINT_STEREO;
       mode_ext = 4;           /* 3 is least severe reduction */
       lay = fr_ps->header->lay;
       do {
          --mode_ext;
          fr_ps->jsbound = js_bound(lay, mode_ext);
          rq_db = I_bits_for_nonoise(perm_smr, fr_ps);
       } while( (rq_db > *adb) && (mode_ext > 0));
       fr_ps->header->mode_ext = mode_ext;
     }    /* well we either eliminated noisy sbs or mode_ext == 0 */
   }
   noisy_sbs = I_a_bit_allocation(perm_smr, bit_alloc, adb, fr_ps);
}
 
 

/***************************** Layer II  ********************************/
 
void II_main_bit_allocation (
	double (*perm_smr)[32], /* minimum masking level *//* JMZ 08/03/1995 Multilingual */
	double (*ltmin)[32], /* minimum masking level *//* JMZ 08/03/1995 Multilingual */
	unsigned int (*scfsi)[32], /* JMZ 08/03/1995 Multilingual */
	unsigned int (*bit_alloc)[32], /* JMZ 08/03/1995 Multilingual */
	int *adb,
	frame_params *fr_ps,
	int *aiff,
	double (*sb_sample)[3][12][32], /* JMZ 08/03/1995 Multilingual */
	unsigned int (*scalar)[3][32], /* JMZ 08/03/1995 Multilingual */
	double (*max_sc)[32], /* JMZ 08/03/1995 Multilingual */
	double (*buffer)[1152], /* JMZ 08/03/1995 Multilingual */
	double (*spiki)[32], /* JMZ 08/03/1995 Multilingual */
	double (*joint_sample)[3][12][32],
	unsigned int (*j_scale)[3][32], 
	int dyn_cr, 
	int aug_dyn_cr, 
	unsigned int (*scfsi_dyn)[SBLIMIT], 
	unsigned int (*scalar_dyn)[3][SBLIMIT]
)
{
    int noisy_sbs, nn;
    int mode, mode_ext, lay, modemc_hlp;
    int rq_db, av_db = *adb, bits_nonoise, bits_indi;
    int a, b, i, l, m, sb, k;
    int sbbound = -30;
    float adb_help;
    double smr_pred[7][SBLIMIT];
    float preco[12][4];
    float prega[3][32];
    int bits1, bits2, ba, ba1, subgr;
    int maxAlloc;
    al_table *alloc = fr_ps->alloc;
    int stereoaug = fr_ps->stereoaug;
    static int sfsPerScfsi[] = { 3,2,1,2 };    /* lookup # sfs per scfsi */
    double sb_sample_sum[5][3][12][32];	    
    int subfr, ch, sum_chs;
    unsigned int scalar_sum[5][3][SBLIMIT];
    unsigned int scfsi_sum[5][SBLIMIT]; 

    /***************************layer II two channels *******************/
    if (*aiff == 0)
    {
	if ((mode = fr_ps->actual_mode) == MPG_MD_JOINT_STEREO)
	{
	    a = 0;
	    b = 1;
	    fr_ps->header->mode = MPG_MD_STEREO;
	    fr_ps->header->mode_ext = 0;
	    fr_ps->jsbound = fr_ps->sblimit;
	    if (rq_db = II_bits_for_nonoise (perm_smr, scfsi, fr_ps, a, b, aiff) > *adb)
	    {
		fr_ps->header->mode = MPG_MD_JOINT_STEREO;
		mode_ext = 4;           /* 3 is least severe reduction */
		lay = fr_ps->header->lay;
		do
		{
		  --mode_ext;
		  fr_ps->jsbound = js_bound (lay, mode_ext);
		  rq_db = II_bits_for_nonoise (perm_smr, scfsi, fr_ps, a, b, aiff);
		} while ((rq_db > *adb) && (mode_ext > 0));
		fr_ps->header->mode_ext = mode_ext;
	    }	/* well we either eliminated noisy sbs or mode_ext == 0 */
	}
    }
    /***************** layer II five channels ****************************/
    else
    {
	trans_chan (fr_ps);
#ifdef Augmentation_7ch
	if (stereoaug == 2)
	    trans_chan_aug (fr_ps); 		    
#endif
	if (verbosity >= 2) 
	    printf ("js actual_mode: %d  mode: %d  mode_ext: %d  jsbound: %d dyn_cr: %2d aug_dyn_cr: %2d\n",
	         fr_ps->actual_mode, fr_ps->header->mode, fr_ps->header->mode_ext, fr_ps->jsbound, dyn_cr, aug_dyn_cr);

	for (sb = 0; sb < SBLIMIT; ++sb)
	{
	    subgr = sbgrp[sb];
	    maxAlloc = (1 << (*alloc)[sb][0].bits) - 1;
	    for (ba = 0; ba < maxAlloc-1; ++ba)
	        if ((-perm_smr[0][sb] + snr[(*alloc)[sb][ba].quant+((ba>0)?1:0)])
				>= fr_ps->mnr_min)
		    break;		/* we found enough bits */
	    for (ba1 = 0; ba1 < maxAlloc-1; ++ba1)
		if ((-perm_smr[no_channel[fr_ps->header->tc_alloc[subgr]][0]][sb] +
		      snr[(*alloc)[sb][ba1].quant+((ba1>0)?1:0)])
				>= fr_ps->mnr_min)
		    break;		/* we found enough bits */
	    if (ba1 > ba)
	    {
	        perm_smr[0][sb] = perm_smr[no_channel[fr_ps->header->tc_alloc[subgr]][0]][sb];		
	        ba = ba1;
	    }
		 
	    maxAlloc = (1<<(*alloc)[sb][0].bits) - 1;
	    for (ba = 0; ba < maxAlloc-1; ++ba)
	        if ((-perm_smr[1][sb] + snr[(*alloc)[sb][ba].quant+((ba>0)?1:0)])
			       >= fr_ps->mnr_min)
		    break;		/* we found enough bits */
	    for (ba1 = 0; ba1 < maxAlloc-1; ++ba1)
	        if ((-perm_smr[no_channel[fr_ps->header->tc_alloc[subgr]][1]][sb] +
		      snr[(*alloc)[sb][ba1].quant+((ba1>0)?1:0)])
			       >= fr_ps->mnr_min)
		    break;		/* we found enough bits */
	    if (ba1 > ba)
	    {
	        perm_smr[1][sb] = perm_smr[no_channel[fr_ps->header->tc_alloc[subgr]][1]][sb];		
	        ba = ba1;
	    }
	}

	/************ 05/24/95, SR, dyn_cross talk **********************/	    
	if (dyn_cr != 0 || (fr_ps->header->dyn_cross_LR &&
			    (fr_ps->config == 302 || fr_ps->config == 202 || fr_ps->config == 102)))
	{
	    combine (fr_ps, sb_sample, sb_sample_sum); 	
    
	    for (ch = 0; ch < CHANMAX2; ch++)
	    {
	        for (subfr = 0; subfr < 3; subfr++)
		    for (sb = 0; sb < fr_ps->sblimit; sb++)
		        scalar_dyn[ch][subfr][sb] = scalar[ch][subfr][sb]; 
    
	        scfsi_calc_dyn (scalar_dyn, ch, fr_ps->sblimit, scfsi_dyn);
	        scfsi_calc (scalar, ch, fr_ps->sblimit, scfsi);
	    }
    
	    if (fr_ps->config == 320)
		sum_chs = 4;
	    else if (fr_ps->config == 302 || fr_ps->config == 310 || fr_ps->config == 220 || fr_ps->config == 202 || fr_ps->config == 102)
		sum_chs = 1;
	    else
		sum_chs = 0;
    
	    for (ch = 0; ch < sum_chs; ch++) 
	    {
		scf_calc (sb_sample_sum, fr_ps->sblimit, ch, scalar_sum);
		scfsi_calc_dyn (scalar_sum, ch, fr_ps->sblimit, scfsi_sum);
	    }
	    take_dyn_cross (dyn_cr, fr_ps, sb_sample_sum, scalar_sum, scfsi_sum, scfsi_dyn, 
			    sb_sample, scalar, scfsi); 				
	}

	/************ 05/24/95, SR, dyn_cross talk **********************/	    
#ifdef Augmentation_7ch
	if (aug_dyn_cr != 0)
	{
	    combine_aug (fr_ps, sb_sample, sb_sample_sum); 	
    
	    for (ch = CHANMAX2; ch < CHANMAX3; ch++)
	    {
	        for (subfr = 0; subfr < 3; subfr++)
		    for (sb = 0; sb < fr_ps->sblimit; sb++)
		        scalar_dyn[ch][subfr][sb] = scalar[ch][subfr][sb]; 
    
	        scfsi_calc_dyn (scalar_dyn, ch, fr_ps->sblimit, scfsi_dyn);
	        scfsi_calc (scalar, ch, fr_ps->sblimit, scfsi);
	    }
    
	    scf_calc (sb_sample_sum, fr_ps->sblimit, 4, scalar_sum);
	    scfsi_calc_dyn (scalar_sum, 4, fr_ps->sblimit, scfsi_sum);

	    take_dyn_cross_aug (aug_dyn_cr, fr_ps, sb_sample_sum, scalar_sum,
				scfsi_sum, scfsi_dyn, sb_sample, scalar, scfsi); 				
	}
#endif
	if ((mode = fr_ps->actual_mode) == MPG_MD_JOINT_STEREO)
	{
	    a = 0;
	    b = 1;
	    fr_ps->header->mode = MPG_MD_STEREO;
	    fr_ps->header->mode_ext = 0;
	    fr_ps->jsbound = fr_ps->sblimit;
	    adb_help = *adb;
	    bits_nonoise = II_bits_for_nonoise (perm_smr, scfsi, fr_ps, a, b, aiff);
	    bits_indi = II_bits_for_indi (perm_smr, scfsi, fr_ps, &a, &b, aiff);
	    rq_db = ((bits_nonoise / 10) % 2) * 10 * bits_nonoise + bits_indi;
	    if (verbosity >= 2) 
		printf ("rq_db: %5d ( %5d + %5d ) available: %5d\n",
		    rq_db, bits_nonoise, bits_indi, *adb);
	    if (rq_db > adb_help)
	    {
	       fr_ps->header->mode = MPG_MD_JOINT_STEREO;
	       mode_ext = 4;           /* 3 is least severe reduction */
	       lay = fr_ps->header->lay;
	       do
	       {
		 --mode_ext;
		 fr_ps->jsbound = js_bound (lay, mode_ext);
		 bits_nonoise = II_bits_for_nonoise (perm_smr, scfsi, fr_ps, a, b, aiff);
		 bits_indi = II_bits_for_indi (perm_smr, scfsi, fr_ps, &a, &b, aiff);
		 rq_db = ((bits_nonoise / 10) % 2) * 10 * bits_nonoise + bits_indi;
		 if (verbosity >= 2) 
		    printf ("rq_db: %5d ( %5d + %5d ) available: %5d\n",
			 rq_db, bits_nonoise, bits_indi, *adb);
	       } while ((rq_db >*adb) && (mode_ext > 0));
	       fr_ps->header->mode_ext = mode_ext;
	    }	 /* well we either eliminated noisy sbs or mode_ext == 0 */
	}
    }  /* end of else (five-channel) */

    noisy_sbs = II_a_bit_allocation (perm_smr, scfsi, bit_alloc, adb, fr_ps, aiff);
}
 
/************************************************************************
/*
/* I_a_bit_allocation  (Layer I)
/* II_a_bit_allocation (Layer II)
/*
/* PURPOSE:Adds bits to the subbands with the lowest mask-to-noise
/* ratios, until the maximum number of bits for the subband has
/* been allocated.
/*
/* SEMANTICS:
/* 1. Find the subband and channel with the smallest MNR (#min_sb#,
/*    and #min_ch#)
/* 2. Calculate the increase in bits needed if we increase the bit
/*    allocation to the next higher level
/* 3. If there are enough bits available for increasing the resolution
/*    in #min_sb#, #min_ch#, and the subband has not yet reached its
/*    maximum allocation, update the bit allocation, MNR, and bits
/*    available accordingly
/* 4. Repeat until there are no more bits left, or no more available
/*    subbands. (A subband is still available until the maximum
/*    number of bits for the subband has been allocated, or there
/*    aren't enough bits to go to the next higher resolution in the
/*    subband.)
/*
/************************************************************************/
 
int I_a_bit_allocation(double (*perm_smr)[32], unsigned int (*bit_alloc)[32], int *adb, frame_params *fr_ps)	/* return noisy sbs */

       /*far*/                      
                                   
         
                    
{
   int i, k, smpl_bits, scale_bits, min_sb, min_ch, oth_ch;
   int bspl, bscf, ad, noisy_sbs, done = 0;
   double mnr[2][SBLIMIT], small;
   char used[2][SBLIMIT];
   int stereo  = fr_ps->stereo;
   int sblimit = fr_ps->sblimit;
   int jsbound = fr_ps->jsbound;
   al_table *alloc = fr_ps->alloc;
   static char init= 0;
   static int bbal, banc, berr;
 
   if (!init) {
      banc = 32;     /* before: berr = 0; 92-08-11 shn */ 
      init = 1;

      if (fr_ps->header->error_protection) berr = 16; else berr= 0; /* added 92-08-11 shn */
   }
   bbal = 4 * ( (jsbound * stereo) + (SBLIMIT-jsbound) );
   *adb -= bbal + berr + banc;
   ad= *adb;
 
   for (i=0;i<SBLIMIT;i++) for (k=0;k<stereo;k++) {
     mnr[k][i]=snr[0]-perm_smr[k][i];
     bit_alloc[k][i] = 0;
     used[k][i] = 0;
   }
   bspl = bscf = 0;
 
   do  {
     /* locate the subband with minimum SMR */
     small = mnr[0][0]+1;    min_sb = -1; min_ch = -1;
     for (i=0;i<SBLIMIT;i++) for (k=0;k<stereo;k++)
       /* go on only if there are bits left */
       if (used[k][i] != 2 && small > mnr[k][i]) {
         small = mnr[k][i];
         min_sb = i;  min_ch = k;
       }
     if(min_sb > -1) {   /* there was something to find */
       /* first step of bit allocation is biggest */
       if (used[min_ch][min_sb])  { smpl_bits = 12; scale_bits = 0; }
       else                       { smpl_bits = 24; scale_bits = 6; }
       if(min_sb >= jsbound)        scale_bits *= stereo;
 
       /* check to see enough bits were available for */
       /* increasing resolution in the minimum band */
 
       if (ad  > bspl + bscf + scale_bits + smpl_bits) {
         bspl += smpl_bits; /* bit for subband sample */
         bscf += scale_bits; /* bit for scale factor */
         bit_alloc[min_ch][min_sb]++;
         used[min_ch][min_sb] = 1; /* subband has bits */
         mnr[min_ch][min_sb] = -perm_smr[min_ch][min_sb]
                               + snr[bit_alloc[min_ch][min_sb]];
         /* Check if subband has been fully allocated max bits */
         if (bit_alloc[min_ch][min_sb] ==  14 ) used[min_ch][min_sb] = 2;
       }
       else            /* no room to improve this band */
         used[min_ch][min_sb] = 2; /*   for allocation anymore */
       if(stereo == 2 && min_sb >= jsbound) {
         oth_ch = 1-min_ch;  /* joint-st : fix other ch */
         bit_alloc[oth_ch][min_sb] = bit_alloc[min_ch][min_sb];
         used[oth_ch][min_sb] = used[min_ch][min_sb];
         mnr[oth_ch][min_sb] = -perm_smr[oth_ch][min_sb]
                               + snr[bit_alloc[oth_ch][min_sb]];
       }
     }
   } while(min_sb>-1);     /* i.e. still some sub-bands to find */

   /* Calculate the number of bits left, add on to pointed var */
   ad -= bspl+bscf;
   *adb = ad;

   /* see how many channels are noisy */
   noisy_sbs = 0; small = mnr[0][0];
   for(k=0; k<stereo; ++k) {
     for(i = 0; i< SBLIMIT; ++i) {
		 if(mnr[k][i] < fr_ps->mnr_min)	 ++noisy_sbs;
       if(small > mnr[k][i])           small = mnr[k][i];
     }
   }
   return noisy_sbs;
}

/***************************** Layer II  ********************************/

int II_a_bit_allocation (double (*perm_smr)[32], unsigned int (*scfsi)[32],
			 unsigned int (*bit_alloc)[32], int *adb,
			 frame_params *fr_ps, int *aiff)
{
    int n_ml_ch = fr_ps->header->multiling_ch; 	/*JMZ 08/03/95 Multilingual */
    int i, min_ch, min_sb, oth_ch, k, increment, scale, seli, ba, j, l;
    int increment1, scale1, seli1;
    int adb_hlp, adb_hlp1, adb_hlp2;
    int bspl, bscf, bsel, ad, noisy_sbs;
    int bspl_mpg1, bscf_mpg1, bsel_mpg1;
    double mnr[14][SBLIMIT], small;		/* JMZ 08/03/1995 Multilingual */
    char used[14][SBLIMIT];				/* JMZ 08/03/1995 Multilingual */
    layer *info  = fr_ps->header;
    int center   = info->center;
    int surround = info->surround;
    int stereo	= fr_ps->stereo;
    int stereomc = fr_ps->stereomc;
    int stereoaug = fr_ps->stereoaug;
    int sblimit = fr_ps->sblimit;
    int sblimit_mc = fr_ps->sblimit_mc;
    int sblimit_ml = fr_ps->sblimit_ml;
    int jsbound = fr_ps->jsbound;
    al_table *alloc = fr_ps->alloc;
    al_table *alloc_mc = fr_ps->alloc_mc;
    al_table *alloc_ml = fr_ps->alloc_ml;
    double dynsmr = 0.0;  /* border of SMR for dynamic datarate */
    static char init= 0;
    static int banc, berr;
    int bbal, bancmc, bancext, bbal_mpg1;
    int ll, pred, pci, adb_mpg1, sbgr, sb, tca_bits, dynx_bits, aug_tca_bits;
    static int sfsPerScfsi[] = { 3,2,1,2 };    /* lookup # sfs per scfsi */

    if (!init)
    { 
	init = 1;
	banc = 32; /* banc: bits for header */;
	/* 960627 FdB reserve extra bits dependent wrt bugs */
	banc += 200;

	if (fr_ps->header->error_protection)
	    berr = 16;
	else
	    berr=0; /* added 92-08-11 shn */
    }

    pred = 0;
    bancmc = 0;
    bbal = 0;
    bbal_mpg1 = 0;
    bancext = 0;
    adb_mpg1 = bitrate[fr_ps->header->lay-1][fr_ps->header->bitrate_index] * 24;
    /* 960814 FdB next line added for debugging */
    /* adb_mpg1 -= banc; */

    if (*aiff == 1)
    {
	if (fr_ps->actual_mode == MPG_MD_JOINT_STEREO)
	{
	    for (i = 0; i < jsbound; ++i)
		bbal += (stereo) * (*alloc)[i][0].bits;
	    for (i = jsbound; i < sblimit; ++i)
		bbal += (stereo-1) * (*alloc)[i][0].bits;
	    for (i = 0; i < sblimit_mc; ++i)
		bbal += (stereomc+stereoaug) * (*alloc_mc)[i][0].bits;
	    for (i = 0; i < sblimit_ml; ++i)
		bbal += (n_ml_ch) * (*alloc_ml)[i][0].bits;
	}	  
	else
	{
	    for (i = 0; i < sblimit; ++i)
		bbal += (stereo) * (*alloc)[i][0].bits;
	    for (i = 0; i < sblimit_mc; ++i)
		bbal += (stereomc+stereoaug) * (*alloc_mc)[i][0].bits;
	    for (i = 0; i < sblimit_ml; ++i)
		bbal += (n_ml_ch) * (*alloc_ml)[i][0].bits;
	}

	if (fr_ps->header->center == 3) bbal -= 41; 
    }
    else
    {
	for (i = 0; i < jsbound; ++i)
	    bbal += stereo * (*alloc)[i][0].bits;
        for (i = jsbound; i < sblimit; ++i)
	    bbal += (*alloc)[i][0].bits;
    }

    if (fr_ps->header->dyn_cross_on == 1)
	for (i = 0; i < 12; i++)
	{
	    bbal -= dyn_bbal (fr_ps->config, fr_ps->header->center, fr_ps->header->dyn_cross[i], i);
	    if (fr_ps->header->surround == 3)
		bbal -= dyn_bbal_2ndst (fr_ps->header->dyn_second_stereo[i], i);
	}
	

    if (fr_ps->header->ext_bit_stream_present == 0)
        bancmc += 35;      /* mc_header + crc + tc_sbgr_select+ dyn_cross_on + 
			    mc_prediction_on  01/05/94,  SR  new! 05/04/94,  SR*/
    else
    {
        bancmc += 43;			    
	bancext = 40;
    }

    /* 960627 FdB TCA bits dependent on configuration */
    if (fr_ps->config == 320)
	tca_bits = 3;
    else if (fr_ps->config == 310)
	tca_bits = 3;
    else if (fr_ps->config == 220)
	tca_bits = 2;
    else if (fr_ps->config == 300 || fr_ps->config == 302 || fr_ps->config == 210)
	tca_bits = 2;
    else
	tca_bits = 0;

    if (fr_ps->header->tc_sbgr_select == 0)
        bancmc += 12 * tca_bits;
    else
        bancmc += tca_bits;	

    if (fr_ps->header->dyn_cross_on == 1)
    {
	/* 960627 FdB DynX dependent on configuration */
	if (fr_ps->config == 320)
	    dynx_bits = 4;
	else if (fr_ps->config == 310 || fr_ps->config == 220)
	    dynx_bits = 3;
	else if (fr_ps->config == 300 || fr_ps->config == 302 || fr_ps->config == 210)
	    dynx_bits = 1;
	else
	    dynx_bits = 0;

	bancmc += 12 * dynx_bits + 1;
	if (fr_ps->header->surround == 3)
	    bancmc += 12;      /* now with dyn_second_stereo,  17/02/95,  SR*/
    }

    if (fr_ps->header->mc_prediction_on == 1)
    {
/*	for (i = 0; i < 8; i++)
	{
	    bancmc += 1;
	    if (fr_ps->header->mc_pred[i] == 1)
	        bancmc += n_pred_coef[fr_ps->header->dyn_cross[i]] * 2;
	}
*/
	/* mc_pred, predsi, delay_comp, pred_coef */
	/* bancmc += 8 * (3 + 6 * (3 + 3 * 8)); */
	for (i = 0; i < 8; i++)
	{
	    bancmc += 3;
	    bancmc += n_pred_coef[fr_ps->header->dyn_cross[i]] * 27;
	}
    }

#ifdef Augmentation_7ch
    if (stereoaug == 2)
    {
	if (fr_ps->header->aug_mtx_proc == 0)
	    aug_tca_bits = 3;
	else if (fr_ps->header->aug_mtx_proc == 1)
	    aug_tca_bits = 2;
	else
	    aug_tca_bits = 0;

        bancmc += 12 * aug_tca_bits;

        if (fr_ps->header->aug_dyn_cross_on == 1)
	{
	    bancmc += 12 * 5;

	    for (i = 0; i < 12; i++)
		bbal -= dyn_bbal_aug (fr_ps->header->aug_dyn_cross[i], i);
	}
    }
#endif

    for (i = 0; i < SBLIMIT; i++) 
/*       for (k = 0; k < (stereo+stereomc+2); k++) 960814 FdB bug for some configurations */
        for (k = 0; k < 12; k++) 
        {
	    mnr[k][i] = snr[0] - perm_smr[k][i];
	    /* mask-to-noise-level = signal-to-noise-level - minimum-masking- */
	    /* threshold */
  
	    bit_alloc[k][i] = 0;
	    used[k][i] = 0;
        }
   
    for (i = 0; i < sblimit_ml; i++) 
	for (k = 7; k < 7 + n_ml_ch; k++) 
	{
	    mnr[k][i]=snr[0]-perm_smr[k][i];
	    /* mask-to-noise-level = signal-to-noise-level - minimum-masking-*/
	    /* threshold*/
    
	    bit_alloc[k][i] = 0;
	    used[k][i] = 0;
	}

    /* dyamic crosstalk, lock sbgr which are use for dyncr. */
    if (fr_ps->header->dyn_cross_on == 1)
    {
	for (sbgr = 0; sbgr < SBGRS; sbgr++)
	{		
	    for (sb = ((sbgr == 0)?0:sb_groups[sbgr-1]+1); sb <= sb_groups[sbgr]; sb++)
	    {
		/* 960627 FdB DynX dependent on configuration */
		if (fr_ps->config == 320)
		{
		    /* 3/2 */
		    switch(fr_ps->header->dyn_cross[sbgr])
		    {
			case 0: 
				break;
			case 1: case 8: case 10: 
				used[T4[sbgr]][sb] = 2;
				break;
			case 2: case 9: 
				used[T3[sbgr]][sb] = 2;
				break; 
			case 3: 
				used[T2[sbgr]][sb] = 2;
				break;
			case 4: case 11: case 12: case 14: 
				used[T3[sbgr]][sb] = 2; 
				used[T4[sbgr]][sb] = 2;
				break;
			case 5: case 13:
				used[T2[sbgr]][sb] = 2; 
				used[T4[sbgr]][sb] = 2;
				break;
			case 6: 
				used[T2[sbgr]][sb] = 2; 
				used[T3[sbgr]][sb] = 2;
				break;
			case 7: 
				used[T2[sbgr]][sb] = 2; 
				used[T4[sbgr]][sb] = 2; 	
				used[T3[sbgr]][sb] = 2;
				break;
		    }
		}
		else if (fr_ps->config == 310 || fr_ps->config == 220)
		{
		    /* 3/1 and 2/2 */
		    switch(fr_ps->header->dyn_cross[sbgr])
		    {
			case 0: 
				break;
			case 1: case 4: 
				used[T3[sbgr]][sb] = 2;
				break;
			case 2: 
				used[T2[sbgr]][sb] = 2;
				break; 
			case 3: 
				used[T2[sbgr]][sb] = 2;
				used[T3[sbgr]][sb] = 2;
				break;
		    }
		}
		else if (fr_ps->config == 300 || fr_ps->config == 302 || fr_ps->config == 210)
		{
		    /* 3/0 (+2/0) and 2/1 */
		    switch(fr_ps->header->dyn_cross[sbgr])
		    {
			case 0: 
				break;
			case 1: 
				used[T2[sbgr]][sb] = 2;
				break;
		    }
		    if (fr_ps->header->dyn_second_stereo[sbgr])
			used[T4[sbgr]][sb] = 2;
		}
		else if (fr_ps->config == 202)
		{
		    if (fr_ps->header->dyn_second_stereo[sbgr])
			used[T3[sbgr]][sb] = 2;
		}
		else if (fr_ps->config == 102)
		{
		    if (fr_ps->header->dyn_second_stereo[sbgr])
			used[T2[sbgr]][sb] = 2;
		}
	    }	/* for(sb.. */
	}	/* for(sbgr.. */
    }		/* if(fr_ps.. */ 
#ifdef Augmentation_7ch
    if (fr_ps->header->aug_dyn_cross_on == 1)
    {
	for (sbgr = 0; sbgr < SBGRS; sbgr++)
	{		
	    for (sb = ((sbgr == 0)?0:sb_groups[sbgr-1]+1); sb <= sb_groups[sbgr]; sb++)
	    {
		/* 5/2 */
		switch (fr_ps->header->aug_dyn_cross[sbgr])
		{
		    case 0: 
			    break;
		    case 1:
		    case 2:
		    case 3:
		    case 4:
			    used[T6[sbgr]][sb] = 2;
			    break;
		    case 5:
		    case 10:
		    case 14:
			    used[T5[sbgr]][sb] = 2;
			    break;
		    case 6: 
		    case 7: 
		    case 8: 
		    case 9: 
		    case 11: 
		    case 12: 
		    case 13: 
		    case 15: 
		    case 16: 
		    case 17: 
		    case 18: 
			    used[T5[sbgr]][sb] = 2;
			    used[T6[sbgr]][sb] = 2;
			    break;
		}
	    }	/* for(sb.. */
	}	/* for(sbgr.. */
    }		/* if(fr_ps.. */ 
#endif

/*
    if (fr_ps->header->mc_prediction_on == 1)
    {
        for (i = 0; i < 8; i++) 
        {
	    if (fr_ps->header->mc_pred[i] == 1)
	    {
	        for (j = 0; j < n_pred_coef[fr_ps->header->dyn_cross[i]]; j++)
	        {
		    if (fr_ps->header->predsi[i][j] != 0)
		    {
		        pred += 3;
		        for (pci = 0; pci < fr_ps->header->predsi[i][j]; pci++)
			    pred += 8;
		    }	
	        }
	    }
        }
    }  
*/
    adb_hlp = *adb;

    if (*aiff != 1)
    {
	*adb -= bbal + berr + banc;
    }
    else 
    {
    	if (fr_ps->header->ext_bit_stream_present == 0)    
	    *adb -= bbal + berr + banc + bancmc + pred +
	    	    (fr_ps->header->n_ad_bytes * 8);
	else
	{
	    *adb -= bbal + berr + banc + bancmc + pred +
	    	   bancext + (fr_ps->header->n_ad_bytes * 8);
	    
            for(i = 0; i < jsbound; ++i)
		bbal_mpg1 += stereo * (*alloc)[i][0].bits;
	    for(i = jsbound; i < sblimit; ++i)
		bbal_mpg1 += (*alloc)[i][0].bits;

	    adb_mpg1 -= bbal_mpg1 + berr + banc + bancmc +
			(fr_ps->header->n_ad_bytes * 8);
	}   
    }		    
    ad = *adb;

    bspl = bscf = bsel = bspl_mpg1 = bscf_mpg1 = bsel_mpg1 = 0;

    do
    {
        /* locate the subband with minimum SMR */
	small = 999999.0;
	min_sb = -1;
	min_ch = -1;

	for (i = 0; i < SBLIMIT; i++)		  /* searching for the sb min SMR */
	{
	    l = sbgrp[i];
	    for (j = 0; j < (stereo+stereomc+stereoaug); ++j)
		if ((j < stereo && i < sblimit) ||
		    (j >= stereo && i < sblimit_mc))
		{
		    k = transmission_channel (fr_ps, l, j);
    
		    if ((i >= 12) && (fr_ps->header->center == 3) && (k == 2))
			used[k][i] = 2;
    
		    if ((used[k][i] != 2) && (small > mnr[k][i])) 
		    {
			 small = mnr[k][i];
			 min_sb = i;
			 min_ch = k;
			 ll = l;		    /*sb-group*/
		    }
		}
	}

/******************************************************************/
/* Multilingual JMZ 08/03/1995 */
	if (n_ml_ch > 0)
	{
	    for (i = 0; i < sblimit_ml; i++)
		for (j = 7; j < (n_ml_ch+7); ++j)
		{
		    k = j;
		    if ((used[k][i] != 2) && (small > mnr[k][i])) 
		    {
			small = mnr[k][i];
			min_sb = i;
			min_ch = k;
			ll = l;		    
		    }
		}
	}
/* Multilingual JMZ 08/03/1995 */
/******************************************************************/


	if (min_sb > -1)
	{   /* there was something to find */
	    /* find increase in bit allocation in subband [min] */
	    if (min_ch < stereo) 
	    {
		increment = 12 * ((*alloc)[min_sb][bit_alloc[min_ch][min_sb]+1].group * 
				  (*alloc)[min_sb][bit_alloc[min_ch][min_sb]+1].bits);
		/* how many bits are needed */
		if (used[min_ch][min_sb])
		    increment -= 12 * ((*alloc)[min_sb][bit_alloc[min_ch][min_sb]].group * 
				       (*alloc)[min_sb][bit_alloc[min_ch][min_sb]].bits);
	    } 
	    else if (min_ch < 7 || n_ml_ch == 0) /* Multichannel */ 
	    {
		increment = 12 * ((*alloc_mc)[min_sb][bit_alloc[min_ch][min_sb]+1].group * 
				  (*alloc_mc)[min_sb][bit_alloc[min_ch][min_sb]+1].bits);
		/* how many bits are needed */
		if (used[min_ch][min_sb])
		    increment -= 12 * ((*alloc_mc)[min_sb][bit_alloc[min_ch][min_sb]].group * 
				       (*alloc_mc)[min_sb][bit_alloc[min_ch][min_sb]].bits);
	    } 
	    else /* MultiLingual 7/8/95 WtK */ 
	    {
		increment = ((*alloc_ml)[min_sb][bit_alloc[min_ch][min_sb]+1].group * 
			     (*alloc_ml)[min_sb][bit_alloc[min_ch][min_sb]+1].bits);
		if (used[min_ch][min_sb])
		    increment -= ((*alloc_ml)[min_sb][bit_alloc[min_ch][min_sb]].group * 
				  (*alloc_ml)[min_sb][bit_alloc[min_ch][min_sb]].bits);
		if (fr_ps->header->multiling_fs == 1) increment *=  6;
		else                                  increment *= 12;
	    }
    
	    /* scale factor bits required for subband [min] */
	    /* above js bound, need both chans */
    
	    if ((fr_ps->actual_mode == MPG_MD_JOINT_STEREO) && 
					((min_ch == 0) || (min_ch == 1)))
		oth_ch = 1 - min_ch;       
		
	    if (used[min_ch][min_sb])
		scale = seli = 0;
	    else 
	    {			  /* this channel had no bits or scfs before */
		seli = 2;
		scale = 6 * sfsPerScfsi[scfsi[min_ch][min_sb]];
    
		if ((fr_ps->actual_mode == MPG_MD_JOINT_STEREO) && (min_sb >= jsbound) &&
				      ((min_ch == 0) || (min_ch == 1))) 
		{
		    /* each new js sb has L+R scfsis */
		    seli += 2;
		    scale += 6 * sfsPerScfsi[scfsi[oth_ch][min_sb]];
		}
		if (fr_ps->header->dyn_cross_on == 1)
		{
		    dyn_bal (scfsi, ll, fr_ps, min_ch, min_sb, &seli, &scale);
		    /* 960819 FdB joint stereo in combination with DynX added */
		    if ((fr_ps->actual_mode == MPG_MD_JOINT_STEREO) && (min_sb >= jsbound) &&
			(stereo == 2) && ((min_ch == 0) || (min_ch == 1))) 
			dyn_bal (scfsi, ll, fr_ps, oth_ch, min_sb, &seli, &scale);
		}
#ifdef Augmentation_7ch
		if (fr_ps->header->aug_dyn_cross_on == 1)
		{
		    dyn_bal_aug (scfsi, ll, fr_ps, min_ch, min_sb, &seli, &scale);
		    /* 960819 FdB joint stereo in combination with DynX added */
		    if ((fr_ps->actual_mode == MPG_MD_JOINT_STEREO) && (min_sb >= jsbound) &&
			(stereo == 2) && ((min_ch == 0) || (min_ch == 1))) 
			dyn_bal_aug (scfsi, ll, fr_ps, oth_ch, min_sb, &seli, &scale);
		}
#endif
	    }
    
	    /* check to see enough bits were available for */
	    /* increasing resolution in the minimum band */

            if (fr_ps->header->ext_bit_stream_present == 1) 
            {
		if ((min_ch == 0) || (min_ch == 1 && stereo == 2))
		{
		    if (adb_mpg1 > bspl_mpg1 + bscf_mpg1 + bsel_mpg1 + seli + scale + increment) 
		    {
			bspl_mpg1 += increment;  /* bits for subband sample */
			bscf_mpg1 += scale;      /* bits for scale factor */
			bsel_mpg1 += seli;       /* bits for scfsi code */
		    }
		    else
			used[min_ch][min_sb] = 2;  /* can't increase this alloc */
		}
            }

	    if ((ad > bspl + bscf + bsel + seli + scale + increment) && (used[min_ch][min_sb] != 2)) 
	    {
		ba = ++bit_alloc[min_ch][min_sb]; /* next up alloc */
		bspl += increment;  /* bits for subband sample */
		bscf += scale;      /* bits for scale factor */
		bsel += seli;       /* bits for scfsi code */
		used[min_ch][min_sb] = 1; /* subband has bits */
      
		if (min_ch < stereo) 
		{
		    mnr[min_ch][min_sb] = -perm_smr[min_ch][min_sb] +
					  snr[(*alloc)[min_sb][ba].quant+1];
		    /* Check if subband has been fully allocated max bits */
		    if (ba >= (1<<(*alloc)[min_sb][0].bits) - 1)
			used[min_ch][min_sb] = 2;
		}
		else if (min_ch < 7 || n_ml_ch == 0)  /* Multichannel */
		{
		    mnr[min_ch][min_sb] = -perm_smr[min_ch][min_sb] +
					  snr[(*alloc_mc)[min_sb][ba].quant+1];
		    /* Check if subband has been fully allocated max bits */
		    if (ba >= (1<<(*alloc_mc)[min_sb][0].bits) - 1)
			used[min_ch][min_sb] = 2;
		}
		else /* MultiLingual 7/8/95 WtK */ 
		{
		    mnr[min_ch][min_sb] = -perm_smr[min_ch][min_sb] +
					    snr[(*alloc_ml)[min_sb][ba].quant+1];
		    if (ba >= (1<<(*alloc_ml)[min_sb][0].bits) - 1)
			used[min_ch][min_sb] = 2;
		}
	    }
	    else
		used[min_ch][min_sb] = 2;  /* can't increase this alloc */

	    if (verbosity >= 3)
	    {
		if (fr_ps->header->ext_bit_stream_present == 1)
		    printf("ch: %02d sb: %02d used: %d adb: %05d used: %05d ad: %05d used: %05d\n",
			    min_ch, min_sb, used[min_ch][min_sb],
			    adb_mpg1, bspl_mpg1 + bscf_mpg1 + bsel_mpg1,
			    ad, bspl + bscf + bsel);
		else
		    printf("ch: %02d sb: %02d used: %d ad: %05d used: %05d\n",
			    min_ch, min_sb, used[min_ch][min_sb],
			    ad, bspl + bscf + bsel);
	    }


	    if ((fr_ps->actual_mode == MPG_MD_JOINT_STEREO) && (min_sb >= jsbound) && (stereo == 2) &&
		    ((min_ch == 0) || (min_ch == 1))) 
	    {
		/* above jsbound, alloc applies L+R */
		ba = bit_alloc[oth_ch][min_sb] = bit_alloc[min_ch][min_sb];
		used[oth_ch][min_sb] = used[min_ch][min_sb];
		mnr[oth_ch][min_sb] = -perm_smr[oth_ch][min_sb] +
					snr[(*alloc)[min_sb][ba].quant+1];
	    }
	    if (fr_ps->header->dyn_cross_on == 1)
	    {
		choose_dyn (fr_ps, min_ch, min_sb, ll, bit_alloc);
		/* 960819 FdB joint stereo in combination with DynX added */
		if ((fr_ps->actual_mode == MPG_MD_JOINT_STEREO) && (min_sb >= jsbound) &&
		    (stereo == 2) && ((min_ch == 0) || (min_ch == 1))) 
		    choose_dyn (fr_ps, oth_ch, min_sb, ll, bit_alloc);
	    }
#ifdef Augmentation_7ch
	    if (fr_ps->header->aug_dyn_cross_on == 1)
	    {
		choose_dyn_aug (fr_ps, min_ch, min_sb, ll, bit_alloc);
		/* 960819 FdB joint stereo in combination with DynX added */
		if ((fr_ps->actual_mode == MPG_MD_JOINT_STEREO) && (min_sb >= jsbound) &&
		    (stereo == 2) && ((min_ch == 0) || (min_ch == 1))) 
		    choose_dyn_aug (fr_ps, oth_ch, min_sb, ll, bit_alloc);
	    }
#endif
	}   /* end of if-loop if min_sb >-1 */
    } while (min_sb > -1);   /* until could find no channel */
    /* Calculate the number of bits left */

    ad -= bspl+bscf+bsel; 
    *adb = ad;
    for (k = 0; k < stereo; k++)
	for (i = sblimit; i < SBLIMIT; i++)
	    bit_alloc[k][i] = 0;
    if (n_ml_ch > 0)
    {
	for (k = stereo; k < 7; k++)
	    for (i = sblimit_mc; i < SBLIMIT; i++)
		bit_alloc[k][i] = 0;
	for (i = sblimit_ml; i < SBLIMIT; i++)
	    for (k = 7; k < 14; k++)
		bit_alloc[k][i] = 0;
    }
    else
	for (k = stereo; k < 12; k++)
	    for (i = sblimit_mc; i < SBLIMIT; i++)
		bit_alloc[k][i] = 0;
 
    /* not used !?! perhaps later!! 8/21/93, SR */
    noisy_sbs = 0;
    small = mnr[0][0];      /* calc worst noise in case */
    for (k = 0; k < (stereo+stereomc+stereoaug); k++)
    {
        for (i = 0; i < sblimit; i++) 
	{
            if (small > mnr[k][i])
		small = mnr[k][i];
	    if (mnr[k][i] < fr_ps->mnr_min)
		noisy_sbs++; /* noise is not masked */
        }
    }

    if (n_ml_ch > 0)
    {
	for (k = 7; k < (7+n_ml_ch); k++)
	{
	    for (i = 0; i < sblimit_ml; i++) 
	    {
		if (small > mnr[k][i])
		    small = mnr[k][i];
		if (mnr[k][i] < fr_ps->mnr_min)
		    noisy_sbs++; /* noise is not masked */
	    }
	}
    }

    return noisy_sbs;
}
 
/************************************************************************
/*
/* I_subband_quantization  (Layer I)
/* II_subband_quantization (Layer II)
/* II_subband_quantisationmc (MPEG2)  SR
/* PURPOSE:Quantizes subband samples to appropriate number of bits
/*
/* SEMANTICS:  Subband samples are divided by their scalefactors, which
/* makes the quantization more efficient. The scaled samples are
/* quantized by the function a*x+b, where a and b are functions of
/* the number of quantization levels. The result is then truncated
/* to the appropriate number of bits and the MSB is inverted.
/*
/* Note that for fractional 2's complement, inverting the MSB for a
/* negative number x is equivalent to adding 1 to it.
/*
/************************************************************************/
 
static double I_a[15] = {
  0.750000000, 0.875000000, 0.937500000,
  0.968750000, 0.984375000, 0.992187500, 0.996093750, 0.998046875,
  0.999023438, 0.999511719, 0.999755859, 0.999877930, 0.999938965,
  0.999969482, 0.999984741 };
 
static double I_b[15] = {
  -0.250000000, -0.125000000, -0.062500000,
  -0.031250000, -0.015625000, -0.007812500, -0.003906250, -0.001953125,
  -0.000976563, -0.000488281, -0.000244141, -0.000122070, -0.000061035,
  -0.000030518, -0.000015259 };
 
static double II_a[17] = {
  0.750000000, 0.625000000, 0.875000000, 0.562500000, 0.937500000,
  0.968750000, 0.984375000, 0.992187500, 0.996093750, 0.998046875,
  0.999023438, 0.999511719, 0.999755859, 0.999877930, 0.999938965,
  0.999969482, 0.999984741 };
 
static double II_b[17] = {
  -0.250000000, -0.375000000, -0.125000000, -0.437500000, -0.062500000,
  -0.031250000, -0.015625000, -0.007812500, -0.003906250, -0.001953125,
  -0.000976563, -0.000488281, -0.000244141, -0.000122070, -0.000061035,
  -0.000030518, -0.000015259 };
 
void I_subband_quantization (unsigned int   (*scalar)[3][32],
			     double	    (*sb_samples)[3][12][32],
			     unsigned int   (*j_scale)[3][32],
			     double	    (*j_samps)[3][12][32],
			     unsigned int   (*bit_alloc)[32],
			     unsigned int   (*sbband)[3][12][32],
			     frame_params   *fr_ps)
{
   int i, j, k, n, sig;
   int stereo  = fr_ps->stereo;
   int sblimit = fr_ps->sblimit;
   int jsbound = fr_ps->jsbound;
   double d;

   for (j=0;j<12;j++) for (i=0;i<SBLIMIT;i++)
     for (k=0;k<((i<jsbound)?stereo:1);k++)
       if (bit_alloc[k][i]) {
         /* for joint stereo mode, have to construct a single subband stream
            for the js channels.  At present, we calculate a set of mono
            subband samples and pass them through the scaling system to
            generate an alternate normalised sample stream.
 
            Could normalise both streams (divide by their scfs), then average
            them.  In bad conditions, this could give rise to spurious
            cancellations.  Instead, we could just select the sb stream from
            the larger channel (higher scf), in which case _that_ channel
            would be 'properly' reconstructed, and the mate would just be a
            scaled version.  Spec recommends averaging the two (unnormalised)
            subband channels, then normalising this new signal without
            actually sending this scale factor... This means looking ahead.
         */
         if(stereo == 2 && i>=jsbound)
           /* use the joint data passed in */
			  d = j_samps[0][0][j][i] / multiple[j_scale[0][0][i]];
         else
           d = sb_samples[k][0][j][i] / multiple[scalar[k][0][i]];
         /* scale and quantize floating point sample */
         n = bit_alloc[k][i];
         d = d * I_a[n-1] + I_b[n-1];
         /* extract MSB N-1 bits from the floating point sample */
         if (d >= 0) sig = 1;
         else { sig = 0; d += 1.0; }
         sbband[k][0][j][i] = (unsigned int) (d * (double) (1L<<n));
         /* tag the inverted sign bit to sbband at position N */
         if (sig) sbband[k][0][j][i] |= 1<<n;
       }
}
 
/***************************** Layer II  ********************************/
 
void II_subband_quantization (unsigned int (*scalar)[3][32], double (*sb_samples)[3][12][32],
			      unsigned int (*j_scale)[3][32], double (*j_samps)[3][12][32],
			      unsigned int (*bit_alloc)[32], unsigned int (*sbband)[3][12][32],
			      frame_params *fr_ps)
{
   int i, j, k, s, n, qnt, sig, l, m;
   int stereo	= fr_ps->stereo;
   int stereomc = fr_ps->stereomc;
   int sblimit = fr_ps->sblimit;
   int jsbound = fr_ps->jsbound;
   unsigned int stps;
   double d;
   al_table *alloc = fr_ps->alloc;

   for (s=0;s<3;s++)
     for (j=0;j<12;j++)
       for (i=0;i<sblimit;i++)
         for (k=0;k<((i<jsbound)?stereo:1);k++)
	 {

	     if (bit_alloc[k][i])
	     {
               /* scale and quantize floating point sample */
               if(stereo == 2 && i>=jsbound)       /* use j-stereo samples */
	         d = j_samps[0][s][j][i] / multiple[j_scale[0][s][i]];
               else
                 d = sb_samples[k][s][j][i] / multiple[scalar[k][s][i]];

	       if (mod(d) >= 1.0) /* > changed to >=, 1992-11-06 shn */
	       {
         		printf("In compatible part, not scaled properly, %d %d %d %d\n",k,s,j,i);
			printf("Value %1.10f\n",sb_samples[k][s][j][i]);
			printf("Channel %d\n", k);
	       }
	       qnt = (*alloc)[i][bit_alloc[k][i]].quant;
               d = d * II_a[qnt] + II_b[qnt];
               /* extract MSB N-1 bits from the floating point sample */
               if (d >= 0) sig = 1;
               else { sig = 0; d += 1.0; }
               n = 0;
#ifndef MS_DOS
               stps = (*alloc)[i][bit_alloc[k][i]].steps;
               while ((1L<<n) < stps) n++;
#else
               while  ( ( (unsigned long)(1L<<(long)n) <
	              ( (unsigned long) ((*alloc)[i][bit_alloc[k][i]].steps)
                          & 0xffff)) && ( n <16)) n++;
#endif
               n--;
               sbband[k][s][j][i] = (unsigned int) (d * (double) (1L<<n));
               /* tag the inverted sign bit to sbband at position N */
               /* The bit inversion is a must for grouping with 3,5,9 steps
                so it is done for all subbands */
               if (sig) sbband[k][s][j][i] |= 1<<n;
	     } 
          }
           for (s=0;s<3;s++)
             for (j=sblimit;j<SBLIMIT;j++)
               for (i=0;i<12;i++) 
	         for (m=0;m<stereo;m++)
		 sbband[m][s][i][j] = 0;
}


void II_subband_quantization_mc (unsigned int (*scalar)[3][32], double (*sb_samples)[3][12][32],
				 unsigned int (*j_scale)[3][32], double (*j_samps)[3][12][32],
				 unsigned int (*bit_alloc)[32], unsigned int (*sbband)[3][12][32],
				 frame_params *fr_ps)
{
    int i, j, k, s, n, qnt, sig, m, l, ll;
    layer *info  = fr_ps->header;
    int center   = info->center;
    int surround = info->surround;
    int stereo	= fr_ps->stereo;
    int stereomc = fr_ps->stereomc;
    int sblimit  = fr_ps->sblimit_mc;
    unsigned int stps;
    double d;
    al_table *alloc = fr_ps->alloc_mc;

    for (s = 0; s < 3; s++)
        for (j = 0; j < 12; j++)
	    for (i = 0; i < sblimit; i++)
	    {
		ll = sbgrp[i];
                for (m = stereo; m < stereo + stereomc; ++m)
	        {
		    k = transmission_channel (fr_ps, ll, m);

		    if (fr_ps->header->dyn_cross_on == 1)
		    {
			/* 960627 FdB DynX dependent on configuration */
			if (dyn_ch (fr_ps, ll, m) == 0)
			    bit_alloc[k][i] = 0;
		    }

		    if (bit_alloc[k][i])
		    {
			d = sb_samples[k][s][j][i] / multiple[scalar[k][s][i]];
			if (mod (d) >= 1.0) /* > changed to >=, 1992-11-06 shn */
			{
			    printf ("In MC, not scaled properly, %d %d %d %d\n", k, s, j, i);
			    printf ("Value %1.10f\n", sb_samples[k][s][j][i]);
			}
			qnt = (*alloc)[i][bit_alloc[k][i]].quant;
			d = d * II_a[qnt] + II_b[qnt];
			/* extract MSB N-1 bits from the floating point sample */
			if (d >= 0) sig = 1;
			else { sig = 0; d += 1.0; }
			n = 0;
#ifndef MS_DOS
			stps = (*alloc)[i][bit_alloc[k][i]].steps;
			while ((1L<<n) < stps) n++;
#else
			while  ( ( (unsigned long)(1L<<(long)n) <
				     ((unsigned long) ((*alloc)[i][bit_alloc[k][i]].steps)
				      & 0xffff
				     )
				  ) && ( n <16)
				) n++;
#endif
			n--;
			sbband[k][s][j][i] = (unsigned int) (d * (double) (1L<<n));
			/* tag the inverted sign bit to sbband at position N */
			/* The bit inversion is a must for grouping with 3,5,9 steps
			   so it is done for all subbands */
			if (sig)
			    sbband[k][s][j][i] |= 1 << n;
		    }
		    else
			sbband[k][s][j][i] = 0;
		}
	    }

    for (s = 0; s < 3; s++)
	for (j = sblimit; j < SBLIMIT; j++)
	    for (i = 0; i < 12; i++)
		for (k = stereo; k < 7; k++)
		    sbband[k][s][i][j] = 0;
}

#ifdef Augmentation_7ch
void II_subband_quantization_aug (unsigned int (*scalar)[3][32], double (*sb_samples)[3][12][32],
				  unsigned int (*j_scale)[3][32], double (*j_samps)[3][12][32],
				  unsigned int (*bit_alloc)[32], unsigned int (*sbband)[3][12][32],
				  frame_params *fr_ps)
{
    int i, j, k, s, n, qnt, sig, m, l, ll;
    layer *info  = fr_ps->header;
    int center   = info->center;
    int surround = info->surround;
    int stereo	= fr_ps->stereo;
    int stereomc = fr_ps->stereomc;
    int sblimit  = fr_ps->sblimit_mc;
    unsigned int stps;
    double d;
    al_table *alloc = fr_ps->alloc_mc;

    for (s = 0; s < 3; s++)
        for (j = 0; j < 12; j++)
	    for (i = 0; i < sblimit; i++)
	    {
		ll = sbgrp[i];
                for (m = 5; m < 7; ++m)
	        {
		    k = transmission_channel (fr_ps, ll, m);

		    if (fr_ps->header->aug_dyn_cross_on == 1)
		    {
			/* 960627 FdB DynX dependent on configuration */
			if (dyn_ch (fr_ps, ll, m) == 0)
			    bit_alloc[k][i] = 0;
		    }

		    if (bit_alloc[k][i])
		    {
			d = sb_samples[k][s][j][i] / multiple[scalar[k][s][i]];
			if (mod (d) >= 1.0) /* > changed to >=, 1992-11-06 shn */
			{
			    printf ("In MC, not scaled properly, %d %d %d %d\n", k, s, j, i);
			    printf ("Value %1.10f\n", sb_samples[k][s][j][i]);
			}
			qnt = (*alloc)[i][bit_alloc[k][i]].quant;
			d = d * II_a[qnt] + II_b[qnt];
			/* extract MSB N-1 bits from the floating point sample */
			if (d >= 0) sig = 1;
			else { sig = 0; d += 1.0; }
			n = 0;
#ifndef MS_DOS
			stps = (*alloc)[i][bit_alloc[k][i]].steps;
			while ((1L<<n) < stps) n++;
#else
			while  ( ( (unsigned long)(1L<<(long)n) <
				     ((unsigned long) ((*alloc)[i][bit_alloc[k][i]].steps)
				      & 0xffff
				     )
				  ) && ( n <16)
				) n++;
#endif
			n--;
			sbband[k][s][j][i] = (unsigned int) (d * (double) (1L<<n));
			/* tag the inverted sign bit to sbband at position N */
			/* The bit inversion is a must for grouping with 3,5,9 steps
			   so it is done for all subbands */
			if (sig)
			    sbband[k][s][j][i] |= 1 << n;
		    }
		    else
			sbband[k][s][j][i] = 0;
		}
	    }

    for (s = 0; s < 3; s++)
	for (j = sblimit; j < SBLIMIT; j++)
	    for (i = 0; i < 12; i++)
		for (k = 7; k < 12; k++)
		    sbband[k][s][i][j] = 0;
}
#endif

void II_subband_quantization_ml (unsigned int (*scalar)[3][32], double (*sb_samples)[3][12][32],
				 unsigned int (*j_scale)[3][32], double (*j_samps)[3][12][32],
				 unsigned int (*bit_alloc)[32], unsigned int (*sbband)[3][12][32],
				 frame_params *fr_ps)
{
    int i, j, k, s, n, qnt, sig, m, l, ll;
    int n_ml_ch   = fr_ps->header->multiling_ch;
    int sblimit = fr_ps->sblimit_ml;
    int ml_lsf	 = fr_ps->header->multiling_fs;
    unsigned int stps;
    double d;
    al_table *alloc = fr_ps->alloc_ml;
   
    for (s=0;s<3;s++)
	for (j=0;j<((ml_lsf)?6:12);j++)    /* WtK half sample frequency 7/8/95 */
	    for (i=0;i<sblimit;i++)
	    {
		for (m = 7; m < 7+n_ml_ch; ++m)
		{
		    k = m;
		    if (bit_alloc[k][i])
		    {
			d = sb_samples[k][s][j][i] / multiple[scalar[k][s][i]];
			if (mod(d) >= 1.0) /* > changed to >=, 1992-11-06 shn */
			{ printf("In ML, not scaled properly, %d %d %d %d\n",k,s,j,i);
			  printf("Value %1.10f\n",sb_samples[k][s][j][i]);
			}
			qnt = (*alloc)[i][bit_alloc[k][i]].quant;
			d = d * II_a[qnt] + II_b[qnt];
			/* extract MSB N-1 bits from the floating point sample */
			if (d >= 0) sig = 1;
			else { sig = 0; d += 1.0; }
			n = 0;
			stps = (*alloc)[i][bit_alloc[k][i]].steps;
			while ((1L<<n) < stps) n++;
			n--;
			sbband[k][s][j][i] = (unsigned int) (d * (double) (1L<<n));
			/* tag the inverted sign bit to sbband at position N */
			/* The bit inversion is a must for grouping with 3,5,9 steps
			   so it is done for all subbands */
			if (sig) sbband[k][s][j][i] |= 1<<n;
		    }
		    else
			sbband[k][s][j][i] = 0;
		}
	    }

    for (s=0;s<3;s++)
	for (j=sblimit;j<SBLIMIT;j++)
	    for (i=0;i<((ml_lsf)?6:12);i++)
		for (k = 7; k < 7+n_ml_ch; k++)
		    sbband[k][s][i][j] = 0;
}





 
/************************************************************************
/*
/* I_encode_bit_alloc  (Layer I)
/* II_encode_bit_alloc (Layer II)
/* II_encode_bit_alloc_mc (Layer II multichannel)
#ifdef Augmentation_7ch
/* II_encode_bit_alloc_aug (Layer II multichannel 7ch-augmentation)
#endif
/*
/* PURPOSE:Writes bit allocation information onto bitstream
/*
/* Layer I uses 4 bits/subband for bit allocation information,
/* and Layer II uses 4,3,2, or 0 bits depending on the
/* quantization table used.
/*
/************************************************************************/
 
void I_encode_bit_alloc(unsigned int (*bit_alloc)[32], frame_params *fr_ps, Bit_stream_struc *bs)
{
   int i,k;
   int stereo  = fr_ps->stereo;
   int sblimit = fr_ps->sblimit;
   int jsbound = fr_ps->jsbound;
 
   for (i=0;i<SBLIMIT;i++)
     for (k=0;k<((i<jsbound)?stereo:1);k++) putbits(bs,bit_alloc[k][i],4);
}
 
/***************************** Layer II  ********************************/
 
void II_encode_bit_alloc (unsigned int (*bit_alloc)[32], frame_params *fr_ps, Bit_stream_struc *bs)
{
   int i,k;
   int stereo  = fr_ps->stereo;
   int sblimit = fr_ps->sblimit;
   int jsbound = fr_ps->jsbound;
   al_table *alloc = fr_ps->alloc;
 
   for (i=0;i<sblimit;i++)
	for (k=0;k<((i<jsbound)?stereo:1);k++)
		putbits(bs,bit_alloc[k][i],(*alloc)[i][0].bits);
}


void II_encode_bit_alloc_mc (unsigned int (*bit_alloc)[32], frame_params *fr_ps, Bit_stream_struc *bs)
{
    int i, k, l, m;
    layer *info     = fr_ps->header;
    int center      = info->center;
    int stereo      = fr_ps->stereo;
    int stereomc    = fr_ps->stereomc;
    int sblimit     = fr_ps->sblimit_mc;
    al_table *alloc = fr_ps->alloc_mc;
 

    for (i = 0; i < sblimit; i++)
    {
	l = sbgrp[i];
	for (m = stereo; m < stereo + stereomc; ++m)
        {        
	    k = transmission_channel (fr_ps, l, m);

            if ((fr_ps->header->center != 3) || (i < 12) || (k !=2))
		/* 960627 FdB DynX dependent on configuration */
		if (dyn_ch (fr_ps, l, m) == 1)
		    putbits (bs, bit_alloc[k][i], (*alloc)[i][0].bits);
	}
    }	
}

#ifdef Augmentation_7ch
void II_encode_bit_alloc_aug (unsigned int (*bit_alloc)[32], frame_params *fr_ps, Bit_stream_struc *bs)
{
    int i, k, l, m;
    layer *info     = fr_ps->header;
    int center      = info->center;
    int stereomc    = fr_ps->stereomc;
    int sblimit     = fr_ps->sblimit_mc;
    al_table *alloc = fr_ps->alloc_mc;
 

    for (i = 0; i < sblimit; i++)
    {
	l = sbgrp[i];
	for (m = 5; m < 7; ++m)
        {        
	    k = transmission_channel (fr_ps, l, m);

	    if (dyn_ch (fr_ps, l, m) == 1)
		putbits (bs, bit_alloc[k][i], (*alloc)[i][0].bits);
	}
    }	
}
#endif


/************************************************************************
/*
/* I_sample_encoding   (Layer I)
/* II_sample_encoding  (Layer II)
/* II_sample_encoding_mc  (Layer II) SR
#ifdef Augmentation_7ch
/* II_sample_encoding_aug  (Layer II) FdB
#endif
/*
/* PURPOSE:Put one frame of subband samples on to the bitstream
/*
/* SEMANTICS:  The number of bits allocated per sample is read from
/* the bit allocation information #bit_alloc#.  Layer 2
/* supports writing grouped samples for quantization steps
/* that are not a power of 2.
/*
/************************************************************************/
 
void I_sample_encoding (unsigned int (*sbband)[3][12][32], unsigned int (*bit_alloc)[32],
			frame_params *fr_ps, Bit_stream_struc *bs)
{
   int i,j,k;
   int stereo  = fr_ps->stereo;
   int sblimit = fr_ps->sblimit;
   int jsbound = fr_ps->jsbound;
 
   for(j=0;j<12;j++) {
     for(i=0;i<SBLIMIT;i++)
       for(k=0;k<((i<jsbound)?stereo:1);k++)
         if(bit_alloc[k][i]) putbits(bs,sbband[k][0][j][i],bit_alloc[k][i]+1);
   }
}
 
/***************************** Layer II  ********************************/
 
void II_sample_encoding (unsigned int (*sbband)[3][12][32], unsigned int (*bit_alloc)[32],
			 frame_params *fr_ps, Bit_stream_struc *bs)
{
   unsigned int temp;
   unsigned int i,j,k,s,x,y;
   int stereo	= fr_ps->stereo;
   int stereomc = fr_ps->stereomc;
   int sblimit = fr_ps->sblimit;
   int jsbound = fr_ps->jsbound;
   al_table *alloc = fr_ps->alloc;
 
   for (s=0;s<3;s++)
     for (j=0;j<12;j+=3)
       for (i=0;i<sblimit;i++)
         for (k=0;k<((i<jsbound)?stereo:1);k++)
           if (bit_alloc[k][i]) {
             if ((*alloc)[i][bit_alloc[k][i]].group == 3) {
               for (x=0;x<3;x++) putbits(bs,sbband[k][s][j+x][i],
                                      (*alloc)[i][bit_alloc[k][i]].bits);

            }
             else {
               y =(*alloc)[i][bit_alloc[k][i]].steps;
               temp = sbband[k][s][j][i] +
                      sbband[k][s][j+1][i] * y +
                      sbband[k][s][j+2][i] * y * y;
               putbits(bs,temp,(*alloc)[i][bit_alloc[k][i]].bits);

             }
           }
}




/******************* Layer II five channels ******************************/


void II_sample_encoding_mc (unsigned int (*sbband)[3][12][32], unsigned int lfe_sbband[12],
			    unsigned int (*bit_alloc)[32], unsigned int lfe_alloc,
			    frame_params *fr_ps, Bit_stream_struc *bs)
{
    unsigned int temp;
    unsigned int i,j,k,s,x,y, l, m;
    layer *info     = fr_ps->header;
    int center      = info->center;
    int stereo      = fr_ps->stereo;
    int stereomc    = fr_ps->stereomc;
    int sblimit     = fr_ps->sblimit_mc;
    al_table *alloc = fr_ps->alloc_mc;
    int lfe  =     fr_ps->header->lfe;
    
    for (s = 0; s < 3; s++)
        for (j = 0; j < 12; j += 3)
	{
	    if (lfe)
	        putbits (bs, lfe_sbband[s*4+j/3], (*alloc)[0][lfe_alloc].bits);

	    for (i = 0; i < sblimit; i++)
	    {
		l = sbgrp[i];
		for (m = stereo; m < stereo+stereomc; m++)
		{
		    k = transmission_channel (fr_ps, l, m);
	     
		    if (bit_alloc[k][i] && (i < 12 || m != 2 || center != 3)) 
		    {
		        if ((*alloc)[i][bit_alloc[k][i]].group == 3) 
			    for (x = 0; x < 3; x++)
				putbits (bs, sbband[k][s][j+x][i],
					 (*alloc)[i][bit_alloc[k][i]].bits);
		        else 
			{
			  y = (*alloc)[i][bit_alloc[k][i]].steps;
			  temp = sbband[k][s][j][i] +
				 sbband[k][s][j+1][i] * y +
				 sbband[k][s][j+2][i] * y * y;
			  putbits (bs, temp, (*alloc)[i][bit_alloc[k][i]].bits);
	   
			}
		    }
		}
	    }
	}	 
}

#ifdef Augmentation_7ch
void II_sample_encoding_aug (unsigned int (*sbband)[3][12][32], unsigned int (*bit_alloc)[32],
			     frame_params *fr_ps, Bit_stream_struc *bs)
{
    unsigned int temp;
    unsigned int i,j,k,s,x,y, l, m;
    int sblimit = fr_ps->sblimit_mc;
    al_table *alloc = fr_ps->alloc_mc;
    
    for (s = 0; s < 3; s++)
        for (j = 0; j < 12; j += 3)
	    for (i = 0; i < sblimit; i++)
	    {
		l = sbgrp[i];
		for (m = 5; m < 7; m++)
		{
		    k = transmission_channel (fr_ps, l, m);
	     
		    if (bit_alloc[k][i]) 
		    {
		        if ((*alloc)[i][bit_alloc[k][i]].group == 3) 
			    for (x = 0; x < 3; x++)
				putbits (bs, sbband[k][s][j+x][i],
					 (*alloc)[i][bit_alloc[k][i]].bits);
		        else 
			{
			  y = (*alloc)[i][bit_alloc[k][i]].steps;
			  temp = sbband[k][s][j][i] +
				 sbband[k][s][j+1][i] * y +
				 sbband[k][s][j+2][i] * y * y;
			  putbits (bs, temp, (*alloc)[i][bit_alloc[k][i]].bits);
			}
		    }
		}
	    }
}
#endif

/************************************************************************
/*
/* encode_CRC
/*
/************************************************************************/
 
void encode_CRC (unsigned int crc, Bit_stream_struc *bs)
{
   putbits (bs, crc, 16);
}

/***************************************************************************
/*
/* ancillary_encode 
/*
/* PURPOSE: The use of ancillary part of the bitstream for information  
/*          storage.
/*         
/*        
/**************************************************************************/

void ancillary_encode(fr_ps,bs,adb)
frame_params *fr_ps;
Bit_stream_struc *bs;
int adb;
{
    int bitsPerSlot; 
    int	samplesPerFrame;
    int	bit_rate;
    int	avg_slots_per_frame;
    int	whole_SpF;
    int	usedAdb;
    int	adbNumberStart;
    int l;
    char *mesg="This bitstream use ancillary part.";
    extern int mesg_index;

    register int i;


    double frac_SpF;
    double slot_lag;
    layer *info = fr_ps->header;


    if (info->lay == 1) { bitsPerSlot = 32; samplesPerFrame = 384;  }
    else               { bitsPerSlot = 8;  samplesPerFrame = 1152; }

    bit_rate = bitrate[info->lay-1][info->bitrate_index];

    avg_slots_per_frame = ((double)samplesPerFrame /
                           s_freq[info->sampling_frequency]) *
                          ((double)bit_rate / (double)bitsPerSlot);
    whole_SpF = (int) avg_slots_per_frame;	 

    adbNumberStart = (whole_SpF+info->padding) * bitsPerSlot;
    usedAdb = adbNumberStart - adb;

    if(usedAdb <= (adbNumberStart-8)) {
	if(usedAdb % 8) {
	   for(i=0;i<(8-(usedAdb%8));i++)
	      put1bit(bs,0);
	   usedAdb += (8 - (usedAdb % 8));
	}
	while(usedAdb < adbNumberStart - 8) {
	   for(i=0;i<8;i++)
	      put1bit(bs,0);
	   usedAdb += 8;
	}
	if(mesg_index >= strlen(mesg)){
	   for(i=0;i<8;i++)
	      put1bit(bs,0);
	} else {
	   putbits(bs,(int)(mesg[mesg_index++]), 8);
	}
   } else {
	for(i=0;i<adb;i++)
	   put1bit(bs,0);
   }

}


   	


	
   	


