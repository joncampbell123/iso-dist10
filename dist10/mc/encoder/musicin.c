/**********************************************************************
 * ISO MPEG Audio Subgroup Software Simulation Group (1996)
 * ISO 13818-3 MPEG-2 Audio Multichannel Encoder
 *
 * $Id: musicin.c 1.9 1996/02/12 07:13:35 rowlands Exp $
 *
 * $Log: musicin.c $
 * Revision 1.9  1996/02/12 07:13:35  rowlands
 * Release following Munich meeting
 *
 * Revision 1.6.2.1  1995/11/06  04:19:12  rowlands
 * Received from Uwe Felderhoff (IRT)
 *
 * Revision 1.8  1995/08/14  07:57:11  tenkate
 * ML-LSF added Warner ten Kate 7/8/95 (Philips)
 * debugging (whole packet)
 * obtain_params() and parse_args() cleaned up.
 * get_audio_ml() and Psycho_one_ml() only add odd frames upon LSF-ML.
 * II_smr() is called separately because of that.
 * sample shifting is added because of that.
 *
 * Revision 1.7  1995/07/31  07:49:58  tenkate
 * addition of phantom coding, at sevral lines, 25/07/95 WtK
 *
 * Revision 1.4.3.1  1995/06/16  08:25:11  rowlands
 * Input from Sang Wook Kim (Samsung AIT)
 *
 * Revision 1.4.2.1  1995/06/16  03:46:42  rowlands
 * Input from Susanne Ritscher (IRT)
 *
 * Revision 1.4.1.4  1995/06/16  02:49:33  rowlands
 * Added dematrix procedure 2
 *
 * Added support for dematrix procedure 2, without predistortion yet.
 * Calculated the compatible stereo signal subband samples by
 * transforming the compatible signal used for the psychoacoustics,
 * instead of recalculating in the subband domain from the other
 * channels.
 *
 **********************************************************************/

/**********************************************************************
 *   date   programmers         comment                               *
 * 3/01/91  Douglas Wong,       start of version 1.1 records          *
 *          Davis Pan                                                 *
 * 3/06/91  Douglas Wong,       rename: setup.h to endef.h            *
 *                              removed extraneous variables          *
 * 3/21/91  J.Georges Fritsch   introduction of the bit-stream        *
 *                              package. This package allows you      *
 *                              to generate the bit-stream in a       *
 *                              binary or ascii format                *
 * 3/31/91  Bill Aspromonte     replaced the read of the SB matrix    *
 *                              by an "code generated" one            *
 * 5/10/91  W. Joseph Carter    Ported to Macintosh and Unix.         *
 *                              Incorporated Jean-Georges Fritsch's   *
 *                              "bitstream.c" package.                *
 *                              Modified to strictly adhere to        *
 *                              encoded bitstream specs, including    *
 *                              "Berlin changes".                     *
 *                              Modified user interface dialog & code *
 *                              to accept any input & output          *
 *                              filenames desired.  Also added        *
 *                              de-emphasis prompt and final bail-out *
 *                              opportunity before encoding.          *
 *                              Added AIFF PCM sound file reading     *
 *                              capability.                           *
 *                              Modified PCM sound file handling to   *
 *                              process all incoming samples and fill *
 *                              out last encoded frame with zeros     *
 *                              (silence) if needed.                  *
 *                              Located and fixed numerous software   *
 *                              bugs and table data errors.           *
 * 27jun91  dpwe (Aware Inc)    Used new frame_params struct.         *
 *                              Clear all automatic arrays.           *
 *                              Changed some variable names,          *
 *                              simplified some code.                 *
 *                              Track number of bits actually sent.   *
 *                              Fixed padding slot, stereo bitrate    *
 *                              Added joint-stereo : scales L+R.      *
 * 6/12/91  Earle Jennings      added fix for MS_DOS in obtain_param  *
 * 6/13/91  Earle Jennings      added stack length adjustment before  *
 *                              main for MS_DOS                       *
 * 7/10/91  Earle Jennings      conversion of all float to FLOAT      *
 *                              port to MsDos from MacIntosh completed*
 * 8/ 8/91  Jens Spille         Change for MS-C6.00                   *
 * 8/22/91  Jens Spille         new obtain_parameters()               *
 *10/ 1/91  S.I. Sudharsanan,   Ported to IBM AIX platform.           *
 *          Don H. Lee,                                               *
 *          Peter W. Farrett                                          *
 *10/ 3/91  Don H. Lee          implemented CRC-16 error protection   *
 *                              newly introduced functions are        *
 *                              I_CRC_calc, II_CRC_calc and encode_CRC*
 *                              Additions and revisions are marked    *
 *                              with "dhl" for clarity                *
 *11/11/91 Katherine Wang       Documentation of code.                *
 *                                (variables in documentation are     *
 *                                surround by the # symbol, and an '*'*
 *                                denotes layer I or II versions)     *
 * 2/11/92  W. Joseph Carter    Ported new code to Macintosh.  Most   *
 *                              important fixes involved changing     *
 *                              16-bit ints to long or unsigned in    *
 *                              bit alloc routines for quant of 65535 *
 *                              and passing proper function args.     *
 *                              Removed "Other Joint Stereo" option   *
 *                              and made bitrate be total channel     *
 *                              bitrate, irrespective of the mode.    *
 *                              Fixed many small bugs & reorganized.  *
 * 2/25/92  Masahiro Iwadare    made code cleaner and more consistent *
 *10 jul 92 Susanne Ritscher    Bug fix in main, scale factor calc.   *
 * 5 aug 92 Soren H. Nielsen    Printout of bit allocation.           *
 *19 aug 92 Soren H. Nielsen	Changed MS-DOS file name extensions.  *
 * 2 dec 92 Susanne Ritscher	Start of changes to multi-channel with*
 *                                                  several options   *										 *
 **********************************************************************
 *                                                                    *
 *                                                                    *
 *  MPEG/audio Phase 2 coding/decoding multichannel                   *
 *                                                                    *
 *                                                                    *
 *  7/27/93        Susanne Ritscher,  IRT Munich                      *
 *                                                                    *
 *	                                                              *
 *  8/13/93        implemented channel-switching by changing          *
 *                 a lot in encode.c                                  *
 *                                                                    *
 *  8/27/93        Susanne Ritscher, IRT Munich                       *
 *                 Channel-Switching is working                       *
 *  9/1/93         Susanne Ritscher,  IRT Munich                      *
 *                 all channels normalized                            *
 *  9/20/93        channel-switching is only performed at a           *
 *                 certain limit of TC_ALLOC dB, which is included    *
 *                 in encoder.h                                       *
 *  1/04/94        try get all the rubbish out!                       *
 *                                                                    *
 *                                                                    *
 *  Version 1.0                                                       *
 *                                                                    *
 *  07/12/94       Susanne Ritscher,  IRT Munich                      *
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
 *  06/06/95	   Yeon Bae Thomas Kim,  Samsung AIT                  *
 *                 ancillary data is working                          *
 *                                                                    *
 *  06/06/95	   Sang Wook Kim,  Samsung AIT                        *
 *                 corrected some bugs                                *
 *                                                                    *
 **********************************************************************/
#ifdef MS_DOS
#include <dos.h>
#endif
#include "common.h"
#include "encoder.h"
#include <math.h>

/* Global variable definitions for "musicin.c" */

FILE               *musicin;
FILE               *musicin_ml;	/* 08/03/1995 JMZ Multilingual */
Bit_stream_struc   bs;
Bit_stream_struc   bs_ext;
Bit_stream_struc   bs_mpg;
char               *programName;
int		   mesg_index=0; /* YBKim060695 ancillary */
int		   verbosity;

char multilingual_file_name[MAX_NAME_SIZE];	/* 08/03/1995 JMZ Multilingual */

/* Implementations */

/************************************************************************
/*
/* obtain_parameters
/*
/* PURPOSE:  Prompts for and reads user input for encoding parameters
/*
/* SEMANTICS:  The parameters read are:
/* - input and output filenames
/* - sampling frequency (if AIFF file, will read from the AIFF file header)
/* - layer number
/* - mode (stereo, joint stereo, dual channel or mono)
/* - psychoacoustic model (I or II)
/* - total bitrate, irrespective of the mode
/* - de-emphasis, error protection, copyright and original or copy flags
/*
/************************************************************************/

void
obtain_parameters (
	frame_params *fr_ps,
	int *psy,
	long unsigned int *num_samples,
	long unsigned int *num_samples_ml,
	char *original_file_name,
	char *encoded_file_name,
	IFF_AIFF *pcm_aiff_data,
	IFF_AIFF *pcm_aiff_data_ml,
	int *aiff,
	int *byte_per_sample,
	int *mat_mode,
	int *aug_mat_mode,
	int *cha_sw,
	int *aug_cha_sw,
	char *encoded_file_name_ext,
	int *bit_rate,
	char *encoded_file_name_mpg,
	int *dyn_cr,
	int *dyn_cr_LR,
	int *aug_dyn_cr,
	int *ancillaryUse,
	int *verbosity)
{
    int j, bool, max_rate, max_rate_index;
    long int i;
    int tot_channels = 0;		/* 21/03/1995 JMZ Multimode */ 
    char configuration[50];	/* 21/03/1995 JMZ Multimode */
    char t[50];

    layer *info = fr_ps->header;

    *aiff = 0;	 /* flag for AIFF-Soundfile*/
	 
    do
    {
       printf ("Enter PCM input file name <required>: ");
       gets (original_file_name);
       if (original_file_name[0] == NULL_CHAR)
       printf ("PCM input file name is required.\n");
    } while (original_file_name[0] == NULL_CHAR);

    printf(">>> PCM input file name is: %s\n", original_file_name);
 
    if ((musicin = fopen (original_file_name, "rb")) == NULL)
    {
       printf ("Could not find \"%s\".\n", original_file_name);
       exit (1);
    }

/***************************************************************************/
/* 08/03/1995 Multi lingual extension JMZ                                  */

    printf ("Do you have multi lingual channels? (Y)es, (<N>)o: ");
    gets (multilingual_file_name);
    if ((*multilingual_file_name == 'Y') || (multilingual_file_name[0] == 'y'))
    {
	printf ("Enter multilingual input file name <%s.%s>: ", original_file_name, DFLT_ML_EXT);
	gets (multilingual_file_name);
	if (multilingual_file_name[0] == NULL_CHAR)
	{
	    printf ("Null char\n");
	    strcat (strcpy (multilingual_file_name, original_file_name), DFLT_ML_EXT);
	}
	if ((musicin_ml = fopen (multilingual_file_name, "rb")) == NULL) 
	{
	    printf ("Could not find \"%s\".\n", multilingual_file_name);
	    exit (1);
	}

	/*remplir fr_ps il y a au moins un multilingual*/
	if (aiff_read_headers (musicin_ml, pcm_aiff_data_ml, byte_per_sample) == 0) 
	{
	    printf (">>> Using Audio IFF multilingual file headers\n");
	    info->multiling_ch = pcm_aiff_data_ml->numChannels;
	    printf (">>> Using %d multilingual channels\n",info->multiling_ch);
	    *num_samples_ml = pcm_aiff_data_ml->numChannels *
			      pcm_aiff_data_ml->numSampleFrames;
	}
	else     /* Not using Audio IFF sound file headers. */
	{
	    printf ("***WARNING: Could not read AIFF header - No MultiLingual coding!!\n");
	    info->multiling_ch = 0;
	}
    }
    else
    {
	printf ("no multi lingual coding!!\n");
	info->multiling_ch = 0;
    }

/* 08/03/1995 Multi lingual extension JMZ                                  */
/***************************************************************************/
	
    printf("Enter MPEG encoded output file name <%s%s>: ",
		original_file_name, DFLT_EXT);

    gets (encoded_file_name);

    if (encoded_file_name[0] == NULL_CHAR)
    {
	strcat (strcpy (encoded_file_name_ext, original_file_name), DFLT_EXT_EXT);
	strcat (strcpy (encoded_file_name, original_file_name), DFLT_EXT);
	strcat (strcpy (encoded_file_name_mpg, encoded_file_name), DFLT_EXT);
    }
    else
    {
	strcat (strcpy (encoded_file_name_ext, encoded_file_name), DFLT_EXT_EXT);
	strcat (encoded_file_name, DFLT_EXT);
	strcat (strcpy (encoded_file_name_mpg, encoded_file_name), DFLT_EXT);
    }
    printf(">>> MPEG encoded output file name is: %s\n", encoded_file_name);
 
    open_bit_stream_w (&bs, encoded_file_name, BUFFER_SIZE);
 
    if (aiff_read_headers (musicin, pcm_aiff_data, byte_per_sample) == 0)
    {

	*aiff = 1;
	printf(">>> Using Audio IFF sound file headers\n");

/**********************************************************/
/* 21/03/1995 JMZ Multimode */

	tot_channels = pcm_aiff_data->numChannels;
	printf (">>> Using %d audio channels\n", tot_channels);

	/********* 27/01/97, FdB, select LFE ********************/
	if (tot_channels > 1)
	{
	    printf ("Do you want to use LFE coding? (y/<n>) ");
	    gets (t);
	    if ((*t == 'y') || (*t == 'Y'))
	    {
		printf (">>> LFE coding is used.\n");
		info->lfe = 1;
	    } 
	    else
	    {
		printf (">>> No LFE coding.\n");	 
		info->lfe = 0;
	    }
        }

#ifdef Augmentation_7ch
	if (tot_channels - info->lfe == 7) 
	{
	    if (info->multiling_ch > 0)
	    {
		printf ("7.1 channel augmentation and multilingual cannot be combined\n");
		exit (1);
	    }
	    fr_ps->stereomc = 3;
	    fr_ps->stereoaug = 2;
	    fr_ps->lfe_pos = 3;
	    info->center = 1; 
	    info->surround = 2; 
	    fr_ps->config = 320;
	    printf (">>> Using 5/2 configuration.\n"); 
	}
	else
#endif
	if (tot_channels - info->lfe == 5) 
	{
	    fr_ps->stereomc = 3;
	    fr_ps->stereoaug = 0;
	    fr_ps->lfe_pos = 3;
	    printf ("Which configuration do you want to use?\n");
	    printf ("Available: 3/2 (<1>), 3/0 + 2/0 (2): ");
	    gets (configuration);
	    switch (*configuration)
    	    {
	    case '1': 
		    info->center = 1; 
		    info->surround = 2; 
		    fr_ps->config = 320;
		    printf (">>> Using 3/2 configuration.\n"); 
		    break;
	    case '2': 
		    info->center = 1; 
		    info->surround = 3; 
		    fr_ps->config = 302;
		    printf (">>> Using 3/0 + 2/0 configuration.\n"); 
		    break;
	    default:  
		    info->center = 1; 
		    info->surround = 2; 
		    fr_ps->config = 320;
		    printf (">>> Using default 3/2 configuration.\n"); 
		    break;
	    }
	}
	else if (tot_channels - info->lfe == 4) 
	{
	    fr_ps->stereomc = 2;
	    fr_ps->stereoaug = 0;
	    printf ("Which configuration do you want to use?\n");
	    printf ("Available: 3/1 (<1>), 2/2 (2), 2/0 + 2/0 (3): ");
	    gets (configuration);
	    switch (*configuration)
    	    {
	    case '1': 
		    info->center = 1; 
		    info->surround = 1; 
		    fr_ps->config = 310;
		    fr_ps->lfe_pos = 3;
		    printf (">>> Using 3/1 configuration.\n"); 
		    break;
	    case '2': 
		    info->center = 0; 
		    info->surround = 2; 
		    fr_ps->config = 220;
		    fr_ps->lfe_pos = 2;
		    printf (">>> Using 2/2 configuration.\n"); 
		    break;
	    case '3': 
		    info->center = 0; 
		    info->surround = 3; 
		    fr_ps->config = 202;
		    fr_ps->lfe_pos = 2;
		    printf (">>> Using 2/0 + 2/0 configuration.\n"); 
		    break;
	    default:  
		    info->center = 1; 
		    info->surround = 1; 
		    fr_ps->config = 310;
		    fr_ps->lfe_pos = 3;
		    printf (">>> Using default 3/1 configuration.\n"); 
		    break;
	    }
	}
	else if (tot_channels - info->lfe == 3) 
	{
	    fr_ps->stereoaug = 0;
	    printf ("Which configuration do you want to use?\n");
	    printf ("Available: 3/0 (<1>), 2/1 (2), 1/0 + 2/0 (3): ");
	    gets (configuration);
	    switch (*configuration)
    	    {
	    case '1': 
		    info->center = 1; 
		    info->surround = 0; 
		    fr_ps->config = 300;
		    fr_ps->stereomc = 1;
		    fr_ps->lfe_pos = 3;
		    printf (">>> Using 3/0 configuration.\n"); 
		    break;
	    case '2': 
		    info->center = 0; 
		    info->surround = 1; 
		    fr_ps->config = 210;
		    fr_ps->stereomc = 1;
		    fr_ps->lfe_pos = 2;
		    printf (">>> Using 2/1 configuration.\n"); 
		    break;
	    case '3': 
		    info->center = 0; 
		    info->surround = 3; 
		    fr_ps->config = 102;
		    fr_ps->stereomc = 2;
		    fr_ps->lfe_pos = 1;
		    printf (">>> Using 1/0 + 2/0 configuration.\n"); 
		    break;
	    default:  
		    info->center = 1; 
		    info->surround = 0; 
		    fr_ps->config = 300;
		    fr_ps->stereomc = 1;
		    fr_ps->lfe_pos = 3;
		    printf (">>> Using default 3/0 configuration.\n"); 
		    break;
	    }
	}
	else if (tot_channels - info->lfe == 2) 
	{
	    fr_ps->config = 200;
	    fr_ps->stereomc = 0;
	    fr_ps->stereoaug = 0;
	    fr_ps->lfe_pos = 2;
	}
	else if (tot_channels - info->lfe == 1) 
	{
	    if (info->lfe)
	    {
		printf ("single channel plus LFE not supported.\n");
		exit (1);
	    }
	    fr_ps->config = 100;
	    fr_ps->stereomc = 0;
	    fr_ps->stereoaug = 0;
	}
	else
	{
	    printf ("Channels = %d not supported.\n", tot_channels);
	    exit (1);
	}

	aiff_check (original_file_name, pcm_aiff_data);

/* 21/03/1995 JMZ Multimode */
/**********************************************************/


        info->sampling_frequency = SmpFrqIndex ((long) pcm_aiff_data->sampleRate);
        printf (">>> %.f Hz sampling frequency selected\n", pcm_aiff_data->sampleRate);

        /* Determine number of samples in sound file */
        *num_samples = pcm_aiff_data->numChannels * pcm_aiff_data->numSampleFrames;

	if (info->multiling_ch > 0)
	{
	    switch (SmpFrqIndex ((long) pcm_aiff_data_ml->sampleRate) - info->sampling_frequency)
	    {
	    case 16 : printf ("MultiLingual is LSF coded.\n");
		      info->multiling_fs = 1;
		      break;
	    case  0 : printf ("MultiLingual sample rate equals that of Audio.\n");
		      info->multiling_fs = 0;
		      break;
	    default : printf ("***WARNING: MultiLingual sample rate does not equal (half) mc sample rate.\n");
	    	      printf ("            No MultiLingual coding!!!\n");
		      info->multiling_ch = 0;
		      break;
	    } /*switch*/
	} /*if (ml_ch>0)*/
    }
    else
    {    /* Not using Audio IFF sound file headers. */
	printf("no multichannel coding!!\n");
	fr_ps->stereomc = 0;
	fr_ps->stereoaug = 0;
	info->multiling_ch = 0;
        printf ("What is the sampling frequency? <48000>[Hz]: ");
        gets (t);
        i = atol(t);
        switch (i)
	{
        case 48000 : info->sampling_frequency = 1;
	             printf (">>> %ld Hz sampling freq selected\n", i);
		     break;
        case 44100 : info->sampling_frequency = 0;
		     printf (">>> %ld Hz sampling freq selected\n", i);
		     break;
        case 32000 : info->sampling_frequency = 2;
		     printf (">>> %ld Hz sampling freq selected\n", i);
		     break;
        default:     info->sampling_frequency = 1;
		     printf (">>> Default 48 kHz samp freq selected\n");
        }
 
        if (fseek (musicin, 0, SEEK_SET) != 0)
	{
          printf ("Could not seek to PCM sound data in \"%s\".\n",
                  original_file_name);
          exit (1);
        }
 
        /* Declare sound file to have "infinite" number of samples. */
        *num_samples = MAX_U_32_NUM;
    }

    printf ("Which layer do you want to use?\n");
    printf ("Available: Layer (1), Layer (<2>): ");
    gets (t);
    switch (*t)
    {
    case '1': info->lay = 1; printf (">>> Using Layer %s\n", t); break;
    case '2': info->lay = 2; printf (">>> Using Layer %s\n", t); break;
    default : info->lay = 2; printf (">>> Using default Layer 2\n"); break;
    }

    printf ("Which mode do you want for the two front channels?\n");
    printf ("Available: (<s>)tereo, (j)oint stereo, ");
    printf ("(d)ual channel, s(i)ngle Channel, n(o)ne: ");
    gets (t);
    if (fr_ps->config < 200) /* compatible is mono */
    {
	info->mode = MPG_MD_MONO;
	info->mode_ext = 0;
	printf (">>> Using default mono mode\n");
    }
    else
	switch (*t)
	{
	case 's':
	case 'S': info->mode = MPG_MD_STEREO; info->mode_ext = 0;
		  printf (">>> Using mode %s\n",t);
		  break;
	case 'j':
	case 'J': info->mode = MPG_MD_JOINT_STEREO;
		  printf (">>> Using mode %s\n",t);
		  break;
	case 'd':
	case 'D': info->mode = MPG_MD_DUAL_CHANNEL; info->mode_ext = 0;
		  printf (">>> Using mode %s\n",t);
		  break;
	case 'i':
	case 'I': info->mode = MPG_MD_MONO; info->mode_ext = 0;
		  printf (">>> Using mode %s\n",t);
		  break;
	case 'o':
	case 'O': info->mode = MPG_MD_NONE; info->mode_ext = 0;
		  printf (">>> Using mode %s\n",t);
		  break;
	default:  info->mode = MPG_MD_STEREO; info->mode_ext = 0;
		  printf (">>> Using default stereo mode\n");
		  break;
	}

    *psy = DFLT_PSY;

    if (info->mode == MPG_MD_MONO)
	max_rate_index = 10;
    else
	max_rate_index = 14;
/*    max_rate_index = 6; */ /* 48 kHz => 96 kbps for multiple table testing */ 
    max_rate = bitrate[info->lay-1][max_rate_index];
    printf ("What is the total bitrate? <%u>[kbps]: ", DFLT_BRT);
    gets (t);
    i = atol (t);
    if (i == 0) i = -10;
    j = 0;
    while (j <= max_rate_index) 
    {
	if (bitrate[info->lay-1][j] == (int) i)
	    break;
	j++;
    }
    if (j > max_rate_index) 
    {
        if (i < max_rate)
        {
	    if (DFLT_BRT <= max_rate)
	    {
	    	i = DFLT_BRT;
		printf (">>> Using default %u kbps\n", DFLT_BRT);
	    }
	    else
	    {
	       	i = max_rate;
		printf (">>> Using max. bitrate %u kbps\n", max_rate);
	    }
	    for (j = 0; j < 15; j++)
		if (bitrate[info->lay-1][j] == (int) i) 
		{
		    info->bitrate_index = j;
		    break;
		}
	}
	else
	{
	    info->ext_bit_stream_present = 1;
	    *bit_rate = i;
	    if (DFLT_BRT <= max_rate)
	    	i = DFLT_BRT;
	    else
	       	i = max_rate;
	    for (j = 0; j < 15; j++)
		if (bitrate[info->lay-1][j] == (int) i) 
		{
		    info->bitrate_index = j;
		    break;
		}
	    info->ext_length = ((*bit_rate - i) * 144) / s_freq[info->sampling_frequency];
	    printf (">>> Bitrate = %d kbps\n", *bit_rate);
	}			   
    }
    else
    {
        info->bitrate_index = j;
        printf (">>> Bitrate = %d kbps\n", bitrate[info->lay-1][j]);
    }

    printf ("What type of de-emphasis should the decoder use?\n");
    printf ("Available: (<n>)one, (5)0/15 microseconds, (c)citt j.17: ");
    gets (t);
    if (*t != 'n' && *t != '5' && *t != 'c')
    {
        printf (">>> Using default no de-emphasis\n");
        info->emphasis = 0;
    }
    else
    {
        if (*t == 'n')      info->emphasis = 0;
        else if (*t == '5') info->emphasis = 1;
        else if (*t == 'c') info->emphasis = 3;
        printf (">>> Using de-emphasis %c\n", *t);
    }
 
/*  Start 2. Part changes for CD Ver 3.2; jsp; 22-Aug-1991 */
 
    printf ("Do you want to set the private bit? (y/<n>): ");
    gets (t);
    if (*t == 'y' || *t == 'Y') info->extension = 1;
    else                        info->extension = 0;
    if (info->extension) printf (">>> Private bit set\n");
    else                 printf (">>> Private bit not set\n");
 
/*  End changes for CD Ver 3.2; jsp; 22-Aug-1991 */
	
    printf ("Do you want error protection? (y/<n>): ");
    gets (t);
    if (*t == 'y' || *t == 'Y') info->error_protection = TRUE;
    else                        info->error_protection = FALSE;
    if (info->error_protection) printf (">>> Error protection used\n");
    else			printf (">>> Error protection not used\n");
	

    printf ("Is the material copyrighted? (y/<n>): ");
    gets (t);
    if (*t == 'y' || *t == 'Y') info->copyright = 1;
    else                        info->copyright = 0;
    if (info->copyright) printf (">>> Copyrighted material\n");
    else                 printf (">>> Material not copyrighted\n");
 
    printf ("Is this the original? (y/<n>): ");
    gets (t);
    if (*t == 'y' || *t == 'Y') info->original = 1;
    else                        info->original = 0;
    if (info->original) printf (">>> Original material\n");
    else	        printf (">>> Material not original\n");
    
    /* Option for multichannel for matricing, 7/12/93,SR*/
    if (*aiff == 1)
    {
        printf ("which kind of matrix do you want(<(-3, -3) = 0>;(-xx, -3) = 1;");
	printf (" (-oo, -3) = 2; (-oo, -oo) = 3)  ");
	gets (t);
	if (strcmp (t,"") == 0) info->matrix = 0;
	else			info->matrix = atoi (t);
	printf ("The matrix %d is chosen\n", info->matrix);
	*mat_mode = info->matrix;

#ifdef Augmentation_7ch
	if (fr_ps->stereoaug == 2)  /* 7.1 channel mode */
	{
	    printf ("which kind of aug matrix do you want? (<0>; 1; 3) ");
	    gets (t);
	    if (strcmp (t, "") == 0) info->aug_mtx_proc = 0;
	    else		     info->aug_mtx_proc = atoi (t);
	    printf ("The aug matrix %d is chosen\n", info->aug_mtx_proc);
	    *aug_mat_mode = info->aug_mtx_proc;
	}
#endif

	printf ("Do you want to have Channel-switching?(<y>/n) ");
	gets (t);
	if (*t == 'y' || *t == 'Y')
	    *cha_sw = *aug_cha_sw = 1;

	if (*cha_sw == 0 || info->matrix == 3)
	{
	    *cha_sw = 0;
	    printf ("Channel-switching is not used!\n");
	    printf ("Which mode of Channel-switching do you want? (not used) ");
	    gets (t);
	    printf("CHA mode: %d\n", *cha_sw);
	}
	else
	{
	    *cha_sw = 1;
	    printf ("Channel-switching is used\n");	 
	    do
	    {
		printf ("Which mode of Channel-switching do you want? (-2 ... 7) ");
		gets (t);
		*cha_sw = atoi (t);
		printf ("CHA mode: %d\n", *cha_sw);
		if (fr_ps->config == 320)
		    bool = *cha_sw > 7;
		else if (fr_ps->config == 310 && info->matrix == 2)
		    bool = *cha_sw > 5;
		else if (fr_ps->config == 310)
		    bool = *cha_sw > 4;
		else if (fr_ps->config == 220)
		    bool = *cha_sw > 3;
		else if (fr_ps->config == 300 || fr_ps->config == 302 || fr_ps->config == 210)
		    bool = *cha_sw > 2;
		else
		    bool = *cha_sw > 0;
		if (bool)
		    printf ("!!! Channel-switching mode %d cannot be used together with Configuration: %d !!!\n", *cha_sw, fr_ps->config);
	    } while (bool);
	}

#ifdef Augmentation_7ch
	if (fr_ps->stereoaug == 2)  /* 7.1 channel mode */
	{
	    if (*aug_cha_sw == 0 || info->aug_mtx_proc == 3)
	    {
		*aug_cha_sw = 0;
		printf ("Aug Channel-switching is not used!\n");
		printf ("Which mode of Aug Channel-switching do you want? (not used) ");
		gets (t);
		printf("Aug CHA mode: %d\n", *aug_cha_sw);
	    }
	    else
	    {
		*aug_cha_sw = 1;
		printf ("Aug Channel-switching is used\n");	 
		do
		{
		    printf ("Which mode of Aug Channel-switching do you want? (-1 ... 7) ");
		    gets (t);
		    *aug_cha_sw = atoi (t);
		    printf ("Aug CHA mode: %d\n", *aug_cha_sw);

		    if (info->aug_mtx_proc == 0 || info->aug_mtx_proc == -1)
			bool = *aug_cha_sw > 7;
		    else if (info->aug_mtx_proc == 1)
			bool = *aug_cha_sw > 3;
		    else
			bool = *aug_cha_sw > 0;

		    if (bool)
		        printf ("!!! TCA7 mode %d cannot be used together with aug_mtx_proc: %d !!!\n",
				*aug_cha_sw, info->aug_mtx_proc);
		} while (bool);
	    }
	}
	else
#endif
	    *aug_cha_sw = 0;

	/********* 27/07/95, WtK , phantom coding ********************/
	if ((fr_ps->stereomc > 0) && (info->center == 1))
	{
	    printf ("Do you want to use Phantom Coding of the Center signal? (y/<n>) ");
	    gets (t);
	    if ((*t == 'y') || (*t == 'Y'))
	    {
		printf (">>> Phantom Coding is used.\n");
		fr_ps->phantom_c = 1;
	    } 
	    else
	    {
		printf (">>> No Phantom Coding.\n");	 
		fr_ps->phantom_c = 0;
	    }
        }

	/********* 05/24/95, SR , dynamic cross_talk ********************/
	printf ("Do you want to have Dynamic Crosstalk?(y/<n>) ");
	gets (t);
	if (*t == 'y')  
	{
	    printf ("Dynamic Crosstalk is used!\n");
	    bool = 1;
	    do
	    {
		printf ("Which mode of Dynamic Crosstalk do you want? (-1 ... 14) ");
		gets (t);
		*dyn_cr = atoi (t);
		printf ("DynX mode: %d\n", *dyn_cr);
		if (fr_ps->phantom_c && fr_ps->config == 320)
		    bool = ((*dyn_cr==9) || (*dyn_cr==10) || (*dyn_cr==11) || (*dyn_cr==12) || (*dyn_cr==14));
		else if (fr_ps->phantom_c && ((fr_ps->config==310) || (fr_ps->config==220)))
		    bool = (*dyn_cr==4);
		else
		    bool = 0;
		if (bool)
		    printf ("!!! Dynamic Crosstalk mode %d cannot be used together with Phantom Coding !!!\n", *dyn_cr);
	    } while (bool);

#ifdef Augmentation_7ch
	    if (fr_ps->stereoaug == 2)  /* 7.1 channel mode */
	    {
		bool = 1;
		do
		{
		    printf ("Which mode of Aug Dynamic Crosstalk do you want? (-1 ... 18) ");
		    gets (t);
		    *aug_dyn_cr = atoi (t);
		    printf ("Aug DynX mode: %d\n", *aug_dyn_cr);
		    bool = *aug_dyn_cr > 18;

		    if (bool)
			printf ("!!! Aug Dynamic Crosstalk mode %d cannot be used !!!\n", *aug_dyn_cr);
		} while (bool);
	    }
#endif
	} 
	else
	{
	    *dyn_cr = 0;
	    printf ("Dynamic Crosstalk is not used\n");	 
	    printf ("Which mode of Dynamic Crosstalk do you want? (not used) ");
	    gets (t);
	    printf ("DynX mode: %d\n", *dyn_cr);
#ifdef Augmentation_7ch
	    if (fr_ps->stereoaug == 2)  /* 7.1 channel mode */
	    {
	        *aug_dyn_cr = 0;
		printf ("Which mode of Aug Dynamic Crosstalk do you want? (-1 ... 18) ");
		gets (t);
		printf ("Aug DynX mode: %d\n", *aug_dyn_cr);
	    }
#endif
	}
	bool = 1;
	do
	{
	    printf ("Value for dyn_cross_LR?(<0>/1) ");
	    gets (t);
	    *dyn_cr_LR = atoi (t);
	    bool = *dyn_cr_LR < -1 || *dyn_cr_LR > 1;

	    if (bool)
		printf ("!!! dyn_cross_LR %d invalid value -1, 0 or 1 !!!\n", *dyn_cr_LR);
	} while (bool);
	printf ("dyn_cross_LR: %d\n", *dyn_cr_LR);

	/********* 27/01/97, FdB, select prediction ********************/
	if (fr_ps->stereomc > 0 && fr_ps->config != 202 && fr_ps->config != 102)
	{
	    printf ("Do you want to use MC prediction? (y/<n>) ");
	    gets (t);
	    if ((*t == 'y') || (*t == 'Y'))
	    {
		printf (">>> MC prediction is used.\n");
		info->mc_prediction_on = 1;
	    } 
	    else
	    {
		printf (">>> No MC prediction.\n");	 
		info->mc_prediction_on = 0;
	    }
        }
    }

    /* YBKim060695 ancillary */
    printf ("Do you want to use ancillary part ? (y:<n>): ");
    gets (t);
    *ancillaryUse = (*t == 'y' || *t == 'Y');
    printf ("Ancillary: %d\n", *ancillaryUse);

    printf ("How verbose would you like the output ? (0, <1>, 2, 3): ");
    gets (t);
    if (*t >= '0' && *t <= '3')
	*verbosity = atoi (t);
    else
	*verbosity = DFLT_VERB;
    printf ("Verbose level: %d\n", *verbosity);

    printf ("Do you wish to exit (last chance before encoding)? (y/<n>): ");
    gets (t);
    if (*t == 'y' || *t == 'Y')
	exit (1);
}   

/************************************************************************
/*
/* parse_args
/*
/* PURPOSE:  Sets encoding parameters to the specifications of the
/* command line.  Default settings are used for parameters
/* not specified in the command line.
/*
/* SEMANTICS:  The command line is parsed according to the following
/* syntax:
/*
/* -l  is followed by the layer number
/* -m  is followed by the mode of the two front channels
/* -r  is followed by the sampling rate
/* -b  is followed by the total bitrate, irrespective of the mode
/* -B  is followed by the MPEG1 bitrate, irrespective of the mode
/* -d  is followed by the emphasis flag
/* -c  turns the copyright flag on
/* -o  turns the original flag on
/* -e  turns the error_protection flag on
/* -C  is followed by the configuration code "???" (front,rear,2ndstereo) channels
/* -L  turns the LFE flag on
/* -x  is followed by the matrix
/* -k  is followed by a fixed tca value, or if '-1' a random value 
/* -y  is followed by the dynamic crosstalk mode, or if '-1' a random value 
#ifdef Augmentation_7ch
/* -X  is followed by the 7.1-ch aug. matrix
/* -K  is followed by a fixed 7.1-ch aug. tca value, or if '-1' a random value 
/* -Y  is followed by the 7.1-ch aug. dynX mode, or if '-1' a random value 
#endif
/* -w  sets phantom coding /* 27/07/95: '-p' already reserved for psy */
/* -g  sets MultiLingual coding
/* -D  turns the dyn_cross_LR flag on
/* -P  turns the 'random prediction values' flag on
/* -n  is followed by the number of ancillary data bytes
/* -a  turns the ancillary flag on
/* -v  sets level of verbosity of debug and progress messages
/*
/* If the input file is in AIFF format, the sampling frequency is read
/* from the AIFF header.
/*
/* The input and output filenames are read into #inpath# and #outpath#.
/*
/************************************************************************/
 
void
parse_args(int argc, char **argv,
	frame_params *fr_ps,
	int *psy,
	long unsigned int *num_samples,
	long unsigned int *num_samples_ml,
	char *original_file_name,
	char *encoded_file_name,
	IFF_AIFF *pcm_aiff_data,
	IFF_AIFF *pcm_aiff_data_ml,
	int *aiff,
	int *byte_per_sample,
	int *mat_mode,
	int *aug_mat_mode,
	int *cha_sw,
	int *aug_cha_sw,
	char *encoded_file_name_ext,
	int *bit_rate,
	char *encoded_file_name_mpg,
	int *dyn_cr,
	int *dyn_cr_LR,
	int *aug_dyn_cr,
	int *ancillaryUse,
	int *verbosity)
{
    FLOAT srate;
    int	 brate = DFLT_BRT;    /* MPEG1 bitrate */
    layer *info = fr_ps->header;
    int   err = 0, bool, i = 0;
    int   ml_present = 0; /* MultiLingual coding */
    
    fr_ps->mnr_min = 0.0;
    *aiff = 0;
    *bit_rate = DFLT_BRT;    /* total bitrate */
    fr_ps->phantom_c = 0;
    *verbosity = DFLT_VERB;
 
    /* preset defaults */
    original_file_name[0] = NULL_CHAR;
    encoded_file_name[0] = NULL_CHAR;
    info->lay = DFLT_LAY;
    switch(DFLT_MOD)
    {
	case 'r': info->mode = MPG_MD_STEREO; info->mode_ext = 0; break;
	case 'd': info->mode = MPG_MD_DUAL_CHANNEL; info->mode_ext=0; break;
	case 'j': info->mode = MPG_MD_JOINT_STEREO; break;
	case 'm': info->mode = MPG_MD_MONO; info->mode_ext = 0; break;
	default : fprintf(stderr, "%s: Bad mode dflt %c\n", programName, DFLT_MOD);
		  abort ();
    }
    *psy = DFLT_PSY;
    if ((info->sampling_frequency = SmpFrqIndex ((long) (1000*DFLT_SFQ))) < 0)
    {
        fprintf (stderr, "%s: bad sfrq default %.2f\n", programName, DFLT_SFQ);
        abort ();
    }
    info->bitrate_index = DFLT_BRT;
    if ((info->bitrate_index = BitrateIndex (info->lay, DFLT_BRT)) < 0)
    {
	fprintf (stderr, "%s: bad default bitrate %u\n", programName, DFLT_BRT);
	abort ();
    }
    switch(DFLT_EMP)
    {
	case 'n': info->emphasis = 0; break;
	case '5': info->emphasis = 1; break;
	case 'c': info->emphasis = 3; break;
	default : fprintf (stderr, "%s: Bad emph dflt %c\n", programName, DFLT_EMP);
		  abort ();
    }
    info->copyright = 0;
    info->original = 0;
    info->error_protection = FALSE;
  
    /* process args */
    while (++i < argc && err == 0)
    {
	char c, *token, *arg, *nextArg;
	int  argUsed;
   
	token = argv[i];
	if (*token++ == '-')
	{
	    if (i+1 < argc)
	       nextArg = argv[i+1];
	    else
	       nextArg = "";
   
	    argUsed = 0;
	    while (c = *token++)
	    {
		if (*token /* NumericQ(token) */)
		   arg = token;
		else
		   arg = nextArg;
    
		switch (c)
		{
		    case 'l':	info->lay = atoi(arg);
				argUsed = 1;
				if (info->lay < 1 || info->lay > 2)
				{
				   fprintf(stderr,"%s: -l layer must be 1 or 2, not %s\n",
					programName, arg);
				   err = 1;
				}
				break;

		    case 'm':	argUsed = 1;
				if (*arg == 's')
				    { info->mode = MPG_MD_STEREO; info->mode_ext = 0; }
				else if (*arg == 'd')
				    { info->mode = MPG_MD_DUAL_CHANNEL; info->mode_ext = 0; }
				else if (*arg == 'j')
				    { info->mode = MPG_MD_JOINT_STEREO; }
				else if (*arg == 'm')
				    { info->mode = MPG_MD_MONO; info->mode_ext = 0; }
				else if (*arg == 'n')
				    { info->mode = MPG_MD_NONE; info->mode_ext = 0; }
				else 
				{
				    fprintf (stderr, "%s: -m mode must be s/d/j/m not %s\n",
						     programName, arg);
				    err = 1;
				}
				break;

		    case 'r':	srate = atof(arg);
				argUsed = 1;
				if ((info->sampling_frequency =
				    SmpFrqIndex ((long) (1000*srate))) < 0)
				    err = 1;
				break;

		    case 'b':	*bit_rate = atoi(arg);
				argUsed = 1;
				if (*bit_rate > brate)
				{
				    info->ext_bit_stream_present = 1;
				    info->ext_length = ((*bit_rate - brate) * 144) / s_freq[info->sampling_frequency];
				}
				else
				{
				    info->ext_bit_stream_present = 0;
				    brate = *bit_rate;
				}
				break;

		    case 'B':	brate = atoi(arg);
				argUsed = 1;
				if (*bit_rate > brate)
				{
				    info->ext_bit_stream_present = 1;
				    info->ext_length = ((*bit_rate - brate) * 144) / s_freq[info->sampling_frequency];
				}
				else
				    info->ext_bit_stream_present = 0;
				break;

		    case 'C':	fr_ps->config = atoi(arg);
				argUsed = 1;
				break;

		    case 'd':	argUsed = 1;
				if (*arg == 'n')	info->emphasis = 0;
				else if (*arg == '5')	info->emphasis = 1;
				else if (*arg == 'c')	info->emphasis = 3;
				else 
				{
				    fprintf (stderr, "%s: -d emp must be n/5/c not %s\n",
						     programName, arg);
				    err = 1;
				}
				break;

		    case 'c':   info->copyright = 1; break;
		    case 'o':   info->original  = 1; break;
		    case 'e':   info->error_protection = TRUE; break;

		    case 'x':	info->matrix = *mat_mode = atoi(arg); 
		                if (info->matrix == 3)
				    *cha_sw = 0;
				argUsed = 1;
				break;
		    case 'k':	*cha_sw = atoi(arg); 
				argUsed = 1;
				break;
          	    case 'y':	*dyn_cr = atoi(arg);	
				argUsed = 1;
				break;		   
#ifdef Augmentation_7ch
		    case 'X':	info->aug_mtx_proc = *aug_mat_mode = atoi(arg); 
		                if (info->aug_mtx_proc == 3)
				    *aug_cha_sw = 0;
				argUsed = 1;
				break;
		    case 'K':	*aug_cha_sw = atoi(arg); 
				argUsed = 1;
				break;
          	    case 'Y':	*aug_dyn_cr = atoi(arg);	
				argUsed = 1;
				break;		   
#endif
		    case 'D':	*dyn_cr_LR = 1;	
				break;		   
		    case 'L':	info->lfe = 1;	
				break;		   
		    case 'w':	fr_ps->phantom_c = 1;	
				break;		   
		    case 'P':	info->mc_prediction_on = 1;	
				break;		   
		    case 'g':	ml_present = 1;	
				break;		   
		    case 'n':	info->n_ad_bytes = atoi(arg);
				argUsed = 1;
				break;
		    case 'a':	*ancillaryUse = 1;
				break;
		    case 'v':	*verbosity = atoi(arg);
				argUsed = 1;
				break;

		    default:    fprintf(stderr,"%s: unrec option %c\n",
                                        programName, c);
                                err = 1;
				break;
		}
	    
	    
		if (argUsed)
		{
		    if (arg == token)
			token = "";		/* no more from token */
		    else
			i++;		/* skip arg we used */
		    arg = "";
		    argUsed = 0;
		}
	    }
	}
	else
	{
	    if (original_file_name[0] == NULL_CHAR)
		strcpy (original_file_name, argv[i]);
	    else if (encoded_file_name[0] == NULL_CHAR)
		strcpy (encoded_file_name, argv[i]);
	    else
	    {
		fprintf (stderr, "%s: excess arg %s\n", programName, argv[i]);
		err = 1;
	    }
	}
    }

    if ((info->bitrate_index = BitrateIndex (info->lay, brate)) < 0)
	err = 1;
    if (err || original_file_name[0] == NULL_CHAR)
	usage ();  /* never returns */
 
    if (encoded_file_name[0] == NULL_CHAR)
    {
	strcat (strcpy (encoded_file_name_ext, original_file_name), DFLT_EXT_EXT);
	strcat (strcpy (encoded_file_name, original_file_name), DFLT_EXT);
	strcat (strcpy (encoded_file_name_mpg, encoded_file_name), DFLT_EXT);
    }
    else
    {
	strcat (strcpy (encoded_file_name_ext, encoded_file_name), DFLT_EXT_EXT);
	strcat (encoded_file_name, DFLT_EXT);
	strcat (strcpy (encoded_file_name_mpg, encoded_file_name), DFLT_EXT);
    }

    if ((musicin = fopen (original_file_name, "rb")) == NULL)
    {
	printf ("Could not find \"%s\".\n", original_file_name);
	exit (0);
    }

    open_bit_stream_w (&bs, encoded_file_name, BUFFER_SIZE);

    if (aiff_read_headers (musicin, pcm_aiff_data, byte_per_sample) == 0)
    {
	int wrong_nr_channels = 0;

	*aiff = 1;
	if (*verbosity >= 2)
	    printf (">>> Using Audio IFF sound file headers\n");

	aiff_check (original_file_name, pcm_aiff_data);

	if (fr_ps->config)
	{
	    /* check if config and number of channels is consistent */
	    switch (fr_ps->config)
	    {
		case 100:   wrong_nr_channels = pcm_aiff_data->numChannels != 1;
			    info->mode = MPG_MD_MONO;
			    info->mode_ext = 0;
			    break;
		case 200:   wrong_nr_channels = pcm_aiff_data->numChannels - info->lfe != 2;
			    fr_ps->lfe_pos = 2;
			    break;
		case 102:   wrong_nr_channels = pcm_aiff_data->numChannels - info->lfe != 3;
			    info->mode = MPG_MD_MONO;
			    info->mode_ext = 0;
			    info->surround = 3; 
			    fr_ps->stereomc = 2;
			    fr_ps->lfe_pos = 1;
			    break;
		case 202:   wrong_nr_channels = pcm_aiff_data->numChannels - info->lfe != 4;
			    info->surround = 3; 
			    fr_ps->stereomc = 2;
			    fr_ps->lfe_pos = 2;
			    break;
		case 210:   wrong_nr_channels = pcm_aiff_data->numChannels - info->lfe != 3;
			    info->surround = 1; 
			    fr_ps->stereomc = 1;
			    fr_ps->lfe_pos = 2;
			    break;
		case 220:   wrong_nr_channels = pcm_aiff_data->numChannels - info->lfe != 4;
			    info->surround = 2; 
			    fr_ps->stereomc = 2;
			    fr_ps->lfe_pos = 2;
			    break;
		case 302:   wrong_nr_channels = pcm_aiff_data->numChannels - info->lfe != 5;
			    info->center = 1; 
			    info->surround = 3; 
			    fr_ps->stereomc = 3;
			    break;
		case 310:   wrong_nr_channels = pcm_aiff_data->numChannels - info->lfe != 4;
			    info->center = 1; 
			    info->surround = 1; 
			    fr_ps->stereomc = 2;
			    fr_ps->lfe_pos = 3;
			    break;
		case 320:   wrong_nr_channels = pcm_aiff_data->numChannels - info->lfe != 5;
			    info->center = 1; 
			    info->surround = 2; 
			    fr_ps->stereomc = 3;
			    fr_ps->lfe_pos = 3;
			    break;
#ifdef Augmentation_7ch
		case 520:   wrong_nr_channels = pcm_aiff_data->numChannels - info->lfe != 7;
			    if (ml_present)
			    {
				printf ("7.1 channel augmentation and multilingual cannot be combined\n");
				exit (1);
			    }
			    fr_ps->config = 320;
			    info->center = 1; 
			    info->surround = 2; 
			    fr_ps->stereomc = 3;
			    fr_ps->stereoaug = 2;
			    fr_ps->lfe_pos = 3;
			    break;
#endif
		default:    printf ("Configuration: %d not supported\n", fr_ps->config);
			    exit (1);
	    }
	}
	else
	{
	    if (pcm_aiff_data->numChannels == 1)
	    {
		fr_ps->config = 100;
		info->mode = MPG_MD_MONO;
		info->mode_ext = 0;
	    }
	    else
	    {
		switch (pcm_aiff_data->numChannels - info->lfe)
		{
		    case 2: fr_ps->config = 200;
			    fr_ps->lfe_pos = 2;
			    break;
		    case 3: fr_ps->config = 210;
			    info->surround = 1; 
			    fr_ps->stereomc = 1;
			    fr_ps->lfe_pos = 2;
			    break;
		    case 4: fr_ps->config = 310;
			    info->center = 1; 
			    info->surround = 1; 
			    fr_ps->stereomc = 2;
			    fr_ps->lfe_pos = 3;
			    break;
		    case 5: fr_ps->config = 320;
			    info->center = 1; 
			    info->surround = 2; 
			    fr_ps->stereomc = 3;
			    fr_ps->lfe_pos = 3;
			    break;
#ifdef Augmentation_7ch
		    case 7: fr_ps->config = 320;
			    info->center = 1; 
			    info->surround = 2; 
			    fr_ps->stereomc = 3;
			    fr_ps->stereoaug = 2;
			    fr_ps->lfe_pos = 3;
			    break;
#endif
		    default: wrong_nr_channels = 1;
		}
	    }
	}
	
	if (wrong_nr_channels)
	{
	    printf ("Configuration: %d lfe: %d do not match with audio channels: %d\n", 
		     fr_ps->config,  info->lfe, pcm_aiff_data->numChannels);
	    exit (1);
	}

	switch (fr_ps->config)
	{
	    case 100:   printf (">>> Using 1/0 configuration.\n"); 
			break;
	    case 200:   printf (">>> Using 2/0 configuration.\n"); 
			break;
	    case 102:   printf (">>> Using 1/0 + 2/0 configuration.\n"); 
			break;
	    case 202:   printf (">>> Using 2/0 + 2/0 configuration.\n"); 
			break;
	    case 210:   printf (">>> Using 2/1 configuration.\n"); 
			break;
	    case 220:   printf (">>> Using 2/2 configuration.\n"); 
			break;
	    case 302:   printf (">>> Using 3/0 + 2/0 configuration.\n"); 
			break;
	    case 310:   printf (">>> Using 3/1 configuration.\n"); 
			break;
	    case 320:   if (fr_ps->stereoaug == 0)
			    printf (">>> Using 3/2 configuration.\n"); 
#ifdef Augmentation_7ch
			else
			    printf (">>> Using 5/2 configuration.\n"); 
#endif
			break;
	}

	info->sampling_frequency = SmpFrqIndex ((long) pcm_aiff_data->sampleRate);
	if (*verbosity >= 2)
	    printf (">>> %.f Hz sampling frequency selected\n",
		    pcm_aiff_data->sampleRate);

	/* Determine number of samples in sound file */
#ifndef MS_DOS
	*num_samples = pcm_aiff_data->numChannels *
		       pcm_aiff_data->numSampleFrames;
#else
	*num_samples = (long) (pcm_aiff_data->numChannels) *
                       (long) (pcm_aiff_data->numSampleFrames);
#endif

	if (*cha_sw > 0)
	{
	    if (fr_ps->config == 320)
		bool = *cha_sw > 7;
	    else if (fr_ps->config == 310 && info->matrix == 2)
		bool = *cha_sw > 5;
	    else if (fr_ps->config == 310)
		bool = *cha_sw > 4;
	    else if (fr_ps->config == 220)
		bool = *cha_sw > 3;
	    else if (fr_ps->config == 300 || fr_ps->config == 302 || fr_ps->config == 210)
		bool = *cha_sw > 2;
	    else
		bool = 1;

	    if (bool)
	    {
		printf ("!!! Channel-switching mode %d cannot be used together with Configuration: %d !!!\n", *cha_sw, fr_ps->config);
		exit (1);
	    }
	}
#ifdef Augmentation_7ch
	if (*aug_cha_sw > 0)
	{
	    if (info->aug_mtx_proc == 0 || info->aug_mtx_proc == -1)
		bool = *aug_cha_sw > 7;
	    else if (info->aug_mtx_proc == 1)
		bool = *aug_cha_sw > 3;
	    else
		bool = 1;

	    if (bool)
	    {
		printf ("!!! TCA7 mode %d cannot be used together with aug_mtx_proc: %d !!!\n",
			*aug_cha_sw, info->aug_mtx_proc);
		exit (1);
	    }
	}
#endif
	if (*dyn_cr > 0)
	{
	    if (fr_ps->config == 320)
		bool = *dyn_cr > 14;
	    else if (fr_ps->config == 310 || fr_ps->config == 220)
		bool = *dyn_cr > 4;
	    else if (fr_ps->config == 300 || fr_ps->config == 302 || fr_ps->config == 210)
		bool = *dyn_cr > 1;
	    else
	    	bool = 1;

	    if (bool)
	    {
		printf ("!!! Dynamic Crosstalk mode %d cannot be used for Configuration: %d !!!\n", *dyn_cr, fr_ps->config);
		exit (1);
	    }
	    
	    if (!bool && fr_ps->phantom_c)
	    {
	    	if (fr_ps->config == 320)
		    bool = ((*dyn_cr==9) || (*dyn_cr==10) || (*dyn_cr==11) || (*dyn_cr==12) || (*dyn_cr==14));
		else if (fr_ps->config == 310)
		    bool = (*dyn_cr==4);

		if (bool)
		{
		    printf ("!!! Dynamic Crosstalk mode %d cannot be used together with Phantom Coding for Configuration: %d!!!\n", *dyn_cr, fr_ps->config);
		    exit (1);
		}
	    }
	}

#ifdef Augmentation_7ch
	if (*aug_dyn_cr > 18)
	{
	    printf ("!!! Aug Dynamic Crosstalk mode %d cannot be used !!!\n", *aug_dyn_cr);
	    exit (1);
	}
#endif
    }
    else
    {
	/* Not using Audio IFF sound file headers. */
   
	if (*verbosity >= 2)
	    printf(" NO MULTICHANNEL CODING!!\n");
	ml_present = 0;

	if (fseek (musicin, 0, SEEK_SET) != 0)
	{
	    printf  ("Could not seek to PCM sound data in \"%s\".\n", original_file_name);
	    exit (0);
	}
 
	/* Declare sound file to have "infinite" number of samples. */
	*num_samples = MAX_U_32_NUM;
    }

    /***************************************************************************/
    /* 7/8/95 Multi lingual extension WtK                                      */

    if (ml_present)
    {
	strcat (strcpy (multilingual_file_name, original_file_name), DFLT_ML_EXT);
	if ((musicin_ml = fopen (multilingual_file_name, "rb")) == NULL) 
	{
	    printf ("Could not find \"%s\".\nNo MultiLingual Coding\n", multilingual_file_name);
	    info->multiling_ch = 0;
	}
	else if (aiff_read_headers (musicin_ml, pcm_aiff_data_ml, byte_per_sample) == 0) 
	{
	    if (*verbosity >= 2)
		printf (">>> Using Audio IFF multilingual file headers\n");
	    info->multiling_ch = pcm_aiff_data_ml->numChannels;
	    if (*verbosity >= 2)
		printf (">>> Using %d multilingual channels\n", info->multiling_ch);
	    *num_samples_ml = pcm_aiff_data_ml->numChannels *
			      pcm_aiff_data_ml->numSampleFrames;
	}
	else     /* Not using Audio IFF sound file headers. */
	{
	    printf("***WARNING: Could not read ML AIFF header - No MultiLingual coding!!\n");
	    info->multiling_ch = 0;
	}

	if (info->multiling_ch > 0)
	{
	    switch (SmpFrqIndex ((long) pcm_aiff_data_ml->sampleRate) - info->sampling_frequency)
	    {
		case 16 :   if (*verbosity >= 2)
				printf ("MultiLingual is LSF coded.\n");
			    info->multiling_fs = 1;
			    break;
		case  0 :   if (*verbosity >= 2)
				printf ("MultiLingual sample rate equals that of Audio.\n");
			    info->multiling_fs = 0;
			    break;
		default :   printf ("***WARNING: MultiLingual sample rate unequals (half) mc sample rate.\n");
       			    printf ("            No MultiLingual coding!!!\n");
			    info->multiling_ch = 0;
			    break;
	    } /*switch*/
	} /*if (ml_ch>0)*/
    }
    else
    {
	info->multiling_ch = 0;
    } /* if (ml_present) */

    /* 7/8/95 Multi lingual extension WtK                                      */
    /***************************************************************************/
}

/************************************************************************
/*
/* print_config
/*
/* PURPOSE:  Prints the encoding parameters used
/*
/************************************************************************/
 
void
print_config (frame_params *fr_ps, int *psy, long unsigned int *num_samples, char *inPath, char *outPath, int *aiff)
{
    layer *info = fr_ps->header;
 
    printf ("Encoding configuration:\n");
    if (*aiff == 1)
	printf ("Layer=%s   mode=%s   extn=%d   psy model=%d\n",
	        layer_names[info->lay-1], mode_names[info->mode],
	        info->mode_ext, *psy);
    else
	printf ("Layer=%s   mode=%s   extn=%d    psy model=%d\n",
	        layer_names[info->lay-1], mode_names[info->mode],
	        info->mode_ext, *psy);

    if (info->bitrate_index != 0)
    {
	if (bitrate[info->lay-1][info->bitrate_index] == 1000)
	    printf ("samp frq=%.1f kHz   total bitrate=dynamic bitrate\n",
		    s_freq[info->sampling_frequency]);
	else
	    printf ("samp frq=%.1f kHz   mpeg1 bitrate=%d kbps\n",
		    s_freq[info->sampling_frequency],
		    bitrate[info->lay-1][info->bitrate_index]);
	if (info->ext_bit_stream_present)
	    printf ("ext bitrate=%d kbps\n", (int) (info->ext_length * s_freq[info->sampling_frequency] / 144));
    }

    if (info->multiling_ch > 0)
    {
        printf ("%d multilingual channels ", info->multiling_ch);
        if (info->multiling_fs == 1)
	    printf ("LSF coded.\n");
        else
	    printf ("full sample rate coded.\n");
    }
    else
	printf ("No multilingual coding.\n");

    printf ("de-emph=%d   c/right=%d   orig=%d   errprot=%d\n",
            info->emphasis, info->copyright, info->original,
            info->error_protection);
    printf ("matrix=%d  center=%d  surround=%d  stereo=%d  stereomc=%d  stereoaug=%d\n",
            info->matrix, info->center, info->surround,
	    fr_ps->stereo, fr_ps->stereomc, fr_ps->stereoaug);
    printf ("input file: '%s'   output file: '%s'\n", inPath, outPath);
    fflush (stdout);
}
 
/************************************************************************
/*
/* main
/*
/* PURPOSE:  MPEG I Encoder supporting layers 1 and 2, and
/* psychoacoustic models 1 (MUSICAM) and 2 (AT&T),now portated
/* to multichannel (two front channels, one center and three surround
/* channels. There are different possibilities to code the AIFF-signal,
/* like coding only the surround channels or the front channels,
/* different bitrates for front and surropund channels etc.
/* dec1992 sr.
/*
/* SEMANTICS:  One overlapping frame of audio of up to 2 channels are
/* processed at a time in the following order:
/* (associated routines are in parentheses)
/*
/* 1.  Filter sliding window of data to get 32 subband
/* samples per channel.
/* (window_subband,filter_subband)
/*
/* 2.  If joint stereo mode, combine left and right channels
/* for subbands above #jsbound#.
/* (*_combine_LR)
/*
/* 3.  Calculate scalefactors for the frame, and if layer 2,
/* also calculate scalefactor select information.
/* (*_scale_factor_calc)
/*
/* 4.  Calculate psychoacoustic masking levels using selected
/* psychoacoustic model.
/* (*_Psycho_One, psycho_anal)
/*
/* 5.  Perform iterative bit allocation for subbands with low
/* mask_to_noise ratios using masking levels from step 4.
/* (*_main_bit_allocation)
/*
/* 6.  If error protection flag is active, add redundancy for
/* error protection.
/* (*_CRC_calc)
/*
/* 7.  Pack bit allocation, scalefactors, and scalefactor select
/* information (layer 2) onto bitstream.
/* (*_encode_bit_alloc,*_encode_scale,II_transmission_pattern)
/*
/* 8.  Quantize subbands and pack them into bitstream
/* (*_subband_quantization, *_sample_encoding)
/*
/************************************************************************/

#ifdef MS_DOS
extern unsigned _stklen = 16384;
#endif

typedef double JSBS[2][3][12][SBLIMIT];
typedef double IN[14][HAN_SIZE];		/* JMZ 08/03/1995 */
typedef unsigned int SUB[14][3][12][SBLIMIT]; /* JMZ 08/03/1995 */
typedef double SAM[12];
typedef unsigned int LSB[12];

int main (int argc, char **argv)
{
    double  sb_sample[14][3][12][SBLIMIT];	/* JMZ 08/03/1995 */
    JSBS    *j_sample;
    IN	    *win_que;
    SUB	    *subband;
    SAM	    *lfe_sample;
    LSB	    *lfe_sbband;
 
    frame_params fr_ps;
    layer info;    
    char original_file_name[MAX_NAME_SIZE];
    char encoded_file_name[MAX_NAME_SIZE];
    char encoded_file_name_ext[MAX_NAME_SIZE];
    char encoded_file_name_mpg[MAX_NAME_SIZE];
    double *win_buf[14];				/* JMZ 08/03/1995 */
    static double buffer[14][1152];			/* JMZ 08/03/1995 */			
    double spiki[14][SBLIMIT];				/* JMZ 08/03/1995 */
    static unsigned int bit_alloc[14][SBLIMIT], scfsi[14][SBLIMIT], scfsi_dyn[14][SBLIMIT];						/* JMZ 08/03/1995 */
    static unsigned int scalar[14][3][SBLIMIT], j_scale[2][3][SBLIMIT],  scalar_dyn[14][3][SBLIMIT];
    static double ltmin[14][SBLIMIT], lgmin[14][SBLIMIT], max_sc[14][SBLIMIT], smr[14][SBLIMIT];							/* JMZ 08/03/1995 */

    static unsigned int lfe_alloc;
    static unsigned int lfe_scalar; 

    FLOAT snr32[32];
    short sam[14][1056];					/* JMZ 08/03/1995 */
    double buffer_matr[14][1152];			/* JMZ 08/03/1995 */
    int whole_SpF, extra_slot = 0;
    double avg_slots_per_frame, frac_SpF, slot_lag;
    int model, stereo, error_protection, stereomc, stereoaug;
    static unsigned int crc, crcmc, crcaug, crcext;
    int i, j, k, adb, p, l, m, lfe, s;
    unsigned long bitsPerSlot, samplesPerFrame, frameNum = 0;
    unsigned long frameBits, sentBits = 0;
    unsigned long num_samples;
    unsigned long num_samples_ml;	               /* 08/03/1995 JMZ Multilingual */
    int odd_ml = 1;                                    /* LSF MultiLingual: audio inputting only at odd frames , WtK 7/8/95 */
    int ml_eof = 0;                                    /* MultiLingual: frame loop stops if either audio or ML-file has been exhausted , WtK 7/8/95 */

    IFF_AIFF aiff_ptr;
    IFF_AIFF aiff_ptr_ml;	                       /* 08/03/1995 JMZ Multilingual */
    int aiff = 0;                                      /* misused to indicate mc-input */
    int byte_per_sample = 0;

    int mat_mode = 0;
    int aug_mat_mode = 0;
    int cha_sw = -2;
    int aug_cha_sw = 0;
    int dyn_cr = 0;
    int dyn_cr_LR = 0;
    int aug_dyn_cr = 0;
    int predis = 0;
    int bit_rate = 0;
    unsigned int crc_buffer[15];

    int lfe_bits = 0;
    int sentBits1, frameBits1;
    int hi,  hu,  ho;

/*    char *blubb = "rm ";*/	
    char blubb[128];	/* Corrected BUG JMZ 29/03/1995*/

    int  ancillaryUse = 0;  /* YBKim060695 ancillary */
    

#ifdef  PRINTOUT
    int       loop_channel,loop_subband;
    
    al_table  *loop_alloc; /* a pointer to a table */
    sb_alloc  loop_struct; /* a structure of 4 int's */
    alloc_ptr loop_str_ptr; /* a pointer to an sb_alloc structure */
    float     loop_bits;
#endif

#ifdef  MACINTOSH
    console_options.nrows = MAC_WINDOW_SIZE;
    argc = ccommand(&argv);
#endif

    program_information();

    /* Most large variables are declared dynamically to ensure
       compatibility with smaller machines */

    j_sample = (JSBS *) mem_alloc (sizeof (JSBS), "j_sample");   

    win_que = (IN *) mem_alloc (sizeof (IN), "Win_que");       
    subband = (SUB *) mem_alloc (sizeof (SUB),"subband");        

    lfe_sample = (SAM *) mem_alloc (sizeof (SAM), "lfe_sample");
    lfe_sbband = (LSB *) mem_alloc (sizeof (LSB),"lfe_sbband");
 
    /* clear buffers */
    memset ((char *) buffer, 0, sizeof (buffer));
    memset ((char *) bit_alloc, 0, sizeof (bit_alloc));
    memset ((char *) scalar, 0, sizeof (scalar));
    memset ((char *) j_scale, 0, sizeof (j_scale));
    memset ((char *) scfsi, 0, sizeof (scfsi));
    memset ((char *) ltmin, 0, sizeof (ltmin));
    memset ((char *) lgmin, 0, sizeof (lgmin));
    memset ((char *) max_sc, 0, sizeof (max_sc));
    memset ((char *) snr32, 0, sizeof (snr32));
    memset ((char *) sam, 0, sizeof (sam));
 
    fr_ps.header = &info;
    info.mode_ext = 0;
    fr_ps.tab_num = -1;             /* no table loaded */
    fr_ps.tab_num_mc = -1;
    fr_ps.tab_num_ml = -1;
    fr_ps.alloc = NULL;
    fr_ps.alloc_mc = NULL;
    fr_ps.alloc_ml = NULL;

    info.version = MPEG_AUDIO_ID;
    info.bitrate_index = 0;
    info.lfe = 0;	    /* no low frequency effect channel present! */
    info.center = 0; 
    info.surround = 0; 

    info.multiling_ch  = 0;
    info.multiling_fs  = 0;
    info.multiling_lay = 0;

    info.matrix = 0;
#ifdef Augmentation_7ch
    info.aug_mtx_proc = 0;
#endif
    info.ext_bit_stream_present = 0;
    info.n_ad_bytes = DFLT_NADB;
    info.audio_mix = 0;
    info.dyn_cross_LR = 0;    /* C out of Lo */
    info.mc_prediction_on = 0;	       
    info.ext_length = 0;
    info.ext_bit = 0;
    info.copy_ident_bit = 0;
    info.copy_ident_start = 0;

    fr_ps.config = 0;
    fr_ps.phantom_c = 0;
    fr_ps.stereomc = 0;
    fr_ps.stereoaug = 0;

    programName = argv[0];
    if (argc == 1)			    /* no command-line args */
	obtain_parameters (&fr_ps, &model, &num_samples, &num_samples_ml, original_file_name,
			   encoded_file_name, &aiff_ptr, &aiff_ptr_ml, &aiff, &byte_per_sample, 
			   &mat_mode, &aug_mat_mode, &cha_sw, &aug_cha_sw,
			   encoded_file_name_ext, &bit_rate, encoded_file_name_mpg,
			   &dyn_cr, &dyn_cr_LR, &aug_dyn_cr, &ancillaryUse, &verbosity);
    else
        parse_args(argc, argv, &fr_ps, &model, &num_samples, &num_samples_ml, original_file_name,
			   encoded_file_name, &aiff_ptr, &aiff_ptr_ml, &aiff, &byte_per_sample, 
			   &mat_mode, &aug_mat_mode, &cha_sw, &aug_cha_sw,
			   encoded_file_name_ext, &bit_rate, encoded_file_name_mpg,
			   &dyn_cr, &dyn_cr_LR, &aug_dyn_cr, &ancillaryUse, &verbosity);

    hdr_to_frps (&fr_ps);

    if (aiff != 1) fr_ps.stereomc = 0;

    print_config (&fr_ps, &model, &num_samples,
		  original_file_name, encoded_file_name, &aiff);
  
    stereo = fr_ps.stereo;
    stereomc = fr_ps.stereomc;
    stereoaug = fr_ps.stereoaug;
    lfe = info.lfe;
    error_protection = info.error_protection;
 
    if (info.lay == 1) { bitsPerSlot = 32; samplesPerFrame = 384;  }
    else               { bitsPerSlot = 8;  samplesPerFrame = 1152; }
    /* Figure average number of 'slots' per frame. */
    /* Bitrate means TOTAL for both channels, not per side. */
    if (bit_rate == 0) bit_rate = bitrate[info.lay-1][info.bitrate_index];
    if (verbosity >= 2) printf("bit_rate = %d\n", bit_rate);
    fflush(stderr);  
  
    avg_slots_per_frame = ((double) samplesPerFrame / s_freq[info.sampling_frequency]) *
			  ((double) bit_rate / (double) bitsPerSlot);

    whole_SpF = (int) avg_slots_per_frame;	 /* Bytes per frame within datastream*/
    if (verbosity >= 2)
	printf ("slots/frame = %d\n", whole_SpF);
    frac_SpF  = avg_slots_per_frame - (double) whole_SpF;
    slot_lag  = -frac_SpF;
    if (verbosity >= 2)
	printf("frac SpF=%.3f, tot bit_rate=%d kbps, s freq=%.1f kHz\n",
                frac_SpF, bit_rate, s_freq[info.sampling_frequency]);
 
    if (frac_SpF != 0)
    {
	if (verbosity >= 2)
	    printf ("Fractional number of slots, padding required\n");
    }
    else
	info.padding = 0;

#ifdef PRINTOUT
    if (verbosity >= 3)
    {
	printf ("\nFrame ");
	for (loop_subband = 0; loop_subband < SBLIMIT; loop_subband++)
	    printf ("%3d",loop_subband);
	printf ("\n");
    }
#endif
 
    while ( 
             ( get_audio (musicin, buffer, num_samples, stereo, &aiff_ptr,
	                  stereomc, stereoaug, &fr_ps, &aiff, &byte_per_sample,
	                  buffer_matr) > 0 ) &&
	     ( !ml_eof )
	  )
    {
	/****************************************************************/
	/*                                                              */
	/*                  START OF FRAME LOOP                         */
	/*                                                              */
	/****************************************************************/

	/* the following allocation must happen within the while-loop. 1/5/93, SR */
	  
	if (mat_mode == -1)
	{
	    if (fr_ps.config == 320 || fr_ps.config == 310)
		info.matrix = rand () % 4;
	    else
	    {
		info.matrix = rand () % 3;
		if (info.matrix == 2)
		    info.matrix++;
	    }
	}
	else
	    info.matrix = mat_mode;

	init_mc_pred (&fr_ps);
	info.tc_sbgr_select = 1;
	info.dyn_cross_on = 0;  
	if (dyn_cr != 0 || fr_ps.config == 302 || fr_ps.config == 202)
	    if (dyn_cr_LR == -1)
		info.dyn_cross_LR = rand () % 2;
	    else
		info.dyn_cross_LR = dyn_cr_LR;
	else
	    info.dyn_cross_LR = 0;

#ifdef Augmentation_7ch
	if (aug_mat_mode == -1)
	{
	    info.aug_mtx_proc = rand () % 3;
	    if (info.aug_mtx_proc == 2)
		info.aug_mtx_proc++;
	}
	else
	    info.aug_mtx_proc = aug_mat_mode;

	info.aug_dyn_cross_on = 0;    
	info.aug_future_ext = 0;
#endif
	for (i = 0; i < 12; i++)
	{
	    info.tc_alloc[i] = 0;
	    info.dyn_cross[i] = 0;
	    info.dyn_second_stereo[i] = 0;
#ifdef Augmentation_7ch
	    info.aug_tc_alloc[i] = 0;
	    info.aug_dyn_cross[i] = 0;
#endif
	}
	
	lfe_alloc = 0;
  
	if (stereomc > 0 && (fr_ps.header->center == 1 || fr_ps.header->center==3))
	{
	    if (fr_ps.phantom_c == 1) fr_ps.header->center = 3;
	    if (fr_ps.phantom_c == 0) fr_ps.header->center = 1;
	}
  
        if (verbosity >= 1)
	{
	    fprintf (stderr, "{%4lu}\r", frameNum++);
	    fflush (stderr);
	}

	win_buf[0] = &buffer[0][0];
	win_buf[1] = &buffer[1][0];
	win_buf[2] = &buffer[2][0];
	win_buf[3] = &buffer[3][0];
	win_buf[4] = &buffer[4][0];
	win_buf[5] = &buffer[5][0];
	win_buf[6] = &buffer[6][0];
	win_buf[7] = &buffer[7][0];	/* JMZ 08/03/1995 Multilingual */
	win_buf[8] = &buffer[8][0];	/* JMZ 08/03/1995 Multilingual */
	win_buf[9] = &buffer[9][0];	/* JMZ 08/03/1995 Multilingual */
	win_buf[10] = &buffer[10][0];	/* JMZ 08/03/1995 Multilingual */
	win_buf[11] = &buffer[11][0];	/* JMZ 08/03/1995 Multilingual */
	win_buf[12] = &buffer[12][0];	/* JMZ 08/03/1995 Multilingual */
	win_buf[13] = &buffer[13][0];	/* JMZ 08/03/1995 Multilingual */

        if (frac_SpF != 0)
	{
	    if (slot_lag > (frac_SpF - 1.0))
	    {
	        slot_lag -= frac_SpF;
	        extra_slot = 0;
	        info.padding = 0;
	        if (verbosity >= 2)
		    printf ("No padding for this frame\n"); 
	    }
	    else
	    {
	        extra_slot = 1;
	        info.padding = 1;
	        slot_lag += (1-frac_SpF);
	    }
        }
	adb = (whole_SpF + extra_slot) * bitsPerSlot;
	if (verbosity >= 2)
	    printf ("Adb0: %ld\n", adb);

/**************************************************************************/
/* 08/03/1995 JMZ Multilingual , WtK 7/8/95 */

	if (info.multiling_ch > 0)
	{
	    if (odd_ml)
	    {
		if (get_audio_ml (musicin_ml, buffer, num_samples_ml, &aiff_ptr, &fr_ps, 
				  &aiff, &byte_per_sample,buffer_matr) != 1152*info.multiling_ch )
		{
		    if (verbosity >= 2)
			printf("\nHit end of MultiLingual file.\n");
		    ml_eof = 1;
		}
	
		for (i = 0; i < 3; i++)
		    for (j = 0; j < 12; j++)
			for (k = 7; k < 7 + info.multiling_ch; k++) 
			{
			    window_subband (&win_buf[k], &(*win_que)[k][0], k);
			    filter_subband (&(*win_que)[k][0], &(sb_sample)[k][i][j][0]);
			}
		if (info.multiling_fs == 1)
		    ml_sb_sample_swap (7, 7 + info.multiling_ch, sb_sample); /* half fs */
	    }
	    else
		ml_sb_sample_shift (7, 7 + info.multiling_ch, sb_sample);
	}

/* 08/03/1995 JMZ Multilingual */
/**************************************************************************/


        switch (info.lay)
        {
 
/***************************** Layer I **********************************/

        case 1: for (j = 0; j < 12; j++)
		    for (k = 0; k < stereo; k++)
		    {
			window_subband (&win_buf[k], &(*win_que)[k][0], k);
			filter_subband (&(*win_que)[k][0], &(/***/sb_sample)[k][0][j][0]);
                    }

		I_scale_factor_calc (sb_sample, scalar, stereo);
		if (fr_ps.actual_mode == MPG_MD_JOINT_STEREO)
		{
		    I_combine_LR (sb_sample, *j_sample);
		    I_scale_factor_calc (/***/sb_sample, scalar, 1);
		}
    
		put_scale (scalar, &fr_ps, max_sc);
    
		I_Psycho_One (buffer, max_sc, ltmin, &fr_ps);
    
		I_main_bit_allocation (ltmin, bit_alloc, &adb, &fr_ps);
    
		if (error_protection)
		    I_CRC_calc (&fr_ps, bit_alloc, &crc);
    
		encode_info (&fr_ps, &bs);
    
		if (error_protection)
		    encode_CRC (crc, &bs);
    
		I_encode_bit_alloc (bit_alloc, &fr_ps, &bs);
		I_encode_scale (scalar, bit_alloc, &fr_ps, &bs);
		I_subband_quantization (scalar,/** */sb_sample, j_scale, *j_sample,
				        bit_alloc, *subband, &fr_ps);
		I_sample_encoding (*subband, bit_alloc, &fr_ps, &bs);
   
		/* YBKim060695 ancillary */
		if(ancillaryUse)
		    ancillary_encode (&fr_ps,&bs,adb);
		else
		    for (i = 0; i < adb; i++)
			put1bit (&bs, 0);

	        break;
 
/***************************** Layer 2 **********************************/
 
	case 2: if (aiff != 1)
		{
		    for (i = 0; i < 3; i++)
			for (j = 0;j < 12; j++)
			    for (k = 0; k < stereo; k++)
			    {
				window_subband (&win_buf[k], &(*win_que)[k][0], k);
				filter_subband (&(*win_que)[k][0], &(sb_sample)[k][i][j][0]);
			    }
					 
		    l = 0;
		    m = stereo;

		    II_scale_factor_calc (&fr_ps, sb_sample, scalar, fr_ps.sblimit, l, m);
		    if (info.multiling_ch > 0)
		        II_scale_factor_calc (&fr_ps, sb_sample, scalar, fr_ps.sblimit_ml, 7, 7 + info.multiling_ch);
    
		    pick_scale (scalar, &fr_ps, max_sc, cha_sw, aug_cha_sw, aiff);
		    if (fr_ps.actual_mode == MPG_MD_JOINT_STEREO)
		    {
			II_combine_LR (sb_sample, *j_sample, fr_ps.sblimit);
			II_scale_factor_calc (&fr_ps, *j_sample, j_scale, fr_ps.sblimit, 0, 1);
		    }
		
				 
		    /* this way we calculate more mono than we need */
                    /* but it is cheap */

		    II_Psycho_One (buffer, max_sc, ltmin, &fr_ps, smr, spiki, aiff);
		    if (info.multiling_ch > 0)
		    {
			if (odd_ml)
			    II_Psycho_One_ml (buffer, max_sc, ltmin, &fr_ps, smr, spiki);
			if (info.multiling_fs)
			{
			    pick_scale_ml_2 (&fr_ps, sb_sample, max_sc);
			    for (k = 7; k < 7 + info.multiling_ch; k++) 
				II_smr (&ltmin[k][0], &smr[k][0], &spiki[k][0], &max_sc[k][0], fr_ps.sblimit_ml, 0, fr_ps.sblimit_ml);
			}
		    }

		    II_transmission_pattern (scalar, scfsi, &fr_ps);

		    II_main_bit_allocation (smr, ltmin, scfsi, bit_alloc, &adb,
					    &fr_ps, &aiff, sb_sample, 
					    scalar, max_sc, buffer_matr,
					    spiki, *j_sample, j_scale,
					    dyn_cr, aug_dyn_cr,
					    scfsi_dyn, scalar_dyn);

		    if (error_protection)
			II_CRC_calc (&fr_ps, bit_alloc, scfsi, &crc);

		    encode_info (&fr_ps, &bs);
    
		    if (error_protection)
			encode_CRC (crc, &bs);

		    II_encode_bit_alloc (bit_alloc, &fr_ps, &bs);
   
		    k = 0;
		    i = stereo;
		    II_encode_scale (bit_alloc, scfsi, scalar, lfe_alloc,
				     lfe_scalar, &fr_ps, &bs, &k , &i);
		    II_subband_quantization (scalar, sb_sample, j_scale, *j_sample,
					     bit_alloc, *subband, &fr_ps);
		    II_sample_encoding (*subband, bit_alloc, &fr_ps, &bs);
        
/*******************************************************/
/* 08/03/1995 JMZ Multilingual */
		    if (info.multiling_ch > 0)
		    {
			k = 7;
			i = info.multiling_ch + 7;		 
			II_encode_bit_alloc_ml (bit_alloc, &fr_ps, &bs);
			II_encode_scale_ml (bit_alloc, scfsi, scalar, &fr_ps, &bs, &k, &i);
			II_subband_quantization_ml (scalar, sb_sample, j_scale,
						    *j_sample, bit_alloc, *subband, &fr_ps);
			II_sample_encoding_ml (*subband, bit_alloc, &fr_ps, &bs);
		    }
/* 08/03/1995 JMZ Multilingual */
/*******************************************************/


		    /* YBKim060695 ancillary */
		    if (ancillaryUse)
			   ancillary_encode (&fr_ps, &bs, adb);
		    else
			for (i = 0; i < adb; i++)
			    put1bit (&bs, 0);
		}

/*********************** Now Layer 2 with MC extension **************************/

		else
		{
		    if (verbosity >= 2)
			printf ("Adb1: %ld\n", adb);
		    if (lfe)
		    {
			lfe_bits = adb;
			lfe_filter (&win_buf[fr_ps.lfe_pos], *lfe_sample);
			lf_scalefactor (*lfe_sample, &lfe_scalar);
			lfe_allocation (&lfe_alloc, &adb);
			lfe_bits -= adb;     
		    }
       
		    for (i = 0; i < 3; i++)
			for (j = 0; j < 12; j++)
			{
			    l = 0;
	   
			    for (k = 0; k < 7; k++)
			    {
				if (lfe && (k == fr_ps.lfe_pos))
				    l +=1; 
				window_subband (&win_buf[l], &(*win_que)[k][0], k);
				filter_subband (&(*win_que)[k][0], &(sb_sample)[k][i][j][0]);
				l++;
			    }
			}
	
#ifdef Augmentation_7ch
		    if (stereoaug == 2)
		    {
			normalizing_aug (sb_sample, &fr_ps);
			matricing_aug (sb_sample, &fr_ps);
		    }
		    else
#endif
			normalizing (sb_sample, &fr_ps);
    
		    matricing (sb_sample, &fr_ps);
    
		    l = 0;
		    if (stereoaug == 2) m = 12;
		    else		m = 7;

		    II_scale_factor_calc (&fr_ps, sb_sample, scalar, fr_ps.sblimit, 0, stereo);
		    II_scale_factor_calc (&fr_ps, sb_sample, scalar, fr_ps.sblimit_mc, stereo, m);
		    if (info.multiling_ch > 0)
		        II_scale_factor_calc (&fr_ps, sb_sample, scalar, fr_ps.sblimit_ml, 7, 7 + info.multiling_ch);

		    pick_scale (scalar, &fr_ps, max_sc, cha_sw, aug_cha_sw, aiff);
    
		    if (fr_ps.actual_mode == MPG_MD_JOINT_STEREO)
		    {
			if (verbosity >= 2)
			{
			    printf ("JOINT!!\n");
			    fflush (stdout);
			}
			i = 0;		
			II_combine_LR (sb_sample, *j_sample, fr_ps.sblimit);
			II_scale_factor_calc1 (*j_sample, j_scale, fr_ps.sblimit, i);
		    }
    
		    II_Psycho_One (buffer_matr, max_sc, ltmin, &fr_ps, smr, spiki, aiff);
		    if (info.multiling_ch > 0)
		    {
			if (odd_ml)
			    II_Psycho_One_ml (buffer_matr, max_sc, ltmin, &fr_ps, smr, spiki);
    
			if (info.multiling_fs)
			{
			    pick_scale_ml_2 (&fr_ps, sb_sample, max_sc);
			    for (k = 7; k < 7 + info.multiling_ch; k++) 
				II_smr (&ltmin[k][0], &smr[k][0], &spiki[k][0], &max_sc[k][0], fr_ps.sblimit_ml, 0, fr_ps.sblimit_ml);
			}
		    }
				    
		    II_transmission_pattern (scalar, scfsi, &fr_ps);
    
		    /*in case of ext_bit_stream adb is the complete number of bits!! 06/22/94, SR*/
     
		    II_main_bit_allocation (smr, ltmin, scfsi, bit_alloc, &adb,
					    &fr_ps, &aiff, sb_sample, scalar,
					    max_sc, buffer_matr, spiki,
					    *j_sample, j_scale, dyn_cr, aug_dyn_cr, scfsi_dyn, scalar_dyn);
		    if (verbosity >= 2)
			printf ("Adb2: %ld\n", adb);
					    
		    if (info.mc_prediction_on)
			set_mc_pred (&fr_ps);

		    /* PREDISTORTION, 4/7/94, SR*/
		    if (fr_ps.config == 320 &&
			FALSE &&		    /* still a problem 7/2/97 FdB */
		    	info.matrix == 0 &&
			info.mode != MPG_MD_JOINT_STEREO &&
			info.dyn_cross_on == 0 &&
#ifdef Augmentation_7ch
			stereoaug == 0 &&
#endif
			info.mc_prediction_on == 0)
		    {
			adb = (whole_SpF + extra_slot) * bitsPerSlot - lfe_bits;
			predistortion (sb_sample, scalar, bit_alloc, *subband,
				       &fr_ps, smr, scfsi, &adb, scfsi_dyn);
			predis = 1; 
			if (verbosity >= 2)
			    printf ("Adb3: %ld\n", adb);
		    }	    
/*
*/
		    /********** PREDISTORTION-END*****/
    
		    if (error_protection)
			II_CRC_calc (&fr_ps, bit_alloc, scfsi, &crc);
			    
		    lfe_alloc = 4;
		    II_CRC_calc_mc (&fr_ps, bit_alloc, lfe_alloc, scfsi, &crcmc);
		    lfe_alloc = 0;

#ifdef Augmentation_7ch
		    if (stereoaug == 2)
			II_CRC_calc_aug (&fr_ps, bit_alloc, scfsi, &crcaug);
#endif

		    encode_info (&fr_ps, &bs);
		   
		    if (error_protection)
			encode_CRC (crc, &bs);
   
		    II_encode_bit_alloc (bit_alloc, &fr_ps, &bs);
		    k = 0;
		    i = stereo;		 
   
		    II_encode_scale (bit_alloc, scfsi, scalar, lfe_alloc, lfe_scalar, &fr_ps, &bs, &k, &i);
		    
		    II_subband_quantization (scalar, sb_sample, j_scale,
					     *j_sample, bit_alloc, *subband, &fr_ps);
				     
				     
		    II_sample_encoding (*subband, bit_alloc, &fr_ps, &bs);

		    /***** Now bitstream for the surround channels **************/

		    encode_info_mc1 (&fr_ps, &bs);
		    
		    encode_CRC (crcmc, &bs);
		    
		    encode_info_mc2 (&fr_ps, &bs);    /*new draft 5.7.93 SR*/
		    
		    if (lfe)
		    {
		        lfe_alloc = 4;
		        II_encode_lfe_alloc (lfe_alloc, &fr_ps, &bs);
		    }
    
		    II_encode_bit_alloc_mc (bit_alloc, &fr_ps, &bs);
		    k = stereo;
		    i = stereo + stereomc;
		    
		    II_encode_scale (bit_alloc, scfsi, scalar, lfe_alloc, lfe_scalar, &fr_ps, &bs, &k, &i);
   
		    II_subband_quantization_mc (scalar, sb_sample, j_scale,
					       *j_sample, bit_alloc, *subband, &fr_ps);
   
		    if (lfe) 
		        II_lfe_quantisation (lfe_scalar, *lfe_sample, lfe_alloc,
					     *lfe_sbband, &fr_ps);
				       
		    II_sample_encoding_mc (*subband, *lfe_sbband, bit_alloc, lfe_alloc, &fr_ps, &bs);

		    if (info.multiling_ch > 0)
		    {
			k = 7;
			i = info.multiling_ch + 7;		 
			II_encode_bit_alloc_ml (bit_alloc, &fr_ps, &bs);
			II_encode_scale_ml (bit_alloc, scfsi, scalar, &fr_ps, &bs, &k, &i);
	
			II_subband_quantization_ml (scalar, sb_sample, j_scale,
						    *j_sample, bit_alloc, *subband, &fr_ps);
	
	
			II_sample_encoding_ml (*subband, bit_alloc, &fr_ps, &bs);
		    }

#ifdef Augmentation_7ch
		    if (stereoaug == 2)
		    {
			/* bitstream for the augmentation channels */
			encode_info_aug (&fr_ps, &bs);
			
			encode_CRC (crcaug, &bs);
			
			II_encode_bit_alloc_aug (bit_alloc, &fr_ps, &bs);

			k = stereo + stereomc;
			i = stereo + stereomc + stereoaug;
			II_encode_scale (bit_alloc, scfsi, scalar, lfe_alloc, lfe_scalar, &fr_ps, &bs, &k, &i);
       
			II_subband_quantization_aug (scalar, sb_sample, j_scale,
						     *j_sample, bit_alloc, *subband, &fr_ps);
       
			II_sample_encoding_aug (*subband, bit_alloc, &fr_ps, &bs);
		    }
#endif

		    frameBits = sstell (&bs) - sentBits;
		    frameBits += info.n_ad_bytes * 8;
		    if (info.ext_bit_stream_present)
			frameBits += 40;

		    if (frameBits < (whole_SpF + extra_slot) * bitsPerSlot)
		    {
			adb = (whole_SpF + extra_slot) * bitsPerSlot - frameBits;
			if (verbosity >= 2)
			    printf ("Adb4: %ld\n", adb);
			adb += info.n_ad_bytes * 8;
			if (info.ext_bit_stream_present)
			    adb += 40;
		    }
		    else if (verbosity >= 2)
			printf ("Adb4: %ld but %ld bits overflow\n",
				adb, frameBits - (whole_SpF + extra_slot) * bitsPerSlot);
   
		    /* YBKim060695 ancillary */
		    if (ancillaryUse)
			ancillary_encode (&fr_ps, &bs, adb);
		    else
			for (i = 0; i < adb; i++)
			    put1bit (&bs, 0);
		}
		break;
/***************************** Layer 3, not done yet!!*******************/

/*	case 3: break; */

	}
 
	frameBits = sstell (&bs) - sentBits;
	if (frameBits % bitsPerSlot)   /* a program failure */
	    fprintf (stderr, "Sent %ld bits = %ld slots plus %ld adb: %ld\n",
		     frameBits, frameBits / bitsPerSlot,
		     frameBits % bitsPerSlot, adb);
	sentBits += frameBits;

#ifdef DEBUG
	if (verbosity >= 3)
	{
	   if ( (info.lay==2) && 
		((int) aiff_ptr.sampleRate == 48000) && 
		(info.ext_bit_stream_present == 0) && 
		(frameBits != 24*bitrate[info.lay-1][info.bitrate_index]) 
	      ) /* for debugging purposes WtK */
	      fprintf (stderr, "\n***Error. %d bits have been written instead of %d bits.\n",
			     frameBits, 24*bitrate[info.lay-1][info.bitrate_index]);
	}
#endif

#ifdef PRINTOUT
	if (verbosity >= 3)
	{
#ifdef Augmentation_7ch
	    if (stereoaug == 2)
		printf ("\nFrame %4lu   channel 1  channel 2  channel 3  channel 4  channel 5  channel 6  channel 7\n", frameNum-1);
	    else
#endif
		printf ("\nFrame %4lu   channel 1  channel 2  channel 3  channel 4  channel 5\n", frameNum-1);
    
	    for (loop_subband = 0; loop_subband < SBLIMIT; loop_subband++)
	    {
		l = sbgrp[i];
		printf ("subband %2d :", loop_subband);
		for (loop_channel = 0; loop_channel < stereo + stereomc + stereoaug; loop_channel++)
		{
		    /* make loop_alloc point to the alloc-table in fr_ps */
		    k = transmission_channel (&fr_ps, l, loop_channel);
		    if (loop_channel < stereo)
		    	loop_alloc = fr_ps.alloc;
		    else
		    	loop_alloc = fr_ps.alloc_mc;
		    loop_bits = (* loop_alloc)[loop_subband][bit_alloc[k][loop_subband]].bits;
		    if ((k == 1) && 
			(info.mode == MPG_MD_JOINT_STEREO) &&
			(loop_subband >= fr_ps.jsbound))
			printf ("      js   ");
		    else
		    {
			if ((* loop_alloc)[loop_subband][bit_alloc[k][loop_subband]].steps == 0)
			    printf("    - bits ");
			else
			{
			    if ((* loop_alloc)[loop_subband][bit_alloc[k][loop_subband]].group == 1)
			    loop_bits /= 3;
			    printf ("%5.1f bits ", loop_bits);
			}
		    }
		} /* for (loop_channel..) */
		printf("\n");
	    } /* for (loop_subband..) */
    
	    /* Now the ML part , WtK */
	    if (info.multiling_ch > 0)
	    {
		printf ("\nFrame %4lu ", frameNum - 1);
		for (loop_channel=7;loop_channel<7+info.multiling_ch; loop_channel++)
		    printf("  MLchann %d", loop_channel);
		printf ("\n");
		for (loop_subband = 0; loop_subband < SBLIMIT; loop_subband++)
		{
		    printf ("subband %2d :", loop_subband);
		    for (loop_channel = 0; loop_channel < info.multiling_ch; loop_channel++)
		    {
			/* make loop_alloc point to the alloc-table in fr_ps */
			loop_alloc = fr_ps.alloc_ml;
			loop_bits = (* loop_alloc)[loop_subband][bit_alloc[loop_channel][loop_subband]].bits;
			if ((* loop_alloc)[loop_subband][bit_alloc[loop_channel][loop_subband]].steps == 0)
			    printf("    - bits ");
			else
			{
			    if ((* loop_alloc)[loop_subband][bit_alloc[loop_channel][loop_subband]].group == 1)
				loop_bits /= 3;
			    printf ("%5.1f bits ", loop_bits);
			}
		    } /* for (loop_channel..) */
		    printf("\n");
		} /* for (loop_subband..) */
	    } /*n_ml_ch>0*/
	    fflush (stdout);
	} /* verbosity */
#endif

	if (info.multiling_fs == 1)
	    odd_ml = 1 - odd_ml; /* ML half fs */
	/****************************************************************/
	/*                                                              */
	/*                   END OF FRAME LOOP                          */
	/*                                                              */
	/****************************************************************/
    } /* end of while (get_audio) - loop */


    close_bit_stream_w (&bs);
    
    if (info.ext_bit_stream_present == 1)
    {
	open_bit_stream_r (&bs, encoded_file_name, BUFFER_SIZE);
	if (encoded_file_name_ext[0] == NULL_CHAR)
	    strcat (strcpy (encoded_file_name_ext, encoded_file_name), DFLT_EXT_EXT);
	if (verbosity >= 2) printf(">>> file name of extension bitstream is: %s\n", encoded_file_name_ext);
	open_bit_stream_w (&bs_ext, encoded_file_name_ext, BUFFER_SIZE);
	if (encoded_file_name_mpg[0] == NULL_CHAR)
	    strcat (strcpy (encoded_file_name_mpg, encoded_file_name), DFLT_EXT);
	open_bit_stream_w (&bs_mpg, encoded_file_name_mpg, BUFFER_SIZE);

	for (m = 0; m < frameNum; m++)
	{
	    i = (((double) samplesPerFrame / s_freq[info.sampling_frequency]) *
		  (double) bitrate[info.lay-1][info.bitrate_index]) - (info.n_ad_bytes * 8);

	    for (j = 0; j < i; j++)
	    {
		crc_buffer[0] = get1bit (&bs);
		put1bit (&bs_mpg, crc_buffer[0]);
	    }
	    for (j = 0; j < info.n_ad_bytes * 8; j++)
		put1bit (&bs_mpg, 0);
	  
	    encode_info_ext1 (&fr_ps, &bs_ext);

	    for (i = 0; i < 14; i++)
		crc_buffer[i] = getbits (&bs, 8);
	    crc_buffer[14] = getbits (&bs, 4);

	    II_CRC_calc_ext (&fr_ps, crc_buffer, &crcext);

	    encode_info_ext2 (&fr_ps, &bs_ext, crcext);

	    for (i = 0; i < 14; i++)	
		putbits (&bs_ext, crc_buffer[i], 8);
	    putbits (&bs_ext, crc_buffer[14], 4);
	
	    i = (info.ext_length * 8) - 40 - 116;
	    for (j = 0; j < i; j++)
	    {
		crc_buffer[0] = get1bit (&bs);
		put1bit (&bs_ext, crc_buffer[0]);
	    }

	    for (j = 0; j < 5 + info.n_ad_bytes; j++)
		crc_buffer[i] = getbits (&bs, 8);
	}

	close_bit_stream_r (&bs);
	close_bit_stream_w (&bs_ext);
	close_bit_stream_w (&bs_mpg);
	
	strcpy (blubb, "rm ");
	strcat (blubb, encoded_file_name);
#ifdef DEBUG
        if (verbosity >= 3)
	    printf ("%s\n", blubb);
#endif 
	system (blubb);

	strcpy (blubb, "mv ");
	strcat (blubb, encoded_file_name_mpg);
	strcat (blubb, " ");
	strcat (blubb, encoded_file_name);
#ifdef DEBUG
        if (verbosity >= 3)
	    printf ("%s\n", blubb);
#endif 
	system (blubb);
    }    	
    
    if (verbosity >= 2)
	printf ("Avg slots/frame = %.3f; b/smp = %.2f; br = %.3f kbps\n",
		(FLOAT) sentBits / (frameNum * bitsPerSlot),
		(FLOAT) sentBits / (frameNum * samplesPerFrame),
		(FLOAT) sentBits / (frameNum * samplesPerFrame) *
			s_freq[info.sampling_frequency]);
    
    if (fclose (musicin) != 0)
    {
	printf ("Could not close \"%s\".\n", original_file_name);
	exit (2);
    }
    
#ifdef  MACINTOSH
	set_mac_file_attr(encoded_file_name, VOL_REF_NUM, CREATOR_ENCODE,
			  FILETYPE_ENCODE);
#endif

    if (verbosity >= 2)
    {
	printf ("Encoding of \"%s\" with psychoacoustic model %d is finished\n",
	        original_file_name, model);
	if (aiff == 1)
	    printf (" It is a multichannel file !\n");
	else
	    printf (" It is a twochannel file!\n");	   
	printf ("The MPEG encoded output file name is \"%s\"\n",
		encoded_file_name);
    }
}
 
/************************************************************************
/*
/* usage
/*
/* PURPOSE:  Writes command line syntax to the file specified by #stderr#
/*
/************************************************************************/

static void usage (void)  /* print syntax & exit */
{
    fprintf (stderr, "usage: %s                         queries for all arguments, or\n", programName);
    fprintf (stderr, "       %s [-l lay][-m mode][-r sfrq][-b br][-B br][-d emp][-C config]\n", programName);
#ifdef Augmentation_7ch
    fprintf (stderr, "          [-x matr][-k tca][-y dynX][-X aug_matr][-K aug_tca][-Y aug_dynX]\n");
#else
    fprintf (stderr, "          [-x matr][-k tca][-y dynX]\n");
#endif
    fprintf (stderr, "          [-c][-o][-e][-w][-g][-v verb][-L][-a][-P][-D] inputPCM [outBS]\n");
    fprintf (stderr, "where\n");
    fprintf (stderr, " -l lay   use layer <lay> coding   (dflt %4u)\n", DFLT_LAY);
    fprintf (stderr, " -m mode  channel mode : s/d/j/m   (dflt %4c)\n", DFLT_MOD);
    fprintf (stderr, " -n mode  surround mode : s/d/j/m  (dflt %4c)\n", DFLT_MOD);
    fprintf (stderr, " -r sfrq  input smpl rate in kHz   (dflt %4.1f)\n", DFLT_SFQ);
    fprintf (stderr, " -b br    total bitrate in kbps    (dflt %4u)\n", DFLT_BRT);
    fprintf (stderr, " -B br    MPEG1 bitrate in kbps    (dflt %4u)\n", DFLT_BRT);
    fprintf (stderr, " -d emp   de-emphasis n/5/c        (dflt %4c)\n", DFLT_EMP);
    fprintf (stderr, " -c       mark as copyright\n");
    fprintf (stderr, " -o       mark as original\n");
    fprintf (stderr, " -e       add error protection\n");
    fprintf (stderr, " -x matr  matrix                   (dflt 0)\n");
    fprintf (stderr, " -k tca   set channel-switching    (dflt -2)\n");
    fprintf (stderr, " -y dynX  set dyn_cross mode       (dflt 0)\n");             
#ifdef Augmentation_7ch
    fprintf (stderr, " -X Amatr 7.1-ch aug. matrix       (dflt 0)\n");
    fprintf (stderr, " -K Atca  set 7.1-ch aug. tca      (dflt 0)\n");
    fprintf (stderr, " -Y AdynX 7.1-ch aug. DynX mode    (dflt 0)\n");             
#endif
    fprintf (stderr, " -w       set phantom coding       (dflt off)\n");
    fprintf (stderr, " -L       set LFE on               (dflt off)\n");
    fprintf (stderr, " -C mode  set configuration        (dflt 320)\n");
    fprintf (stderr, " -g       set MultiLingual file    (dflt off) input ML filename = inputPCM+\"%s\"\n", DFLT_ML_EXT);
    fprintf (stderr, " -P       set mc prediction on     (dflt off)\n");
    fprintf (stderr, " -D       set dyn_cross_LR on      (dflt off)\n");
    fprintf (stderr, " -n nr    number of anc data Bytes (dflt %d)\n", DFLT_NADB);
    fprintf (stderr, " -a       set ancillary data on    (dflt off)\n");
    fprintf (stderr, " -v verb  level of verbosity 0-3 	 (dflt %d)\n", DFLT_VERB);
    fprintf (stderr, " inputPCM input PCM sound file (standard or AIFF)\n");
    fprintf (stderr, " outBS    output bit stream of encoded audio (dflt inName+%s)\n", DFLT_EXT);
    exit (1);
}

/************************************************************************
/*
/* aiff_check
/*
/* PURPOSE:  Checks AIFF header information to make sure it is valid.
/*           Exits if not.
/*
/************************************************************************/

void aiff_check (char *file_name, IFF_AIFF *pcm_aiff_data)
                          /* Pointer to name of AIFF file */
                          /* Pointer to AIFF data structure */
{

    if (strcmp (pcm_aiff_data->sampleType, IFF_ID_SSND) != 0)
    {
       printf ("Sound data is not PCM in \"%s\".\n", file_name);
       //exit (1);
    }

    if (SmpFrqIndex ((long) pcm_aiff_data->sampleRate) < 0)
    {
       printf ("in \"%s\".\n", file_name);
       exit (1);
    }

    if (pcm_aiff_data->sampleSize != sizeof(short) * BITS_IN_A_BYTE)
    {
        printf ("Sound data is not %d bits in \"%s\".\n",
               sizeof(short) * BITS_IN_A_BYTE, file_name);
        exit(1);
    }

    if (pcm_aiff_data->numChannels < 1 || pcm_aiff_data->numChannels > 8)
    {
       printf ("Sound data is not 1 - 8 channel in \"%s\".\n", file_name);
       exit (1);
    }

    if (pcm_aiff_data->blkAlgn.blockSize != 0)
    {
       printf ("Block size is not %lu bytes in \"%s\".\n", 0, file_name);
       exit (1);
    }

    if (pcm_aiff_data->blkAlgn.offset != 0)
    {
       printf ("Block offset is not %lu bytes in \"%s\".\n", 0, file_name);
       exit (1);
    }
}
