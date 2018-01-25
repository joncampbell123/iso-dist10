/**********************************************************************
 * ISO MPEG Audio Subgroup Software Simulation Group (1996)
 * ISO 13818-3 MPEG-2 Audio Multichannel Decoder
 *
 * $Id: decode.c 1.12 1997/01/17 04:26:01 rowlands Exp $
 *
 * $Log: decode.c $
 * Revision 1.12  1997/01/17 04:26:01  rowlands
 * Fixed base bitstream joint stereo bugs.
 * Fixed PCM output file cleanup bugs.
 *
 * Revision 1.11  1996/04/18 05:37:23  rowlands
 * Release following Florence meeting
 *
 * Revision 1.10  1996/02/12 07:13:06  rowlands
 * Release following Munich meeting
 *
 * Revision 1.9.1.4  1996/01/29  03:36:47  rowlands
 * Fixed problem with dynamic crosstalk in 2/2 configuration
 * Fixed bug in dynamic crosstalk for 3/1 configuration.
 *
 * Revision 1.9.1.1  1996/01/20  17:28:45  rowlands
 * Received from Ralf Schwalbe (Telekom FTZ) - includes prediction
 *
 * Revision 1.7.1.3  1995/08/14  08:12:10  tenkate
 * ML-LSF added Warner ten Kate 7/8/95 (Philips)
 * change "alloc" and "sblimit" into "alloc_ml" and
 * "sblimit_ml" where appropriate.
 *
 * Revision 1.7.1.1  1995/07/14  06:12:46  rowlands
 * Updated dynamic crosstalk from FTZ: revision FTZ_03
 *
 * Revision 1.3.3.1  1995/06/16  08:00:46  rowlands
 * Input from Sang Wook Kim (Samsung AIT)
 *
 * Revision 1.3.2.1  1995/06/16  03:27:20  rowlands
 * Corrected prediction select syntax according to IS
 * Added dematrixing procedure 2. Corrected dematrix weighting values.
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
 *                                                                    * 
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
 *								      *	
 * 01/12/96	  Ralf Schwalbe, Deutsche Telekom TZ Darmstadt	      *
 *		    - decoder prediction installed		      *
 *		    - problem with compl. bitstream 19 solved,        *
 *                                                                    *         
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
 **********************************************************************/
 /*********************************************************************/

#include        "common.h"
#include        "decoder.h"

/***************************************************************
/*
/* This module contains the core of the decoder ie all the
/* computational routines. (Layer I and II only)
/* Functions are common to both layer unless
/* otherwise specified.
/*
/***************************************************************/

/*****************************************************************
/*
/* The following routines decode the system information
/*
/****************************************************************/

/************************* Layer II  ****************************/

void decode_info(Bit_stream *bs,
		 frame_params *fr_ps)
{
   layer *hdr = fr_ps->header;

   hdr->version = get1bit (bs);
   hdr->lay = 4 - getbits (bs, 2);
   hdr->error_protection = !get1bit (bs); /* error protect. TRUE/FALSE */
   hdr->bitrate_index = getbits (bs, 4);
   hdr->sampling_frequency = getbits (bs, 2);
   hdr->padding = get1bit (bs);
   hdr->extension = get1bit (bs);
   hdr->mode = getbits (bs, 2);
   hdr->mode_ext = getbits (bs, 2);
   hdr->copyright = get1bit (bs);
   hdr->original = get1bit (bs);
   hdr->emphasis = getbits (bs, 2);
}



/**********************************************************************/
/*																	  */
/*  7.7.93 Susanne Ritscher Systeminformation for multi-channel       */
/* 27.5.94 Ralf Schwalbe    Systeminformation and names due to		  */
/*					MPEG 2 DIS from March 1994  */					                        
/*								*/
/**********************************************************************/

void mc_header(Bit_stream *bs,
	       frame_params *fr_ps)
{
   layer *hdr = fr_ps->header;
   hdr->ext_bit_stream_present = get1bit (bs);
   if( hdr->ext_bit_stream_present == 1)
	hdr->n_ad_bytes =  getbits (bs, 8);
   hdr->center = getbits (bs, 2);
   hdr->surround = getbits (bs, 2);
   hdr->lfe = get1bit (bs);
   hdr->audio_mix = get1bit (bs);     /* large or small room  R.S. */
   hdr->dematrix_procedure = getbits (bs, 2);
   hdr->no_of_multi_lingual_ch = getbits (bs, 3);
   hdr->multi_lingual_fs = get1bit (bs);
   hdr->multi_lingual_layer = get1bit (bs);
   hdr->copyright_ident_bit = get1bit (bs);
   hdr->copyright_ident_start = get1bit (bs);
}

/* R.S. prediction table MPEG-2 IS November, 1995 */
/* FdB  prediction table MPEG-2 CD 13818-3.2 March, 1996 */
int pred_coef_table[6][16] = {{6,4,4,4,2,2,2,0,2,2,2,0,0,0,0,0},
			      {4,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0},
			      {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			      {4,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0},
			      {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			      {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}};


void mc_composite_status_info(Bit_stream *bs,
			      frame_params *fr_ps, int tca_log, int dynx_log)
{
   layer *hdr = fr_ps->header;
   int sbgr, j, pci;


   hdr->tc_sbgr_select = get1bit (bs);
   hdr->dyn_cross_on = get1bit (bs);
   hdr->mc_prediction_on = get1bit (bs);

   if(hdr->tc_sbgr_select == 1)
   {
	hdr->tc_allocation = getbits (bs, fr_ps->alloc_bits);
	/* tc_allocation is valid for all sbgr R.S. */
	for(sbgr = 0; sbgr < 12; sbgr++)   
		hdr->tc_alloc[sbgr] = hdr->tc_allocation;
   }
   else
   {
	hdr->tc_allocation = 0;
	for(sbgr = 0; sbgr < 12; sbgr++)
		hdr->tc_alloc[sbgr] = getbits (bs, fr_ps->alloc_bits);
   }

   if (tca_log)
      for (sbgr = 0; sbgr < 12; sbgr++)
	 printf ("tc_alloc[ %2d ] = %2d\n", sbgr, hdr->tc_alloc[sbgr]);

   if( hdr->dyn_cross_on == 1)
   {
		hdr->dyn_cross_LR = get1bit (bs);
		for(sbgr = 0; sbgr < 12; sbgr++)
		{
			hdr->dyn_cross_mode[sbgr] = getbits (bs, fr_ps->dyn_cross_bits);
			/* 960816 FdB dyn_second_stereo added */
			if (hdr->surround == 3)
				hdr->dyn_second_stereo[sbgr] = get1bit (bs);
		}
   }
   else
   {
		hdr->dyn_cross_LR = 0;
		for(sbgr = 0; sbgr < 12; sbgr++)
			hdr->dyn_cross_mode[sbgr] = 0;
   }

   if (dynx_log)
      for (sbgr = 0; sbgr < 12; sbgr++)
	 printf ("dynx_mod[ %2d ] = %2d\n", sbgr, hdr->dyn_cross_mode[sbgr]);

   if( hdr->mc_prediction_on == 1)
   {
	  for(sbgr = 0; sbgr < 8; sbgr++) 
	  {  
		if( (hdr->mc_prediction[sbgr] = get1bit (bs)) == 1 )
		{
/* R.S. read from npredcoef-table max number of coef. for 3/2 configuration */
/* and then the predsi info -> 0    : no prediction    */
/*			    -> 1..3 : 1..3 coefficient */
		    for(pci=0; pci< pred_coef_table[fr_ps->pred_mode][hdr->dyn_cross_mode[sbgr]] ; pci++)
			hdr->mc_predsi[sbgr][pci] = getbits (bs, 2);
		}
	  }
   }
}



#ifdef Augmentation_7ch
void mc_aug_composite_status_info (Bit_stream *bs, frame_params *fr_ps, int tca_log, int dynx_log)
{
   layer *hdr = fr_ps->header;
   int sbgr, j, pci;

   hdr->aug_mtx_proc = getbits (bs, 2);
   hdr->aug_dyn_cross_on = get1bit (bs);
   hdr->aug_future_ext = get1bit (bs);

   if(hdr->aug_mtx_proc == 0)
   {
	for(sbgr = 0; sbgr < 12; sbgr++)   
		hdr->tc_aug_alloc[sbgr] = getbits (bs, 3);
   }
   else if(hdr->aug_mtx_proc == 1)
   {
	for(sbgr = 0; sbgr < 12; sbgr++)
		hdr->tc_aug_alloc[sbgr] = getbits (bs, 2);
   }
   else
	for(sbgr = 0; sbgr < 12; sbgr++)
		hdr->tc_aug_alloc[sbgr] = 0;

   if (tca_log)
      for (sbgr = 0; sbgr < 12; sbgr++)
	 printf ("tc_aug_alloc[ %2d ] = %2d\n", sbgr, hdr->tc_aug_alloc[sbgr]);

   if( hdr->aug_dyn_cross_on == 1)
   {
	for(sbgr = 0; sbgr < 12; sbgr++)
		hdr->dyn_cross_aug_mode[sbgr] = getbits (bs, 5);
   }
   else
	for(sbgr = 0; sbgr < 12; sbgr++)
		hdr->dyn_cross_aug_mode[sbgr] = 0;

   if (dynx_log)
      for (sbgr = 0; sbgr < 12; sbgr++)
	 printf ("dynx_aug_mod[ %2d ] = %2d\n", sbgr, hdr->dyn_cross_aug_mode[sbgr]);
}
#endif

/*******************************************************************
/*
/* The bit allocation information is decoded. Layer I
/* has 4 bit per subband whereas Layer II is Ws and bit rate
/* dependent.
/*
/********************************************************************/

void I_decode_bitalloc (Bit_stream   *bs,
			frame_params *fr_ps,
			unsigned int bit_alloc[7][SBLIMIT],	
			int	     bits_log)
{
   int i,j;
   int stereo  = fr_ps->stereo;
   int sblimit = fr_ps->sblimit;
   int jsbound = fr_ps->jsbound;
   int b;

   for (i=0;i<jsbound;i++)
      for (j=0;j<stereo;j++)
	 bit_alloc[j][i] = getbits (bs, 4);

   for (i=jsbound; i<SBLIMIT; i++)
   {
      b = getbits (bs, 4);
      for (j=0; j<stereo; j++)
	 bit_alloc[j][i] = b;
   }

   if (bits_log)
   {
      for (j= 0; j < stereo; j++)
         for (i = 0; i < SBLIMIT; i++)
	 {
	    int alloc_bits = 0, alloc_id = bit_alloc[j][i];

	    if ((j == 0 || i < jsbound) && bit_alloc[j][i] > 0)
	       alloc_bits = 12 * (alloc_id + 1);

	    printf ("alloc_bi[ %d ][ %2d ]= %3d\n", j, i, alloc_bits);
	 }
   }
}


void II_decode_bitalloc (Bit_stream *bs,
			 frame_params *fr_ps,
			 unsigned int bit_alloc[7][SBLIMIT],	
			 int bits_log)
{
   int i,j,c,sbgr;
   int stereo = fr_ps->stereo;
   int sblimit = fr_ps->sblimit;
   int jsbound = fr_ps->jsbound;
   al_table *alloc = fr_ps->alloc;
   unsigned int actual_alloc[7][SBLIMIT];

   for (i = 0; i < SBLIMIT; i++)
      for (j = 0; j < stereo; j++)
	 actual_alloc[j][i] = 0;

   for (i=0; i<jsbound; i++)
      for (j=0;j<stereo;j++)
	 actual_alloc[j][i] = bit_alloc[j][i] = (char) getbits (bs, (*alloc)[i][0].bits);

   for (i=jsbound; i<sblimit; i++)   /* expand to 2 channels */
      actual_alloc[0][i] = bit_alloc[0][i] = bit_alloc[1][i] = (char) getbits (bs, (*alloc)[i][0].bits);

   for (i=sblimit; i<SBLIMIT; i++)
      for (j=0; j<stereo; j++)
	  bit_alloc[j][i] = 0;

   if (bits_log)
   {
      for (j= 0; j < stereo; j++)
         for (i = 0; i < SBLIMIT; i++)
	 {
	    int alloc_bits = 0, alloc_id = bit_alloc[j][i];

	    if (actual_alloc[j][i] > 0)
	       alloc_bits = 12 * (*alloc)[i][alloc_id].bits * (*alloc)[i][alloc_id].group;

	    printf ("alloc_bi[ %d ][ %2d ]= %3d\n", j, i, alloc_bits);
	 }
   }
}


void II_decode_bitalloc_mc (Bit_stream *bs,
			    frame_params *fr_ps,
			    unsigned int bit_alloc[7][SBLIMIT],	
			    int *l, int *m, int bits_log)
{
   layer *info = fr_ps->header;
   int i,j,c,sbgr;
   int sblimit = fr_ps->sblimit_mc;
   al_table *alloc = fr_ps->alloc_mc;
   unsigned int actual_alloc[7][SBLIMIT];

   for (i = 0; i < SBLIMIT; i++)
      for (j = *l; j < *m; j++)
	 actual_alloc[j][i] = 0;

   /* 10/31/95 Ralf Schwalbe LFE */
   if (info->lfe)
      info->lfe_alloc = (char) getbits (bs, (*alloc)[0][0].bits);

   for (i=0; i<sblimit; i++)
      if (info->dyn_cross_on == 0)
	 for (j=*l;j<*m;j++)
	 {
	    if ((fr_ps->header->center != 3) || (i < 12) || (j !=2))
	       actual_alloc[j][i] = bit_alloc[j][i] = (char) getbits (bs, (*alloc)[i][0].bits);
	    else
	       bit_alloc[j][i] = 0;
	 }
      else  /* dyn. cross mode */
      {
	 if (i == 0) sbgr = 0;
	 else
	    for (c = 1; c < 12; c++)
	    {
	       if ((sb_groups[c-1] < i) && (i <= sb_groups[c]))
	       {
		  sbgr = c;  /* search the valid subband group */
		  break;
	       }
	    }
 
	 /* 960816 FdB new setup for dyn. crosstalk modes */
	 if (info->dyn_cross_mode[sbgr] == 0)
	 {
	    for (j = *l; j < *m; j++)
	       if (fr_ps->header->center == 3 && i >= 12 && j == 2)
		  bit_alloc[j][i] = 0;
	       else if (info->surround == 3 && info->dyn_second_stereo[sbgr] == 1)
	       {
		  if (info->center != 0 && j == 4)
		     bit_alloc[j][i] = bit_alloc[3][i];
		  else if (info->center == 0 && j == 3)
		     bit_alloc[j][i] = bit_alloc[2][i];
		  else
		     actual_alloc[j][i] = bit_alloc[j][i] = (char) getbits (bs, (*alloc)[i][0].bits);
	       }
	       else
		  actual_alloc[j][i] = bit_alloc[j][i] = (char) getbits (bs, (*alloc)[i][0].bits);
	 }
	 else if (fr_ps->dyn_cross_bits == 1)   /* for channel mode 3/0 and 2/1 */
	 {
	    /* DynX mode has to be 1 */
	    if ((info->center == 3) && (i >= 12))    /* 3/0 + phantom center */
	       bit_alloc[2][i] = 0;
	    else if (info->tc_alloc[sbgr] == 1)
	       bit_alloc[2][i] = bit_alloc[0][i];
	    else if (info->tc_alloc[sbgr] == 2)
	       bit_alloc[2][i] = bit_alloc[1][i];
	    else
	       if (info->dyn_cross_LR)
		  bit_alloc[2][i] = bit_alloc[1][i];
	       else
		  bit_alloc[2][i] = bit_alloc[0][i];
 
	    if (info->surround == 3)		/* 3/0 + 2/0 */
	    {
	       actual_alloc[3][i] = bit_alloc[3][i] = (char) getbits (bs, (*alloc)[i][0].bits);
	       if (info->dyn_second_stereo[sbgr] == 1)
		  bit_alloc[4][i] = bit_alloc[3][i];
	       else
		  actual_alloc[4][i] = bit_alloc[4][i] = (char) getbits (bs, (*alloc)[i][0].bits);
	    }
	 }
	 else if (fr_ps->dyn_cross_bits == 3)   /* for channel mode 3/1 and 2/2 */
	 {
	    if ((info->center == 3) && (i >= 12))    /* 3/1 + phantom center */
	       bit_alloc[2][i] = 0;
	    else if ((info->dyn_cross_mode[sbgr] == 1) || (info->dyn_cross_mode[sbgr] == 4))
	       actual_alloc[2][i] = bit_alloc[2][i] = (char) getbits (bs, (*alloc)[i][0].bits);
	    else
	    {
	       /* T2 not transmitted */
	       if (fr_ps->header->surround == 2 ||	/* 2/2 mode */		
		   fr_ps->header->tc_alloc[sbgr] == 1 ||	/* 3/1 mode */
		   fr_ps->header->tc_alloc[sbgr] == 5 ||	/* 3/1 mode (matrix mode 2 only) */
		   fr_ps->header->tc_alloc[sbgr] != 2 && !fr_ps->header->dyn_cross_LR)
		  bit_alloc[2][i] = bit_alloc[0][i];	/* C, L or Ls from L0 */
	       else
		  bit_alloc[2][i] = bit_alloc[1][i];	/* C, R or Rs from RO */
	    }
 
	    if (info->dyn_cross_mode[sbgr] == 2)
	       actual_alloc[3][i] = bit_alloc[3][i] = (char) getbits (bs, (*alloc)[i][0].bits);
	    else if (info->dyn_cross_mode[sbgr] == 4)
	       bit_alloc[3][i] = bit_alloc[2][i];
	    else
	    {
	       /* T3 not transmitted */
	       if (fr_ps->header->surround == 2 ||	/* 2/2 mode */
		   fr_ps->header->tc_alloc[sbgr] == 4 ||	/* 3/1 mode */
		   fr_ps->header->tc_alloc[sbgr] == 5 ||	/* 3/1 mode (matrix mode 2 only) */
		   fr_ps->header->tc_alloc[sbgr] < 3 && fr_ps->header->dyn_cross_LR)
		  bit_alloc[3][i] = bit_alloc[1][i];	/* S, R or Rs from R0 */
	       else
		  bit_alloc[3][i] = bit_alloc[0][i];	/* S, L or Ls from LO */
	    }
	 } 
	 else if (fr_ps->dyn_cross_bits == 4)   /* for channel mode 3/2 */
	 {
	    /* T2 */
	    if ((info->center == 3) && (i >= 12))    /* 3/2 + phantom center */
	       bit_alloc[2][i] = 0;
	    else switch (info->dyn_cross_mode[sbgr])
	    {
	    case 1 :   /* T2 contains bitalloc info */
	    case 2 :
	    case 4 :
	    case 8 :
	    case 9 :
	    case 10: 
	    case 11: 
	    case 12: 
	    case 14: 
		       actual_alloc[2][i] = bit_alloc[2][i] = (char) getbits (bs, (*alloc)[i][0].bits);
		       break;
	    case 3 :   /* T2 contains no bitalloc info */
	    case 5 :
	    case 6 :
	    case 7 :
	    case 13: 
		       if ((info->tc_alloc[sbgr] == 1) || (info->tc_alloc[sbgr] == 7))
			  bit_alloc[2][i] = bit_alloc[0][i];
		       else if ((info->tc_alloc[sbgr] == 2) || (info->tc_alloc[sbgr] == 6))
			  bit_alloc[2][i] = bit_alloc[1][i];
		       else
			  if (info->dyn_cross_LR)
			     bit_alloc[2][i] = bit_alloc[1][i];
			  else
			     bit_alloc[2][i] = bit_alloc[0][i];
		       break;
	    }
 
	    /* T3 */
	    switch (info->dyn_cross_mode[sbgr])
	    {
	    case 1 :   /* T3 contains bitalloc info */
	    case 3 :
	    case 5 :
	    case 8 :
	    case 10: 
	    case 13: 
		       actual_alloc[3][i] = bit_alloc[3][i] = (char) getbits (bs, (*alloc)[i][0].bits);
		       break;
	    case 2 :   /* T3 has to copy its bitalloc from T0 */
	    case 4 :
	    case 6 :
	    case 7 :
	    case 12: 
		       bit_alloc[3][i] = bit_alloc[0][i];
		       break;
	    case 9 :   /* T3 has to copy its bitalloc from T2 */
	    case 11:
	    case 14: 
		       bit_alloc[3][i] = bit_alloc[2][i];
		       break;
	    }
 
	    /* T4 */
	    switch (info->dyn_cross_mode[sbgr])
	    {
	    case 2 :   /* T4 contains bitalloc info */
	    case 3 :
	    case 6 :
	    case 9 : 
		       actual_alloc[4][i] = bit_alloc[4][i] = (char) getbits (bs, (*alloc)[i][0].bits);
		       break;
	    case 1 :   /* T4 has to copy its bitalloc from T1 */
	    case 4 :
	    case 5 :
	    case 7 :
	    case 11: 
		       bit_alloc[4][i] = bit_alloc[1][i];
		       break;
	    case 10:   /* T4 has to copy its bitalloc from T2 */
	    case 12:
	    case 14: 
		       bit_alloc[4][i] = bit_alloc[2][i];
		       break;
	    case 8 :   /* T4 has to copy its bitalloc from T3 */
	    case 13: 
		       bit_alloc[4][i] = bit_alloc[3][i];
		       break;
	    }
	 }
      }

   for (i=sblimit; i<SBLIMIT; i++)
      for (j=*l; j<*m; j++)
	  bit_alloc[j][i] = 0;

   if (bits_log)
   {
      for (j= *l; j < *m; j++)
         for (i = 0; i < SBLIMIT; i++)
	 {
	    int alloc_bits = 0, alloc_id = bit_alloc[j][i];

	    if (actual_alloc[j][i] > 0)
	       alloc_bits = 12 * (*alloc)[i][alloc_id].bits * (*alloc)[i][alloc_id].group;

	    printf ("alloc_bi[ %d ][ %2d ]= %3d\n", j, i, alloc_bits);
	 }
   }
}

#ifdef Augmentation_7ch
void II_decode_bitalloc_aug (Bit_stream *bs,
			     frame_params *fr_ps,
			     unsigned int bit_alloc[7][SBLIMIT],	
			     int *l, int *m, int bits_log)
{
   layer *info = fr_ps->header;
   int i,j,c,sbgr;
   /* int stereo = fr_ps->stereo; */ /* not used for mc - decoding */
   int sblimit = fr_ps->sblimit_mc;
   al_table *alloc = fr_ps->alloc_mc;
   unsigned int actual_alloc[7][SBLIMIT];

   for (i = 0; i < SBLIMIT; i++)
      for (j = *l; j < *m; j++)
	 actual_alloc[j][i] = 0;

   for (i=0; i<sblimit; i++)
      if (info->aug_dyn_cross_on == 0)
	 for (j=*l;j<*m;j++)
	    actual_alloc[j][i] = bit_alloc[j][i] = (char) getbits (bs, (*alloc)[i][0].bits);
      else  /* dyn. cross mode */
      {
        int T5T0 = 0, T6T0 = 0, T6T1 = 1;
    
	 if(i == 0) sbgr = 0;
	 else
	    for(c = 1; c < 12; c++)
	    {
	       if((sb_groups[c-1] < i) && (i <= sb_groups[c]))
	       {
		 sbgr = c;  /* search the valid subband group */
		 break;
		}
	     }
 
        if (info->tc_aug_alloc[sbgr] == 4 || info->tc_aug_alloc[sbgr] == 5)
    	  if (info->dyn_cross_LR == 0)
	    T6T1 = 0;
	  else
	    T6T0 = 1;
        else if (info->tc_aug_alloc[sbgr] == 6 || info->tc_aug_alloc[sbgr] == 7)
          if (info->dyn_cross_LR)
	    T5T0 = 1;

	 /* read bitalloc info from bitstream */
	 switch (info->dyn_cross_aug_mode[sbgr])
	 {
	 case  0:   /* T5 and T6 contains bitalloc info */
		    actual_alloc[5][i] = bit_alloc[5][i] = (char) getbits (bs, (*alloc)[i][0].bits);
		    actual_alloc[6][i] = bit_alloc[6][i] = (char) getbits (bs, (*alloc)[i][0].bits);
		    break;
	 case  1:   /* T5 contains bitalloc info */
	 case  2:
	 case  3:
	 case  4:
		    actual_alloc[5][i] = bit_alloc[5][i] = (char) getbits (bs, (*alloc)[i][0].bits);
		    break;
 
	 case  5:   /* T6 contains bitalloc info */
	 case 10:
	 case 14:
		    actual_alloc[6][i] = bit_alloc[6][i] = (char) getbits (bs, (*alloc)[i][0].bits);
		    break;
	 }
 
	 /* copy bitalloc info from other channels */
	 switch (info->dyn_cross_aug_mode[sbgr])
	 {
	 case  1:   /* copy T6 from T1 */
		    bit_alloc[6][i] = bit_alloc[T6T1][i];
		    break;
	 case  2:   /* copy T6 from T2 */
		    bit_alloc[6][i] = bit_alloc[2][i];
		    break;
	 case  3:   /* copy T6 from T4 */
		    bit_alloc[6][i] = bit_alloc[4][i];
		    break;
	 case  4:   /* copy T6 from T5 */
		    bit_alloc[6][i] = bit_alloc[5][i];
		    break;
	 case  5:   /* copy T5 from T0 */
		    bit_alloc[5][i] = bit_alloc[T5T0][i];
		    break;
	 case  6:   /* copy T5 from T0 and T6 from T1 */
		    bit_alloc[5][i] = bit_alloc[T5T0][i];
		    bit_alloc[6][i] = bit_alloc[T6T1][i];
		    break;
	 case  7:   /* copy T5 from T0 and T6 from T2 */
		    bit_alloc[5][i] = bit_alloc[T5T0][i];
		    bit_alloc[6][i] = bit_alloc[2][i];
		    break;
	 case  8:   /* copy T5 from T0 and T6 from T4 */
		    bit_alloc[5][i] = bit_alloc[T5T0][i];
		    bit_alloc[6][i] = bit_alloc[4][i];
		    break;
	 case  9:   /* copy T5 and T6 from T0 */
		    bit_alloc[5][i] = bit_alloc[T5T0][i];
		    bit_alloc[6][i] = bit_alloc[T6T0][i];
		    break;
	 case 10:   /* copy T5 from T2 */
		    bit_alloc[5][i] = bit_alloc[2][i];
		    break;
	 case 11:   /* copy T5 from T2 and T6 from T1 */
		    bit_alloc[5][i] = bit_alloc[2][i];
		    bit_alloc[6][i] = bit_alloc[T6T1][i];
		    break;
	 case 12:   /* copy T5 and T6 from T2 */
		    bit_alloc[5][i] = bit_alloc[6][i] = bit_alloc[2][i];
		    break;
	 case 13:   /* copy T5 from T2 and T6 from T4 */
		    bit_alloc[5][i] = bit_alloc[2][i];
		    bit_alloc[6][i] = bit_alloc[4][i];
		    break;
	 case 14:   /* copy T5 from T3 */
		    bit_alloc[5][i] = bit_alloc[3][i];
		    break;
	 case 15:   /* copy T5 from T3 and T6 from T1 */
		    bit_alloc[5][i] = bit_alloc[3][i];
		    bit_alloc[6][i] = bit_alloc[T6T1][i];
		    break;
	 case 16:   /* copy T5 from T3 and T6 from T2 */
		    bit_alloc[5][i] = bit_alloc[3][i];
		    bit_alloc[6][i] = bit_alloc[2][i];
		    break;
	 case 17:   /* copy T5 from T3 and T6 from T4 */
		    bit_alloc[5][i] = bit_alloc[3][i];
		    bit_alloc[6][i] = bit_alloc[4][i];
		    break;
	 case 18:   /* copy T5 and T6 from T3 */
		    bit_alloc[5][i] = bit_alloc[6][i] = bit_alloc[3][i];
		    break;
	 }
      }

   for (i=sblimit; i<SBLIMIT; i++)
      for (j=*l; j<*m; j++)
	 bit_alloc[j][i] = 0;

   if (bits_log)
   {
      for (j= *l; j < *m; j++)
         for (i = 0; i < SBLIMIT; i++)
	 {
	    int alloc_bits = 0, alloc_id = bit_alloc[j][i];

	    if (actual_alloc[j][i] > 0)
	       alloc_bits = 12 * (*alloc)[i][alloc_id].bits * (*alloc)[i][alloc_id].group;

	    printf ("alloc_bi[ %d ][ %2d ]= %3d\n", j, i, alloc_bits);
	 }
   }
}
#endif

void II_decode_bitalloc_ml(Bit_stream *bs,
			   frame_params *fr_ps,
			   unsigned int bit_alloc_ml[7][SBLIMIT],	
			   int *m)
{
   int i,j;
   int sblimit_ml = fr_ps->sblimit_ml;
   al_table *alloc_ml = fr_ps->alloc_ml;

   /* JR: no dynamic crosstalk for multilingual channels */
   /* JR: no phantom center coding for multilingual channels */
   /* JR: no joint coding for multilingual channels */
   /* JR: they're really simple, aren't they? */

   for (i=0;i<sblimit_ml;i++)
    for (j=0;j<*m;j++) 
     bit_alloc_ml[j][i] = (char) getbits (bs, (*alloc_ml)[i][0].bits);
   for (i=sblimit_ml;i<SBLIMIT;i++) 
    for (j=0;j<*m;j++) 
     bit_alloc_ml[j][i] = 0;
}

/*****************************************************************
/*
/* The following two functions implement the layer II
/* format of scale factor extraction. Layer I involves reading
/* 6 bit per subband as scale factor. Layer II requires reading
/* first the scfsi which in turn indicate the number of scale factors
/* transmitted.
/*   Layer II : II_decode_scale
/*
/*************************** Layer II stuff ***************************/
/* 28.9.93 R.S. reading of the prediction coefficients */
/* 03.6.94 R.S. read pred-coef. due to MPEG 2 - DIS    */
/**********************************************************************/
/* JMZ 09/03/1995 Multilingual adaptations , WtK 7/8/95               */
/**********************************************************************/

void I_decode_scale (Bit_stream   *bs,
		     frame_params *fr_ps,
		     unsigned int bit_alloc[7][SBLIMIT],	
		     unsigned int scale_index[7][3][SBLIMIT],	
		     int	  part,
		     int	  scf_log)
{
   int i,j;
   int stereo = fr_ps->stereo;
   int sblimit = fr_ps->sblimit;

   for (i=0; i<SBLIMIT; i++)
      for (j=0;j<stereo;j++)
	 if (!bit_alloc[j][i])
	    scale_index[j][part][i] = SCALE_RANGE-1;
	 else                    /* 6 bit per scale factor */
	    scale_index[j][part][i] = getbits (bs, 6);
}

void II_decode_scale (Bit_stream *bs,
		      frame_params *fr_ps,
		      unsigned int scfsi[7][SBLIMIT], 		
		      unsigned int bit_alloc[7][SBLIMIT],	
		      unsigned int scale_index[7][3][SBLIMIT],	
		      int *l, int *m, int scfsi_log)
{
    layer *info = fr_ps->header;
    int stereo = fr_ps->stereo;
    int i,j;
    int px,pci;
    int sblimit = fr_ps->sblimit;

    if (*m == stereo)
	sblimit = fr_ps->sblimit;
    else
    	sblimit = fr_ps->sblimit_mc;

    for (i = 0; i < sblimit; i++) 
    {
	for (j = *l; j < *m; j++)   /* 2 bit scfsi */
	    if (bit_alloc[j][i]) 
	 	scfsi[j][i] = (char) getbits (bs, 2);
	    else
		scfsi[j][i] = 4;
    }

    for (i = sblimit; i < SBLIMIT; i++) 
	for (j = *l; j < *m; j++)
	    scfsi[j][i] = 4;

    if (scfsi_log)
    {
	for (j= *l; j < *m; j++)
	    for (i = 0; i < SBLIMIT; i++)
	    {
		int scf_bits;
    
		switch (scfsi[j][i])
		{
		    /* all three scale factors transmitted */
		    case 0 : scf_bits = 20;
			     break;
		    /* two scale factors transmitted */
		    case 1 :
		    case 3 : scf_bits = 14;
			     break;
		    /* only one scale factor transmitted */
		    case 2 : scf_bits = 8;
			     break;
		    /* no scale factors transmitted */
		    default : scf_bits = 0;
		}
		printf ("scf_bits[ %d ][ %2d ]= %3d\n", j, i, scf_bits);
	    }
    }

    /* 3.6.94 R.S. read the prediction coefficients in the mc - part */
    if (*m > stereo && *m < 7)
	if (fr_ps->header->mc_prediction_on == 1)
	    for (i = 0; i < 8; i++)
		if (fr_ps->header->mc_prediction[i] == 1)
		    for (px = 0; px < pred_coef_table[fr_ps->pred_mode][fr_ps->header->dyn_cross_mode[i]]; px++)
			if (fr_ps->header->mc_predsi[i][px] != 0)
			{
			    /* predictors are transfered */
			    fr_ps->header->mc_delay_comp[i][px] = getbits (bs, 3);
			    for (pci = 0; pci < fr_ps->header->mc_predsi[i][px]; pci++)
				fr_ps->header->mc_pred_coeff[i][px][pci] = getbits (bs, 8);
			}
			else
			{
			    /* no prediction coef. */
			    fr_ps->header->mc_pred_coeff[i][px][0] = 127;     /* Index 127 -> 0.0 */
			    fr_ps->header->mc_delay_comp[i][px] = 0;
			}

    /* 31/10/95 Ralf Schwalbe LFE */
    if (*l == stereo)
	if (info->lfe)
	    if (info->lfe_alloc)
		info->lfe_scf = getbits (bs, 6);

    for (i = 0; i < sblimit; i++) 
	for (j = *l; j < *m; j++)
	    if (bit_alloc[j][i])
		switch (scfsi[j][i])
		{
		    /* all three scale factors transmitted */
		    case 0 : scale_index[j][0][i] = getbits (bs, 6);
			     scale_index[j][1][i] = getbits (bs, 6);
			     scale_index[j][2][i] = getbits (bs, 6);
			     break;
		    /* scale factor 1 & 3 transmitted */
		    case 1 : scale_index[j][0][i] =
			     scale_index[j][1][i] = getbits (bs, 6);
			     scale_index[j][2][i] = getbits (bs, 6);
			     break;
		    /* scale factor 1 & 2 transmitted */
		    case 3 : scale_index[j][0][i] = getbits (bs, 6);
			     scale_index[j][1][i] =
			     scale_index[j][2][i] = getbits (bs, 6);
			     break;
		    /* only one scale factor transmitted */
		    case 2 : scale_index[j][0][i] =
			     scale_index[j][1][i] =
			     scale_index[j][2][i] = getbits (bs, 6);
			     break;
		    default : scale_index[j][0][i] =
			      scale_index[j][1][i] =
			      scale_index[j][2][i] = SCALE_RANGE-1;
			      break;
		}
	    else
		scale_index[j][0][i] = scale_index[j][1][i] = scale_index[j][2][i] = SCALE_RANGE-1;

    for (i = sblimit; i < SBLIMIT; i++)
	for (j = *l; j < *m; j++)
	    scale_index[j][0][i] = scale_index[j][1][i] = scale_index[j][2][i] = SCALE_RANGE-1;
}

/**********************************************************************/
/* JMZ 15/03/1995 Multilingual                                        */
/**********************************************************************/

void II_decode_scale_ml(Bit_stream *bs,
		        frame_params *fr_ps,
		        unsigned int scfsi[7][SBLIMIT], 		
		        unsigned int bit_alloc[7][SBLIMIT],	
		        unsigned int scale_index[7][3][SBLIMIT],	
		        int *m)
{
   int i,j;
   int px,pci;
   int sblimit_ml = fr_ps->sblimit_ml;


   for (i=0;i<sblimit_ml;i++) for (j=0;j<*m;j++)   /* 2 bit scfsi */
	if (bit_alloc[j][i]) 
		{
	  	scfsi[j][i] = (char) getbits (bs, 2);
	  	}
	else scfsi[j][i] = 4;
   for (i=sblimit_ml;i<SBLIMIT;i++) for (j=0;j<*m;j++)
	scfsi[j][i] = 4;

   for (i = 0; i < sblimit_ml; i++) for (j = 0; j < *m; j++) {
	  if (bit_alloc[j][i])
	    switch (scfsi[j][i]) {
		 /* all three scale factors transmitted */
		 case 0 : scale_index[j][0][i] = getbits (bs, 6);
			  scale_index[j][1][i] = getbits (bs, 6);
			  scale_index[j][2][i] = getbits (bs, 6);
			  break;
		 /* scale factor 1 & 3 transmitted */
		 case 1 : scale_index[j][0][i] =
			  scale_index[j][1][i] = getbits (bs, 6);
			  scale_index[j][2][i] = getbits (bs, 6);
			  break;
	         /* scale factor 1 & 2 transmitted */
		 case 3 : scale_index[j][0][i] = getbits (bs, 6);
			  scale_index[j][1][i] =
			  scale_index[j][2][i] = getbits (bs, 6);
         		  break;
		 /* only one scale factor transmitted */
	         case 2 : scale_index[j][0][i] =
		          scale_index[j][1][i] =
		          scale_index[j][2][i] = getbits (bs, 6);
         		  break;
		 default : scale_index[j][0][i] =
		           scale_index[j][1][i] =
		           scale_index[j][2][i] = SCALE_RANGE-1;
         		   break;
          }
          else {
                          scale_index[j][0][i] = scale_index[j][1][i] =
	                  scale_index[j][2][i] = SCALE_RANGE-1;
	  }
   }

   for (i=sblimit_ml;i<SBLIMIT;i++) for (j=0;j<*m;j++) {
		          scale_index[j][0][i] = scale_index[j][1][i] =
		          scale_index[j][2][i] = SCALE_RANGE-1;
   }
}

/**************************************************************
/*
/* The following two routines take care of reading the
/* compressed sample from the bit stream for layer 2.
/* For layer 1, read the number of bits as indicated
/* by the bit_alloc information. For layer 2, if grouping is
/* indicated for a particular subband, then the sample size has
/* to be read from the bits_group and the merged samples has
/* to be decompose into the three distinct samples. Otherwise,
/* it is the same for as layer one.
/*
/*************************** Layer II stuff ************************/

void I_buffer_sample (Bit_stream *bs,
		      frame_params *fr_ps,
		      unsigned int sample[7][3][SBLIMIT],
		      unsigned int bit_alloc[7][SBLIMIT])
{
   int i,j,k;
   int stereo = fr_ps->stereo;
   int sblimit = fr_ps->sblimit;
   int jsbound = fr_ps->jsbound;
   unsigned int s;

   for (i=0; i<jsbound; i++)
      for (j=0; j<stereo; j++)
	 if ((k = bit_alloc[j][i]) == 0)
	    sample[j][0][i] = 0;
	 else 
	    sample[j][0][i] = (unsigned int) getbits (bs, k+1);

   for (i=jsbound; i<SBLIMIT; i++)
   {
      if ((k = bit_alloc[0][i]) == 0)
	 s = 0;
      else 
	 s = (unsigned int) getbits (bs, k+1);

      for (j=0; j<stereo; j++)
	 sample[j][0][i] = s;
   }
}

void II_buffer_sample (Bit_stream *bs,
		       frame_params *fr_ps,
		       unsigned int sample[7][3][SBLIMIT],
		       unsigned int bit_alloc[7][SBLIMIT])
{
   int i,j,k,m;
   int stereo = fr_ps->stereo;
   int sblimit = fr_ps->sblimit;
   int jsbound = fr_ps->jsbound;
   al_table *alloc = fr_ps->alloc;

   for (i=0;i<sblimit;i++)
      for (j= 0;j<((i<jsbound)? stereo:1);j++)
      {
	   if (bit_alloc[j][i])
	   {
		 /* check for grouping in subband */
		 if ((*alloc)[i][bit_alloc[j][i]].group==3)
	 	 {
			for (m=0;m<3;m++)
			{
			  k = (*alloc)[i][bit_alloc[j][i]].bits;
			  sample[j][m][i] = (unsigned int) getbits (bs, k);
			 }
		 }
		 else
		 {    /* bit_alloc = 3, 5, 9 */
		   unsigned int nlevels, c=0;

		   nlevels = (*alloc)[i][bit_alloc[j][i]].steps;
		   k=(*alloc)[i][bit_alloc[j][i]].bits;
		   c = (unsigned int) getbits (bs, k);

		   for (k=0;k<3;k++)
		   {
			 sample[j][k][i] = c % nlevels;
			 c /= nlevels;
		   }
		 }
	   }
	   else
	   {      /* for no sample transmitted */
		 for (k=0;k<3;k++) sample[j][k][i] = 0;
	   }
	   if(stereo == 2 && i>= jsbound)  /* joint stereo : copy L to R */
		  for (k=0;k<3;k++) sample[1][k][i] = sample[0][k][i];
      }
   for (i = sblimit; i < SBLIMIT; i++)
	 for (j = 0; j < stereo; j++)
		for (k = 0; k < 3; k++)
		   sample[j][k][i] = 0;
}

/******************** mc - layer2 stuff ******************************/
/* 19.10.93 R.S. */

void II_buffer_sample_mc (Bit_stream *bs,
			  frame_params *fr_ps,
			  unsigned int sample[7][3][SBLIMIT],
			  unsigned int bit_alloc[7][SBLIMIT],
			  int ch_start, int channels, int gr)
{
   layer *info = fr_ps->header;
   int i,j,k,m,sbgr,l;
   unsigned int nlevels, c=0;
   int sblimit = fr_ps->sblimit_mc;
   int transmitted;
   al_table *alloc = fr_ps->alloc_mc;

   /* 31/10/95 Ralf Schwalbe LFE */
   /* 961003 FdB LFE number of bits corrected */
   if(info->lfe && info->lfe_alloc > 0)
	info->lfe_spl[gr] = (unsigned int) getbits (bs, info->lfe_alloc+1);

   for (i = 0; i < sblimit; i++)
   {
	  if( i == 0) sbgr = 0 ;
	  else
	     for(l = 1; l < 12; l++)
		if((sb_groups[l-1] < i) && (i <= sb_groups[l]))
		{
		   sbgr = l; break;
		}

	 for (j = ch_start; j < channels; j++)
	 {
	   if (bit_alloc[j][i])
	   {
	     transmitted = 1;
	     if (fr_ps->header->dyn_cross_on == 1)
	     {
		if (fr_ps->dyn_cross_bits == 4 && (
			(fr_ps->header->dyn_cross_mode[sbgr]==1  && j==4) ||
			(fr_ps->header->dyn_cross_mode[sbgr]==2  && j==3) ||
			(fr_ps->header->dyn_cross_mode[sbgr]==3  && j==2) ||
			(fr_ps->header->dyn_cross_mode[sbgr]==4  && j!=2) ||
			(fr_ps->header->dyn_cross_mode[sbgr]==5  && j!=3) ||
			(fr_ps->header->dyn_cross_mode[sbgr]==6  && j!=4) ||
			(fr_ps->header->dyn_cross_mode[sbgr]==7) ||
			(fr_ps->header->dyn_cross_mode[sbgr]==8  && j==4) ||
			(fr_ps->header->dyn_cross_mode[sbgr]==9  && j==3) ||
			(fr_ps->header->dyn_cross_mode[sbgr]==10 && j==4) ||
			(fr_ps->header->dyn_cross_mode[sbgr]==11 && j!=2) ||
			(fr_ps->header->dyn_cross_mode[sbgr]==12 && j!=2) ||
			(fr_ps->header->dyn_cross_mode[sbgr]==13 && j!=3) ||
			(fr_ps->header->dyn_cross_mode[sbgr]==14 && j!=2)
					          ))
		   transmitted = 0;		/* 3/2 */
		if (fr_ps->dyn_cross_bits == 3 && (
			(fr_ps->header->dyn_cross_mode[sbgr]==1  && j==3) ||
			(fr_ps->header->dyn_cross_mode[sbgr]==2  && j==2) ||
			(fr_ps->header->dyn_cross_mode[sbgr]==3) ||
			(fr_ps->header->dyn_cross_mode[sbgr]==4  && j==3)
						  ))
		   transmitted = 0;		/* 3/1 and 2/2 */
		if (fr_ps->dyn_cross_bits == 1 && fr_ps->header->dyn_cross_mode[sbgr]==1 && j==2)
		   transmitted = 0;		/* 3/0 (+2/0) and 2/1 */
		if (fr_ps->header->surround == 3 && fr_ps->header->dyn_second_stereo[sbgr] == 1)
		{
		   if ((fr_ps->header->center == 1 || fr_ps->header->center == 3) && j == 4)
	              transmitted = 0;
	           else if (fr_ps->header->center == 0 && j == 3)
	              transmitted = 0;
		}
	     }
	   }
	   else
	      transmitted = 0;

	   if (transmitted == 1)
	   {
		/* check for grouping in subband */
		if ((*alloc)[i][bit_alloc[j][i]].group==3)
		{
			for (m=0;m<3;m++)
			{
				k = (*alloc)[i][bit_alloc[j][i]].bits;
				sample[j][m][i] = (unsigned int) getbits (bs, k);
			}
		}
		else
		{	/* bit_alloc = 3, 5, 9 */
			nlevels = (*alloc)[i][bit_alloc[j][i]].steps;
			k=(*alloc)[i][bit_alloc[j][i]].bits;
			c = (unsigned int) getbits (bs, k);
			for (k=0;k<3;k++)
			{
				sample[j][k][i] = c % nlevels;
				c /= nlevels;
			}
		}
	   }
	   else   /* no samples transmitted */
	      for (k=0; k<3; k++)
		 sample[j][k][i] = 0;
	 }  /* for channel loop */
   }  /* for sblimit loop */

   for (i = sblimit; i < SBLIMIT; i++)
	 for (j = ch_start; j < channels; j++)
		for (k = 0; k < 3; k++)  sample[j][k][i] = 0;
}

#ifdef Augmentation_7ch
void II_buffer_sample_aug(Bit_stream *bs,
			frame_params *fr_ps,
			unsigned int sample[7][3][SBLIMIT],
			unsigned int bit_alloc[7][SBLIMIT],
			int gr)
{
   int i,j,k,m,sbgr,l;
   unsigned int nlevels, c=0;
   int sblimit = fr_ps->sblimit_mc;
   int transmitted;
   al_table *alloc = fr_ps->alloc_mc;

   for (i = 0; i < sblimit; i++)
   {
      if( i == 0) sbgr = 0;
      else
	 for(l = 1; l < 12; l++)
	    if((sb_groups[l-1] < i) && (i <= sb_groups[l]))
	    {
	       sbgr = l; break;
	    }

      for (j = 5; j < 7; j++)
      {
	 if (bit_alloc[j][i])
	 {
	    transmitted = 1;
	    if (fr_ps->header->aug_dyn_cross_on == 1)
	    {
	       if (j == 5)
	       {
		  if (fr_ps->header->dyn_cross_aug_mode[sbgr] > 4)
		     transmitted = 0;
	       }
	       else
	       {
		  transmitted = 0;
		  switch (fr_ps->header->dyn_cross_aug_mode[sbgr])
		  {
		  case  0:
		  case  5:
		  case 10:
		  case 14:
			   transmitted = 1;
			   break;
		  }
	       }
	    }
	 }
	 else
	    transmitted = 0;

	 if (transmitted == 1)
	 {
	      /* check for grouping in subband */
	      if ((*alloc)[i][bit_alloc[j][i]].group==3)
	      {
		      for (m=0;m<3;m++)
		      {
			      k = (*alloc)[i][bit_alloc[j][i]].bits;
			      sample[j][m][i] = (unsigned int) getbits (bs, k);
		      }
	      }
	      else
	      {	/* bit_alloc = 3, 5, 9 */
		      nlevels = (*alloc)[i][bit_alloc[j][i]].steps;
		      k=(*alloc)[i][bit_alloc[j][i]].bits;
		      c = (unsigned int) getbits (bs, k);
		      for (k=0;k<3;k++)
		      {
			      sample[j][k][i] = c % nlevels;
			      c /= nlevels;
		      }
	      }
	 }
	 else   /* no samples transmitted */
	    for (k=0; k<3; k++)
	       sample[j][k][i] = 0;
      }  /* for channel loop */
   }  /* for sblimit loop */

   for (i = sblimit; i < SBLIMIT; i++)
	 for (j = 5; j < 7; j++)
		for (k = 0; k < 3; k++)  sample[j][k][i] = 0;
}
#endif

/***************************************************************/
/* 09/03/1995 JMZ Multilingual */

/***************************************************************/
/* 15/03/1995 JMZ */

void II_buffer_sample_ml(Bit_stream *bs,
			 frame_params *fr_ps,
			 unsigned int sample_ml[7][3][SBLIMIT],
			 unsigned int bit_alloc_ml[7][SBLIMIT],
			 int *n_ml_ch)
{
   int i,j,k,m,sbgr,l;
   unsigned int nlevels, c=0;
   int sblimit_ml = fr_ps->sblimit_ml;
   al_table *alloc_ml = fr_ps->alloc_ml;

   for (i = 0; i < sblimit_ml; i++)
   {
	 for (j = 0; j < *n_ml_ch; j++)
	 {
	   if (bit_alloc_ml[j][i])
	   {
		/* JR: no dynamic crosstalk for multilingual channels */
		/* check for grouping in subband */
		if ((*alloc_ml)[i][bit_alloc_ml[j][i]].group==3)
		{

			for (m=0;m<3;m++)
			{
				k = (*alloc_ml)[i][bit_alloc_ml[j][i]].bits;
				sample_ml[j][m][i] = (unsigned int) getbits (bs, k);
			}

		}
		else
		{    /* bit_alloc = 3, 5, 9 */
		  nlevels = (*alloc_ml)[i][bit_alloc_ml[j][i]].steps;
		  k=(*alloc_ml)[i][bit_alloc_ml[j][i]].bits;
		  c = (unsigned int) getbits (bs, k);
		  for (k=0;k<3;k++)
		  {
			sample_ml[j][k][i] = c % nlevels;
			c /= nlevels;
		  }
		}
	   }
	   else
	   {      /* for no sample transmitted */
		 for (k=0;k<3;k++) sample_ml[j][k][i] = 0;
	   }
	 }
   }

   for (i = sblimit_ml; i < SBLIMIT; i++)
	 for (j = 0; j < *n_ml_ch; j++)
		for (k = 0; k < 3; k++)
		   sample_ml[j][k][i] = 0;
}

/**************************************************************
/*
/*   Restore the compressed sample to a factional number.
/*   first complement the MSB of the sample
/*    for layer I :
/*    Use s = (s' + 2^(-nb+1) ) * 2^nb / (2^nb-1)
/*   for Layer II :
/*   Use the formula s = s' * c + d
/*
/**************************************************************/

static double c[17] = { 1.33333333333, 1.60000000000, 1.14285714286,
						1.77777777777, 1.06666666666, 1.03225806452,
						1.01587301587, 1.00787401575, 1.00392156863,
						1.00195694716, 1.00097751711, 1.00048851979,
						1.00024420024, 1.00012208522, 1.00006103888,
						1.00003051851, 1.00001525902 };

static double d[17] = { 0.500000000, 0.500000000, 0.250000000, 0.500000000,
						0.125000000, 0.062500000, 0.031250000, 0.015625000,
						0.007812500, 0.003906250, 0.001953125, 0.0009765625,
						0.00048828125, 0.00024414063, 0.00012207031,
						0.00006103516, 0.00003051758 };

/************************** Layer II stuff ************************/

void I_dequantize_sample (unsigned int sample[7][3][SBLIMIT],
			  int part,
			  unsigned int bit_alloc[7][SBLIMIT],
			  double fraction[7][SBLIMIT][3][12],
			  frame_params *fr_ps, int *z)
{
   int i, nb, k, gr_id, gr_sam;
   int stereo = fr_ps->stereo;
   int sblimit = fr_ps->sblimit;

   gr_id = 4*part + (*z / 3);
   gr_sam = *z % 3;

   for (i=0; i<SBLIMIT; i++)
      for (k=0; k<stereo; k++)
	 if (bit_alloc[k][i])
	 {
	    nb = bit_alloc[k][i] + 1;
	    if (((sample[k][0][i] >> nb-1) & 1) == 1)
	       fraction[k][i][gr_sam][gr_id] = 0.0;
	    else
	       fraction[k][i][gr_sam][gr_id] = -1.0;

	    fraction[k][i][gr_sam][gr_id] += (double) (sample[k][0][i] & ((1<<nb-1)-1)) /
		 (double) (1L<<nb-1);

	    fraction[k][i][gr_sam][gr_id] =
		 (double) (fraction[k][i][gr_sam][gr_id] + 1.0 / (double)(1L<<nb-1)) *
		     (double) (1L<<nb) / (double) ((1L<<nb)-1);
	 }
	 else
	    fraction[k][i][gr_sam][gr_id] = 0.0;
}
void II_dequantize_sample (unsigned int sample[7][3][SBLIMIT],
			   unsigned int bit_alloc[7][SBLIMIT],
			   double fraction[7][SBLIMIT][3][12],
			   frame_params *fr_ps, int *z)
{
   int i, j, k, x;
   int stereo = fr_ps->stereo;
   int sblimit = fr_ps->sblimit;
   al_table *alloc = fr_ps->alloc;

   for (i=0;i<sblimit;i++)  for (j=0;j<3;j++) for (k = 0; k < stereo ; k++)
	 if (bit_alloc[k][i])
	 {
	   /* locate MSB in the sample */
	   x = 0;
#ifndef MSDOS
	   while ((1L<<x) < (*alloc)[i][bit_alloc[k][i]].steps) x++;
#else
	   /* microsoft C thinks an int is a short */
	   while (( (unsigned long) (1L<<(long)x) <
				(unsigned long)( (*alloc)[i][bit_alloc[k][i]].steps)
			  ) && ( x < 16) ) x++;
#endif

	   /* MSB inversion */
	   if (((sample[k][j][i] >> (x-1)) & 1) == 1)
		  fraction[k][i][j][*z] = 0.0;
	   else  fraction[k][i][j][*z] = -1.0;

	   /* Form a 2's complement sample */
	   fraction[k][i][j][*z] += (double) (sample[k][j][i] & ((1<<(x-1))-1)) /
							(double) (1L<<(x-1));

	   /* Dequantize the sample */
	   fraction[k][i][j][*z] += d[(*alloc)[i][bit_alloc[k][i]].quant];
	   fraction[k][i][j][*z] *= c[(*alloc)[i][bit_alloc[k][i]].quant];
	 }
	 else fraction[k][i][j][*z] = 0.0;

   for (i=sblimit;i<SBLIMIT;i++)
	  for (j=0;j<3;j++)
		 for(k = 0; k < stereo; k++)
			 fraction[k][i][j][*z] = 0.0;
}


/************************** MC Layer II stuff ************************/

void II_dequantize_sample_mc (unsigned int sample[7][3][SBLIMIT],
			      unsigned int bit_alloc[7][SBLIMIT],
			      double fraction[7][SBLIMIT][3][12],
			      frame_params *fr_ps, int ch_start, int channels, int *z)
{
   int i, j, k, x,sbgr,l;
   int sblimit = fr_ps->sblimit_mc;
   al_table *alloc = fr_ps->alloc_mc;

   for (i = 0; i < sblimit; i++)
   {
	if (i == 0) sbgr = 0;
	else
	   for (l = 1; l < 12; l++)
		if ((sb_groups[l-1] < i) && (i <= sb_groups[l]))
		{
		    sbgr = l; break;
		}

	for (j = 0; j < 3; j++)
	    for (k = ch_start; k < channels; k++)
		if (bit_alloc[k][i])
		{
		    if (fr_ps->header->dyn_cross_on == 0)
		    {
			/* locate MSB in the sample */
			x = 0;
#ifndef MSDOS
			while ((1L << x) < (*alloc)[i][bit_alloc[k][i]].steps)
			    x++;
#else
			/* microsoft C thinks an int is a short */
			while (((unsigned long) (1L << (long) x) <
			        (unsigned long) ((*alloc)[i][bit_alloc[k][i]].steps)) &&
			       (x < 16))
			    x++;
#endif
			/* MSB inversion */
			if (((sample[k][j][i] >> (x-1)) & 1) == 1)
			    fraction[k][i][j][*z] = 0.0;
			else 
			    fraction[k][i][j][*z] = -1.0;

			/* Form a 2's complement sample */
			fraction[k][i][j][*z] += (double) (sample[k][j][i] & ((1<<(x-1))-1)) /
						     (double) (1L<<(x-1));

			/* Dequantize the sample */
			fraction[k][i][j][*z] += d[(*alloc)[i][bit_alloc[k][i]].quant];
			fraction[k][i][j][*z] *= c[(*alloc)[i][bit_alloc[k][i]].quant];
		    }
		    else   /* 21.07.94 Ralf Schwalbe dyn. cross mode */
		    {
			if (fr_ps->dyn_cross_bits == 4  &&
			    ((fr_ps->header->dyn_cross_mode[sbgr]==0) ||
			     (fr_ps->header->dyn_cross_mode[sbgr]==1  && k!=4) ||
			     (fr_ps->header->dyn_cross_mode[sbgr]==2  && k!=3) ||
			     (fr_ps->header->dyn_cross_mode[sbgr]==3  && k!=2) ||
			     (fr_ps->header->dyn_cross_mode[sbgr]==8  && k!=4) ||
			     (fr_ps->header->dyn_cross_mode[sbgr]==9  && k!=3) ||
			     (fr_ps->header->dyn_cross_mode[sbgr]==10 && k!=4)))
			{
			    x = 0;
#ifndef MSDOS
			    while ((1L << x) < (*alloc)[i][bit_alloc[k][i]].steps)
				x++;
#else
			    /* microsoft C thinks an int is a short */
			    while (((unsigned long) (1L << (long) x) <
				    (unsigned long) ((*alloc)[i][bit_alloc[k][i]].steps)) &&
				   (x < 16))
				x++;
#endif
			    /* MSB inversion */
			    if (((sample[k][j][i] >> (x-1)) & 1) == 1)
				fraction[k][i][j][*z] = 0.0;
			    else
				fraction[k][i][j][*z] = -1.0;

			    /* Form a 2's complement sample */
			    fraction[k][i][j][*z] += (double) (sample[k][j][i] & ((1<<(x-1))-1)) /
						    (double) (1L<<(x-1));

			    /* Dequantize the sample */
			    fraction[k][i][j][*z] += d[(*alloc)[i][bit_alloc[k][i]].quant];
			    fraction[k][i][j][*z] *= c[(*alloc)[i][bit_alloc[k][i]].quant];
			}  /* end if 2-channel dyn-cross mode */

			if (fr_ps->dyn_cross_bits != 4 ||
			    ((fr_ps->header->dyn_cross_mode[sbgr]==4  && k==2) ||
			     (fr_ps->header->dyn_cross_mode[sbgr]==5  && k==3) ||
			     (fr_ps->header->dyn_cross_mode[sbgr]==6  && k==4) ||
			     (fr_ps->header->dyn_cross_mode[sbgr]==11 && k==2) ||
			     (fr_ps->header->dyn_cross_mode[sbgr]==12 && k==2) ||
			     (fr_ps->header->dyn_cross_mode[sbgr]==13 && k==3) ||
			     (fr_ps->header->dyn_cross_mode[sbgr]==14 && k==2))) 
			{
			     if ((fr_ps->dyn_cross_bits == 3 && ((fr_ps->header->dyn_cross_mode[sbgr] == 1 && k==3) ||
								 (fr_ps->header->dyn_cross_mode[sbgr] == 2 && k==2) ||
								 fr_ps->header->dyn_cross_mode[sbgr] == 3 ||
								 (fr_ps->header->dyn_cross_mode[sbgr] == 4 && k==3)
								)) ||
				 (fr_ps->dyn_cross_bits == 1 && fr_ps->header->dyn_cross_mode[sbgr] == 1 && k == 2) ||
				 (fr_ps->header->surround == 3 && fr_ps->header->dyn_second_stereo[sbgr] == 1 &&
				  ((fr_ps->header->center != 0 && j == 4) || (fr_ps->header->center == 0 && j == 3))))
			     {
				     /* no samples to dequantize */
			     }
			     else
			     {
				x = 0;
#ifndef MSDOS
				while ((1L << x) < (*alloc)[i][bit_alloc[k][i]].steps)
				    x++;
#else
				/* microsoft C thinks an int is a short */
				while (((unsigned long) (1L << (long) x) <
					(unsigned long) ((*alloc)[i][bit_alloc[k][i]].steps)) &&
				       (x < 16))
				    x++;
#endif
				/* MSB inversion */
				if (((sample[k][j][i] >> (x-1)) & 1) == 1)
				    fraction[k][i][j][*z] = 0.0;
				else
				    fraction[k][i][j][*z] = -1.0;

				/* Form a 2's complement sample */
				fraction[k][i][j][*z] += (double) (sample[k][j][i] & ((1<<(x-1))-1)) /
							(double) (1L<<(x-1));

				/* Dequantize the sample */
				fraction[k][i][j][*z] += d[(*alloc)[i][bit_alloc[k][i]].quant];
				fraction[k][i][j][*z] *= c[(*alloc)[i][bit_alloc[k][i]].quant];
			    } /* end if ..bits */
			}   /* end if 1-channel dyn-cross mode */
		    } /* end if dyn-cross on */
		} /* if bit_alloc */
	  	else
		    fraction[k][i][j][*z] = 0.0;
    }
    for (i = sblimit; i < SBLIMIT; i++)
	for (j = 0; j < 3; j++)
	    for (k = ch_start; k < channels; k++)
		fraction[k][i][j][*z] = 0.0;
}

#ifdef Augmentation_7ch
void II_dequantize_sample_aug (unsigned int sample[7][3][SBLIMIT],
			       unsigned int bit_alloc[7][SBLIMIT],
			       double fraction[7][SBLIMIT][3][12],
			       frame_params *fr_ps, int *z)
{
   int i, j, k, x,sbgr,l;
   int sblimit = fr_ps->sblimit_mc;
   al_table *alloc = fr_ps->alloc_mc;

   for (i = 0; i < sblimit; i++)
   {
	if( i == 0) sbgr = 0 ;
	else
	   for(l = 1; l < 12; l++)
		if((sb_groups[l-1] < i) && (i <= sb_groups[l]))
		{
		    sbgr = l; break;
		}
	for (j = 0; j < 3; j++)
	   for ( k = 5; k < 7; k++)
		if (bit_alloc[k][i])
		{
		   if ((fr_ps->header->dyn_cross_aug_mode[sbgr]== 0) ||
		       (fr_ps->header->dyn_cross_aug_mode[sbgr]== 1 && k==5) ||
		       (fr_ps->header->dyn_cross_aug_mode[sbgr]== 2 && k==5) ||
		       (fr_ps->header->dyn_cross_aug_mode[sbgr]== 3 && k==5) ||
		       (fr_ps->header->dyn_cross_aug_mode[sbgr]== 4 && k==5) ||
		       (fr_ps->header->dyn_cross_aug_mode[sbgr]== 5 && k==6) ||
		       (fr_ps->header->dyn_cross_aug_mode[sbgr]==10 && k==6) ||
		       (fr_ps->header->dyn_cross_aug_mode[sbgr]==14 && k==6))
		   {
			x = 0;
#ifndef MSDOS
			while ((1L<<x) < (*alloc)[i][bit_alloc[k][i]].steps) x++;
#else
			/* microsoft C thinks an int is a short */
			while (( (unsigned long) (1L<<(long)x) <
			(unsigned long)( (*alloc)[i][bit_alloc[k][i]].steps)) && ( x < 16) ) x++;
#endif
			/* MSB inversion */
			if (((sample[k][j][i] >> (x-1)) & 1) == 1)
			      fraction[k][i][j][*z] = 0.0;
			else  fraction[k][i][j][*z] = -1.0;

			/* Form a 2's complement sample */
			fraction[k][i][j][*z] += (double) (sample[k][j][i] & ((1<<(x-1))-1)) /
						(double) (1L<<(x-1));

			/* Dequantize the sample */
			fraction[k][i][j][*z] += d[(*alloc)[i][bit_alloc[k][i]].quant];
			fraction[k][i][j][*z] *= c[(*alloc)[i][bit_alloc[k][i]].quant];
		   }  /* end if 2-channel dyn-cross mode */
		} /* if bit_alloc */
		else
		   fraction[k][i][j][*z] = 0.0;
   }
   for (i = sblimit; i < SBLIMIT; i++)
	  for (j = 0; j < 3; j++)
		 for(k = 5; k < 7; k++)
			 fraction[k][i][j][*z] = 0.0;
}
#endif

/*************************************************************************/
/* JMZ 09/03/1995 Multilingual */

void II_dequantize_sample_ml(unsigned int sample_ml[7][3][SBLIMIT],
			     unsigned int bit_alloc_ml[7][SBLIMIT],
			     double fraction_ml[7][SBLIMIT][3][12],
			     frame_params *fr_ps,
			     int *n_ml_ch,
			     int *z)
{
   int i, j, k, x,sbgr,l;
   int sblimit_ml = fr_ps->sblimit_ml;
   al_table *alloc_ml = fr_ps->alloc_ml;

   for (i = 0; i < sblimit_ml; i++)
   {
	  for (j = 0; j < 3; j++)
		 for ( k = 0; k < *n_ml_ch; k++)
			if (bit_alloc_ml[k][i])
			{
			  /* JR: ditto */
			  /* locate MSB in the sample */
			   x = 0;
#ifndef MSDOS
			   while ((1L<<x) < (*alloc_ml)[i][bit_alloc_ml[k][i]].steps) x++;
#else
			   /* microsoft C thinks an int is a short */
			   while (( (unsigned long) (1L<<(long)x) <
				(unsigned long)( (*alloc_ml)[i][bit_alloc_ml[k][i]].steps)
			   ) && ( x < 16) ) x++;
#endif
			   /* MSB inversion */
			   if (((sample_ml[k][j][i] >> (x-1)) & 1) == 1)
					fraction_ml[k][i][j][*z] = 0.0;
			   else  fraction_ml[k][i][j][*z] = -1.0;

			   /* Form a 2's complement sample */
			   fraction_ml[k][i][j][*z] += (double) (sample_ml[k][j][i] & ((1<<(x-1))-1)) /
							(double) (1L<<(x-1));

			   /* Dequantize the sample */
			   fraction_ml[k][i][j][*z] += d[(*alloc_ml)[i][bit_alloc_ml[k][i]].quant];
			   fraction_ml[k][i][j][*z] *= c[(*alloc_ml)[i][bit_alloc_ml[k][i]].quant];
			} /* if bit_alloc */
			else fraction_ml[k][i][j][*z] = 0.0;
   }
   for (i = sblimit_ml; i < SBLIMIT; i++)
	  for (j = 0; j < 3; j++)
		 for(k = 0; k < *n_ml_ch; k++)
			 fraction_ml[k][i][j][*z] = 0.0;
}

/**************************************************************************
II_lfe_calc();  R.Schwalbe 
**************************************************************************/
void II_lfe_calc(frame_params *fr_ps)
{
   layer *info = fr_ps->header;
   int x,i;
   al_table *alloc = fr_ps->alloc_mc;

   for(i = 0; i < 12; i++)
   {
	x = 0;
#ifndef MSDOS
   	while ((1L<<x) < (*alloc)[0][info->lfe_alloc].steps) x++;
#else
   	/* microsoft C thinks an int is a short */
   	while (( (unsigned long) (1L<<(long)x) <
   	(unsigned long)( (*alloc)[0][info->lfe_alloc].steps)) && ( x < 16) ) x++;
#endif
   	/* MSB inversion */
   	if (((info->lfe_spl[i] >> (x-1)) & 1) == 1)
     		info->lfe_spl_fraction[i] = 0.0;
   	else 	info->lfe_spl_fraction[i] = -1.0;

   	/* Form a 2's complement sample */
   	info->lfe_spl_fraction[i] += (double) (info->lfe_spl[i] & ((1<<(x-1))-1)) /
	 			(double) (1L<<(x-1));

   	/* Dequantize the sample */
   	info->lfe_spl_fraction[i] += d[(*alloc)[0][info->lfe_alloc].quant];
   	info->lfe_spl_fraction[i] *= c[(*alloc)[0][info->lfe_alloc].quant];

   	/* Denormalize the sample */
   	info->lfe_spl_fraction[i] *= multiple[info->lfe_scf];
    }
}

/************************************************************
/*
/*   Restore the original value of the sample ie multiply
/*    the fraction value by its scalefactor.
/*
/************************* Layer II Stuff **********************/

void I_denormalize_sample (double fraction[7][SBLIMIT][3][12],
			   unsigned int scale_index[7][3][SBLIMIT],
			   int part,
			   frame_params *fr_ps,
			   int *z)
{
   int i, j, k, gr_id, gr_sam;
   int stereo = fr_ps->stereo;
   int sblimit = fr_ps->sblimit;

   gr_id = 4*part + (*z / 3);
   gr_sam = *z % 3;

   for (i=0; i<SBLIMIT; i++)
      for (j=0; j<stereo; j++)
	 fraction[j][i][gr_sam][gr_id] *= multiple[scale_index[j][part][i]];
}

void II_denormalize_sample(double fraction[7][SBLIMIT][3][12],
			   unsigned int scale_index[7][3][SBLIMIT],
			   frame_params *fr_ps,
			   int x,
			   int *z)
{
   int i,j,k;
   int stereo = fr_ps->stereo;
   int sblimit = fr_ps->sblimit;

   for (i=0;i<sblimit;i++)
	for (j = 0;j < stereo; j++)
	{
	  fraction[j][i][0][*z] *= multiple[scale_index[j][x][i]];
	  fraction[j][i][1][*z] *= multiple[scale_index[j][x][i]];
	  fraction[j][i][2][*z] *= multiple[scale_index[j][x][i]];
	}
}

/************************* MC Layer II Stuff **********************/

void II_denormalize_sample_mc(double fraction[7][SBLIMIT][3][12],
			     unsigned int scale_index[7][3][SBLIMIT],
			     frame_params *fr_ps,
			     int x, int ch_start, int channels, int *z)
{
   int i,j,k,sbgr,l,bl=0;
   int sblimit = fr_ps->sblimit_mc;

   for (i = 0; i < sblimit; i++)
   {
      if (i == 0) sbgr = 0 ;
      else
	for (l = 1; l < 12; l++)
		if ((sb_groups[l-1] < i) && (i <= sb_groups[l]))
		{
			sbgr = l; break;
		}

      /* 960821 FdB new setup for dyn. crosstalk modes */
      for (bl = 0; bl < 3; bl++)	  		
	if (fr_ps->header->dyn_cross_on == 0)
		for (j = ch_start; j < channels; j++)
			fraction[j][i][bl][*z] *= multiple[scale_index[j][x][i]];
	else if (fr_ps->dyn_cross_bits == 0)   /* for channel mode 2/0 (+2/0) */
	{
		if (fr_ps->header->surround == 3)	/* 2/0 + 2/0 and 1/0 + 2/0 */
		{
			fraction[ch_start][i][bl][*z] *= multiple[scale_index[ch_start][x][i]];
			if (fr_ps->header->dyn_second_stereo[sbgr] == 0)
				fraction[ch_start+1][i][bl][*z] *= multiple[scale_index[ch_start+1][x][i]];
			else
				fraction[ch_start+1][i][bl][*z] = (fraction[ch_start][i][bl][*z] / multiple[scale_index[ch_start][x][i]])
									 * multiple[scale_index[ch_start+1][x][i]];
		}
	}
	else if (fr_ps->dyn_cross_bits == 1)   /* for channel mode 3/0 (+2/0) and 2/1 */
	{
		switch (fr_ps->header->dyn_cross_mode[sbgr])
		{
		  case 0 :	fraction[2][i][bl][*z] *= multiple[scale_index[2][x][i]];
				if (fr_ps->header->surround == 3)		/* 3/0 + 2/0 */
				{
					fraction[3][i][bl][*z] *= multiple[scale_index[3][x][i]];
					if (fr_ps->header->dyn_second_stereo[sbgr] == 0)
						fraction[4][i][bl][*z] *= multiple[scale_index[4][x][i]];
					else
						fraction[4][i][bl][*z] = (fraction[3][i][bl][*z] / multiple[scale_index[3][x][i]])
											 * multiple[scale_index[4][x][i]];
				}
			  	break;
		  case 1 :	if (fr_ps->header->tc_alloc[sbgr] == 0)
					if (fr_ps->header->dyn_cross_LR)	/* C,S from R0 */ 
						fraction[2][i][bl][*z] = (fraction[1][i][bl][*z] / multiple[scale_index[1][x][i]])
							 					 * multiple[scale_index[2][x][i]];
				    	else					/* C,S from L0 */
						fraction[2][i][bl][*z] = (fraction[0][i][bl][*z] / multiple[scale_index[0][x][i]])
												 * multiple[scale_index[2][x][i]];
				if (fr_ps->header->tc_alloc[sbgr] == 1)
					fraction[2][i][bl][*z] = (fraction[0][i][bl][*z] / multiple[scale_index[0][x][i]])
											 * multiple[scale_index[2][x][i]];
				if (fr_ps->header->tc_alloc[sbgr] == 2)
					fraction[2][i][bl][*z] = (fraction[1][i][bl][*z] / multiple[scale_index[1][x][i]])
											 * multiple[scale_index[2][x][i]];
				if (fr_ps->header->surround == 3)		/* 3/0 + 2/0 */
				{
					fraction[3][i][bl][*z] *= multiple[scale_index[3][x][i]];
					if (fr_ps->header->dyn_second_stereo[sbgr] == 0)
						fraction[4][i][bl][*z] *= multiple[scale_index[4][x][i]];
					else
						fraction[4][i][bl][*z] = (fraction[3][i][bl][*z] / multiple[scale_index[3][x][i]])
											 * multiple[scale_index[4][x][i]];
				}
		  	break;
		}
	}
	else if (fr_ps->dyn_cross_bits == 3)   /* for channel mode 3/1 and 2/2 */
	{
		switch (fr_ps->header->dyn_cross_mode[sbgr])
		{
		  case 0 :	for (j = ch_start; j < channels; j++)
					fraction[j][i][bl][*z] *= multiple[scale_index[j][x][i]];
			  	break;
		  case 1 :	fraction[2][i][bl][*z] *= multiple[scale_index[2][x][i]];
				if (fr_ps->header->surround == 2 ||		/* 2/2 mode */
				    fr_ps->header->tc_alloc[sbgr] == 4 ||	/* 3/1 mode */
				    fr_ps->header->tc_alloc[sbgr] == 5 ||	/* 3/1 mode, matrix mode 2 */
				    (fr_ps->header->tc_alloc[sbgr] != 3 && fr_ps->header->dyn_cross_LR))
					/* S, R or Rs from R0 */
					fraction[3][i][bl][*z]  = (fraction[1][i][bl][*z] / multiple[scale_index[1][x][i]])
											  * multiple[scale_index[3][x][i]];
				else
					/* S, L or Ls from L0 */
					fraction[3][i][bl][*z]  = (fraction[0][i][bl][*z] / multiple[scale_index[0][x][i]])
											  * multiple[scale_index[3][x][i]];
			  	break;
		  case 2 :	fraction[3][i][bl][*z] *= multiple[scale_index[3][x][i]];
				if (fr_ps->header->surround == 2 ||		/* 2/2 mode */
				    fr_ps->header->tc_alloc[sbgr] == 1 ||	/* 3/1 mode */
				    fr_ps->header->tc_alloc[sbgr] == 5 ||	/* 3/1 mode, matrix mode 2 */
				    (fr_ps->header->tc_alloc[sbgr] != 2 && !fr_ps->header->dyn_cross_LR))
					/* C, L or Ls from L0 */
					fraction[2][i][bl][*z] = (fraction[0][i][bl][*z] / multiple[scale_index[0][x][i]])
											 * multiple[scale_index[2][x][i]];
				else
					/* C, R or Rs from RO */
					fraction[2][i][bl][*z] = (fraction[1][i][bl][*z] / multiple[scale_index[1][x][i]])
											 * multiple[scale_index[2][x][i]];
			  	break;
		  case 3 :	if (fr_ps->header->surround == 2 ||		/* 2/2 mode */
				    fr_ps->header->tc_alloc[sbgr] == 1 ||	/* 3/1 mode */
				    fr_ps->header->tc_alloc[sbgr] == 5 ||	/* 3/1 mode, matrix mode 2 */
				    (fr_ps->header->tc_alloc[sbgr] != 2 && !fr_ps->header->dyn_cross_LR))
					/* C, L or Ls from L0 */
					fraction[2][i][bl][*z] = (fraction[0][i][bl][*z] / multiple[scale_index[0][x][i]])
											 * multiple[scale_index[2][x][i]];
				else
					/* C, R or Rs from RO */
					fraction[2][i][bl][*z] = (fraction[1][i][bl][*z] / multiple[scale_index[1][x][i]])
											 * multiple[scale_index[2][x][i]];
				if (fr_ps->header->surround == 2 ||		/* 2/2 mode */
				    fr_ps->header->tc_alloc[sbgr] == 4 ||	/* 3/1 mode */
				    fr_ps->header->tc_alloc[sbgr] == 5 ||	/* 3/1 mode, matrix mode 2 */
				    (fr_ps->header->tc_alloc[sbgr] != 3 && fr_ps->header->dyn_cross_LR))
					/* S, R or Rs from R0 */
					fraction[3][i][bl][*z]  = (fraction[1][i][bl][*z] / multiple[scale_index[1][x][i]])
											  * multiple[scale_index[3][x][i]];
				else
					/* S, L or Ls from L0 */
					fraction[3][i][bl][*z]  = (fraction[0][i][bl][*z] / multiple[scale_index[0][x][i]])
											  * multiple[scale_index[3][x][i]];
			  	break;
		  case 4 :	fraction[3][i][bl][*z] = fraction[2][i][bl][*z];
				fraction[2][i][bl][*z] *= multiple[scale_index[2][x][i]];
				fraction[3][i][bl][*z] *= multiple[scale_index[3][x][i]];
			  	break;
		}
	}
	else if (fr_ps->dyn_cross_bits == 4)   /* for channel mode 3/2 */
	{
		switch (fr_ps->header->dyn_cross_mode[sbgr])
		{
		  case 0 :	for (j = ch_start; j < channels; j++)
					fraction[j][i][bl][*z] *= multiple[scale_index[j][x][i]];
			  	break;
		  case 1 :	/* C from T2 */
				fraction[2][i][bl][*z] *= multiple[scale_index[2][x][i]];
				fraction[3][i][bl][*z] *= multiple[scale_index[3][x][i]];
				fraction[4][i][bl][*z]  = (fraction[1][i][bl][*z] / multiple[scale_index[1][x][i]])
										  * multiple[scale_index[4][x][i]];
			  	break;
		  case 2 :	/* C from T2 */
				fraction[2][i][bl][*z] = fraction[2][i][bl][*z] * multiple[scale_index[2][x][i]];
				fraction[3][i][bl][*z]  = (fraction[0][i][bl][*z] / multiple[scale_index[0][x][i]])
										  * multiple[scale_index[3][x][i]];
				fraction[4][i][bl][*z] *= multiple[scale_index[4][x][i]];
			  	break;
		  case 3 :	if ((fr_ps->header->tc_alloc[sbgr] == 1) || (fr_ps->header->tc_alloc[sbgr] == 7))
					fraction[2][i][bl][*z] = (fraction[0][i][bl][*z] / multiple[scale_index[0][x][i]])
											 * multiple[scale_index[2][x][i]];
				else if ((fr_ps->header->tc_alloc[sbgr] == 2) || (fr_ps->header->tc_alloc[sbgr] == 6))
					fraction[2][i][bl][*z] = (fraction[1][i][bl][*z] / multiple[scale_index[1][x][i]])
											 * multiple[scale_index[2][x][i]];
				else if (fr_ps->header->dyn_cross_LR)	/* C from RO */
					fraction[2][i][bl][*z] = (fraction[1][i][bl][*z] / multiple[scale_index[1][x][i]])
											 * multiple[scale_index[2][x][i]];
				else					/* C from LO */
					fraction[2][i][bl][*z] = (fraction[0][i][bl][*z] / multiple[scale_index[0][x][i]])
											 * multiple[scale_index[2][x][i]];

				fraction[3][i][bl][*z] *= multiple[scale_index[3][x][i]]; 
				fraction[4][i][bl][*z] *= multiple[scale_index[4][x][i]];
			  	break;
		  case 4 :	/* C from T2 */
				fraction[2][i][bl][*z] *= multiple[scale_index[2][x][i]];
				fraction[3][i][bl][*z]  = (fraction[0][i][bl][*z] / multiple[scale_index[0][x][i]])
										  * multiple[scale_index[3][x][i]];
				fraction[4][i][bl][*z]  = (fraction[1][i][bl][*z] / multiple[scale_index[1][x][i]])
										  * multiple[scale_index[4][x][i]];
			  	break;
		  case 5 :	if ((fr_ps->header->tc_alloc[sbgr] == 1) || (fr_ps->header->tc_alloc[sbgr] == 7))
					fraction[2][i][bl][*z] = (fraction[0][i][bl][*z] / multiple[scale_index[0][x][i]])
											 * multiple[scale_index[2][x][i]];
				else if ((fr_ps->header->tc_alloc[sbgr] == 2) || (fr_ps->header->tc_alloc[sbgr] == 6))
					fraction[2][i][bl][*z] = (fraction[1][i][bl][*z] / multiple[scale_index[1][x][i]])
											 * multiple[scale_index[2][x][i]];
				else if (fr_ps->header->dyn_cross_LR)  /* C from RO */
					fraction[2][i][bl][*z] = (fraction[1][i][bl][*z] / multiple[scale_index[1][x][i]])
											 * multiple[scale_index[2][x][i]];
				else					/* C from LO */
					fraction[2][i][bl][*z] = (fraction[0][i][bl][*z] / multiple[scale_index[0][x][i]])
											 * multiple[scale_index[2][x][i]];

				fraction[3][i][bl][*z] *= multiple[scale_index[3][x][i]];
				fraction[4][i][bl][*z]  = (fraction[1][i][bl][*z] / multiple[scale_index[1][x][i]])
										  * multiple[scale_index[4][x][i]];
			  	break;
		  case 6 :	if ((fr_ps->header->tc_alloc[sbgr] == 1) || (fr_ps->header->tc_alloc[sbgr] == 7))
					fraction[2][i][bl][*z] = (fraction[0][i][bl][*z] / multiple[scale_index[0][x][i]])
											 * multiple[scale_index[2][x][i]];
				else if ((fr_ps->header->tc_alloc[sbgr] == 2) || (fr_ps->header->tc_alloc[sbgr] == 6))
					fraction[2][i][bl][*z] = (fraction[1][i][bl][*z] / multiple[scale_index[1][x][i]])
											 * multiple[scale_index[2][x][i]];
				else if (fr_ps->header->dyn_cross_LR)	/* C from RO */
					fraction[2][i][bl][*z] = (fraction[1][i][bl][*z] / multiple[scale_index[1][x][i]])
											 * multiple[scale_index[2][x][i]];
				else					/* C from LO */
					fraction[2][i][bl][*z] = (fraction[0][i][bl][*z] / multiple[scale_index[0][x][i]])
											 * multiple[scale_index[2][x][i]];

				fraction[3][i][bl][*z]  = (fraction[0][i][bl][*z] / multiple[scale_index[0][x][i]])
										  * multiple[scale_index[3][x][i]];
				fraction[4][i][bl][*z] *= multiple[scale_index[4][x][i]];
			  	break;
		  case 7 :	if ((fr_ps->header->tc_alloc[sbgr] == 1) || (fr_ps->header->tc_alloc[sbgr] == 7))
					fraction[2][i][bl][*z] = (fraction[0][i][bl][*z] / multiple[scale_index[0][x][i]])
											 * multiple[scale_index[2][x][i]];
				else if ((fr_ps->header->tc_alloc[sbgr] == 2) || (fr_ps->header->tc_alloc[sbgr] == 6))
					fraction[2][i][bl][*z] = (fraction[1][i][bl][*z] / multiple[scale_index[1][x][i]])
											 * multiple[scale_index[2][x][i]];
				else if (fr_ps->header->dyn_cross_LR)	/* C from RO */
					fraction[2][i][bl][*z] = (fraction[1][i][bl][*z] / multiple[scale_index[1][x][i]])
											 * multiple[scale_index[2][x][i]];
				else					/* C from LO */
					fraction[2][i][bl][*z] = (fraction[0][i][bl][*z] / multiple[scale_index[0][x][i]])
											 * multiple[scale_index[2][x][i]];
				fraction[3][i][bl][*z]  = (fraction[0][i][bl][*z] / multiple[scale_index[0][x][i]])
										  * multiple[scale_index[3][x][i]];
				fraction[4][i][bl][*z]  = (fraction[1][i][bl][*z] / multiple[scale_index[1][x][i]])
										  * multiple[scale_index[4][x][i]];
			  	break;
		  case 8 :	/* C from T2 */
				fraction[2][i][bl][*z] *= multiple[scale_index[2][x][i]];
				fraction[4][i][bl][*z]  = fraction[3][i][bl][*z];
				fraction[3][i][bl][*z] *= multiple[scale_index[3][x][i]];
				fraction[4][i][bl][*z] *= multiple[scale_index[4][x][i]];
			  	break;
		  case 9 :	/* C from T2 */
				fraction[3][i][bl][*z] = fraction[2][i][bl][*z];
				fraction[2][i][bl][*z] *= multiple[scale_index[2][x][i]];
				fraction[3][i][bl][*z] *= multiple[scale_index[3][x][i]];
				fraction[4][i][bl][*z] *= multiple[scale_index[4][x][i]];
			  	break;
		  case 10 :	/* C from T2 */
				fraction[4][i][bl][*z]  = fraction[2][i][bl][*z];
				fraction[2][i][bl][*z] *= multiple[scale_index[2][x][i]];
				fraction[3][i][bl][*z] *= multiple[scale_index[3][x][i]];
				fraction[4][i][bl][*z] *= multiple[scale_index[4][x][i]];
			  	break;
		  case 11 :	fraction[3][i][bl][*z]  = fraction[2][i][bl][*z];
				fraction[2][i][bl][*z] *= multiple[scale_index[2][x][i]];
				fraction[3][i][bl][*z] *= multiple[scale_index[3][x][i]];
				fraction[4][i][bl][*z]  = (fraction[1][i][bl][*z] / multiple[scale_index[1][x][i]])
										  * multiple[scale_index[4][x][i]];
			  	break;
		  case 12 :	fraction[4][i][bl][*z]  = fraction[2][i][bl][*z];
				fraction[2][i][bl][*z] *= multiple[scale_index[2][x][i]];
				fraction[3][i][bl][*z]  = (fraction[0][i][bl][*z] / multiple[scale_index[0][x][i]])
										  * multiple[scale_index[3][x][i]];
				fraction[4][i][bl][*z] *= multiple[scale_index[4][x][i]];
			  	break;
		  case 13:	if ((fr_ps->header->tc_alloc[sbgr] == 1) || (fr_ps->header->tc_alloc[sbgr] == 7))
					fraction[2][i][bl][*z] = (fraction[0][i][bl][*z] / multiple[scale_index[0][x][i]])
											 * multiple[scale_index[2][x][i]];
				else if ((fr_ps->header->tc_alloc[sbgr] == 2) || (fr_ps->header->tc_alloc[sbgr] == 6))
					fraction[2][i][bl][*z] = (fraction[1][i][bl][*z] / multiple[scale_index[1][x][i]])
											 * multiple[scale_index[2][x][i]];
				else if (fr_ps->header->dyn_cross_LR)	/* C from RO */
					fraction[2][i][bl][*z] = (fraction[1][i][bl][*z] / multiple[scale_index[1][x][i]])
											 * multiple[scale_index[2][x][i]];
				else					/* C from LO */
					fraction[2][i][bl][*z] = (fraction[0][i][bl][*z] / multiple[scale_index[0][x][i]])
											 * multiple[scale_index[2][x][i]];
				fraction[4][i][bl][*z]  = fraction[3][i][bl][*z];
				fraction[3][i][bl][*z] *= multiple[scale_index[3][x][i]];
				fraction[4][i][bl][*z] *= multiple[scale_index[4][x][i]];
			  	break;
		  case 14 :	fraction[4][i][bl][*z]  = fraction[2][i][bl][*z];
				fraction[3][i][bl][*z]  = fraction[2][i][bl][*z];
				/* C from T2 */
				fraction[2][i][bl][*z] *= multiple[scale_index[2][x][i]];
				fraction[3][i][bl][*z] *= multiple[scale_index[3][x][i]];
				fraction[4][i][bl][*z] *= multiple[scale_index[4][x][i]];	
				break;
		}
	}
   } /* for sblimit */
}

#ifdef Augmentation_7ch
void II_denormalize_sample_aug(double fraction[7][SBLIMIT][3][12],
			     unsigned int scale_index[7][3][SBLIMIT],
			     frame_params *fr_ps,
			     int x,
			     int *z)
{
   layer *info = fr_ps->header;
   int i,j,k,sbgr,l,bl=0;
   int sblimit = fr_ps->sblimit_mc;
    
   for (i = 0; i < sblimit; i++)
   {
      int T5T0 = 0, T6T0 = 0, T6T1 = 1;

      if (i == 0) sbgr = 0 ;
      else
	 for (l = 1; l < 12; l++)
	    if ((sb_groups[l-1] < i) && (i <= sb_groups[l]))
	    {
	       sbgr = l; break;
	    }

      if (info->tc_aug_alloc[sbgr] == 4 || info->tc_aug_alloc[sbgr] == 5)
    	if (info->dyn_cross_LR == 0)
	  T6T1 = 0;
	else
	  T6T0 = 1;
      else if (info->tc_aug_alloc[sbgr] == 6 || info->tc_aug_alloc[sbgr] == 7)
        if (info->dyn_cross_LR)
	  T5T0 = 1;


      /* 960821 FdB new setup for dyn. crosstalk modes */
      for (bl = 0; bl < 3; bl++)
      {
	 switch (fr_ps->header->dyn_cross_aug_mode[sbgr])
	 {
	   case  0 :	/* T5 and T6 present */
			 for (j = 5; j < 7; j++)
			    fraction[j][i][bl][*z] *= multiple[scale_index[j][x][i]];
			 break;
	   case  1 :	/* T6 from T1 */
			 fraction[5][i][bl][*z] *= multiple[scale_index[5][x][i]];
			 fraction[6][i][bl][*z]  = (fraction[T6T1][i][bl][*z] / multiple[scale_index[T6T1][x][i]])
									      * multiple[scale_index[6][x][i]];
			 break;
	   case  2 :	/* T6 from T2 */
			 fraction[5][i][bl][*z] *= multiple[scale_index[5][x][i]];
			 fraction[6][i][bl][*z]  = (fraction[2][i][bl][*z] / multiple[scale_index[2][x][i]])
									   * multiple[scale_index[6][x][i]];
			 break;
	   case  3 :	/* T6 from T4 */
			 fraction[5][i][bl][*z] *= multiple[scale_index[5][x][i]];
			 fraction[6][i][bl][*z]  = (fraction[4][i][bl][*z] / multiple[scale_index[4][x][i]])
									   * multiple[scale_index[6][x][i]];
			 break;
	   case  4 :	/* T6 from T5 */
			 fraction[5][i][bl][*z] *= multiple[scale_index[5][x][i]];
			 fraction[6][i][bl][*z]  = (fraction[5][i][bl][*z] / multiple[scale_index[5][x][i]])
									   * multiple[scale_index[6][x][i]];
			 break;
	   case  5 :	/* T5 from T0 */
			 fraction[5][i][bl][*z]  = (fraction[T5T0][i][bl][*z] / multiple[scale_index[T5T0][x][i]])
									      * multiple[scale_index[5][x][i]];
			 fraction[6][i][bl][*z] *= multiple[scale_index[6][x][i]];
			 break;
	   case  6 :	/* T5 from T0 and T6 from T1 */
			 fraction[5][i][bl][*z]  = (fraction[T5T0][i][bl][*z] / multiple[scale_index[T5T0][x][i]])
									      * multiple[scale_index[5][x][i]];
			 fraction[6][i][bl][*z]  = (fraction[T6T1][i][bl][*z] / multiple[scale_index[T6T1][x][i]])
									      * multiple[scale_index[6][x][i]];
			 break;
	   case  7 :	/* T5 from T0 and T6 from T2 */
			 fraction[5][i][bl][*z]  = (fraction[T5T0][i][bl][*z] / multiple[scale_index[T5T0][x][i]])
									      * multiple[scale_index[5][x][i]];
			 fraction[6][i][bl][*z]  = (fraction[2][i][bl][*z] / multiple[scale_index[2][x][i]])
									   * multiple[scale_index[6][x][i]];
			 break;
	   case  8 :	/* T5 from T0 and T6 from T4 */
			 fraction[5][i][bl][*z]  = (fraction[T5T0][i][bl][*z] / multiple[scale_index[T5T0][x][i]])
									      * multiple[scale_index[5][x][i]];
			 fraction[6][i][bl][*z]  = (fraction[4][i][bl][*z] / multiple[scale_index[4][x][i]])
									   * multiple[scale_index[6][x][i]];
			 break;
	   case  9 :	/* T5 and T6 from T0 */
			 fraction[5][i][bl][*z]  = (fraction[T5T0][i][bl][*z] / multiple[scale_index[T5T0][x][i]])
									      * multiple[scale_index[5][x][i]];
			 fraction[6][i][bl][*z]  = (fraction[T6T0][i][bl][*z] / multiple[scale_index[T6T0][x][i]])
									      * multiple[scale_index[6][x][i]];
			 break;
	   case 10 :	/* T5 from T2 */
			 fraction[5][i][bl][*z]  = (fraction[2][i][bl][*z] / multiple[scale_index[2][x][i]])
									   * multiple[scale_index[5][x][i]];
			 fraction[6][i][bl][*z] *= multiple[scale_index[6][x][i]];
			 break;
	   case 11 :	/* T5 from T2 and T6 from T1 */
			 fraction[5][i][bl][*z]  = (fraction[2][i][bl][*z] / multiple[scale_index[2][x][i]])
									   * multiple[scale_index[5][x][i]];
			 fraction[6][i][bl][*z]  = (fraction[T6T1][i][bl][*z] / multiple[scale_index[T6T1][x][i]])
									   * multiple[scale_index[6][x][i]];
			 break;
	   case 12 :	/* T5 and T6 from T2 */
			 fraction[5][i][bl][*z]  = (fraction[2][i][bl][*z] / multiple[scale_index[2][x][i]])
									   * multiple[scale_index[5][x][i]];
			 fraction[6][i][bl][*z]  = (fraction[2][i][bl][*z] / multiple[scale_index[2][x][i]])
									   * multiple[scale_index[6][x][i]];
			 break;
	   case 13 :	/* T5 from T2 and T6 from T4 */
			 fraction[5][i][bl][*z]  = (fraction[2][i][bl][*z] / multiple[scale_index[2][x][i]])
									   * multiple[scale_index[5][x][i]];
			 fraction[6][i][bl][*z]  = (fraction[4][i][bl][*z] / multiple[scale_index[4][x][i]])
									   * multiple[scale_index[6][x][i]];
			 break;
	   case 14 :	/* T5 from T3 */
			 fraction[5][i][bl][*z]  = (fraction[3][i][bl][*z] / multiple[scale_index[3][x][i]])
									   * multiple[scale_index[5][x][i]];
			 fraction[6][i][bl][*z] *= multiple[scale_index[6][x][i]];
			 break;
	   case 15 :	/* T5 from T3 and T6 from T1 */
			 fraction[5][i][bl][*z]  = (fraction[3][i][bl][*z] / multiple[scale_index[3][x][i]])
									   * multiple[scale_index[5][x][i]];
			 fraction[6][i][bl][*z]  = (fraction[T6T1][i][bl][*z] / multiple[scale_index[T6T1][x][i]])
									      * multiple[scale_index[6][x][i]];
			 break;
	   case 16 :	/* T5 from T3 and T6 from T2 */
			 fraction[5][i][bl][*z]  = (fraction[3][i][bl][*z] / multiple[scale_index[3][x][i]])
									   * multiple[scale_index[5][x][i]];
			 fraction[6][i][bl][*z]  = (fraction[2][i][bl][*z] / multiple[scale_index[2][x][i]])
									   * multiple[scale_index[6][x][i]];
			 break;
	   case 17 :	/* T5 from T3 and T6 from T4 */
			 fraction[5][i][bl][*z]  = (fraction[3][i][bl][*z] / multiple[scale_index[3][x][i]])
									   * multiple[scale_index[5][x][i]];
			 fraction[6][i][bl][*z]  = (fraction[4][i][bl][*z] / multiple[scale_index[4][x][i]])
									   * multiple[scale_index[6][x][i]];
			 break;
	   case 18 :	/* T5 and T6 from T3 */
			 fraction[5][i][bl][*z]  = (fraction[3][i][bl][*z] / multiple[scale_index[3][x][i]])
									   * multiple[scale_index[5][x][i]];
			 fraction[6][i][bl][*z]  = (fraction[3][i][bl][*z] / multiple[scale_index[3][x][i]])
									   * multiple[scale_index[6][x][i]];
			 break;
	 }
      }	  		
   } /* for sblimit */
}
#endif

/*************************************************************/
/* JMZ 09/03/1995 Multilingual */

void II_denormalize_sample_ml(double fraction_ml[7][SBLIMIT][3][12],
			      unsigned int scale_index_ml[7][3][SBLIMIT],
			      frame_params *fr_ps,
			      int x,
			      int *n_ml_ch,
			      int *z)
{
   int i,j;
   int sblimit_ml = fr_ps->sblimit_ml;

   for (i = 0; i < sblimit_ml; i++)
    for (j = 0; j < *n_ml_ch; j++)
     {
       fraction_ml[j][i][0][*z] *= multiple[scale_index_ml[j][x][i]];
       fraction_ml[j][i][1][*z] *= multiple[scale_index_ml[j][x][i]];
       fraction_ml[j][i][2][*z] *= multiple[scale_index_ml[j][x][i]];
     }
}

/*****************************************************************
/*
/* The following are the subband synthesis routines. They apply
/* to both layer I and layer II stereo or mono. The user has to
/* decide what parameters are to be passed to the routines.
/*
/***************************************************************/

/*************************************************************
/*
/*   Pass the subband sample through the synthesis window
/*
/**************************************************************/

/* create in synthesis filter */

static void create_syn_filter(double filter[64][SBLIMIT])
{
   register int i,k;

   for (i=0; i<64; i++)
      for (k=0; k<32; k++) {
		 if ((filter[i][k] = 1e9*cos((double)((PI64*i+PI4)*(2*k+1)))) >= 0)
			modf(filter[i][k]+0.5, &filter[i][k]);
         else
            modf(filter[i][k]-0.5, &filter[i][k]);
		 filter[i][k] *= 1e-9;
	  }
}

/***************************************************************
/*
/*   Window the restored sample
/*
/***************************************************************/

/* read in synthesis window */

static void read_syn_window(double window[HAN_SIZE])
{
   int i,j[4];
   FILE *fp;
   double f[4];
   char t[150];

   if (!(fp = OpenTableFile("dewindow") )) {
	  fprintf (stderr, "Please check synthesis window table 'dewindow'\n");
	  exit (1);
   }
   for (i=0;i<512;i+=4) {
		fgets(t,150, fp);
	  sscanf(t,"D[%d] = %lf D[%d] = %lf D[%d] = %lf D[%d] = %lf\n",
			 j, f,j+1,f+1,j+2,f+2,j+3,f+3);
	  if (i==j[0]) {
		 window[i] = f[0];
		 window[i+1] = f[1];
		 window[i+2] = f[2];
		 window[i+3] = f[3];
	  }
	  else {
		 fprintf (stderr, "Check index in synthesis window table\n");
			exit (1);
	  }
	  fgets(t,80,fp);
   }
   fclose(fp);
}


int SubBandSynthesis (double *bandPtr,
		      int channel,
		      long *samples)
{
    long foo;
    register int i,j,k;
    register double *bufOffsetPtr, sum;
    static int init = 1;
    typedef double NN[64][32];
    static NN *filter;
    typedef double BB[7][2*HAN_SIZE]; 
    static BB *buf;
    static int bufOffset = 64;
    static double *window;
    int clip = 0;               /* count & return how many samples clipped */

    if (init) {
		buf = (BB *) mem_alloc(sizeof(BB),"BB");
		filter = (NN *) mem_alloc(sizeof(NN), "NN");
		create_syn_filter(*filter);
		window = (double *) mem_alloc(sizeof(double) * HAN_SIZE, "WIN");
		read_syn_window(window);
		bufOffset = 64;
		init = 0;
	}
	if (channel == 0) bufOffset = (bufOffset - 64) & 0x3ff;
	bufOffsetPtr = &((*buf)[channel][bufOffset]);

	for (i=0; i<64; i++)
	{
		sum = 0;
		for (k=0; k<32; k++)
			sum += bandPtr[k] * (*filter)[i][k];
		bufOffsetPtr[i] = sum;
	}

	/*  S(i,j) = D(j+32i) * U(j+32i+((i+1)>>1)*64)  */
	/*  samples(i,j) = MWindow(j+32i) * bufPtr(j+32i+((i+1)>>1)*64)  */
	for (j=0; j<32; j++)
	{
        sum = 0;
		for (i=0; i<16; i++)
		{
			k = j + (i<<5);
            sum += window[k] * (*buf) [channel] [( (k + ( ((i+1)>>1) <<6) ) +
                                                  bufOffset) & 0x3ff];
        }

/*   {long foo = (sum > 0) ? sum * SCALE + 0.5 : sum * SCALE - 0.5; */
/*   {long foo = sum * SCALE;  */
     
#ifdef	SB_OUTPUT
	 sum = bandPtr[j];	/* 960814 FdB for subband sample generation */
#endif

	 foo = floor(sum * SCALE + 0.5);
	 if (foo >= (long) SCALE)      {samples[j] = SCALE-1; ++clip;}
	 else if (foo < (long) -SCALE) {samples[j] =-SCALE;  ++clip;}
	 else                           samples[j] = foo;
 
    }
    return(clip);
}

int SubBandSynthesis_ml (double *bandPtr,
		         int channel,
		         long *samples)
{
    long foo;
    register int i,j,k;
    register double *bufOffsetPtr, sum;
    static int init = 1;
    typedef double NN[64][32];
    static NN *filter;
    typedef double BB[7][2*HAN_SIZE]; 
    static BB *buf;
    static int bufOffset = 64;
    static double *window;
    int clip = 0;               /* count & return how many samples clipped */

    if (init) {
		buf = (BB *) mem_alloc(sizeof(BB),"BB");
		filter = (NN *) mem_alloc(sizeof(NN), "NN");
		create_syn_filter(*filter);
		window = (double *) mem_alloc(sizeof(double) * HAN_SIZE, "WIN");
		read_syn_window(window);
		bufOffset = 64;
		init = 0;
	}
	if (channel == 0) bufOffset = (bufOffset - 64) & 0x3ff;
	bufOffsetPtr = &((*buf)[channel][bufOffset]);

	for (i=0; i<64; i++)
	{
		sum = 0;
		for (k=0; k<32; k++)
			sum += bandPtr[k] * (*filter)[i][k];
		bufOffsetPtr[i] = sum;
	}

	/*  S(i,j) = D(j+32i) * U(j+32i+((i+1)>>1)*64)  */
	/*  samples(i,j) = MWindow(j+32i) * bufPtr(j+32i+((i+1)>>1)*64)  */
	for (j=0; j<32; j++)
	{
        sum = 0;
		for (i=0; i<16; i++)
		{
			k = j + (i<<5);
            sum += window[k] * (*buf) [channel] [( (k + ( ((i+1)>>1) <<6) ) +
                                                  bufOffset) & 0x3ff];
        }

/*   {long foo = (sum > 0) ? sum * SCALE + 0.5 : sum * SCALE - 0.5; */
/*   {long foo = sum * SCALE;  */
     
#ifdef	SB_OUTPUT
	 sum = bandPtr[j];	/* 960814 FdB for subband sample generation */
#endif

	 foo = floor(sum * SCALE + 0.5);
	 if (foo >= (long) SCALE)      {samples[j] = SCALE-1; ++clip;}
	 else if (foo < (long) -SCALE) {samples[j] =-SCALE;  ++clip;}
	 else                           samples[j] = foo;
 
    }
    return (clip);
}


void out_fifo (long pcm_sample[7][3][SBLIMIT],
	       int num,
	       frame_params *fr_ps,
	       int done,
	       FILE *outFile,
	       unsigned long *psampFrames,
	       int ch)
{
   int i,j,l;
static short int outsamp[1600];
static long k = 0;

    if (!done)
	for (i = 0; i < num; i++)
	    for (j = 0; j < SBLIMIT; j++) 
	    {
		(*psampFrames)++;
		for (l = 0; l < ch; l++)
		{
		    if (!(k % 1600) && k) 
		    {
			fwrite (outsamp, 2, 1600, outFile);
			k = 0;
		    }
		    outsamp[k++] = pcm_sample[l][i][j];
		}
	    }
    else if (k > 0)
    {
	fwrite (outsamp, 2, (int) k, outFile);
	k = 0;
    }
}

void out_fifo_ml(long pcm_sample[7][3][SBLIMIT],
	         int num,
	         frame_params *fr_ps,
	         int done,
	         FILE *outFile,
	         unsigned long *psampFrames)
{
   int i,j,l;
   int n_ml_ch = fr_ps->header->no_of_multi_lingual_ch;
   static short int outsamp[1600];
   static long k = 0;

   if (!done)
      for (i=0; i<num; i++)
	 for (j=0; j<SBLIMIT; j++)
	 {
	    (*psampFrames)++;
	    for (l=0; l<n_ml_ch; l++)
	    {
	       if (!(k%1600) && k)
	       {
		  fwrite (outsamp, 2, 1600, outFile);
		  k = 0;
	       }
	       outsamp[k++] = pcm_sample[l][i][j];
	    }
	 }
   else if (k > 0)
   {
      fwrite (outsamp, 2, (int) k, outFile);
      k = 0;
   }
}

/*************************************************/
/* JMZ 10/03/1995 Multilingual */

/* JMZ 10/03/1995 Multilingual */
/*************************************************/

void  buffer_CRC(Bit_stream  *bs,
		 unsigned int  *old_crc)
{
   *old_crc = getbits (bs, 16);
}

void  recover_CRC_error(long pcm_sample[7][3][SBLIMIT],
			int error_count,
			frame_params *fr_ps,
			FILE *outFile,
			unsigned long *psampFrames,
			int ch)
{
   int  stereo = fr_ps->stereo;
   int  num, done, i;
   int  samplesPerFrame, samplesPerSlot;
   layer  *hdr = fr_ps->header;
   long  offset;
   short  *temp;

   num = 3;
   if (hdr->lay == 1) num = 1;

   samplesPerSlot = SBLIMIT * num * stereo;
   samplesPerFrame = samplesPerSlot * 32;

   if (error_count == 1) {   /* replicate previous error_free frame */
      done = 1;
	  /* flush out fifo */
	  out_fifo(pcm_sample, num, fr_ps, done, outFile, psampFrames, ch);
	  /* go back to the beginning of the previous frame */
	  offset = sizeof(short int) * samplesPerFrame;
      fseek(outFile, -offset, SEEK_CUR);
      done = 0;
      for (i = 0; i < 12; i++) {
		 fread(pcm_sample, 2, samplesPerSlot, outFile);
		 out_fifo(pcm_sample, num, fr_ps, done, outFile, psampFrames, ch);
      }
   }
   else{   /* mute the frame */
      temp = (short*) pcm_sample;
      done = 0;
      for (i = 0; i < 2*3*SBLIMIT; i++)
         *temp++ = MUTE;   /* MUTE value is in decoder.h */
      for (i = 0; i < 12; i++)
         out_fifo(pcm_sample, num, fr_ps, done, outFile, psampFrames, ch);
   }
}


/*******************************************************/
/* prediction corrected, Heiko Purnhagen 08-nov-94 */
/*******************************************************/

/* this is only a local function */
static double predict (double pred_buf[2][8][36+PREDDEL], 
		frame_params *fr_ps,
		int sb, 
		int jj, 
		int j,
		int ch,
		int idx) 
{
  int i;
  double t;

  t = 0;
  for (i=0; i<fr_ps->header->mc_predsi[sb][idx]; i++)
    t += pred_buf[ch][sb][PREDDEL+3*jj+j-i-fr_ps->header->mc_delay_comp[sb][idx]]
      * (fr_ps->header->mc_pred_coeff[sb][idx][i] - 127) * STEP;
  return t;
}


/**************************************************************/
/* JMZ 09/03/1995 Multilingual modifications */

void dematricing_mc (double pcm_sample[7][SBLIMIT][3][12], 
		     frame_params *fr_ps,
		     double pred_buf[2][8][36+PREDDEL])
{
  int i, j, jj, k, tc_alloc, l, sbgr = 0 ;
  layer *info = fr_ps->header;
  double tmp_sample, tmp_sample1, surround_sample;
  
  for (jj=0; jj<12; jj++)
  for( j = 0; j < 3; ++j)
   {
	for(k = 0; k < SBLIMIT; k ++)
	{
	    if(fr_ps->header->tc_sbgr_select == 1)
		tc_alloc = fr_ps->header->tc_allocation; /* one tc_alloc is valid for all sbgr */
	    else
	    {
		if(k == 0) sbgr = 0;
		else
			for(l = 1; l < 12; l++)
			{
				if((sb_groups[l-1] < k) && (k <= sb_groups[l]))
				{
					sbgr = l;  /* search the valid subband group */
					break;
				}
			}
			tc_alloc = fr_ps->header->tc_alloc[sbgr]; /* no prediction, but different tc_alloc's
									per subband*/
	    }  /* else tc_sbgr_select == 0 */

	    if (fr_ps->header->mc_prediction_on && k < 8 && fr_ps->header->mc_prediction[k])
	    {
		/* prediction on in sbgr */
   		if ((fr_ps->header->surround == 2) && (fr_ps->header->center != 0))
		{    
		    /* FdB channel mode 3/2 */
		    switch (fr_ps->header->dyn_cross_mode[k])   /* if prediction on in sb */
		    {
			case 0: pcm_sample[2][k][j][jj] += predict(pred_buf,fr_ps,k,jj,j,0,0)
			                                 + predict(pred_buf,fr_ps,k,jj,j,1,1);
			    	pcm_sample[3][k][j][jj] += predict(pred_buf,fr_ps,k,jj,j,0,2)
					 	         + predict(pred_buf,fr_ps,k,jj,j,1,3);
			    	pcm_sample[4][k][j][jj] += predict(pred_buf,fr_ps,k,jj,j,0,4)
						         + predict(pred_buf,fr_ps,k,jj,j,1,5);
				break;
			
			case 1: pcm_sample[2][k][j][jj] += predict(pred_buf,fr_ps,k,jj,j,0,0)
			                                 + predict(pred_buf,fr_ps,k,jj,j,1,1);
			    	pcm_sample[3][k][j][jj] += predict(pred_buf,fr_ps,k,jj,j,0,2)
					 	         + predict(pred_buf,fr_ps,k,jj,j,1,3);
			    	break;
			
			case 2: pcm_sample[2][k][j][jj] += predict(pred_buf,fr_ps,k,jj,j,0,0)
			                                 + predict(pred_buf,fr_ps,k,jj,j,1,1);
			    	pcm_sample[4][k][j][jj] += predict(pred_buf,fr_ps,k,jj,j,0,2)
						         + predict(pred_buf,fr_ps,k,jj,j,1,3);
				break;
			
			case 3: pcm_sample[3][k][j][jj] += predict(pred_buf,fr_ps,k,jj,j,0,0)
					 	         + predict(pred_buf,fr_ps,k,jj,j,1,1);
			    	pcm_sample[4][k][j][jj] += predict(pred_buf,fr_ps,k,jj,j,0,2)
						         + predict(pred_buf,fr_ps,k,jj,j,1,3);
				break;
			
			case 4: pcm_sample[2][k][j][jj] += predict(pred_buf,fr_ps,k,jj,j,0,0)
			                                 + predict(pred_buf,fr_ps,k,jj,j,1,1);
			    	break;
			
			case 5: pcm_sample[3][k][j][jj] += predict(pred_buf,fr_ps,k,jj,j,0,0)
					 	         + predict(pred_buf,fr_ps,k,jj,j,1,1);
			    	break;
			
			case 6: pcm_sample[4][k][j][jj] += predict(pred_buf,fr_ps,k,jj,j,0,0)
						         + predict(pred_buf,fr_ps,k,jj,j,1,1);
				break;

			case 8: pcm_sample[2][k][j][jj] += predict(pred_buf,fr_ps,k,jj,j,0,0)
			                                 + predict(pred_buf,fr_ps,k,jj,j,1,1);
			    	break;

			case 9: pcm_sample[4][k][j][jj] += predict(pred_buf,fr_ps,k,jj,j,0,0)
						         + predict(pred_buf,fr_ps,k,jj,j,1,1);
				break;

		       case 10: pcm_sample[3][k][j][jj] += predict(pred_buf,fr_ps,k,jj,j,0,0)
						         + predict(pred_buf,fr_ps,k,jj,j,1,1);
			    	break;

		     }
		} /* 3/2 mode */
		else if ((fr_ps->header->surround == 1 && fr_ps->header->center != 0) ||
			 (fr_ps->header->surround == 2 && fr_ps->header->center == 0))
		{
		    /* FdB channel modes 3/1 and 2/2 */
		    switch(fr_ps->header->dyn_cross_mode[k])   /* if prediction on in sb */
		    {
			case 0: pcm_sample[2][k][j][jj] += predict(pred_buf,fr_ps,k,jj,j,0,0)
							 + predict(pred_buf,fr_ps,k,jj,j,1,1);
				pcm_sample[3][k][j][jj] += predict(pred_buf,fr_ps,k,jj,j,0,2)
							 + predict(pred_buf,fr_ps,k,jj,j,1,3);
				break;
			
			case 1: pcm_sample[2][k][j][jj] += predict(pred_buf,fr_ps,k,jj,j,0,0)
							 + predict(pred_buf,fr_ps,k,jj,j,1,1);
				break;
			
			case 2: pcm_sample[3][k][j][jj] += predict(pred_buf,fr_ps,k,jj,j,0,0)
							 + predict(pred_buf,fr_ps,k,jj,j,1,1);
				break;

		    }
		} /* 3/1 and 2/2 modes */
		else if (((fr_ps->header->surround == 0 || fr_ps->header->surround == 3) && fr_ps->header->center != 0) ||
			 (fr_ps->header->surround == 1 && fr_ps->header->center == 0))
		{
		    /* FdB channel modes 3/0 (+ 2/0) and 2/1 */
		    switch(fr_ps->header->dyn_cross_mode[k])   /* if prediction on in sb */
		    {
			case 0: pcm_sample[2][k][j][jj] += predict(pred_buf,fr_ps,k,jj,j,0,0)
							 + predict(pred_buf,fr_ps,k,jj,j,1,1);
				break;
			
		    }
		}
	    }    

	    if (fr_ps->header->dematrix_procedure != 3)
   		if ((fr_ps->header->surround == 2) && (fr_ps->header->center != 0))
		{    
		    /* FdB channel mode 3/2 */
		    switch(tc_alloc)
		    {
		     case 0:
			    if (fr_ps->header->dematrix_procedure == 2) {
				surround_sample = (pcm_sample[3][k][j][jj] + pcm_sample[4][k][j][jj]) / 2.0;
				pcm_sample[0][k][j][jj] = pcm_sample[0][k][j][jj] - pcm_sample[2][k][j][jj] + surround_sample;
				pcm_sample[1][k][j][jj] = pcm_sample[1][k][j][jj] - pcm_sample[2][k][j][jj] - surround_sample;
			    }
			    else {
				pcm_sample[0][k][j][jj] = pcm_sample[0][k][j][jj] - pcm_sample[2][k][j][jj] - pcm_sample[3][k][j][jj];
				pcm_sample[1][k][j][jj] = pcm_sample[1][k][j][jj] - pcm_sample[2][k][j][jj] - pcm_sample[4][k][j][jj];
			    }
			    break;
    
		     case 1:
			    if (fr_ps->header->dematrix_procedure == 2) {
				    surround_sample = (pcm_sample[3][k][j][jj] + pcm_sample[4][k][j][jj]) / 2.0;
				    tmp_sample = pcm_sample[2][k][j][jj]; /* L */
				    pcm_sample[2][k][j][jj] = pcm_sample[0][k][j][jj] - pcm_sample[2][k][j][jj] + surround_sample;
				    pcm_sample[1][k][j][jj] = pcm_sample[1][k][j][jj] - pcm_sample[2][k][j][jj] - surround_sample;
				    pcm_sample[0][k][j][jj] = tmp_sample;
			    }
			    else {
				    tmp_sample = pcm_sample[2][k][j][jj]; /* L */
				    pcm_sample[2][k][j][jj] = pcm_sample[0][k][j][jj] - pcm_sample[2][k][j][jj] - pcm_sample[3][k][j][jj];
				    pcm_sample[1][k][j][jj] = pcm_sample[1][k][j][jj] - pcm_sample[2][k][j][jj] - pcm_sample[4][k][j][jj];
				    pcm_sample[0][k][j][jj] = tmp_sample;
			    }
			    break;
    
		     case 2:
			    if (fr_ps->header->dematrix_procedure == 2) {
				    surround_sample = (pcm_sample[3][k][j][jj] + pcm_sample[4][k][j][jj]) / 2.0;
				    tmp_sample = pcm_sample[2][k][j][jj]; /* R */
				    pcm_sample[2][k][j][jj] = pcm_sample[1][k][j][jj] - pcm_sample[2][k][j][jj] - surround_sample;
				    pcm_sample[0][k][j][jj] = pcm_sample[0][k][j][jj] - pcm_sample[2][k][j][jj] + surround_sample;
				    pcm_sample[1][k][j][jj] = tmp_sample;
			    }
			    else {
				    tmp_sample = pcm_sample[2][k][j][jj]; /* R */
				    pcm_sample[2][k][j][jj] = pcm_sample[1][k][j][jj] - pcm_sample[2][k][j][jj] - pcm_sample[4][k][j][jj];
				    pcm_sample[0][k][j][jj] = pcm_sample[0][k][j][jj] - pcm_sample[2][k][j][jj] - pcm_sample[3][k][j][jj];
				    pcm_sample[1][k][j][jj] = tmp_sample;
			    }
			    break;
    
		     case 3:
			    if (fr_ps->header->dematrix_procedure == 2) {
				    tmp_sample = pcm_sample[3][k][j][jj];  /* L in T3 */
				    pcm_sample[3][k][j][jj] = -2.0 * (pcm_sample[0][k][j][jj] - pcm_sample[2][k][j][jj] -  pcm_sample[3][k][j][jj]) - pcm_sample[4][k][j][jj];
				    pcm_sample[1][k][j][jj] = pcm_sample[0][k][j][jj] + pcm_sample[1][k][j][jj] - 2.0 * pcm_sample[2][k][j][jj] - tmp_sample;
				    pcm_sample[0][k][j][jj] = tmp_sample;
			    }
			    else {
				    tmp_sample = pcm_sample[3][k][j][jj];  /* L in T3 */
				    pcm_sample[3][k][j][jj] = pcm_sample[0][k][j][jj] - pcm_sample[3][k][j][jj] - pcm_sample[2][k][j][jj];
				    pcm_sample[1][k][j][jj] = pcm_sample[1][k][j][jj] - pcm_sample[2][k][j][jj] - pcm_sample[4][k][j][jj];
				    pcm_sample[0][k][j][jj] = tmp_sample;
			    }
			    break;
    
		     case 4:
			    if (fr_ps->header->dematrix_procedure == 2) {
				    tmp_sample = pcm_sample[4][k][j][jj];  /* R in T4 */
				    pcm_sample[0][k][j][jj] = pcm_sample[0][k][j][jj] + pcm_sample[1][k][j][jj] - 2.0 * pcm_sample[2][k][j][jj] - pcm_sample[4][k][j][jj];
				    pcm_sample[4][k][j][jj] = 2.0 * pcm_sample[1][k][j][jj] - 2.0 * (pcm_sample[2][k][j][jj] + pcm_sample[4][k][j][jj]) - pcm_sample[3][k][j][jj];
				    pcm_sample[1][k][j][jj] = tmp_sample;
			    }
			    else {
				    tmp_sample = pcm_sample[4][k][j][jj];  /* R in T4 */
				    pcm_sample[0][k][j][jj] = pcm_sample[0][k][j][jj] - pcm_sample[2][k][j][jj] - pcm_sample[3][k][j][jj];
				    pcm_sample[4][k][j][jj] = pcm_sample[1][k][j][jj] - pcm_sample[4][k][j][jj] - pcm_sample[2][k][j][jj];
				    pcm_sample[1][k][j][jj] = tmp_sample;
			    }
			    break;
    
		     case 5:
			    if (fr_ps->header->dematrix_procedure == 2) {
				    tmp_sample = pcm_sample[3][k][j][jj];  /* L in T3 */
				    pcm_sample[3][k][j][jj] = 0.5 * (pcm_sample[1][k][j][jj] - pcm_sample[0][k][j][jj] + pcm_sample[3][k][j][jj] - pcm_sample[4][k][j][jj]);
				    pcm_sample[0][k][j][jj] = tmp_sample;
				    pcm_sample[1][k][j][jj] = pcm_sample[4][k][j][jj]; /* R in T4 */
				    pcm_sample[4][k][j][jj] = pcm_sample[3][k][j][jj]; /* RS = LS */
			    }
			    else {
				    tmp_sample = pcm_sample[3][k][j][jj];
				    pcm_sample[3][k][j][jj] = pcm_sample[0][k][j][jj] - pcm_sample[3][k][j][jj] - pcm_sample[2][k][j][jj];
				    pcm_sample[0][k][j][jj] = tmp_sample;
				    tmp_sample = pcm_sample[4][k][j][jj];
				    pcm_sample[4][k][j][jj] = pcm_sample[1][k][j][jj] - pcm_sample[4][k][j][jj] - pcm_sample[2][k][j][jj];
				    pcm_sample[1][k][j][jj] = tmp_sample;
			    }
			    break;
    
		     case 6:
			    if (fr_ps->header->dematrix_procedure == 2) {
				    tmp_sample = pcm_sample[2][k][j][jj];  /* R in T2 */
				    tmp_sample1 = pcm_sample[3][k][j][jj];  /* L in T3 */
				    pcm_sample[3][k][j][jj] = pcm_sample[1][k][j][jj] - pcm_sample[0][k][j][jj] - pcm_sample[2][k][j][jj] + pcm_sample[3][k][j][jj] - pcm_sample[4][k][j][jj];
				    pcm_sample[2][k][j][jj] = 0.5 * (pcm_sample[0][k][j][jj] + pcm_sample[1][k][j][jj] - pcm_sample[2][k][j][jj] - tmp_sample1);
				    pcm_sample[1][k][j][jj] = tmp_sample;
				    pcm_sample[0][k][j][jj] = tmp_sample1;
			    }
			    else {
				    tmp_sample = pcm_sample[2][k][j][jj];  /* R in T2 */
				    pcm_sample[2][k][j][jj] = pcm_sample[1][k][j][jj] - pcm_sample[2][k][j][jj] - pcm_sample[4][k][j][jj];
				    pcm_sample[1][k][j][jj] = tmp_sample;
				    tmp_sample = pcm_sample[3][k][j][jj];  /* L in T3 */
				    pcm_sample[3][k][j][jj] = pcm_sample[0][k][j][jj] - pcm_sample[3][k][j][jj] - pcm_sample[2][k][j][jj];
				    pcm_sample[0][k][j][jj] = tmp_sample;
			    }
			    break;
    
		     case 7:
			    if (fr_ps->header->dematrix_procedure == 2) {
				    tmp_sample = pcm_sample[2][k][j][jj];  /* L in T2 */
				    tmp_sample1 = pcm_sample[4][k][j][jj];  /* R in T4 */
				    pcm_sample[4][k][j][jj] = pcm_sample[1][k][j][jj] - pcm_sample[0][k][j][jj] + pcm_sample[2][k][j][jj] - pcm_sample[3][k][j][jj] - pcm_sample[4][k][j][jj];
				    pcm_sample[2][k][j][jj] = 0.5 * (pcm_sample[0][k][j][jj] + pcm_sample[1][k][j][jj] - pcm_sample[2][k][j][jj] - tmp_sample1);
				    pcm_sample[0][k][j][jj] = tmp_sample;
				    pcm_sample[1][k][j][jj] = tmp_sample1;
			    }
			    else {
				    tmp_sample = pcm_sample[2][k][j][jj];
				    pcm_sample[2][k][j][jj] = pcm_sample[0][k][j][jj] - pcm_sample[2][k][j][jj] - pcm_sample[3][k][j][jj];
				    pcm_sample[0][k][j][jj] = tmp_sample;
				    tmp_sample = pcm_sample[4][k][j][jj];
				    pcm_sample[4][k][j][jj] = pcm_sample[1][k][j][jj] - pcm_sample[4][k][j][jj] - pcm_sample[2][k][j][jj];
				    pcm_sample[1][k][j][jj] = tmp_sample;
			    }
			    break;
    
		    } /* switch end loop*/
		}
		else if (fr_ps->header->surround == 1 && fr_ps->header->center != 0)  
		{    
		    /* FdB channel mode 3/1 */
		    switch(tc_alloc)
		    {
			 case 0: 
				if (fr_ps->header->dematrix_procedure == 2)
				    pcm_sample[0][k][j][jj] = pcm_sample[0][k][j][jj] - pcm_sample[2][k][j][jj] + pcm_sample[3][k][j][jj];
				else
				    pcm_sample[0][k][j][jj] = pcm_sample[0][k][j][jj] - pcm_sample[2][k][j][jj] - pcm_sample[3][k][j][jj];
				pcm_sample[1][k][j][jj] = pcm_sample[1][k][j][jj] - pcm_sample[2][k][j][jj] - pcm_sample[3][k][j][jj];
				break;

			 case 1:
				tmp_sample = pcm_sample[2][k][j][jj]; /* L */
				if (fr_ps->header->dematrix_procedure == 2)
				    pcm_sample[2][k][j][jj] = pcm_sample[0][k][j][jj] - pcm_sample[2][k][j][jj] + pcm_sample[3][k][j][jj];
				else
				    pcm_sample[2][k][j][jj] = pcm_sample[0][k][j][jj] - pcm_sample[2][k][j][jj] - pcm_sample[3][k][j][jj];
				pcm_sample[1][k][j][jj] = pcm_sample[1][k][j][jj] - pcm_sample[2][k][j][jj] - pcm_sample[3][k][j][jj];
				pcm_sample[0][k][j][jj] = tmp_sample;
				break;

			 case 2: 
				tmp_sample = pcm_sample[2][k][j][jj]; /* R */
				pcm_sample[2][k][j][jj] = pcm_sample[1][k][j][jj] - pcm_sample[2][k][j][jj] - pcm_sample[3][k][j][jj];
				if (fr_ps->header->dematrix_procedure == 2)
				    pcm_sample[0][k][j][jj] = pcm_sample[0][k][j][jj] - pcm_sample[2][k][j][jj] + pcm_sample[3][k][j][jj];
				else
				    pcm_sample[0][k][j][jj] = pcm_sample[0][k][j][jj] - pcm_sample[2][k][j][jj] - pcm_sample[3][k][j][jj];
				pcm_sample[1][k][j][jj] = tmp_sample;
				break;

			 case 3: 
				tmp_sample = pcm_sample[3][k][j][jj];  /* L in T3 */
				if (fr_ps->header->dematrix_procedure == 2)
				    pcm_sample[3][k][j][jj] = -pcm_sample[0][k][j][jj] + pcm_sample[2][k][j][jj] + pcm_sample[3][k][j][jj];
				else
				    pcm_sample[3][k][j][jj] = pcm_sample[0][k][j][jj] - pcm_sample[2][k][j][jj] - pcm_sample[3][k][j][jj];
				pcm_sample[1][k][j][jj] = pcm_sample[1][k][j][jj] - pcm_sample[2][k][j][jj] - pcm_sample[3][k][j][jj];
				pcm_sample[0][k][j][jj] = tmp_sample;
				break;

			 case 4:
				tmp_sample = pcm_sample[3][k][j][jj];  /* R in T3 */
				pcm_sample[3][k][j][jj] = pcm_sample[1][k][j][jj] - pcm_sample[2][k][j][jj] - pcm_sample[3][k][j][jj];
				if (fr_ps->header->dematrix_procedure == 2)
				    pcm_sample[0][k][j][jj] = pcm_sample[0][k][j][jj] - pcm_sample[2][k][j][jj] + pcm_sample[3][k][j][jj];
				else
				    pcm_sample[0][k][j][jj] = pcm_sample[0][k][j][jj] - pcm_sample[2][k][j][jj] - pcm_sample[3][k][j][jj];
				pcm_sample[1][k][j][jj] = tmp_sample;
				break;

			 case 5:
				tmp_sample = pcm_sample[2][k][j][jj];  /* L in T2 */
				tmp_sample1 = pcm_sample[3][k][j][jj];  /* R in T3 */
				pcm_sample[2][k][j][jj] = 0.5 * (pcm_sample[0][k][j][jj] + pcm_sample[1][k][j][jj] - tmp_sample - tmp_sample1);
				pcm_sample[3][k][j][jj] = 0.5 * (pcm_sample[1][k][j][jj] - pcm_sample[0][k][j][jj] + tmp_sample - tmp_sample1);
				pcm_sample[0][k][j][jj] = tmp_sample;
				pcm_sample[1][k][j][jj] = tmp_sample1;
				break;
		    }   /* switch end */
		}
		else if (fr_ps->header->surround == 1 || fr_ps->header->center != 0)
		{
		    /* FdB channel modes 3/0 (+ 2/0) and 2/1 */
		    switch(tc_alloc)
		    {
			 case 0: 
				pcm_sample[0][k][j][jj] = pcm_sample[0][k][j][jj] - pcm_sample[2][k][j][jj]; 
				pcm_sample[1][k][j][jj] = pcm_sample[1][k][j][jj] - pcm_sample[2][k][j][jj];
				break;

			 case 1:
				tmp_sample = pcm_sample[2][k][j][jj]; /* L */
				pcm_sample[2][k][j][jj] = pcm_sample[0][k][j][jj] - pcm_sample[2][k][j][jj];
				pcm_sample[1][k][j][jj] = pcm_sample[1][k][j][jj] - pcm_sample[2][k][j][jj];
				pcm_sample[0][k][j][jj] = tmp_sample;
				break;

			 case 2: 
				tmp_sample = pcm_sample[2][k][j][jj]; /* R */
				pcm_sample[2][k][j][jj] = pcm_sample[1][k][j][jj] - pcm_sample[2][k][j][jj];
				pcm_sample[0][k][j][jj] = pcm_sample[0][k][j][jj] - pcm_sample[2][k][j][jj];
				pcm_sample[1][k][j][jj] = tmp_sample;
				break;
		    }   /* switch end */
		}
		else if (fr_ps->header->surround == 2)  
		{
		    /* FdB channel mode 2/2 */
		    switch(tc_alloc)
		    {
			 case 0: 
				pcm_sample[0][k][j][jj] = pcm_sample[0][k][j][jj] - pcm_sample[2][k][j][jj];
			    	pcm_sample[1][k][j][jj] = pcm_sample[1][k][j][jj] - pcm_sample[3][k][j][jj];
				break;

			 case 1:
				tmp_sample = pcm_sample[2][k][j][jj]; /* L */
				pcm_sample[1][k][j][jj] = pcm_sample[1][k][j][jj] - pcm_sample[3][k][j][jj];
				pcm_sample[2][k][j][jj] = pcm_sample[0][k][j][jj] - pcm_sample[2][k][j][jj];
				pcm_sample[0][k][j][jj] = tmp_sample;
				break;

			 case 2: 
				tmp_sample = pcm_sample[3][k][j][jj]; /* R */
				pcm_sample[0][k][j][jj] = pcm_sample[0][k][j][jj] - pcm_sample[2][k][j][jj];
				pcm_sample[3][k][j][jj] = pcm_sample[1][k][j][jj] - pcm_sample[3][k][j][jj];
				pcm_sample[1][k][j][jj] = tmp_sample;
				break;

			 case 3: 
				tmp_sample = pcm_sample[2][k][j][jj]; /* L */
				pcm_sample[2][k][j][jj] = pcm_sample[0][k][j][jj] - pcm_sample[2][k][j][jj];
				pcm_sample[0][k][j][jj] = tmp_sample;
				tmp_sample = pcm_sample[3][k][j][jj]; /* R */
				pcm_sample[3][k][j][jj] = pcm_sample[1][k][j][jj] - pcm_sample[3][k][j][jj];
				pcm_sample[1][k][j][jj] = tmp_sample;
				break;
		    }   /* switch end */
		}
	}     /* for k < sblimit loop */
      }     /* for j < 3 loop */
}

void denormalizing_mc (double pcm_sample[7][SBLIMIT][3][12], frame_params *fr_ps, int channels)
{
  double matr1;   /* normalizing factor */
  double matr2;   /* matricing factor   */
  double matr3;   /* matricing factor   */
  int i, j, jj, k, l;
  layer *info = fr_ps->header;
  int  stereo = fr_ps->stereo;
  
  switch (info->dematrix_procedure)
  {
    /* factors according to International Standard */
    case 0:
    case 2: matr1 = 1 + sqrt(2.0);	/* factor for L and R   */
	    matr2 = sqrt(2.0) * matr1;	/* factor for C */
	    matr3 = sqrt(2.0) * matr1;	/* factor for Ls, Rs */
	    break;
    case 1: matr1 = (1.5 + 0.5*sqrt(2.0));/* factor for L, R      */
	    matr2 = sqrt(2.0) * matr1;	/* factor for C         */
	    matr3 = 2 * matr1;		/* factor for Ls, Rs  */
	    break;
    case 3: matr1 = 1.0;
	    matr2 = 1.0;
	    matr3 = 1.0;
	    break;
  }

  /* denormalized signals */
  if (fr_ps->header->dematrix_procedure != 3 ) 		/* dematrixing */
    for (jj=0; jj<12; jj++)
      for( j = 0; j < 3; ++j)
	for (k = 0; k < SBLIMIT; k ++)
	{   /* Lo / Ro */
	    for (i = 0; i < stereo; i++)
		pcm_sample[i][k][j][jj] = pcm_sample[i][k][j][jj] * matr1;

	    if (fr_ps->header->dematrix_procedure != 1)  /* matrix 0 and 2, since C, Ls, Rs, and S all use the same value */
	    {
		/* second stereo channels */
		if (fr_ps->header->surround == 3)
		{
		    if (fr_ps->header->center != 0)
			pcm_sample[2][k][j][jj] = pcm_sample[2][k][j][jj] * matr2;
		}
		else   
		{
		    for (l = 2; l < channels; l++)
		       pcm_sample[l][k][j][jj] = pcm_sample[l][k][j][jj] * matr2;
		}
	    }
	    else   /* matrix 1 */
	    {
		if (fr_ps->header->surround == 3)
		{
		    if (fr_ps->header->center != 0)
			pcm_sample[2][k][j][jj] = pcm_sample[2][k][j][jj] * matr2;
		}
		else if (fr_ps->mc_channel == 3)   /* R.S. matr2 = C */
		{
		    pcm_sample[2][k][j][jj] = pcm_sample[2][k][j][jj] * matr2;
		    pcm_sample[3][k][j][jj] = pcm_sample[3][k][j][jj] * matr3;
		    pcm_sample[4][k][j][jj] = pcm_sample[4][k][j][jj] * matr3;
		}
		else if (fr_ps->mc_channel == 2)
		{
		    if (fr_ps->header->surround == 2)	/* 2/2 */
		    {
			pcm_sample[2][k][j][jj] = pcm_sample[2][k][j][jj] * matr3;  
			pcm_sample[3][k][j][jj] = pcm_sample[3][k][j][jj] * matr3;
		    }
		    else				/* 3/1 */
		    {
			pcm_sample[2][k][j][jj] = pcm_sample[2][k][j][jj] * matr2;
			pcm_sample[3][k][j][jj] = pcm_sample[3][k][j][jj] * matr3;
		    }
		}
		else	/* mc_channel == 1 */
		{
		    if (fr_ps->header->center == 0)	/* 2/1 */
			pcm_sample[2][k][j][jj] = pcm_sample[2][k][j][jj] * matr3; 	
		    else				/* 3/0 */
			pcm_sample[2][k][j][jj] = pcm_sample[2][k][j][jj] * matr2;
		}
	    }
	}
}

#ifdef Augmentation_7ch
void dematricing_aug (double pcm_sample[7][SBLIMIT][3][12], frame_params *fr_ps)
{
  double c0, c1;   /* denormalization factors */
  int i, j, jj, k, tc_aug_alloc, l, sbgr = 0 ;
  layer *info = fr_ps->header;
  double tmp_sample1, tmp_sample2;
  
  c0 = 1.0 / 3.0;
  if (info->dematrix_procedure != 3)
    c0 *= sqrt (0.5);
  c1 = 1.0 / c0;

  for (jj=0; jj<12; jj++)
    for( j = 0; j < 3; ++j)
    {
      for(k = 0; k < SBLIMIT; k ++)
      {
	if (k == 0) sbgr = 0;
	else
	{
	  for(l = 1; l < 12; l++)
	  {
	    if((sb_groups[l-1] < k) && (k <= sb_groups[l]))
	    {
	      sbgr = l;  /* search the valid subband group */
	      break;
	    }
	  }
	}
	tc_aug_alloc = fr_ps->header->tc_aug_alloc[sbgr];

	if (info->aug_mtx_proc == 0)
	  switch (tc_aug_alloc)
	  {
	  case 0:  pcm_sample[0][k][j][jj] -= pcm_sample[5][k][j][jj];
		   pcm_sample[1][k][j][jj] -= pcm_sample[6][k][j][jj];
		   pcm_sample[2][k][j][jj] -= c0 * (pcm_sample[5][k][j][jj] + pcm_sample[6][k][j][jj]);
		   break;
	  case 1:  tmp_sample1 = pcm_sample[0][k][j][jj];
		   pcm_sample[0][k][j][jj] = pcm_sample[5][k][j][jj];
		   pcm_sample[5][k][j][jj] = tmp_sample1 - pcm_sample[5][k][j][jj];
		   pcm_sample[1][k][j][jj] -= pcm_sample[6][k][j][jj];
		   pcm_sample[2][k][j][jj] -= c0 * (pcm_sample[5][k][j][jj] + pcm_sample[6][k][j][jj]);
		   break;
	  case 2:  pcm_sample[0][k][j][jj] -= pcm_sample[5][k][j][jj];
		   tmp_sample2 = pcm_sample[1][k][j][jj];
		   pcm_sample[1][k][j][jj] = pcm_sample[6][k][j][jj];
		   pcm_sample[6][k][j][jj] = tmp_sample2 - pcm_sample[6][k][j][jj];
		   pcm_sample[2][k][j][jj] -= c0 * (pcm_sample[5][k][j][jj] + pcm_sample[6][k][j][jj]);
		   break;
	  case 3:  tmp_sample1 = pcm_sample[0][k][j][jj];
		   pcm_sample[0][k][j][jj] = pcm_sample[5][k][j][jj];
		   pcm_sample[5][k][j][jj] = tmp_sample1 - pcm_sample[5][k][j][jj];
		   tmp_sample2 = pcm_sample[1][k][j][jj];
		   pcm_sample[1][k][j][jj] = pcm_sample[6][k][j][jj];
		   pcm_sample[6][k][j][jj] = tmp_sample2 - pcm_sample[6][k][j][jj];
		   pcm_sample[2][k][j][jj] -= c0 * (pcm_sample[5][k][j][jj] + pcm_sample[6][k][j][jj]);
		   break;
	  case 4:  pcm_sample[0][k][j][jj] -= pcm_sample[5][k][j][jj];
		   tmp_sample1 = pcm_sample[2][k][j][jj];
		   tmp_sample2 = pcm_sample[6][k][j][jj];
		   pcm_sample[2][k][j][jj] = tmp_sample2;
		   pcm_sample[6][k][j][jj] = c1 * (tmp_sample1 - tmp_sample2) - pcm_sample[5][k][j][jj];
		   pcm_sample[1][k][j][jj] -= pcm_sample[6][k][j][jj];
		   break;
	  case 5:  tmp_sample1 = pcm_sample[0][k][j][jj];
		   pcm_sample[0][k][j][jj] = pcm_sample[5][k][j][jj];
		   pcm_sample[5][k][j][jj] = tmp_sample1 - pcm_sample[5][k][j][jj];
		   tmp_sample1 = pcm_sample[2][k][j][jj];
		   tmp_sample2 = pcm_sample[6][k][j][jj];
		   pcm_sample[2][k][j][jj] = tmp_sample2;
		   pcm_sample[6][k][j][jj] = c1 * (tmp_sample1 - tmp_sample2) - pcm_sample[5][k][j][jj];
		   pcm_sample[1][k][j][jj] -= pcm_sample[6][k][j][jj];
		   break;
	  case 6:  pcm_sample[1][k][j][jj] -= pcm_sample[6][k][j][jj];
		   tmp_sample1 = pcm_sample[2][k][j][jj];
		   tmp_sample2 = pcm_sample[5][k][j][jj];
		   pcm_sample[2][k][j][jj] = tmp_sample2;
		   pcm_sample[5][k][j][jj] = c1 * (tmp_sample1 - tmp_sample2) - pcm_sample[6][k][j][jj];
		   pcm_sample[0][k][j][jj] -= pcm_sample[5][k][j][jj];
		   break;
	  case 7:  tmp_sample1 = pcm_sample[1][k][j][jj];
		   pcm_sample[1][k][j][jj] = pcm_sample[6][k][j][jj];
		   pcm_sample[6][k][j][jj] = tmp_sample1 - pcm_sample[6][k][j][jj];
		   tmp_sample1 = pcm_sample[2][k][j][jj];
		   tmp_sample2 = pcm_sample[5][k][j][jj];
		   pcm_sample[2][k][j][jj] = tmp_sample2;
		   pcm_sample[5][k][j][jj] = c1 * (tmp_sample1 - tmp_sample2) - pcm_sample[6][k][j][jj];
		   pcm_sample[0][k][j][jj] -= pcm_sample[5][k][j][jj];
		   break;
	  }
	else if (info->aug_mtx_proc == 1)
	  switch (tc_aug_alloc)
	  {
	  case 0:  pcm_sample[0][k][j][jj] -= pcm_sample[5][k][j][jj];
		   pcm_sample[1][k][j][jj] -= pcm_sample[6][k][j][jj];
		   break;
	  case 1:  tmp_sample1 = pcm_sample[0][k][j][jj];
		   pcm_sample[0][k][j][jj] = pcm_sample[5][k][j][jj];
		   pcm_sample[5][k][j][jj] = tmp_sample1 - pcm_sample[5][k][j][jj];
		   pcm_sample[1][k][j][jj] -= pcm_sample[6][k][j][jj];
		   break;
	  case 2:  pcm_sample[0][k][j][jj] -= pcm_sample[5][k][j][jj];
		   tmp_sample2 = pcm_sample[1][k][j][jj];
		   pcm_sample[1][k][j][jj] = pcm_sample[6][k][j][jj];
		   pcm_sample[6][k][j][jj] = tmp_sample2 - pcm_sample[6][k][j][jj];
		   break;
	  case 3:  tmp_sample1 = pcm_sample[0][k][j][jj];
		   pcm_sample[0][k][j][jj] = pcm_sample[5][k][j][jj];
		   pcm_sample[5][k][j][jj] = tmp_sample1 - pcm_sample[5][k][j][jj];
		   tmp_sample2 = pcm_sample[1][k][j][jj];
		   pcm_sample[1][k][j][jj] = pcm_sample[6][k][j][jj];
		   pcm_sample[6][k][j][jj] = tmp_sample2 - pcm_sample[6][k][j][jj];
		   break;
	  }
      }     /* for k < sblimit loop */
    }     /* for j < 3 loop */
}


void denormalizing_aug (double pcm_sample[7][SBLIMIT][3][12], frame_params *fr_ps)
{
  double c[7], d, denorm;   /* denormalization factors */
  int j, jj, k, l;
  layer *info = fr_ps->header;

  for (l = 0; l < 7; l++)
    c[l] = 1.0;

  switch(info->dematrix_procedure)
  {
    /* factors according to International Standard */
    case 0:
    case 2: c[2] = c[3] = c[4] = sqrt (2.0);	/* unweigh factor for C, Ls and Rs */
	    break;
    case 1: c[2] = sqrt (2.0);			/* unweigh factor for C            */
	    c[3] = c[4] = 2.0;			/* unweigh factor for Ls, Rs       */
	    break;
  }

  if (info->aug_mtx_proc == 0)
    /* factors according to 7-ch augmentation */
    c[5] = c[6] = 4.0 / 3;			/* unweigh factor for LC, RC */

  /* denormalization factor */
  switch (info->dematrix_procedure * 10 + info->aug_mtx_proc)
  {
    case 00:
    case 20: denorm = 1.75 + 1.25 * sqrt (2.0);
	     break;
    case 10: denorm = 2.25 + 0.75 * sqrt (2.0);
	     break;
    case 30: denorm = 1.75;
	     break;
    case 01:
    case 21: denorm = 2.0 + sqrt (2.0);
	     break;
    case 11: denorm = 2.5 + 0.5 * sqrt (2.0);
	     break;
    case 31: denorm = 2.0;
	     break;
    case 03:
    case 23: denorm = 1.0 + sqrt (2.0);
	     break;
    case 13: denorm = 1.5 + 0.5 * sqrt (2.0);
	     break;
    case 33: denorm = 1.0;
	     break;
  }

  for (l = 0; l < 7; l++)
    c[l] *= denorm;

  /* denormalizing */
  if (fr_ps->header->dematrix_procedure != 3 || fr_ps->header->aug_mtx_proc != 3)
    for (jj=0; jj<12; jj++)
      for( j = 0; j < 3; ++j)
	for (k = 0; k < SBLIMIT; k ++)
	  for (l = 0; l < 7; l++)
	    pcm_sample[l][k][j][jj] *= c[l];
}
#endif
