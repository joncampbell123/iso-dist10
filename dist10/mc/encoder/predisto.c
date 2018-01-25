/**********************************************************************
 * ISO MPEG Audio Subgroup Software Simulation Group (1996)
 * ISO 13818-3 MPEG-2 Audio Multichannel Encoder
 *
 * $Id: predisto.c 1.6 1996/02/12 07:13:35 rowlands Exp $
 *
 * $Log: predisto.c $
 * Revision 1.6  1996/02/12 07:13:35  rowlands
 * Release following Munich meeting
 *
 * Revision 1.5  1996/02/12 05:51:17  rowlands
 * Added verbosity switch to control text output.
 * Fixed uninitialized variable d in subband_quantization_pre.
 *
 * Revision 1.3.2.1  1995/11/06  04:19:12  rowlands
 * Received from Uwe Felderhoff (IRT)
 *
 * Revision 1.4  1995/08/14  08:06:01  tenkate
 * ML-LSF added Warner ten Kate 7/8/95 (Philips)
 * change alloc and sblimit into alloc_ml and sblimit_ml where appropriate
 *
 **********************************************************************/

 /*********************************************************************
 *                                                                    *
 *                                                                    *
 *  MPEG/audio Phase 2 coding/decoding multichannel                   *
 *                                                                    *
 *                                                                    *
 *                                                                    *
 *  Version 4.0    Susanne Ritscher, IRT Munich                       *
 *                                                                    *
 *  9/20/93        Implemented Prediction written by                  *
 *                 Heiko Purnhagen,  Uni Hannover in                  *
 *                 new source-file called prediction.c                *
 *                                                                    *
 *                                                                    *
 *                                                                    *
 *  version 4.1    Susanne Ritscher, IRT Munich                       *
 *                                                                    *
 *  9/20/93        channel-switching is only performed at a           *
 *                 certain limit of TC_ALLOC dB, which is included    *
 *                 in encoder.h                                       *
 *                                                                    *
 * 01/16/94        prediction will not work with trans._pat._errsig   *
 *		   Have to do a new strategy with a new bal for       *
 *		   the add. channels                                  *
 *                                                                    *
 * 04/08/94	   started with predistortion                         *
 *                                                                    *
 * 06/28/94        started with predistortion according to the DIS    *
 *		   and the tc_table                                   *
 *                                                                    *
 *                                                                    *
 *  version 5.0    Susanne Ritscher,  IRT Munich                      *
 *                                                                    *
 *  7/12/94        bitstream according to DIS                         *
 *                 extension bitstream is working                     *
 *                 predistortion is working                           *
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
 *********************************************************************/ 

/**********************************************************************
 * 
 *  This program computes the predistortion.
 *  The three additional channels are AFTER THE BITALLOCATION
 *  quantized and dequantized. With these dequantized samples
 *  the frontchannels are matriced again in order to dematrice them in a
 *  more exact way. This should improve the quality of matriced signals.
 * 
 *  In case of prediction the whole process of decoding predicted signals
 *  is worked off????
 * 
 **************************************************************************/
#include "common.h"
#include "encoder.h"


void predistortion(double (*sb_sample)[3][12][32], unsigned int (*scalar)[3][32], 
		    unsigned int (*bit_alloc)[32], unsigned int (*subband)[3][12][32], 
		    frame_params *fr_ps, double (*perm_smr)[32], unsigned int (*scfsi)[32], 
		    int *adb, unsigned int (*scfsi_dyn)[32])
{
    double sbs_sample[7][3][12][SBLIMIT];  /*predistortion*/
    int l, m, i, n, k;
    int sblimit;
    int ch, gr, sb;
    int ad;
    sblimit = fr_ps->sblimit;
    l = 2; m = 7;
    
    pre_quant(sbs_sample, l, m, sb_sample, scalar, bit_alloc, subband, fr_ps);
	    
    matri(sbs_sample, fr_ps, sb_sample, scalar, sblimit, scfsi, scfsi_dyn,
          bit_alloc, subband);

    l = 0; m = 2;
    
    /*II_scale_factor_calc(sb_sample,scalar,sblimit, l, m);*/
    trans_pattern(scalar, scfsi, fr_ps,scfsi_dyn);
      
    bit_all(perm_smr, scfsi, bit_alloc, adb, fr_ps);
}









static double snr[18] = { 0.00,  6.03, 11.80, 15.81, /* 0, 3, 5, 7 */
			 19.03, 23.50, 29.82, 35.99, /* 9,15,31,63 */
			 42.08, 48.13, 54.17, 60.20, /* 127, ...   */
			 66.22, 72.25, 78.27, 84.29, /* 2047, ...  */
			 90.31, 96.33};              /* 16383, ... */


/* Multilingual TEST 17/03/1995 JMZ */
int bit_all(double (*perm_smr)[32], unsigned int (*scfsi)[32], unsigned int (*bit_alloc)[32], int *adb, frame_params *fr_ps)
                              /* minimum masking level */

/* bit_all() recalculates allocation for compatible signals Lo and Ro */
/* which are channels k==0 and k==1. This recalculation is performed  */
/* because of signal modifications by predistortion.                  */
/* WtK 7/8/95. Information from SR.                                   */
         
                    
{
    int n_ml_ch = fr_ps->header->multiling_ch; 	/*JMZ 17/03/95 Multilingual */
    int i, min_ch, min_sb, oth_ch, k, increment, scale, seli, ba, j, l;
    int adb_hlp, adb_hlp1, adb_hlp2;
    int bspl, bscf, bsel, ad, noisy_sbs;
    int bspl_mpg1, bscf_mpg1, bsel_mpg1;
    double mnr[14][SBLIMIT], small;				/*JMZ 17/03/95 Multilingual */
    char used[14][SBLIMIT];						/*JMZ 17/03/95 Multilingual */
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
    int bbal, bancmc, pred, bancext, bbal_mpg1;
    int ll, pci, adb_mpg1;
    int bits = 0; /*bits already used for the front-channels*/
    static int sfsPerScfsi[] = { 3,2,1,2 };    /* lookup # sfs per scfsi */
    
    banc = 32; /* banc: bits for header */;
    /* 960627 FdB reserve extra bits wrt bugs */
    banc += 200;

    if (fr_ps->header->error_protection)
	berr = 16;
    else
	berr = 0; /* added 92-08-11 shn */

    pred = 0;
    bancmc = 0;
    bancext = 0;
    bbal = 0;
    bbal_mpg1 = 0;
    adb_mpg1 = bitrate[fr_ps->header->lay-1][fr_ps->header->bitrate_index] * 24;

#ifdef DEBUG
if (verbosity >= 3) printf("stereo+stereomc+stereoaug+n_ml_ch=%d\n", stereo+stereomc+stereoaug+n_ml_ch);
#endif 
	
    if (fr_ps->header->mode == MPG_MD_STEREO)
    {
	for (i = 0; i < sblimit; ++i)
	    bbal += (stereo) * (*alloc)[i][0].bits;
	for (i = 0; i < sblimit_mc; ++i)
	    bbal += (stereomc+stereoaug) * (*alloc)[i][0].bits;
	for (i = 0; i < sblimit_ml; ++i)
	    bbal += (n_ml_ch) * (*alloc_ml)[i][0].bits;
    }	 
    if (fr_ps->header->mode == MPG_MD_JOINT_STEREO)
    {
	for (i = 0; i < jsbound; ++i)
	    bbal += (stereo) * (*alloc)[i][0].bits;
	for (i = jsbound; i < sblimit; ++i)
	    bbal += (stereo-1) * (*alloc)[i][0].bits;
	for (i = 0; i < sblimit_mc; ++i)
	    bbal += (stereomc+stereoaug) * (*alloc)[i][0].bits;
	for (i = 0; i < sblimit_ml; ++i)
	    bbal += (n_ml_ch) * (*alloc_ml)[i][0].bits;
    }

    if (fr_ps->header->center == 3) bbal -= 41; 

    if (fr_ps->header->ext_bit_stream_present == 0)
	bancmc += 35;      /* mc_header + crc + tc_sbgr_select+ dyn_cross_on + 
			      mc_prediction_on  01/05/94,  SR  new! 05/04/94,  SR*/
    else
    {
	bancmc += 43;			    
	bancext = 40;
    }
      			    
    if (fr_ps->header->tc_sbgr_select == 0)
	bancmc += 36;
    else
	bancmc += 3;	

    if (fr_ps->header->dyn_cross_on == 1)
	bancmc += 49;      /* now with dyn_cross_LR,  505/04/94,  SR*/

    if (fr_ps->header->mc_prediction_on == 1)
    {
	for (i = 0; i < 8; i++)   /*now only 8 sb_groups, 05/04/94, SR*/
	{
	    bancmc += 1;
	    if (fr_ps->header->mc_pred[i] == 1)
		bancmc += n_pred_coef[fr_ps->header->dyn_cross[i]] * 2;
	}

	for (i = 0; i < 8; i++) 
	    if (fr_ps->header->mc_pred[i] == 1)
		for (j = 0; j < n_pred_coef[fr_ps->header->dyn_cross[i]]; j++)
		    if (fr_ps->header->predsi[i][j] != 0)
		    {
			pred += 3;
			for (pci = 0; pci < fr_ps->header->predsi[i][j]; pci++)
			    pred += 8;
		    }	
    }  

	
    for (l = 0; l < sblimit; l++)
	for (k = 0; k < 2; k++)
	    used[k][l] = 0;
    
    for (l = 0; l < SBLIMIT; l++)		  /* searching for the sb min SMR */
    {
	i = sbgrp[l];

	for (j = stereo; j < stereo+stereomc+stereoaug; j++)
	    if ((j < stereo && l < sblimit) ||
		(j >= stereo && l < sblimit_mc))
	    {
		/* k = transmission_channel5[fr_ps->header->tc_alloc[i]][j]; */
	  	k = transmission_channel (fr_ps, i, j);

		if (j < stereo)
		    bits += 12 * ((*alloc)[l][bit_alloc[k][l]].group *
				  (*alloc)[l][bit_alloc[k][l]].bits);
		else if (j < 7 || n_ml_ch == 0) /* Multichannel */ 
		    bits += 12 * ((*alloc_mc)[l][bit_alloc[k][l]].group *
				  (*alloc_mc)[l][bit_alloc[k][l]].bits);
		else /* MultiLingual */ 
		    bits += 12 * ((*alloc_ml)[l][bit_alloc[k][l]].group *
				  (*alloc_ml)[l][bit_alloc[k][l]].bits);

		if (bit_alloc[k][l] != 0)
		{
		    bits += 2;
		    switch (scfsi[k][l])
		    {
			case 0: bits += 18; break;
			case 1: bits += 12; break;		
			case 2: bits += 6; break;
			case 3: bits += 12; break;		
		    }   
		}
	    }
    }

    for (i = 0; i < 12; i++)
    {
	if ((fr_ps->header->tc_alloc[i] == 1) || (fr_ps->header->tc_alloc[i] == 7))
	{
	    bits += 12 * ((*alloc)[l][bit_alloc[0][l]].group *
			  (*alloc)[l][bit_alloc[0][l]].bits);
	    if (bit_alloc[0][l] != 0)
	    {
		bits += 2;
		switch(scfsi[0][l])
		{
		    case 0: bits += 18; break;
		    case 1: bits += 12; break;		
		    case 2: bits += 6; break;
		    case 3: bits += 12; break;		
		}   
	    }
	    used[0][l] = 2;
	}

	if ((fr_ps->header->tc_alloc[i] == 2) || (fr_ps->header->tc_alloc[i] == 6))
	{
	    bits += 12 * ((*alloc)[l][bit_alloc[1][l]].group *
			  (*alloc)[l][bit_alloc[1][l]].bits);
	    if (bit_alloc[1][l] != 0)
	    {
		bits += 2;
		switch(scfsi[1][l])
		{
		    case 0: bits += 18; break;
		    case 1: bits += 12; break;		
		    case 2: bits += 6; break;
		    case 3: bits += 12; break;		
		}   
	    }
	    used[1][l] = 2;
	}
    }	  

    	if(fr_ps->header->ext_bit_stream_present == 0)    
	    *adb -= bbal + berr + banc + bancmc + pred + bits;
	else
	{
	    *adb = *adb - bbal - berr - banc - bancmc - pred - bits - 
	    bancext - (fr_ps->header->n_ad_bytes * 8);
	    for(i = 0; i < sblimit; ++i)
		 bbal_mpg1 += 2 * (*alloc)[i][0].bits;
	    adb_mpg1 -= bbal_mpg1 + berr + banc + bancmc +
	     (fr_ps->header->n_ad_bytes * 8);
	 }   
        ad = *adb;
 
	    for(l = 0;l < sblimit; l++)
	    {
		for( k = 0; k < 2; k++)
		{
				mnr[k][l]=snr[0]-perm_smr[k][l];
				if(used[k][l] == 0)
			        bit_alloc[k][l] = 0;
		}		
	    }
   
/********************************************************************/
/* JMZ 08/03/1995 Multilingual */  

	for (i = 0; i < sblimit_ml; i++) 
		for (k = 7; k < 7+n_ml_ch; k++) 
		{
		mnr[k][i]=snr[0]-perm_smr[k][i];
		 /* mask-to-noise-level = signal-to-noise-level - minimum-masking-*/
		 /* threshold*/

		bit_alloc[k][i] = 0;
		used[k][i] = 0;
		}

/* JMZ 08/03/1995 Multilingual */   
/********************************************************************/
   
   
   
	bspl = bscf = bsel = bspl_mpg1 = bscf_mpg1 = bsel_mpg1 = 0;
 
    do  {
	small = 999999.0;
	min_sb = -1;
	min_ch = -1;


        for(i = 0; i < sblimit; i++)
	{
	     for( k = 0; k < 2; k++)
		{
		    if ((used[k][i] != 2) && (small > mnr[k][i])) 
		    {
			small = mnr[k][i];
			min_sb = i;  min_ch = k;
		    }
		}  
	 }
	 
/******************************************************************/
/* Multilingual JMZ 08/03/1995 */

if(n_ml_ch >0)
{
	for (i = 0; i < sblimit_ml; i++)
	  for(j = 7; j < (n_ml_ch+7); ++j)
	  {
	     k=j;
	     if ((used[k][i] != 2) && (small > mnr[k][i])) 
	     {
                  small = mnr[k][i];
                  min_sb = i;  min_ch = k;
	     }
	}
}

/* Multilingual JMZ 08/03/1995 */
/******************************************************************/

	 
     if(min_sb > -1) 
     { 
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
	  increment   = ((*alloc_ml)[min_sb][bit_alloc[min_ch][min_sb]+1].group * 
			 (*alloc_ml)[min_sb][bit_alloc[min_ch][min_sb]+1].bits);
	  if (used[min_ch][min_sb])
	   increment -= ((*alloc_ml)[min_sb][bit_alloc[min_ch][min_sb]].group * 
			 (*alloc_ml)[min_sb][bit_alloc[min_ch][min_sb]].bits);
	  if (fr_ps->header->multiling_fs==1) increment *=  6;
	  else                                increment *= 12;
       }

       /* scale factor bits required for subband [min] */
       if (used[min_ch][min_sb]) scale = seli = 0;
       else 
       {			  /* this channel had no bits or scfs before */
         seli = 2;
         scale = 6 * sfsPerScfsi[scfsi[min_ch][min_sb]];
	}
	
       if(fr_ps->header->ext_bit_stream_present == 1)
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
	
       
       if ((ad > bspl + bscf + bsel + seli + scale + increment) && (used[min_ch][min_sb] != 2)) 
       {
	    ba = ++bit_alloc[min_ch][min_sb]; /* next up alloc */
	    bspl += increment;		    /* bits for subband sample */
	    bscf += scale;		    /* bits for scale factor */
	    bsel += seli;			    /* bits for scfsi code */
	    used[min_ch][min_sb] = 1;	    /* subband has bits */
	    if (min_ch<7)
	      mnr[min_ch][min_sb] = -perm_smr[min_ch][min_sb] +
				    snr[(*alloc)[min_sb][ba].quant+1];
	    else
	      mnr[min_ch][min_sb] = -perm_smr[min_ch][min_sb] +
				    snr[(*alloc_ml)[min_sb][ba].quant+1];
			    
	    /* Check if subband has been fully allocated max bits */
	    if (min_ch < stereo) 
	    {
	     if (ba >= (1<<(*alloc)[min_sb][0].bits)-1) used[min_ch][min_sb] = 2;
	    } 
	    else if (min_ch < 7 || n_ml_ch == 0) /* Multichannel */ 
	    {
	     if (ba >= (1<<(*alloc_mc)[min_sb][0].bits)-1) used[min_ch][min_sb] = 2;
	    } 
	    else 
	    {
	     if (ba >= (1<<(*alloc_ml)[min_sb][0].bits)-1) used[min_ch][min_sb] = 2;
	    }
	}
	else used[min_ch][min_sb] = 2;  /* can't increase this alloc */

      }   /* end of if-loop if min_sb >-1 */
   } while(min_sb > -1);   /* until could find no channel */
   /* Calculate the number of bits left */

   ad -= bspl+bscf+bsel; 
   *adb = ad;
   for (i=sblimit;i<SBLIMIT;i++) for (k = 0; k < 2; k++) 
      bit_alloc[k][i]=0;
   for (i=sblimit_ml;i<SBLIMIT;i++) for (k = 7; k < 7+n_ml_ch; k++) 
      bit_alloc[k][i]=0;
}
 

void matri(double (*sbs_sample)[3][12][32], frame_params *fr_ps, double (*sb_sample)[3][12][32],
	 unsigned int (*scalar)[3][32], int sblimit, unsigned int (*scfsi)[32], 
	 unsigned int (*scfsi_dyn)[32], unsigned int (*bit_alloc)[32], 
	 unsigned int (*subband)[3][12][32])
{
  int i, j, k, l, ch1, ch2, m, n;
  
  layer *info = fr_ps->header;

	for(k = 0; k < 8; k++)
	{
          if(fr_ps->header->mc_pred[k] == 0)
	  {	
	    switch(fr_ps->header->tc_alloc[k])
	    {
		case 0: 
	 	    for( j = 0; j < 3; ++j)
			for(l = 0; l < 12; l ++)
			{
				sb_sample[0][j][l][k] = sb_sample[5][j][l][k] + sbs_sample[2][j][l][k] + sbs_sample[3][j][l][k];
				sb_sample[1][j][l][k] = sb_sample[6][j][l][k] + sbs_sample[2][j][l][k] + sbs_sample[4][j][l][k];
			}
			m = 0; n = 2;
			scale_factor_calc(scalar,sblimit, m, n, k, sb_sample);
		     break;
		case 1:
	 	    for( j = 0; j < 3; ++j)
			for(l = 0; l < 12; l ++)
			{
				sb_sample[0][j][l][k] = sbs_sample[5][j][l][k] + sb_sample[2][j][l][k] + sbs_sample[3][j][l][k];
			}
			
			m = 0; n = 1;
			scale_factor_calc(scalar,sblimit, m, n, k, sb_sample);
			trans_pattern(scalar, scfsi, fr_ps,scfsi_dyn);
			pre_quant(sbs_sample, m, n, sb_sample, scalar, bit_alloc, subband, fr_ps);

	 	    for( j = 0; j < 3; ++j)
			for(l = 0; l < 12; l ++)
			{
				sbs_sample[2][j][l][k] = sbs_sample[0][j][l][k] - sbs_sample[5][j][l][k] - sbs_sample[3][j][l][k];
				sb_sample[1][j][l][k] = sb_sample[6][j][l][k] + sbs_sample[2][j][l][k] + sbs_sample[4][j][l][k];
			}
			m = 1; n = 2;
			scale_factor_calc(scalar,sblimit, m, n, k, sb_sample);
			
		     break;
		case 2:
	 	    for( j = 0; j < 3; ++j)
			for(l = 0; l < 12; l ++)
			{
				sb_sample[1][j][l][k] = sbs_sample[6][j][l][k] + sb_sample[2][j][l][k] + sbs_sample[4][j][l][k];
			}
			
			m = 1; n = 2;
			scale_factor_calc(scalar,sblimit, m, n, k, sb_sample);
			trans_pattern(scalar, scfsi, fr_ps,scfsi_dyn);
			pre_quant(sbs_sample, m, n, sb_sample, scalar, bit_alloc, subband, fr_ps);

	 	    for( j = 0; j < 3; ++j)
			for(l = 0; l < 12; l ++)
			{
				sbs_sample[2][j][l][k] = sbs_sample[1][j][l][k] - sbs_sample[6][j][l][k] - sbs_sample[4][j][l][k];
				sb_sample[0][j][l][k] = sb_sample[5][j][l][k] + sbs_sample[2][j][l][k] + sbs_sample[3][j][l][k];
			}
			m = 0; n = 1;
			scale_factor_calc(scalar,sblimit, m, n, k, sb_sample);
			
		     break;
		case 3: 
	 	    for( j = 0; j < 3; ++j)
			for(l = 0; l < 12; l ++)
			{
				sb_sample[0][j][l][k] = sbs_sample[5][j][l][k] + sbs_sample[2][j][l][k] + sb_sample[3][j][l][k];
				sb_sample[1][j][l][k] = sb_sample[6][j][l][k] + sbs_sample[2][j][l][k] + sbs_sample[4][j][l][k];
			}
			m = 0; n = 2;
			scale_factor_calc(scalar,sblimit, m, n, k, sb_sample);
		     break;
		case 4: 
	 	    for( j = 0; j < 3; ++j)
			for(l = 0; l < 12; l ++)
			{
				sb_sample[0][j][l][k] = sb_sample[5][j][l][k] + sbs_sample[2][j][l][k] + sbs_sample[3][j][l][k];
				sb_sample[1][j][l][k] = sbs_sample[6][j][l][k] + sbs_sample[2][j][l][k] + sb_sample[4][j][l][k];
			}
			m = 0; n = 2;
			scale_factor_calc(scalar,sblimit, m, n, k, sb_sample);
		     break;
		case 5: 
	 	    for( j = 0; j < 3; ++j)
			for(l = 0; l < 12; l ++)
			{
				sb_sample[0][j][l][k] = sbs_sample[5][j][l][k] + sbs_sample[2][j][l][k] + sb_sample[3][j][l][k];
				sb_sample[1][j][l][k] = sbs_sample[6][j][l][k] + sbs_sample[2][j][l][k] + sb_sample[4][j][l][k];
			}
			m = 0; n = 2;
			scale_factor_calc(scalar,sblimit, m, n, k, sb_sample);
		     break;
		case 6:
	 	    for( j = 0; j < 3; ++j)
			for(l = 0; l < 12; l ++)
			{
				sb_sample[1][j][l][k] = sbs_sample[6][j][l][k] + sb_sample[2][j][l][k] + sbs_sample[4][j][l][k];
			}
			
			m = 1; n = 2;
			scale_factor_calc(scalar,sblimit, m, n, k, sb_sample);
			trans_pattern(scalar, scfsi, fr_ps,scfsi_dyn);
			pre_quant(sbs_sample, m, n, sb_sample, scalar, bit_alloc, subband, fr_ps);

	 	    for( j = 0; j < 3; ++j)
			for(l = 0; l < 12; l ++)
			{
				sbs_sample[2][j][l][k] = sbs_sample[1][j][l][k] - sbs_sample[6][j][l][k] - sbs_sample[4][j][l][k];
				sb_sample[0][j][l][k] = sbs_sample[5][j][l][k] + sbs_sample[2][j][l][k] + sb_sample[3][j][l][k];
			}
			m = 0; n = 1;
			scale_factor_calc(scalar,sblimit, m, n, k, sb_sample);
			
		     break;
		case 7:
	 	    for( j = 0; j < 3; ++j)
			for(l = 0; l < 12; l ++)
			{
				sb_sample[0][j][l][k] = sbs_sample[5][j][l][k] + sb_sample[2][j][l][k] + sbs_sample[3][j][l][k];
			}
			
			m = 0; n = 1;
			scale_factor_calc(scalar,sblimit, m, n, k, sb_sample);
			trans_pattern(scalar, scfsi, fr_ps,scfsi_dyn);
			pre_quant(sbs_sample, m, n, sb_sample, scalar, bit_alloc, subband, fr_ps);

	 	    for( j = 0; j < 3; ++j)
			for(l = 0; l < 12; l ++)
			{
				sbs_sample[2][j][l][k] = sbs_sample[0][j][l][k] - sbs_sample[5][j][l][k] - sbs_sample[3][j][l][k];
				sb_sample[1][j][l][k] = sbs_sample[6][j][l][k] + sbs_sample[2][j][l][k] + sb_sample[4][j][l][k];
			}
			m = 1; n = 2;
			scale_factor_calc(scalar,sblimit, m, n, k, sb_sample);
			
		     break;
	    }
	  }
	} 
	
	for(i = 8; i < 12; i++)
	{
          if(fr_ps->header->mc_pred[k] == 0)
	  {
	    switch(fr_ps->header->tc_alloc[i])
	    {
		case 0: 
	 	    for( j = 0; j < 3; ++j)
			for(l = 0; l < 12; l ++)
			    for( k = sb_groups[i-1] + 1; k <= sb_groups[i]; k++)
			    {
				sb_sample[0][j][l][k] = sb_sample[5][j][l][k] + sbs_sample[2][j][l][k] + sbs_sample[3][j][l][k];
				sb_sample[1][j][l][k] = sb_sample[6][j][l][k] + sbs_sample[2][j][l][k] + sbs_sample[4][j][l][k];
			    }
			 m = 0; n = 2;    
			 scale_factor_calc(scalar,sblimit, m, n, i, sb_sample);
		     break;
		case 1:
	 	    for( j = 0; j < 3; ++j)
			for(l = 0; l < 12; l ++)
			    for( k = sb_groups[i-1] + 1; k <= sb_groups[i]; k++)
			    {
				sb_sample[0][j][l][k] = sbs_sample[5][j][l][k] + sb_sample[2][j][l][k] + sbs_sample[3][j][l][k];
			    }
			
			m = 0; n = 1;
			scale_factor_calc(scalar,sblimit, m, n, i, sb_sample);
			trans_pattern(scalar, scfsi, fr_ps,scfsi_dyn);
			pre_quant(sbs_sample, m, n, sb_sample, scalar, bit_alloc, subband, fr_ps);

	 	    for( j = 0; j < 3; ++j)
			for(l = 0; l < 12; l ++)
			    for( k = sb_groups[i-1] + 1; k <= sb_groups[i]; k++)
			    {
				sbs_sample[2][j][l][k] = sbs_sample[0][j][l][k] - sbs_sample[5][j][l][k] - sbs_sample[3][j][l][k];
				sb_sample[1][j][l][k] = sb_sample[6][j][l][k] + sbs_sample[2][j][l][k] + sbs_sample[4][j][l][k];
			    }
			m = 1; n = 2;
			scale_factor_calc(scalar,sblimit, m, n, i, sb_sample);
		     break;
		case 2:
	 	    for( j = 0; j < 3; ++j)
			for(l = 0; l < 12; l ++)
			    for( k = sb_groups[i-1] + 1; k <= sb_groups[i]; k++)
			    {
				sb_sample[1][j][l][k] = sbs_sample[6][j][l][k] + sb_sample[2][j][l][k] + sbs_sample[4][j][l][k];
			    }
			
			m = 1; n = 2;
			scale_factor_calc(scalar,sblimit, m, n, i, sb_sample);
			trans_pattern(scalar, scfsi, fr_ps,scfsi_dyn);
			pre_quant(sbs_sample, m, n, sb_sample, scalar, bit_alloc, subband, fr_ps);

	 	    for( j = 0; j < 3; ++j)
			for(l = 0; l < 12; l ++)
			    for( k = sb_groups[i-1] + 1; k <= sb_groups[i]; k++)
			    {
				sbs_sample[2][j][l][k] = sbs_sample[1][j][l][k] - sbs_sample[6][j][l][k] - sbs_sample[4][j][l][k];
				sb_sample[0][j][l][k] = sb_sample[5][j][l][k] + sbs_sample[2][j][l][k] + sbs_sample[3][j][l][k];
			    }
			m = 0; n = 1;
			scale_factor_calc(scalar,sblimit, m, n, i, sb_sample);
			    
		     break;
		case 3: 
	 	    for( j = 0; j < 3; ++j)
			for(l = 0; l < 12; l ++)
			    for( k = sb_groups[i-1] + 1; k <= sb_groups[i]; k++)
			    {
				sb_sample[0][j][l][k] = sbs_sample[5][j][l][k] + sbs_sample[2][j][l][k] + sb_sample[3][j][l][k];
				sb_sample[1][j][l][k] = sb_sample[6][j][l][k] + sbs_sample[2][j][l][k] + sbs_sample[4][j][l][k];
			    }
			m = 0; n = 2;
			scale_factor_calc(scalar,sblimit, m, n, i, sb_sample);
		     break;
		case 4: 
	 	    for( j = 0; j < 3; ++j)
			for(l = 0; l < 12; l ++)
			    for( k = sb_groups[i-1] + 1; k <= sb_groups[i]; k++)
			    {
				sb_sample[0][j][l][k] = sb_sample[5][j][l][k] + sbs_sample[2][j][l][k] + sbs_sample[3][j][l][k];
				sb_sample[1][j][l][k] = sbs_sample[6][j][l][k] + sbs_sample[2][j][l][k] + sb_sample[4][j][l][k];
			    }
			m = 0; n = 2;
			scale_factor_calc(scalar,sblimit, m, n, i, sb_sample);
		     break;
		case 5: 
	 	    for( j = 0; j < 3; ++j)
			for(l = 0; l < 12; l ++)
			    for( k = sb_groups[i-1] + 1; k <= sb_groups[i]; k++)
			    {
				sb_sample[0][j][l][k] = sbs_sample[5][j][l][k] + sbs_sample[2][j][l][k] + sb_sample[3][j][l][k];
				sb_sample[1][j][l][k] = sbs_sample[6][j][l][k] + sbs_sample[2][j][l][k] + sb_sample[4][j][l][k];
			    }
			m = 0; n = 2;
			scale_factor_calc(scalar,sblimit, m, n, i, sb_sample);
		     break;
		case 6:
	 	    for( j = 0; j < 3; ++j)
			for(l = 0; l < 12; l ++)
			    for( k = sb_groups[i-1] + 1; k <= sb_groups[i]; k++)
			    {
				sb_sample[1][j][l][k] = sbs_sample[6][j][l][k] + sb_sample[2][j][l][k] + sbs_sample[4][j][l][k];
			    }
			
			m = 1; n = 2;
			scale_factor_calc(scalar,sblimit, m, n, i, sb_sample);
			trans_pattern(scalar, scfsi, fr_ps,scfsi_dyn);
			pre_quant(sbs_sample, m, n, sb_sample, scalar, bit_alloc, subband, fr_ps);

	 	    for( j = 0; j < 3; ++j)
			for(l = 0; l < 12; l ++)
			    for( k = sb_groups[i-1] + 1; k <= sb_groups[i]; k++)
			    {
				sbs_sample[2][j][l][k] = sbs_sample[1][j][l][k] - sbs_sample[6][j][l][k] - sbs_sample[4][j][l][k];
				sb_sample[0][j][l][k] = sbs_sample[5][j][l][k] + sbs_sample[2][j][l][k] + sb_sample[3][j][l][k];
			    }
			m = 0; n = 1;
			scale_factor_calc(scalar,sblimit, m, n, i, sb_sample);
			
		     break;
		case 7:
	 	    for( j = 0; j < 3; ++j)
			for(l = 0; l < 12; l ++)
			    for( k = sb_groups[i-1] + 1; k <= sb_groups[i]; k++)
			    {
				sb_sample[0][j][l][k] = sbs_sample[5][j][l][k] + sb_sample[2][j][l][k] + sbs_sample[3][j][l][k];
			    }
			
			m = 0; n = 1;
			scale_factor_calc(scalar,sblimit, m, n, i, sb_sample);
			trans_pattern(scalar, scfsi, fr_ps,scfsi_dyn);
			pre_quant(sbs_sample, m, n, sb_sample, scalar, bit_alloc, subband, fr_ps);

	 	    for( j = 0; j < 3; ++j)
			for(l = 0; l < 12; l ++)
			    for( k = sb_groups[i-1] + 1; k <= sb_groups[i]; k++)
			    {
				sbs_sample[2][j][l][k] = sbs_sample[0][j][l][k] - sbs_sample[5][j][l][k] - sbs_sample[3][j][l][k];
				sb_sample[1][j][l][k] = sbs_sample[6][j][l][k] + sbs_sample[2][j][l][k] + sb_sample[4][j][l][k];
			    }
			m = 1; n = 2;
			scale_factor_calc(scalar,sblimit, m, n, i, sb_sample);
			
		     break;
	    }
	  }
	}
 }




buffer_sample(unsigned int (*keep_it)[36][32], unsigned int (*sample)[3][32], unsigned int (*bit_alloc)[32], frame_params *fr_ps, int l, int z, int q)
{
   int i,j,k,m;
   int stereo = fr_ps->stereo;
   int sblimit = fr_ps->sblimit;
   int jsbound = fr_ps->jsbound;
   al_table *alloc = fr_ps->alloc;
   unsigned int x;

   for (i=0;i<sblimit;i++) for (j= l;j<((i<jsbound)?z:l+1);j++) 
   {
       if (bit_alloc[j][i]) 
       {
         /* check for grouping in subband */
	 /* if ((*alloc)[i][bit_alloc[j][i]].group==3)*/
           for (m=0;m<3;m++) 
	   {
              sample[j][m][i] = keep_it[j][q*3+m][i];
           }         
       }
       else 
       {      /* for no sample transmitted */
         for (k=0;k<3;k++) sample[j][k][i] = 0;
       }
       if(stereo == 2 && i>= jsbound)  /* joint stereo : copy L to R */
          for (k=0;k<3;k++) sample[1][k][i] = sample[0][k][i];
   }
   for (i=sblimit;i<SBLIMIT;i++) for (j= l;j< z;j++) for (k=0;k<3;k++)
      sample[j][k][i] = 0;
}      



static double a[17] = {
  0.750000000, 0.625000000, 0.875000000, 0.562500000, 0.937500000,
  0.968750000, 0.984375000, 0.992187500, 0.996093750, 0.998046875,
  0.999023438, 0.999511719, 0.999755859, 0.999877930, 0.999938965,
  0.999969482, 0.999984741 };
 
static double b[17] = {
  -0.250000000, -0.375000000, -0.125000000, -0.437500000, -0.062500000,
  -0.031250000, -0.015625000, -0.007812500, -0.003906250, -0.001953125,
  -0.000976563, -0.000488281, -0.000244141, -0.000122070, -0.000061035,
  -0.000030518, -0.000015259 };
 


void subband_quantization_pre(unsigned int (*scalar)[3][32], double (*sb_samples)[3][12][32], unsigned int (*bit_alloc)[32], unsigned int (*sbband)[3][12][32], frame_params *fr_ps, int ch1, int ch2)
{
   int i, j, k, s, n, qnt, sig, m, l, ll, z;
   int stereo	= fr_ps->stereo;
   int stereomc = fr_ps->stereomc;
   int sblimit = fr_ps->sblimit;
   unsigned int stps, y;
   double d;
   al_table *alloc = fr_ps->alloc;

 

   for (s=0;s<3;s++)
     for (j=0;j<12;j= j+3)
	for (i=0;i<sblimit;i++)
	  for(k = ch1; k < ch2; k++)
	  {
	    for(z = 0; z < 3; z ++)
	    {
		    if(bit_alloc[k][i])
	               d = sb_samples[k][s][j+z][i] / multiple[scalar[k][s][i]];
		    else
		       d = 0;
	   
	   
	    if (mod(d) >= 1.0) /* > changed to >=, 1992-11-06 shn */
	    {
		printf("In predistortion, not scaled properly, %d %d %d %d\n",k,s,j+z,i);
		printf("Value %1.10f\n",sb_samples[k][s][j+z][i]);
	    }
	    qnt = (*alloc)[i][bit_alloc[k][i]].quant;
            d = d * a[qnt] + b[qnt];
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
             sbband[k][s][j+z][i] = (unsigned int) (d * (double) (1L<<n));
             /* tag the inverted sign bit to sbband at position N */
             /* The bit inversion is a must for grouping with 3,5,9 steps
                so it is done for all subbands */
             if (sig) sbband[k][s][j+z][i] |= 1<<n;
	     
	     if(!bit_alloc[k][i]) sbband[k][s][j+z][i] = 0;
	    }

/*try try try ! 7/7/94 */

/*             if ((*alloc)[i][bit_alloc[k][i]].group != 3) 
	     {
               y =(*alloc)[i][bit_alloc[k][i]].steps;
               sbband[k][s][j][i] = sbband[k][s][j][i] +
                      		    sbband[k][s][j+1][i] * y +
                                    sbband[k][s][j+2][i] * y * y;
               sbband[k][s][j+1][i] = sbband[k][s][j][i];
               sbband[k][s][j+2][i] = sbband[k][s][j][i];
	      }*/

	     
	     
	 }
           for (s=0;s<3;s++)
             for (j=sblimit;j<SBLIMIT;j++)
		for (i=0;i<12;i++) for (k = 0; k < 7; k++) sbband[k][s][i][j] = 0;
}






void trans_pattern(unsigned int (*scalar)[3][32], unsigned int (*scfsi)[32], frame_params *fr_ps, unsigned int (*scfsi_dyn)[32])
{
   int stereo  = fr_ps->stereo;
   int stereomc = fr_ps->stereomc;
   int sblimit = fr_ps->sblimit;
	int dscf[2];
	int class[2],i,j,k;
static int pattern[5][5] = {0x123, 0x122, 0x122, 0x133, 0x123,
                            0x113, 0x111, 0x111, 0x444, 0x113,
                            0x111, 0x111, 0x111, 0x333, 0x113,
                            0x222, 0x222, 0x222, 0x333, 0x123,
                            0x123, 0x122, 0x122, 0x133, 0x123};
 
  for (k = 0; k < 2; k++)
     for (i=0;i<sblimit;i++) 
     {
       dscf[0] =  (scalar[k][0][i]-scalar[k][1][i]);
       dscf[1] =  (scalar[k][1][i]-scalar[k][2][i]);
       for (j=0;j<2;j++)
       {
         if (dscf[j]<=-3) class[j] = 0;
         else if (dscf[j] > -3 && dscf[j] <0) class[j] = 1;
              else if (dscf[j] == 0) class[j] = 2;
                   else if (dscf[j] > 0 && dscf[j] < 3) class[j] = 3;
                        else class[j] = 4;
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
   
	}
    }
}    	   





void pre_quant(double (*sbs_sample)[3][12][32], int l, int m, double (*sb_sample)[3][12][32], 
		unsigned int (*scalar)[3][32], unsigned int (*bit_alloc)[32], 
		unsigned int (*subband)[3][12][32], frame_params *fr_ps)
{
    double sb_pre_sample[7][3][12][SBLIMIT];  /*predistortion*/
    unsigned int sample[7][3][SBLIMIT];
    unsigned int keep_it[7][36][SBLIMIT];
    int ch, sb, gr, i;
    int hi,  hu;

    subband_quantization_pre(scalar, sb_sample, bit_alloc, subband, fr_ps, l, m);

		
    for (ch = l; ch < m; ch++)
      for (sb = 0; sb < SBLIMIT; sb++)
	for (gr = 0; gr < 3; gr++)
	    for (i = 0; i < 12; i++)
		keep_it[ch][i+gr*12][sb] = subband[ch][gr][i][sb];

		for(i = 0; i < 12; i++) 
		{      
		    buffer_sample(keep_it,sample,bit_alloc,fr_ps, l, m, i);

		    II_dequantize_sample(sample, bit_alloc, sb_pre_sample, fr_ps, l, m, i);
		    II_denormalize_sample(sb_pre_sample, scalar, fr_ps, i>>2, l, m, i);
	        }      	    
		
    for (ch = l; ch < m; ch++)
      for (sb=0;sb<SBLIMIT;sb++)
	for (i = 0; i < 4; i++)
	    for (gr=0;gr<3;gr++)
		sbs_sample[ch][0][i*3 + gr][sb] = sb_pre_sample[ch][gr][i][sb];
		
    for (ch = l; ch < m; ch++)
      for (sb=0;sb<SBLIMIT;sb++)
	for (i = 4; i < 8; i++)
	    for (gr=0;gr<3;gr++)
		sbs_sample[ch][1][(i-4)*3 + gr][sb] = sb_pre_sample[ch][gr][i][sb];

    for (ch = l; ch < m; ch++)
      for (sb=0;sb<SBLIMIT;sb++)
	for (i = 8; i < 12; i++)
	    for (gr=0;gr<3;gr++)
		sbs_sample[ch][2][(i-8)*3 + gr][sb] = sb_pre_sample[ch][gr][i][sb];


}

/**************************************************************
/*
/*   Restore the compressed sample to a factional number.
/*   first complement the MSB of the sample
/*   for Layer II :
/*   Use the formula s = s' * c + d
/*
/*  taken out of decoder,modified for prediction 9/16/93,SR
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

void II_dequantize_sample(unsigned int (*sample)[3][32], unsigned int (*bit_alloc)[32], double (*fraction)[3][12][32], frame_params *fr_ps, int l, int m, int z)
{
   int i, j, k, x;
   int stereo = fr_ps->stereo;
   int sblimit = fr_ps->sblimit;
   al_table *alloc = fr_ps->alloc;

   for (i=0;i<sblimit;i++)  for (j=0;j<3;j++) for (k = l;k < m ;k++)
     if (bit_alloc[k][i]) {

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
       if (((sample[k][j][i] >> x-1) & 1) == 1)
          fraction[k][j][z][i] = 0.0;
       else  fraction[k][j][z][i] = -1.0;

       /* Form a 2's complement sample */
       fraction[k][j][z][i] += (double) (sample[k][j][i] & ((1<<x-1)-1)) /
                            (double) (1L<<x-1);

       /* Dequantize the sample */
       fraction[k][j][z][i] += d[(*alloc)[i][bit_alloc[k][i]].quant];
       fraction[k][j][z][i] *= c[(*alloc)[i][bit_alloc[k][i]].quant];
     }
     else fraction[k][j][z][i] = 0.0;   
   
   for (i=sblimit;i<SBLIMIT;i++) for (j=0;j<3;j++) for(k = l; k < m; k++)
     fraction[k][j][z][i] = 0.0;
}












void II_denormalize_sample(double (*fraction)[3][12][32], unsigned int (*scale_index)[3][32], frame_params *fr_ps, int x, int l, int m, int z)
       /*far*/                             
                                        
                    
      
            
{
   int i,j,k;
   int stereo = fr_ps->stereo;
   int sblimit = fr_ps->sblimit;

   for (i=0;i<sblimit;i++) for (j= l;j < m; j++) {
      fraction[j][0][z][i] *= multiple[scale_index[j][x][i]];
      fraction[j][1][z][i] *= multiple[scale_index[j][x][i]];
      fraction[j][2][z][i] *= multiple[scale_index[j][x][i]];
   }
}



void scale_factor_calc(unsigned int (*scalar)[3][32], int sblimit, int l, int m, int z, double (*sb_sample)[3][12][32])
                                   
            
            
                                          /* (see above) */
{
  int i,j, k,t;
  double s[SBLIMIT];
 
  for (k = l; k< m; k++) for (t=0;t<3;t++) 
  {
   for (i=((z == 0) ? 0:(sb_groups[z-1]+1));i<=sb_groups[z];i++) 
   {
	for (j=1, s[i] = mod(sb_sample[k][t][0][i]);j<SCALE_BLOCK;j++)
		  if (mod(sb_sample[k][t][j][i]) > s[i])
			 s[i] = mod(sb_sample[k][t][j][i]);
   } 
    for (i=((z == 0) ? 0:(sb_groups[z-1]+1));i<=sb_groups[z];i++)
    { 
        for (j=SCALE_RANGE-1,scalar[k][t][i]=0;j>=0;j--)
		if (s[i] < multiple[j]) 
		{                       /* <= changed to <, 1992-11-06 shn*/
                   scalar[k][t][i] = j;
                   break;
                }
     }
   }
} 




