/**********************************************************************
 * ISO MPEG Audio Subgroup Software Simulation Group (1996)
 * ISO 13818-3 MPEG-2 Audio Multichannel Decoder
 *
 * $Id: musicout.c 1.11 1996/04/18 05:37:23 rowlands Exp $
 *
 * $Log: musicout.c $
 * Revision 1.11  1996/04/18 05:37:23  rowlands
 * Release following Florence meeting
 *
 * Revision 1.10  1996/02/12 07:13:06  rowlands
 * Release following Munich meeting
 *
 * Revision 1.9.1.1  1996/01/20  17:28:45  rowlands
 * Received from Ralf Schwalbe (Telekom FTZ) - includes prediction
 *
 * Revision 1.7.1.3  1995/08/14  08:13:37  tenkate
 * ML-LSF added Warner ten Kate 7/8/95 (Philips)
 * debugging
 * change "alloc" and "sblimit" into "alloc_ml" and
 * "sblimit_ml" where appropriate.
 * adapt sample-loops to 6 or 12 depending on half or full ML rate.
 *
 * Revision 1.7.1.1  1995/07/14  06:12:46  rowlands
 * Updated dynamic crosstalk from FTZ: revision FTZ_03
 *
 * Revision 1.3.3.1  1995/06/16  08:00:46  rowlands
 * Input from Sang Wook Kim (Samsung AIT)
 *
 * Revision 1.3.1.1  1995/06/14  04:36:30  rowlands
 * Added support for dematrixing procedure 2.
 *
 **********************************************************************/

/**********************************************************************
 *   date   programmers                comment                        *
 * 2/25/91  Douglas Wong        start of version 1.0 records          *
 * 3/06/91  Douglas Wong        rename setup.h to dedef.h             *
 *                              removed extraneous variables          *
 *                              removed window_samples (now part of   *
 *                              filter_samples)                       *
 * 3/07/91  Davis Pan           changed output file to "codmusic"     *
 * 5/10/91  Vish (PRISM)        Ported to Macintosh and Unix.         *
 *                              Incorporated new "out_fifo()" which   *
 *                              writes out last incomplete buffer.    *
 *                              Incorporated all AIFF routines which  *
 *                              are also compatible with SUN.         *
 *                              Incorporated user interface for       *
 *                              specifying sound file names.          *
 *                              Also incorporated user interface for  *
 *                              writing AIFF compatible sound files.  *
 * 27jun91  dpwe (Aware)        Added musicout and &sample_frames as  *
 *                              args to out_fifo (were glob refs).    *
 *                              Used new 'frame_params' struct.       *
 *                              Clean,simplify, track clipped output  *
 *                              and total bits/frame received.        *
 * 7/10/91  Earle Jennings      changed to floats to FLOAT            *
 *10/ 1/91  S.I. Sudharsanan,   Ported to IBM AIX platform.           *
 *          Don H. Lee,                                               *
 *          Peter W. Farrett                                          *
 *10/ 3/91  Don H. Lee          implemented CRC-16 error protection   *
 *                              newly introduced functions are        *
 *                              buffer_CRC and recover_CRC_error      *
 *                              Additions and revisions are marked    *
 *                              with "dhl" for clarity                *
 * 2/11/92  W. Joseph Carter    Ported new code to Macintosh.  Most   *
 *                              important fixes involved changing     *
 *                              16-bit ints to long or unsigned in    *
 *                              bit alloc routines for quant of 65535 *
 *                              and passing proper function args.     *
 *                              Removed "Other Joint Stereo" option   *
 *                              and made bitrate be total channel     *
 *                              bitrate, irrespective of the mode.    *
 *                              Fixed many small bugs & reorganized.  *
 **********************************************************************
 *                                                                    *
 *                                                                    *
 *  MPEG/audio Phase 2 coding/decoding multichannel                   *
 *                                                                    *
 *  Version 1.0                                                       *
 *                                                                    *
 *  7/27/93        Susanne Ritscher,  IRT Munich                      *
 *                                                                    *
 *                  thanks to                                         *
 *                  Ralf Schwalbe,    Telekom FTZ Berlin              *
 *                  Heiko Purnhagen,  Uni Hannover                    *
 *                                                                    *
 *  Version 2.0                                                       *
 *                                                                    *
 *  8/27/93        Susanne Ritscher, IRT Munich                       *
 *                 Channel-Switching is working                       *
 *                                                                    *
 *  Version 2.1                                                       *
 *                                                                    *
 *  9/1/93         Susanne Ritscher,  IRT Munich                      *
 *                 all channels normalized                            *
 *                                                                    *
 *  Version 3.0                                                       *
 *                                                                    *
 *  06/16/94       Ralf Schwalbe, Telekom FTZ Berlin                  *
 *                 all sources and variables adapted due to MPEG-2 -  *
 *                 DIS from March 1994                                *
 *                  - dematrix and denormalize procedure              *
 *                  - new tc - allocation (0-7)                       *
 *                  - some new structures and variables as a basis    *
 *                    for further decoding modes                      *
 **********************************************************************
 *								      *
 *  Version 1.0                                                       *
 *                                                                    *
 *  11/04/94       Ralf Schwalbe,  Telekom FTZ Berlin                 *
 *                  - decoding tc-allocation                          *
 *                  - some new subroutines, globale variables and     *
 *                    structures (important to handle the ext. bitst.)*
 *                  - changed all functions to ANSI-C funktion header *
 *		    - corrected some bugs to decode bitstreams > 512kB*
 *								      *
 *  Version 1.1                                                       *
 *                                                                    *
 *  12/07/94       Ralf Schwalbe,  Telekom FTZ Berlin                 *
 *                  - decoding extension bitstream                    *
 *                                                                    *
 *  Version 1.1.1                                                     *
 *                 Ralf Schwalbe, Telekom FTZ Berlin                  *
 *                  - fixed some bugs                                 *
 *                                                                    *
 *  Version 1.2                                                       *
 *                                                                    *
 *  6/21/95       Ralf Schwalbe, Deutsche Telekom FTZ Berlin          *
 *            	    - decoding dynamic crosstalk                      *
 *                  - decoding phantom center                         * 
 *                  - decoding MPEG1 compatible part only (stereo)    *
 *                  - corrected some settings and bugs                *
 * 								      *
 *  7/12/95	  Ralf Schwalbe, Deutsche Telekom FTZ Berlin          *
 *		    - corrected dynamic crosstalk 		      *
 *		    - 3/2,3/1,3/0,2/1, channel configurations         *
 *                    are working                                     *
 *		  						      *
 * 10/31/95	  Ralf Schwalbe, Deutsche Telekom FTZ Berlin	      *
 *		    - decoding of LFE-channel is working  	      *
 *		    - corrected any settings and bugs		      *
 *		    - corrected table-switch for channel mode < 3/2   *
 *		      and tc-allocation / dyn-crosstalk		      *
 *		    - still a problem with compl. bitstream 18/19     *
 * 01/12/96	  Ralf Schwalbe, Deutsche Telekom TZ Darmstadt	      *
 *		    - decoder prediction installed		      *
 *		    - problem with compl. bitstream 19 solved,        *
 * 03/27/96	  Ralf Schwalbe, Deutsche Telekom TZ Darmstadt        *
 *		    - Multilingual mode adapted for max. 7 channels   *
 *                                                                    *				      *		    
 *  Version 2.0                                                       *
 *                                                                    *
 * 01/28/97       Frans de Bont, Philips Sound & Vision, Eindhoven    *
 *		    - simultaneous use of tc allocation and dynamic   *
 *		      crosstalk working for all configurations	      *
 *		    - prediction working for all configurations	      *
 *		    - layer 1 MC working			      *
 *		    - variable bit rate and extension bitstreams      *
 *		    - fully compliant to 13818-3.2                    *
 *                                                                    *				      *		    
 **********************************************************************/
/**********************************************************************
 *                                                                    *
 *  06/06/95       Yeon Bae Thomas Kim,  Samsung AIT                  *
 *                 ancillary data is working                          *
 *								      *
 *  06/06/95       Sang Wook Kim,  Samsung AIT                        *
 *                 corrected some bugs                                *
 *								      *
 **********************************************************************/

#include        "common.h"
#include        "decoder.h"

/********************************************************************
*
*        This part contains the MPEG I / II decoder for Layers II.
*
*        Core of the Layer II decoder.  Default layer is Layer II.
*
*********************************************************************/

/* Global variable definitions for "musicout.c" */

char 		   *programName;
int 		   Frame_Bits,Bitrate;
double		   S_freq;

/* Implementations */

int main (int argc, char **argv)  /* R.S. 7 channels for ML */
{
typedef long PCM[7][3][SBLIMIT];		
	PCM  *pcm_sample;
	PCM  *pcm_sample_ml;			/* 10/03/1995 JMZ Multilingual*/
typedef unsigned int SAM[7][3][SBLIMIT];	
	SAM  *sample;
	SAM  *sample_ml;			/* 10/03/1995 JMZ Multilingual*/
typedef double FRA[7][SBLIMIT][3][12];		/* 7.10.93 R.S. mem - alloc for DOS */ 
	FRA  *fraction;
	FRA  *fraction_ml;			/*JMZ 09/03/1995 Multilingual */
typedef double FRA_HELP[12][7][3][SBLIMIT];	/* 10/03/1995 JMZ Multilingual*/
	FRA_HELP *fraction_help;
	FRA_HELP *fraction_help_ml;
typedef double FRA_BUF[2][8][36+PREDDEL];   
	FRA_BUF  *pred_buf; 
typedef double VE[7][HAN_SIZE];		
	VE  *w;

	frame_params      fr_ps;
	layer 		  info;
	Bit_stream        bs_mpg, bs_ext, bs_mc;
	FILE              *musicout, *lfe;
	FILE              *musicout_ml;			/* 10/03/1995 JMZ Multilingual*/
	unsigned long     sample_frames;
	unsigned long     sample_frames_ml;
	int		  i,jj, j, k, ii, stereo, done=FALSE, clip, sync,f;
	int               error_protection, crc_check = 1;
	int		  crc_error_count, total_error_count;
	int               crc_error_count_mc, total_error_count_mc;
#ifdef Augmentation_7ch
	int               crc_error_count_aug, total_error_count_aug;
#endif
	unsigned int      old_crc, new_crc;
	unsigned int      I_bit_alloc[3][7][SBLIMIT];
	unsigned int      bit_alloc[7][SBLIMIT], scfsi[7][SBLIMIT],
			  scale_index[7][3][SBLIMIT]; 
	unsigned int      bit_alloc_ml[7][SBLIMIT], scfsi_ml[7][SBLIMIT],
			  scale_index_ml[7][3][SBLIMIT]; /* 09/03/1995 JMZ Multilingual */
	unsigned long     bitsPerSlot, samplesPerFrame;
	IFF_AIFF          pcm_aiff_data;
	char 	      	  encoded_file_name[MAX_NAME_SIZE];
	char 	      	  encoded_file_name1[MAX_NAME_SIZE]; /* 8/11/92.sr*/
	char              decoded_file_name[MAX_NAME_SIZE];
	char              decoded_file_name_ml[MAX_NAME_SIZE];/* 10/03/1995 JMZ Multilingual*/
	char		  ext_bitstream_name[MAX_NAME_SIZE];
	char		  frame_name[MAX_NAME_SIZE];
	char		  lfe_file_name[MAX_NAME_SIZE];
	char              t[50];
	int               need_aiff;
	int		  l, m, print_out = 0, ml =0;
	int		  ch_start,lfe_init = 1;
	int		  tca_log = 0, dynx_log = 0, bits_log = 0, rate_log = 0, scfsi_log = 0, any_log = 0;
	short int	  lfe_tmp[12];
	unsigned int	  sym_sample_ml[12][7][3][SBLIMIT];	/* 18/03/1996 FDB Multilingual*/
	int		  part, layer_I_frames;
	int		  mpeg = 2; /* R.S. mpeg default for mc */
	int		  channels=2, mc_channel=0;
	unsigned long	  frameNum = 0L;
	unsigned long	  frameMod = 1L;
	unsigned long	  frameBits;
/*****************************************************************************/
	int hi, hu, ho;
	FILE *fp1;
	FILE *fp2;
	FILE *fp3;
	FILE *fp4;
	FILE *fp5;
	FILE *fp6;
/******************************************************************************/

#ifdef  MACINTOSH
	console_options.nrows = MAC_WINDOW_SIZE;
	argc = ccommand(&argv);
#endif
	/* Most large variables are declared dynamically to ensure
	   compatibility with smaller machines */

	pcm_sample = (PCM *) mem_alloc((long) sizeof(PCM), "PCM Samp");
	pcm_sample_ml = (PCM *) mem_alloc((long) sizeof(PCM), "PCM Samp");
	sample 	   = (SAM *) mem_alloc((long) sizeof(SAM), "Sample");
	sample_ml   = (SAM *) mem_alloc((long) sizeof(SAM), "Sample");
	fraction   = (FRA *) mem_alloc((long) sizeof(FRA), "fraction");  /* R.S. */
	fraction_ml   = (FRA *) mem_alloc((long) sizeof(FRA), "fraction");  /* R.S. */
	fraction_help = (FRA_HELP *) mem_alloc((long) sizeof(FRA_HELP), "fraction");
	fraction_help_ml = (FRA_HELP *) mem_alloc((long) sizeof(FRA_HELP), "fraction");
	pred_buf  = (FRA_BUF *) mem_alloc((long) sizeof(FRA_BUF), "pred_buf");
				
	w = (VE *) mem_alloc((long) sizeof(VE), "w");

	bs_mpg.header_size = 0;
	bs_mpg.bits = (unsigned char *) mem_alloc ((long) 13824, "MPG Bits");
	bs_ext.header_size = 0;
	bs_ext.bits = (unsigned char *) mem_alloc ((long) 16376, "EXT Bits");
	bs_mc.bits  = (unsigned char *) mem_alloc ((long) 30208, "MC Bits");

	fr_ps.header = &info;
	fr_ps.bs_mpg = &bs_mpg;
	fr_ps.bs_ext = &bs_ext;
	fr_ps.bs_mc  = &bs_mc;
	fr_ps.tab_num = -1;                /* no table loaded */
	fr_ps.tab_num_mc = -1;                /* no table loaded */
	fr_ps.tab_num_ml = -1;
	fr_ps.alloc = NULL;
	fr_ps.alloc_mc = NULL;
	fr_ps.alloc_ml = NULL;

	info.mode_ext = 0;
	info.version = MPEG_AUDIO_ID;
	info.bitrate_index = 0;
	info.lfe = 0;	    		   /* no low frequency effect channel present! */

	info.no_of_multi_lingual_ch = 0;
	info.multi_lingual_fs = 0;
	info.multi_lingual_layer = 0;

	info.ext_bit_stream_present = 0;
	info.ext_length = 0;
	info.n_ad_bytes = 0;    /* 7.12.94 R.S. */

	info.lfe = 0;

	for (i=0;i<HAN_SIZE;i++) for (j=0;j<5;j++) (*w)[j][i] = 0.0;

	program_information();
	programName = argv[0];
	if(argc==1)
	{        /* no command line args -> interact */
	   do
	   {
		  printf ("Enter encoded file name <required>: ");
		  fgets (encoded_file_name,81,stdin);
		  f = strlen(encoded_file_name)-4;	  /*cut off extension.8/11/92.sr*/
		  if (encoded_file_name[0] == NULL_CHAR)
			 printf ("Encoded file name is required. \n");
	   } while (encoded_file_name[0] == NULL_CHAR);

	   printf (">>> Encoded file name is: %s \n", encoded_file_name);
	   strcpy(encoded_file_name1, encoded_file_name);  /*8/11/92.sr*/
	   strcpy(&encoded_file_name1[f], DFLT_OPEXT_DEC);	  /*.dec-extension.8/11/92.sr*/
	   printf ("Enter MPEG decoded file name <%s>: ", encoded_file_name1);
	   fgets (decoded_file_name,81,stdin);
	   if (decoded_file_name[0] == NULL_CHAR)
		strcpy(decoded_file_name, encoded_file_name1);

/* JMZ 10/03/1995 Multilingual */

	   printf (">>> MPEG decoded file name is: %s \n", decoded_file_name);

           /* encoded_file_name1 only for help */
	   strcpy(encoded_file_name1, encoded_file_name);
	   strcpy(&encoded_file_name1[f], DFLT_IPEXT_EXT);	  /* .ext */
	   printf (">>> Enter MPEG 2 decoded extension filename <%s>: ", encoded_file_name1);
	   fgets(ext_bitstream_name,81,stdin);
	   if( ext_bitstream_name[0] == NULL_CHAR )
		strcpy(ext_bitstream_name, encoded_file_name1);
	   printf("Extension bitstream <%s> will be decoded \n",ext_bitstream_name);
	   /* 6/21/95 Ralf Schwalbe: stand a chance to decode MPEG1 compatible part */
	   if ((open_bit_stream_r (&bs_ext, ext_bitstream_name, BUFFER_SIZE)) == 0)
	   { 	
		printf("No extension bitstream <%s> present \n",ext_bitstream_name);
		printf("Do you want to decode an MPEG 2 bitstream ? (<y>/n) : ");
		fgets(t,81,stdin);
	   	if (*t == 'N' || *t == 'n') 
		{
			mpeg = 1;
			printf("An MPEG-1 Layer II bitstream will be decoded \n");
		}
	   	else
		{
			mpeg = 2;
			printf("An MPEG-2 Layer II bitstream will be decoded \n");
		}
	   } 
	   printf("Do you wish to write an AIFF compatible sound file ? (<y>/n) : ");
	   fgets(t,81,stdin);
	   if (*t == 'N' || *t == 'n') need_aiff = FALSE;
	   else                        need_aiff = TRUE;
	   if (need_aiff)
		printf(">>> An AIFF compatible sound file will be written\n");
	   else printf(">>> A non-headered PCM sound file will be written\n");
	   
	   printf("Do you want to print out all decoding information ? (y/<n>) :");
	   fgets(t,81,stdin);
	   if( *t == 'y' || *t == 'Y' ) print_out = 1;
	   else		      		print_out = 0;

	   printf(
		  "Do you wish to exit (last chance before decoding) ? (y/<n>) : ");
	   fgets(t,81,stdin);
	   if (*t == 'y' || *t == 'Y') exit(0);
	}

	else	/************** interpret CL Args *****************/
		/* default: -write an AIFF file
	 		    -print encoded bitstream information  */
	{        
	   int i=0, err=0;

#ifdef UNIX /* default values */
	   need_aiff = TRUE;
	   print_out = 0;   /* 10/31/95 Ralf Schwalbe: set print_out off in CL Args */
	   mpeg = 2;
#else
	   need_aiff = FALSE;
#endif
	   encoded_file_name[0] = NULL_CHAR;
	   decoded_file_name[0] = NULL_CHAR;
	   ext_bitstream_name[0] = NULL_CHAR;

	   while(++i<argc && err == 0)
	   {
		  char c, *token, *arg, *nextArg;
		  int  argUsed;

		  token = argv[i];
		  if(*token++ == '-')
		  {
			 if(i+1 < argc) nextArg = argv[i+1];
			 else           nextArg = "";
			 argUsed = 0;
			 while((c = *token++) != 0)
			 {
				if(*token /* NumericQ(token) */) arg = token;
				else                             arg = nextArg;
				switch(c)
				{
				   case 'a':  need_aiff = FALSE;
					      break;

				   case 'b':  bits_log = 1;
					      break;
					    			
				   case 'd':  dynx_log = 1;
					      break;
					    			
				   case 'h':  bs_mpg.header_size = 2304;
					      bs_ext.header_size = 2304;
					      break;

				   case 'i':  print_out = 1;
					      break;
					    			
				   case 'm':  mpeg = 1;
					      break;
#ifdef Augmentation_7ch
				   case 'n':  mpeg = 3;
					      break;
#endif
				   case 'r':  rate_log = 1;
					      break;
					    			
				   case 's':  scfsi_log = 1;
					      break;
					    			
				   case 't':  tca_log = 1;
					      break;
					    			
				   default:   fprintf(stderr,"%s: unrecognized option %c\n",
							  programName, c);
					      err = 1;
					      break;
				}
				if(argUsed)
				{
					if(arg == token) token = ""; /* no more from token */
					else  ++i; /* skip arg we used */
					arg = ""; argUsed = 0;
				}
			 }
		  }
		  else
		  {
		     if (encoded_file_name[0] == NULL_CHAR)
		     {
			strcpy (encoded_file_name, argv[i]);
			f = strlen (encoded_file_name);
			strcpy (&encoded_file_name[f], DFLT_IPEXT_MPG);
			strcpy (ext_bitstream_name, encoded_file_name);
			strcpy (&ext_bitstream_name[f], DFLT_IPEXT_EXT);
		     }
		     else if (decoded_file_name[0] == NULL_CHAR)
		     {
			strcpy (decoded_file_name, argv[i]);
			f = strlen (decoded_file_name);
			strcpy (&decoded_file_name[f], DFLT_OPEXT_DEC);
		     }
		     else
		     {
			fprintf (stderr, "%s: excess arg %s\n", programName, argv[i]);
			err = 1;
		     }
		  }
       } /* end while argc */

       if (bits_log || dynx_log || rate_log || scfsi_log || tca_log)
	  any_log = 1;


       if (err || encoded_file_name[0] == NULL_CHAR)
          usage ();  /* never returns */

       if ((open_bit_stream_r (&bs_ext, ext_bitstream_name, BUFFER_SIZE)) == 0)
       {
		printf("No extension bitstream <%s> present \n", ext_bitstream_name);
		ext_bitstream_name[0] = NULL_CHAR;
       }
       
       if (decoded_file_name[0] == NULL_CHAR)
       {
	  f = strlen (encoded_file_name) - 4;
	  strcpy (decoded_file_name, encoded_file_name);
	  strcpy (&decoded_file_name[f], DFLT_OPEXT_DEC);
       }

    }  /* else arg-line end */

    /* JMZ 10/03/1995 Multilingual */
    strcpy(decoded_file_name_ml, decoded_file_name);
    strcat(decoded_file_name_ml, DFLT_OPEXT_ML);

	/* report results of dialog / command line */
    printf("Input file = '%s' \nOutput file = '%s' ",
		   encoded_file_name, decoded_file_name);
    if (mpeg >= 2 && ext_bitstream_name[0] != NULL_CHAR)
    	printf("Ext_bitstream = '%s'\n", ext_bitstream_name);
    if(mpeg == 1)
	   	printf(">>>\nA MPEG 1 Layer-1/2 bitstream will be decoded\n");
	   else
		printf(">>>\nA MPEG 2 Layer-1/2 bitstream will be decoded\n");  
	  
    if(need_aiff) printf("Output file written in AIFF format\n");

    if ((musicout = fopen(decoded_file_name, "w+b")) == NULL)
    {
	   fprintf (stderr, "Could not create \"%s\".\n", decoded_file_name);
	   exit (1);
    }
    if (open_bit_stream_r (&bs_mpg, encoded_file_name, BUFFER_SIZE) == 0)
    {
	fprintf (stderr, "Could not open \"%s\".\n", encoded_file_name);
	exit (1);
    }

    if (need_aiff)
	  if (aiff_seek_to_sound_data(musicout) == -1)
	  {
		  fprintf (stderr, "Could not seek to PCM sound data in \"%s\".\n",
				 decoded_file_name);
		  exit (1);
	   }

    sample_frames = 0;
    sample_frames_ml = 0;
    total_error_count = total_error_count_mc = 0;
#ifdef Augmentation_7ch
    total_error_count_aug = 0;
#endif

    while (!end_bs (&bs_mpg))
    {
    /****************************************************************/
    /*                                                              */
    /*                  START OF FRAME LOOP                         */
    /*                                                              */
    /****************************************************************/

	   sync = seek_sync_mpg (&bs_mpg);
	   frameBits = bs_mpg.totbits;
	   if (!sync)
	   {
		  printf("\nFrame cannot be located, end of input stream \n");
		  done = TRUE;
		  /* finally write out the buffer */
		  out_fifo (*pcm_sample, 3, &fr_ps, done, musicout, &sample_frames, channels);
		  out_fifo_ml (*pcm_sample_ml, 3, &fr_ps, done, musicout_ml, &sample_frames_ml);
		  break;
	   }

	   info.lfe = 0;  		/* R.S. ! */
	   decode_info (&bs_mpg, &fr_ps);
	   hdr_to_frps (&fr_ps);

	   stereo = fr_ps.stereo;
	   error_protection = info.error_protection;
	   crc_error_count = 0;
	   if (frameNum == 0) WriteHdr (&fr_ps, stdout);  /* printout layer/mode */

	   if (frameNum % frameMod == 0)
	   {
		fprintf(stderr, "{%4lu}\r", frameNum++); fflush(stderr);
		if (frameNum / frameMod >= 100) frameMod *= 10;
	   }
	   else
		frameNum++;

	   Frame_Bits = 144*Bitrate/S_freq; 
	   if(info.padding)
		Frame_Bits++;    /* R.Schwalbe add a padding byte */
	   Frame_Bits*=8;	 /* R.Schwalbe to get bits per frame *8 */
	   if (error_protection) buffer_CRC(&bs_mpg, &old_crc);
          

           /* 7/8/95 added message WtK 7/8/95 */
	   if (info.lay == 3)
	   {
	     fprintf (stderr, "\nLayer = %d, not supported.\nexit.\n\n",info.lay); break;
	     exit (1);
	   }
	   
	   bs_mc.totbits = bs_mc.curpos = 0;

	   if (info.lay == 1)
	   {
	      if (mpeg > 1)
		 layer_I_frames = 3;
	      else
	      	 layer_I_frames = 1;

	      for (part = 0; part < layer_I_frames; part++)
	      {
		 bitsPerSlot = 32;
		 samplesPerFrame = 384;
		 ch_start = 0;
		 channels = stereo;

		 if (part > 0)
		 {
		    sync = seek_sync_mpg (&bs_mpg);
		    frameBits = bs_mpg.totbits;
		    if (!sync)
		    {
			   fprintf (stderr, "2nd or 3th Frame cannot be located, end of input stream\n");
			   exit (1);
		    }

		    decode_info (&bs_mpg, &fr_ps);
		    hdr_to_frps (&fr_ps);
	 
		    stereo = fr_ps.stereo;
		    error_protection = info.error_protection;
		    crc_error_count = 0;

		    Frame_Bits = 12*Bitrate/S_freq; 
		    if (info.padding)
			 Frame_Bits++;    /* R.Schwalbe add a padding byte */
		    Frame_Bits *=32;	 /* R.Schwalbe to get bits per frame *32 */
		    if (error_protection)
		       buffer_CRC (&bs_mpg, &old_crc);
          
		    /* 7/8/95 added message WtK 7/8/95 */
		    if (info.lay != 1)
		    {
		      fprintf (stderr, "Change of Layer = %d, not supported.\nexit.\n\n", info.lay);
		      exit (1);
		    }
		 }

		 I_decode_bitalloc (&bs_mpg, &fr_ps, I_bit_alloc[part], bits_log);
		 I_decode_scale (&bs_mpg, &fr_ps, I_bit_alloc[part], scale_index, part, scfsi_log);

		 if (error_protection)
		 {
			I_CRC_calc (&fr_ps, I_bit_alloc[part], &new_crc);
			if (new_crc != old_crc)
			{
			   printf(" \n ERROR in LAYER 1 - CRC! \n");
			   crc_error_count++;
			   total_error_count++;
			   recover_CRC_error (*pcm_sample, crc_error_count,
					 &fr_ps, musicout, &sample_frames, channels);
			}
			else
			   crc_error_count = 0;
		 }

		 clip = 0;

		 for (i=0;i<12;i++)
		 {
			I_buffer_sample (&bs_mpg, &fr_ps, (*sample), I_bit_alloc[part]);
if (!any_log)
{
			I_dequantize_sample ((*sample), part, I_bit_alloc[part], *fraction, &fr_ps, &i);
			I_denormalize_sample (*fraction, scale_index, part, &fr_ps, &i);
}
		 }   /* end of for loop */

		 if (mpeg > 1)
		 {
		    while (bs_mpg.curpos < bs_mpg.totbits-1)
		       bs_mc.bits[bs_mc.totbits++] = bs_mpg.bits[bs_mpg.curpos++];

		    if (part == 0)
		    {
		       mc_header (&bs_mc, &fr_ps);
		       mc_hdr_to_frps (&fr_ps);
		    }

		    bs_mc.totbits -= fr_ps.header->n_ad_bytes * 8;
		 }
	      }
	      if (mpeg > 1)
	      {
		 if (fr_ps.header->ext_bit_stream_present)
		 {
		    sync = seek_sync_ext (&bs_ext, &fr_ps);
		    frameBits += bs_ext.totbits;
		    if (!sync)
		    {
		       fprintf(stderr, "Extension frame cannot be located, end of input stream\n");
		       exit (1);
		    }

		    while (bs_ext.curpos < bs_ext.totbits)
		       bs_mc.bits[bs_mc.totbits++] = bs_ext.bits[bs_ext.curpos++];
		 }

		 for (i=0; i < channels; i++)
		    for (j=0; j < SBLIMIT; j++)
		       if (I_bit_alloc[0][i][j] > 0)
		          bit_alloc[i][j] = I_bit_alloc[0][i][j];
		       else if (I_bit_alloc[1][i][j] > 0)
		          bit_alloc[i][j] = I_bit_alloc[1][i][j];
		       else if (I_bit_alloc[2][i][j] > 0)
		          bit_alloc[i][j] = I_bit_alloc[2][i][j];
		       else
		          bit_alloc[i][j] = 0;
	      }
	   }
	   else if (info.lay == 2 )
	   {
	      layer_I_frames = 3;
	      bitsPerSlot = 8;
	      samplesPerFrame = 1152;
	      ch_start = 0;
	      channels = stereo;
	      II_decode_bitalloc (&bs_mpg, &fr_ps, bit_alloc, bits_log);
	      II_decode_scale (&bs_mpg, &fr_ps, scfsi, bit_alloc, scale_index, &ch_start, &channels, scfsi_log);

	      if (error_protection)
	      {
		 II_CRC_calc(&fr_ps, bit_alloc, scfsi, &new_crc);
		 if (new_crc != old_crc)
		 {
		    printf(" \n ERROR in LAYER 2 - CRC! \n");
		    crc_error_count++;
		    total_error_count++;
		    recover_CRC_error (*pcm_sample, crc_error_count,
				       &fr_ps, musicout, &sample_frames, channels);
		 }
		 else crc_error_count = 0;
	      }

	      clip = 0;

	      for (i=0;i<12;i++)
	      {
		 II_buffer_sample (&bs_mpg, &fr_ps, (*sample), bit_alloc);
if (!any_log)
{
		 II_dequantize_sample ((*sample), bit_alloc, *fraction, &fr_ps, &i);
		 II_denormalize_sample (*fraction, scale_index, &fr_ps, i>>2, &i);
}
	      }   /* end of for loop */

	      if (mpeg > 1)
	      {
		 while (bs_mpg.curpos < bs_mpg.totbits)
		    bs_mc.bits[bs_mc.totbits++] = bs_mpg.bits[bs_mpg.curpos++];

		 mc_header (&bs_mc, &fr_ps);
		 mc_hdr_to_frps (&fr_ps);

		 bs_mc.totbits -= fr_ps.header->n_ad_bytes * 8;

		 if (fr_ps.header->ext_bit_stream_present)
		 {
		    sync = seek_sync_ext (&bs_ext, &fr_ps);
		    frameBits += bs_ext.totbits;
		    if (!sync)
		    {
		       fprintf(stderr, "Extension frame cannot be located, end of input stream\n");
		       exit (1);
		    }

		    while (bs_ext.curpos < bs_ext.totbits)
		       bs_mc.bits[bs_mc.totbits++] = bs_ext.bits[bs_ext.curpos++];
		 }
	      }
	   }

	   /**********************************************************/
	   /*     	multichannel - decoding    	           */
	   /*             7.07.93 Susanne Ritscher                   */
	   /*            13.10.93 Ralf Schwalbe                      */
	   /*	       30.05.94 Ralf Schwalbe			   */
	   /**********************************************************/

	   if (rate_log)
	     printf ("frame: %5d bitrate: %8.3f kbps\n", frameNum, frameBits * S_freq / samplesPerFrame);
	   else if (mpeg >= 2)
	   {	
	     if ( (frameNum==1) && (fr_ps.header->no_of_multi_lingual_ch>0) )
	     {
	       /*frameNum==1 NOT frameNum==0*/
	       if (fr_ps.header->no_of_multi_lingual_ch>7)
	       {
		 printf("\n\n***Warning. There are %d multilingual channels.\n", fr_ps.header->no_of_multi_lingual_ch);
		 printf    ("            This decoder supports only upto 7 channels.\n\n");
	       }
	       printf("There are %d multilingual channels, coded at ",fr_ps.header->no_of_multi_lingual_ch);
	       if (fr_ps.header->multi_lingual_fs==0) printf("full"); else printf("half");
	       printf(" sampling rate.\n They are outputted in \"%s\".\n",decoded_file_name_ml);
	       if ((musicout_ml = fopen(decoded_file_name_ml, "w+b")) == NULL)
	       {
		 fprintf (stderr, "***Error. Could not create \"%s\".\nExit\n", decoded_file_name_ml);
		 exit (1);
	       }
	       if(need_aiff)
	       {
		 printf("Multilingual file written in AIFF format\n");
		 if (aiff_seek_to_sound_data (musicout_ml) == -1)
		 {
		    fprintf (stderr, "Could not seek to ML PCM sound data in \"%s\".\n",
			    decoded_file_name_ml);
		    exit (1);
		 }  
	       }
	     }

	     mc_channel = fr_ps.mc_channel;
	     crc_error_count_mc = 0;
	     buffer_CRC (&bs_mc, &old_crc);   /* read CRC - check from header */
	     mc_composite_status_info (&bs_mc, &fr_ps, tca_log, dynx_log);
	     
	     if (frameNum == 1) 
	   	if (info.lfe)
		    printf ("mc-channel=%d, MPEG2-ext-length=%d kbit/s lfe-channel present\n",
		    		mc_channel, (int) (info.ext_length * s_freq[info.sampling_frequency] / 144));
 		else
		    printf ("mc-channel=%d, MPEG2-ext-length=%d kbit/s\n",
		    		mc_channel, (int) (info.ext_length * s_freq[info.sampling_frequency] / 144));

	     /* Achtung: fr_ps.jsbound = 27; wird in Abhängikeit von fs gesetzt*/
	     ch_start = stereo;
	     channels = stereo + mc_channel;

	     /* decoding bitallocation's */
	   
	     II_decode_bitalloc_mc (&bs_mc, &fr_ps, bit_alloc, &ch_start, &channels, bits_log);
	     II_decode_scale (&bs_mc, &fr_ps, scfsi, bit_alloc, scale_index, &ch_start, &channels, scfsi_log);

	     /* mandatory CRC check is set */
	     if (crc_check)
	     {
		mc_error_check (&fr_ps, bit_alloc, scfsi, &new_crc, ch_start, channels);
		if (new_crc != old_crc)
		{
		   crc_error_count_mc++;
		   total_error_count_mc++;
		   for (i = 0; i < SBLIMIT; i ++)
		      for (ii = ch_start; ii < channels; ++ii)
			 bit_alloc[ii][i] = 0;
		   printf ("\nERROR in MC-CRC -> mc frame can't be decoded !\n");
		}
		else
		   crc_error_count_mc = 0;
	     }  /* endif crc_check */

	     clip = 0;
	     for (i = 0; i < 12; i++)
	     {
		II_buffer_sample_mc (&bs_mc, &fr_ps, (*sample), bit_alloc, ch_start, channels, i);
if (!any_log)
{
		II_dequantize_sample_mc ((*sample), bit_alloc, *fraction, &fr_ps, ch_start, channels, &i);
		II_denormalize_sample_mc (*fraction, scale_index, &fr_ps, i>>2, ch_start, channels, &i);
}
	     }

	     /* 10/31/95 Ralf Schwalbe LFE */
	     if( info.lfe ) 
	     { 
		if(lfe_init)
		{
		   f = strlen (encoded_file_name) - 4;
		   strcpy (lfe_file_name, encoded_file_name);
		   strcpy (&lfe_file_name[f], DFLT_OPEXT_LFE);
		   if ((lfe = fopen (lfe_file_name, "w+b")) == NULL)
		   {
		      fprintf (stderr, "Could not create \"%s\".\n",lfe_file_name );
		      exit (1);
		   }
		   if (need_aiff)
		   {
		      printf ("LFE file written in AIFF format\n");
		      if (aiff_seek_to_sound_data (lfe) == -1)
		      {
			 fprintf (stderr, "Could not seek to LFE PCM sound data in \"%s\".\n",
				 lfe_file_name);
			 exit (1);
		      }  
		   }	
		}
		lfe_init = 0; /* R.S. only one time */	
		II_lfe_calc(&fr_ps);  
	     }
/*********************************************************************/
/* JMZ 09/03/1995 Multilingual , WtK 7/8/95 */

	     if (info.no_of_multi_lingual_ch>0) 
	     {
		/* SWKim060695 add the condion with 32, 44.1kHz case */
		if (info.sampling_frequency == 1)
		   fr_ps.jsbound = 27;
		else
		   fr_ps.jsbound = 30;

		m = info.no_of_multi_lingual_ch;

		II_decode_bitalloc_ml (&bs_mc, &fr_ps, bit_alloc_ml, &m);
		II_decode_scale_ml (&bs_mc, &fr_ps, scfsi_ml, bit_alloc_ml, scale_index_ml, &m);


		   clip = 0;
		   for (i=0; i< ( (info.multi_lingual_fs==0) ? 12 : 6 ); i++) /* ngr : n_of_granules loop */
		   {
		      II_buffer_sample_ml (&bs_mc, &fr_ps, (*sample_ml), bit_alloc_ml, &m);
if (!any_log)
{
		      II_dequantize_sample_ml ((*sample_ml), bit_alloc_ml, *fraction_ml, &fr_ps, &m, &i);
		      II_denormalize_sample_ml (*fraction_ml, scale_index_ml, &fr_ps,
					      ( (info.multi_lingual_fs==0) ? (i>>2) : (i>>1) ), &m, &i);
}
		      for (k = 0; k < info.no_of_multi_lingual_ch; k++)
			 for (ii = 0; ii < 3; ii++)
			    for (j = 0; j < 32; j++)
			       sym_sample_ml[i][k][ii][j] = (*sample_ml)[k][ii][j];
		   }
	     }

/* JMZ 09/03/1995 Multilingual */
/*********************************************************************/

#ifdef Augmentation_7ch
	     if (mpeg == 3)
	       if (stereo != 2 || fr_ps.header->surround != 2 || fr_ps.header->center == 0)
	       {
		 /* no 3/2 mode */
		 printf ("WARNING: Stream does not contain 5/2 info\n");
		 printf ("         Continue decoding of MPEG2 MC part\n");
		 mpeg = 2;
	       }

	     if (mpeg == 3)
	     {
	       crc_error_count_aug = 0;
	       mc_aug_composite_status_info (&bs_mc, &fr_ps, tca_log, dynx_log);

	       buffer_CRC (&bs_mc, &old_crc);   /* read CRC - check from header */
	       
	       /* Achtung: fr_ps.jsbound = 27; wird in Abhängikeit von fs gesetzt*/
	       ch_start = 5;
	       channels = 7;

	       /* decoding bitallocation's */
	     
	       II_decode_bitalloc_aug (&bs_mc, &fr_ps, bit_alloc, &ch_start, &channels, bits_log);
	       II_decode_scale (&bs_mc, &fr_ps, scfsi, bit_alloc, scale_index, &ch_start, &channels, scfsi_log);

	       /* mandatory CRC check is set */
	       if (crc_check)
	       {
		     mc_aug_error_check(&fr_ps, bit_alloc, scfsi, &new_crc);
		     if (new_crc != old_crc)
		     {
			     crc_error_count_aug++;
			     total_error_count_aug++;
			     for( i = 0; i < SBLIMIT; i ++)
				     for(ii = ch_start; ii < channels; ++ii)
					     bit_alloc[ii][i] = 0;

			     printf("ERROR in MC-AUG-CRC -> mc aug frame can't be decoded !\n");
		     }
		     else
			     crc_error_count_aug = 0;
	       }  /* endif crc_check */

		       clip = 0;
		       for (i=0;i<12;i++)
		       {
			     II_buffer_sample_aug (&bs_mc, &fr_ps, (*sample), bit_alloc, i);
if (!any_log)
{
			     II_dequantize_sample_aug ((*sample), bit_alloc, *fraction, &fr_ps, &i);
			     II_denormalize_sample_aug (*fraction, scale_index, &fr_ps, i>>2, &i);
}
			}
	     }   /* endif mpeg == 3 */
#endif

	     if (info.mc_prediction_on)
		for (k=0; k<2; k++)
		   for (ii=0; ii<8; ii++) 
		   {
		      for (j=0; j<PREDDEL; j++)
			 (*pred_buf)[k][ii][j] = (*pred_buf)[k][ii][j+36];
		      for (j =0; j <3; j++)
			 for (jj=0;jj<12;jj++)
			    (*pred_buf)[k][ii][PREDDEL+j+3*jj] = (*fraction)[k][ii][j][jj];
		   }

	     /* prediction & dematricing MPEG2 part */
if (!any_log)
	     if (crc_error_count_mc == 0)
		dematricing_mc (*fraction, &fr_ps, *pred_buf);

#ifdef Augmentation_7ch
	     if (mpeg == 3)
	     {
if (!any_log)
		if (crc_error_count_aug == 0)
		{
		   dematricing_aug (*fraction, &fr_ps);
		   denormalizing_aug (*fraction, &fr_ps);
		}
	      }
	      else
#endif
	      if (crc_error_count_mc == 0)
		denormalizing_mc (*fraction, &fr_ps, channels);
	   }   /* endif mpeg >= 2 */

if (!any_log)
	   for (jj=0;jj<4*layer_I_frames;jj++)
	      for (k=0; k < channels; k++)
		  for (j =0; j <3; j++)
		     for (ii=0; ii< SBLIMIT; ii++)
			(*fraction_help)[jj][k][j][ii] = (*fraction)[k][ii][j][jj];
if (!any_log)
	   for (i=0;i<4*layer_I_frames;i++)
	   {			  
	      for (j=0;j<3;j++) 
	         for (k=0; k < channels; k++)
		    clip += SubBandSynthesis (&((*fraction_help)[i][k][j][0]), k,
					    &((*pcm_sample)[k][j][0]));
   
	      out_fifo (*pcm_sample, 3, &fr_ps, done, musicout,
				    &sample_frames, channels);
	   }

	/* 31/10/95 Ralf Schwalbe LFE output */
if (!any_log)
	   if (info.lfe) 
	   {
	      for (jj = 0; jj < 12; jj++)
	      {
		/* lfe_tmp[jj] = (short int) (info.lfe_spl_fraction[jj] * SCALE); */
		double foo = floor (info.lfe_spl_fraction[jj] * SCALE + 0.5);

		if (foo >= (long) SCALE)      {lfe_tmp[jj] = SCALE-1; clip++;}
		else if (foo < (long) -SCALE) {lfe_tmp[jj] =-SCALE;   clip++;}
		else                           lfe_tmp[jj] = foo;
	      }
	      fwrite (&lfe_tmp[0], 2, 12, lfe); 
	   }
	   if (clip > 0)
	      printf("\n%d samples clipped\n", clip);
			
/***************************************************************/
/* 10/03/1995 JMZ Multilingual , WtK 7/8/95 */
	   if (info.no_of_multi_lingual_ch>0) 
	   {
	      for(jj=0;jj<( (info.multi_lingual_fs==0) ? 12 : 6 );jj++)
		 for(k=0; k<info.no_of_multi_lingual_ch; k++)
		     for(j =0; j <3; j++)
			for(ii=0; ii< SBLIMIT; ii++)
	      (*fraction_help_ml)[jj][k][j][ii] = (*fraction_ml)[k][ii][j][jj];

	      clip = 0;
	      for (i=0;i<( (info.multi_lingual_fs==0) ? 12 : 6 );i++)
	      {			  
		for (j=0;j<3;j++) 
		  for (k=0; k<info.no_of_multi_lingual_ch; k++)
		      clip += SubBandSynthesis_ml(&((*fraction_help_ml)[i][k][j][0]), k,
					       &((*pcm_sample_ml)[k][j][0]));

		out_fifo_ml(*pcm_sample_ml, 3, &fr_ps, done, musicout_ml, &sample_frames_ml);
	      }
	      if(clip > 0) printf("\n%d MultiLingual samples clipped\n", clip);
	   }
/* 10/03/1995 JMZ Multilingual */
/**************************************************************/
		
if( print_out == 1)
{
	if(frameNum == 1) 
	{
		fp6 = fopen("SUB_ML","w+");
		fp5 = fopen("DYN_CROSS","w+");
		fp4 = fopen("INFODEC", "w+");
		fp2 = fopen("BALDEC", "w+");
		fp3 = fopen("SCFSIDEC", "w+");
		fp1 = fopen("SCFDEC", "w+");
	}
/*********************Ausgabe***************************************************/
fprintf(fp1, "******************** FRAME %d *****************************\n", frameNum-1);
fprintf(fp2, "\n******************** FRAME %d *****************************\n", frameNum-1);
fprintf(fp3, "\n******************** FRAME %d *****************************\n", frameNum-1);
fprintf(fp4, "******************** FRAME %d *****************************\n", frameNum-1);
fprintf(fp6, "******************** FRAME %d *****************************\n", frameNum-1);

fprintf(fp4, "version = %d, ", info.version);
fprintf(fp4, "lay = %d, ", info.lay);
fprintf(fp4, "error_protection = %d\n", info.error_protection);

fprintf(fp4, "bitrate_index = %d, ", info.bitrate_index);

fprintf(fp4, "sampling_frequency = %d,", info.sampling_frequency);
fprintf(fp4, "padding = %d, ", info.padding);
fprintf(fp4, "extension = %d\n", info.extension);

fprintf(fp4, "mode = %d, ", info.mode);
fprintf(fp4, "mode_ext = %d,", info.mode_ext);
fprintf(fp4, "copyright = %d, ", info.copyright);
fprintf(fp4, "original = %d, ", info.original);
fprintf(fp4, "emphasis = %d\n", info.emphasis);

fprintf(fp4, "center = %d, ", info.center);
fprintf(fp4, "surround = %d, ", info.surround);
fprintf(fp4, "dematrix_procedure = %d, ", info.dematrix_procedure);
fprintf(fp4, "lfe = %d\n", info.lfe);

fprintf(fp4, "no_of_multi_lingual_ch = %d, ", info.no_of_multi_lingual_ch);
fprintf(fp4, "multi_lingual_fs = %d, ", info.multi_lingual_fs);
fprintf(fp4, "multi_lingual_layer = %d\n", info.multi_lingual_layer);

fprintf(fp4, "dyn_cross_on = %d\n", info.dyn_cross_on);
fprintf(fp4, "mc_prediction_on = %d\n", info.mc_prediction_on);

#ifdef	DEBUG_PREDICTION
if(info.mc_prediction_on == 1)
   for(hu = 0; hu < 8; ++ hu)
   {
	fprintf(fp4, "mc_prediction[%d] = %d ", hu, info.mc_prediction[hu]);
	for(ho = 0; ho < 4; ho++)
	{
		fprintf(fp4, "mc_predsi[%d][%d] = %d  ", hu,ho, info.mc_predsi[hu][ho]);
		fprintf(fp4, "mc_delay[%d][%d] = %d  ", hu,ho, info.mc_delay_comp[hu][ho]);
		fprintf(fp4, " \n");
		for(hi = 0;hi <3; hi++)
		fprintf(fp4, "pred_coeff[%d][%d][%d] = %d ",hu,ho,hi,info.mc_pred_coeff[hu][ho][hi]);
		fprintf(fp4, " \n");
	}
   }
#endif

if(info.dyn_cross_on == 1)
{
	fprintf(fp5, "\n******************** FRAME %d *****************************\n", frameNum-1);
	fprintf(fp5,"dyn_cross_LR = %d\n",info.dyn_cross_LR);
	for(ho = 0; ho < 12; ho ++)
	{
		fprintf(fp5,"dyn_cross[%d] = %d   ",ho,info.dyn_cross_mode[ho]);
		if((ho+1)%4 == 0) fprintf(fp5,"\n");
	}
	if (info.surround == 3)
		for(ho = 0; ho < 12; ho ++)
		{
			fprintf(fp5,"dyn_second_stereo[%d] = %d   ",ho,info.dyn_second_stereo[ho]);
			if((ho+1)%4 == 0) fprintf(fp5,"\n");
		}
}

fprintf(fp4, "tc_sbgr_select = %d\n", info.tc_sbgr_select);
if(info.tc_sbgr_select == 1)
  fprintf(fp4, "tc_allocation = %d\n", info.tc_allocation);
else
	{
		for(ho = 0; ho < 12; ho ++)
	{
		if(info.mc_prediction[ho] == 0)
		fprintf(fp4, "tc_alloc[%d] = %d\n", ho, info.tc_alloc[ho]);
		else{
		for(hu = 0; hu < 4; hu++)
				fprintf(fp4, "predsi[%d][%d] = %d\n", ho, hu, info.mc_predsi[ho][hu]);
		}
	}
	fprintf(fp4, "\n");
	}

for(hu = 0; hu < SBLIMIT; ++hu)
{
	fprintf(fp2, "\n");
	fprintf(fp3, "\n");

	for(hi = 0; hi < channels; hi ++)
	{
	fprintf(fp1, "\n");
	if (layer_I_frames == 1)
	   fprintf(fp2, "BAL[%d][%02d] = %d   ", hi, hu, I_bit_alloc[0][hi][hu]);
	else
	   fprintf(fp2, "BAL[%d][%02d] = %d   ", hi, hu, bit_alloc[hi][hu]);
	fprintf(fp3, "SCFSI[%d][%02d] = %d   ", hi, hu, scfsi[hi][hu]);

	for( ho = 0; ho < layer_I_frames; ho ++)
	{
	   fprintf(fp1, "SCF[%d][%d][%02d] = %d   ", hi, ho, hu, scale_index[hi][ho][hu]);
	}
	}
	for(hi = 0; hi < info.no_of_multi_lingual_ch; hi ++)
	{
	   fprintf(fp1, "\n");
	   /* 960315 added FDB Multi-Lingual bitalloc */
	   fprintf(fp2, "BAL_ML[%d][%02d] = %d   ", hi, hu, bit_alloc_ml[hi][hu]);
	   /* 960315 added FDB Multi-Lingual scf select info */
	   fprintf(fp3, "SCFSI_ML[%d][%02d] = %d   ", hi, hu, scfsi_ml[hi][hu]);
	   /* 960315 added FDB Multi-Lingual scfs */
	   for( ho = 0; ho < 3; ho ++)
	      fprintf(fp1, "SCF_ML[%d][%d][%02d] = %d   ", hi, ho, hu, scale_index_ml[hi][ho][hu]);
        }
	fprintf(fp1, "\n");
}

/* 960318 added FDB Multi-Lingual subband samples symbols*/
for(k=0; k<info.no_of_multi_lingual_ch; k++)
   for(ii=0; ii< SBLIMIT; ii++)
   {
      fprintf(fp6, "SB_SYM_ML[%d][%02d]: ", k, ii);
      for(jj=0;jj<( (info.multi_lingual_fs==0) ? 12 : 6 );jj++)
         for(j =0; j <3; j++)
            fprintf(fp6, " %6d", sym_sample_ml[jj][k][j][ii]);
      fprintf(fp6, "\n");
   }


/* 960318 added FDB Multi-Lingual subband samples */
for(k=0; k<info.no_of_multi_lingual_ch; k++)
   for(ii=0; ii< SBLIMIT; ii++)
   {
      fprintf(fp6, "SB_ML[%d][%02d]: ", k, ii);
      for(jj=0;jj<( (info.multi_lingual_fs==0) ? 12 : 6 );jj++)
         for(j =0; j <3; j++)
            fprintf(fp6, " %12.5e", (*fraction_help_ml)[jj][k][j][ii]);
      fprintf(fp6, "\n");
   }
fflush (fp1);
fflush (fp2);
fflush (fp3);
fflush (fp4);
fflush (fp5);
fflush (fp6);
/*******************************************************************************/

} /* endif of print_out == 1 */

    /****************************************************************/
    /*                                                              */
    /*                   END OF FRAME LOOP                          */
    /*                                                              */
    /****************************************************************/

}/*end of while(!endof(bs)) - loop */

	if (need_aiff)
	{
#ifdef Augmentation_7ch
	   if (mpeg == 3)
	      pcm_aiff_data.numChannels       = 7;
	   else
#endif
	      pcm_aiff_data.numChannels       = stereo + mc_channel;
	   pcm_aiff_data.numSampleFrames   = sample_frames;
	   pcm_aiff_data.sampleSize        = 16;
	   pcm_aiff_data.sampleRate        = s_freq[info.sampling_frequency]*1000;
	   strcpy(pcm_aiff_data.sampleType, IFF_ID_SSND);
	   pcm_aiff_data.blkAlgn.offset    = 0;
	   pcm_aiff_data.blkAlgn.blockSize = 0;

	   if (aiff_write_headers(musicout, &pcm_aiff_data) == -1)
	   {
		  fprintf (stderr, "Could not write AIFF headers to \"%s\"\n",
				 decoded_file_name);
		  exit (1);
           }

           if (fr_ps.header->no_of_multi_lingual_ch>0)
	   {
	     pcm_aiff_data.numChannels       = fr_ps.header->no_of_multi_lingual_ch;
	     pcm_aiff_data.numSampleFrames   = sample_frames;
	     pcm_aiff_data.sampleSize        = 16;
	     pcm_aiff_data.sampleRate        = s_freq[info.sampling_frequency]*1000;
	     if (fr_ps.header->multi_lingual_fs==1) pcm_aiff_data.sampleRate *= 0.5;
	     strcpy(pcm_aiff_data.sampleType, IFF_ID_SSND);
	     pcm_aiff_data.blkAlgn.offset    = 0;
	     pcm_aiff_data.blkAlgn.blockSize = 0;
  
	     if (aiff_write_headers(musicout_ml, &pcm_aiff_data) == -1)
	     {
		    fprintf (stderr, "Could not write AIFF headers to \"%s\"\n",
				   decoded_file_name_ml);
		    exit (1);
	     }
	   }
	   if (lfe_init == 0)
	   {
	     pcm_aiff_data.numChannels       = 1;
	     pcm_aiff_data.numSampleFrames   = sample_frames / 96;
	     pcm_aiff_data.sampleSize        = 16;
	     pcm_aiff_data.sampleRate        = (s_freq[info.sampling_frequency]*1000) / 96;
	     strcpy(pcm_aiff_data.sampleType, IFF_ID_SSND);
	     pcm_aiff_data.blkAlgn.offset    = 0;
	     pcm_aiff_data.blkAlgn.blockSize = 0;

	     if (aiff_write_headers(lfe, &pcm_aiff_data) == -1)
	     {
		    fprintf (stderr, "Could not write AIFF headers to \"%s\"\n",
				   lfe_file_name);
		    exit (1);
	     }
	   }
	 }

	close_bit_stream_r(&bs_mpg);

	if (info.ext_bit_stream_present)
	{
	     close_bit_stream_r(&bs_ext);

	}
	fclose(musicout);
	if(info.lfe) fclose(lfe);


#ifdef  MACINTOSH
	if (need_aiff) set_mac_file_attr(decoded_file_name, VOL_REF_NUM,
									 CREATR_DEC_AIFF, FILTYP_DEC_AIFF);
	else           set_mac_file_attr(decoded_file_name, VOL_REF_NUM,
									 CREATR_DEC_BNRY, FILTYP_DEC_BNRY);
#endif

	printf("Decoding of \"%s\" with %lu frames is finished\n", encoded_file_name, frameNum);
	printf("The decoded PCM output file name is \"%s\"\n", decoded_file_name);
	if (need_aiff)
	   printf("\"%s\" has been written with AIFF header information\n",
			  decoded_file_name);

	if (fr_ps.header->no_of_multi_lingual_ch>0) {
          printf("There are %d multilingual channels, coded at ",fr_ps.header->no_of_multi_lingual_ch);
          if (fr_ps.header->multi_lingual_fs==0) printf("full"); else printf("half");
          printf(" sampling rate.\n They are outputted in \"%s\".\n",decoded_file_name_ml);
          if (need_aiff)
	   printf("Multilingual file has been written with AIFF header information\n");
	}

	if (total_error_count_mc != 0)
	    printf ("There were %d frames,  which were not in multichannel!!\n", total_error_count_mc);
#ifdef Augmentation_7ch
	if (total_error_count_aug != 0)
	    printf ("There were %d frames,  which were not in augmented multichannel!!\n", total_error_count_aug);
#endif
    if (print_out == 1)
    {
	fclose (fp1);
	fclose (fp2);
	fclose (fp3);
	fclose (fp4);
	fclose (fp5);
	fclose (fp6);
    }
    return (1);
}


void usage (void)  /* print syntax & exit */
{
   fprintf (stderr,
      "usage: %s                         queries for all arguments, or\n",
	   programName);
   fprintf (stderr,
      "       %s [-a] [-h] [-i] [-m] inputBS [outPCM]\n", programName);
   fprintf (stderr,"where\n");
   fprintf (stderr," -a       write a RAW data sound file  (default: write an AIFF sound file)\n");
   fprintf (stderr," -b       write alloc bits per subband to stdout (default: off)\n");
   fprintf (stderr," -d       write dynx modes to stdout (default: off)\n");
   fprintf (stderr," -h       bitstreams contain a 2304 byte header  (default: no header)\n");
   fprintf (stderr," -i       write decoded bitstream-information in a file\n"); 
			      /* default: write encoded information */
   fprintf (stderr," -m       encoded bitstream is a MPEG 1 Layer-2 file\n"); 
			      /* default: decode a MPEG 2 Layer-2 bitstream */
#ifdef Augmentation_7ch
   fprintf (stderr," -n       encoded bitstream is a MPEG 2 Layer-2 file with 7.1 augmentation\n"); 
			      /* default: decode a MPEG 2 Layer-2 bitstream */
#endif
   fprintf (stderr," -s       write scf bits per subband to stdout (default: off)\n");
   fprintf (stderr," -t       write tca modes to stdout (default: off)\n");
   fprintf (stderr," inputBS  input bit stream of encoded audio\n");
   fprintf (stderr," outPCM   output PCM sound file (dflt inName)\n");
   exit (1);
}
