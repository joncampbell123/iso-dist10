/**********************************************************************
 * ISO MPEG Audio Subgroup Software Simulation Group (1996)
 * ISO 13818-3 MPEG-2 Audio Multichannel Encoder
 *
 * $Id: dyn_cross.c 1.3 1996/02/12 07:13:35 rowlands Exp $
 *
 * $Log: dyn_cross.c $
 * Revision 1.3  1996/02/12 07:13:35  rowlands
 * Release following Munich meeting
 *
 **********************************************************************/

#include "common.h"
#include "encoder.h"

/************************************************************************
 *
 * take_dyn_cross
 *
 * Version 1.0	    94/10/24	Andreas Silzle, IRT
 *
 * Copy the sums, scf, scfsi, if required, in the normal positions.
 *
 ************************************************************************/


/* #define LOW_SBGR	5	original value 960619 FdB			*/
#define LOW_SBGR	0   /* lowest subbandgroup, in which dyn_cross is done.*/
			    /* All sb-groups under that boarder are not	       */
			    /* affected by dyn_cross			       */

void take_dyn_cross (
	int		dyn_cr,	
	frame_params	*fr_ps,  		    
	    /* out: int	fr_ps->header->dyn_cross_on, 
	     *	    int	fr_ps->header->dyn_cross_LR, 
    	     *	    int	fr_ps->header->dyn_cross[SBGRS],	    
	     */			    
	double		sb_sample_sum[5][3][SCALE_BLOCK][SBLIMIT], 	    
						    /* in, T23, T24, T34, T234, T56 */
	unsigned int	scalar_sum[][3][SBLIMIT], 			/* in */ 
	unsigned int	scfsi_sum[][SBLIMIT],  				/* in */
	unsigned int	scfsi_dyn[][SBLIMIT],  				/* in */
	double		sb_sample[CHANMAX3][3][SCALE_BLOCK][SBLIMIT],	/* out */
	unsigned int	scalar[][3][SBLIMIT], 				/* out */ 
	unsigned int	scfsi[][SBLIMIT]) 				/* out */
{
    int	    sbgr, sb, sufr, smp, dyn_second_stereo = 0;
    layer *info  = fr_ps->header;
    int center   = info->center;
    int surround = info->surround;

    fr_ps->header->dyn_cross_on = 0;
    if (fr_ps->header->dyn_cross_LR && (fr_ps->config == 302 || fr_ps->config == 202 || fr_ps->config == 102))
	dyn_second_stereo = 1;

/*    printf ("dyn_cr: %2d\n", dyn_cr); */

    for (sbgr = LOW_SBGR; sbgr < SBGRS; sbgr++)
    {
	/* signalization */
	if (dyn_cr == 0 && dyn_second_stereo == 0)
	{
	    fr_ps->header->dyn_cross[sbgr] = 0;
	    fr_ps->header->dyn_second_stereo[sbgr] = 0;
	}
	else
	{
	    fr_ps->header->dyn_cross_on = 1;
	    /* original value 960619 FdB */ 
	    /* fr_ps->header->dyn_cross[sbgr] = dyn_cr; */
	    /* systematic variation of dynamic crosstalk over all subbands */
	    if (dyn_cr == -1)
	    {
		switch (fr_ps->config)
		{
		    case 320:	fr_ps->header->dyn_cross[sbgr] = rand () % 15;
				break;
		    case 310:
		    case 220:	fr_ps->header->dyn_cross[sbgr] = rand () % 5;
				break;
		    case 300:
		    case 302:
		    case 210:	fr_ps->header->dyn_cross[sbgr] = rand () % 2;
				break;
		    default:	fr_ps->header->dyn_cross[sbgr] = 0;
				break;
		}
	    }
	    else
		fr_ps->header->dyn_cross[sbgr] = dyn_cr;

	    fr_ps->header->dyn_second_stereo[sbgr] = dyn_second_stereo;
/*
            printf ("sbgr: %2d dyn_cr: %2d dyn_2nd_st: %2d\n",
	    	    sbgr, fr_ps->header->dyn_cross[sbgr],
		    fr_ps->header->dyn_second_stereo[sbgr]);
*/
	    /* copying sums, scf, scfsi */
	    
	    /* 960627 FdB DynX dependent on configuration */
	    if (fr_ps->config == 320)
	    {
		/* 3/2 */
		if(fr_ps->header->dyn_cross[sbgr] == 9 || fr_ps->header->dyn_cross[sbgr] == 11) {
		    T3from[sbgr] = T2[sbgr];
		    for(sb=((sbgr==0) ? 0:(sb_groups[sbgr-1]+1)); 
			    sb<=sb_groups[sbgr]; sb++) {
			scfsi[T2[sbgr]][sb] = scfsi_sum[T23][sb];
			for(sufr = 0; sufr<3; ++sufr) {
			    scalar[T2[sbgr]][sufr][sb] = scalar_sum[T23][sufr][sb];
			    for(smp = 0; smp<SCALE_BLOCK; ++smp) {
				sb_sample[T2[sbgr]][sufr][smp][sb] = 
				    sb_sample_sum[T23][sufr][smp][sb];
			    }   /* for(smp..) */
			}	    /* for(sufr.). */
		    }	    /* for(sb..) */
		} 
		else if(fr_ps->header->dyn_cross[sbgr] == 10 || fr_ps->header->dyn_cross[sbgr] == 12) {
		    T4from[sbgr] = T2[sbgr];
		    for(sb=((sbgr==0) ? 0:(sb_groups[sbgr-1]+1)); 
			    sb<=sb_groups[sbgr]; sb++) {
			scfsi[T2[sbgr]][sb] = scfsi_sum[T24][sb];
			for(sufr = 0; sufr<3; ++sufr) {
			    scalar[T2[sbgr]][sufr][sb] = scalar_sum[T24][sufr][sb];
			    for(smp = 0; smp<SCALE_BLOCK; ++smp) {
				sb_sample[T2[sbgr]][sufr][smp][sb] = 
				    sb_sample_sum[T24][sufr][smp][sb];
			    }   /* for(smp..) */
			}	    /* for(sufr..) */
		    }	    /* for(sb..) */
		} 
		else if(fr_ps->header->dyn_cross[sbgr] == 8 || fr_ps->header->dyn_cross[sbgr] == 13) {
		    T4from[sbgr] = T3[sbgr];
		    for(sb=((sbgr==0) ? 0:(sb_groups[sbgr-1]+1)); 
			    sb<=sb_groups[sbgr]; sb++) {
			scfsi[T3[sbgr]][sb] = scfsi_sum[T34][sb];
			for(sufr = 0; sufr<3; ++sufr) {
			    scalar[T3[sbgr]][sufr][sb] = scalar_sum[T34][sufr][sb];
			    for(smp = 0; smp<SCALE_BLOCK; ++smp) {
				sb_sample[T3[sbgr]][sufr][smp][sb] = 
				    sb_sample_sum[T34][sufr][smp][sb];
			    }   /* for(smp..) */
			}	    /* for(sufr..) */
		    }	    /* for(sb..) */
		} 
		else if(fr_ps->header->dyn_cross[sbgr] == 14) {
		    T3from[sbgr] = T4from[sbgr] = T2[sbgr];
		    for(sb=((sbgr==0) ? 0:(sb_groups[sbgr-1]+1)); 
			    sb<=sb_groups[sbgr]; sb++) {
			scfsi[T2[sbgr]][sb] = scfsi_sum[T234][sb];
			for(sufr = 0; sufr<3; ++sufr) {
			    scalar[T2[sbgr]][sufr][sb] = scalar_sum[T234][sufr][sb];
			    for(smp = 0; smp<SCALE_BLOCK; ++smp) {
				sb_sample[T2[sbgr]][sufr][smp][sb] = 
				    sb_sample_sum[T234][sufr][smp][sb];
			    }   /* for(smp..) */
			}	    /* for(sufr..) */
		    }	    /* for(sb..) */
		}		    /* if(dyncr_do[sbgr]..) */
	
		/* scfsi_dyn in scfsi of L0 and R0 */
		/* T2 */
		switch(fr_ps->header->dyn_cross[sbgr]) {
	      case 3: case 5: case 6: case 7: case 13:
		    T2from[sbgr] = T2outof[sbgr];
		    for(sb=((sbgr==0) ? 0:(sb_groups[sbgr-1]+1)); 
			    sb<=sb_groups[sbgr]; sb++) 
			scfsi[T2outof[sbgr]][sb] = scfsi_dyn[T2outof[sbgr]][sb];
		    break;	    
		}	/* switch(dyncr_do[sbgr]) */
		/* T3 */
		switch(fr_ps->header->dyn_cross[sbgr]) {
	      case 2: case 4: case 6: case 7: case 12:
		    T3from[sbgr] = T3outof[sbgr];
		    for(sb=((sbgr==0) ? 0:(sb_groups[sbgr-1]+1)); 
			    sb<=sb_groups[sbgr]; sb++) 
			scfsi[L0][sb] = scfsi_dyn[L0][sb];
		    break;	    
		}	/* switch(dyncr_do[sbgr]) */
		/* T4 */
		switch(fr_ps->header->dyn_cross[sbgr]) {
	      case 1: case 4: case 5: case 7: case 11:
		    T4from[sbgr] = T4outof[sbgr];
		    for(sb=((sbgr==0) ? 0:(sb_groups[sbgr-1]+1)); 
			    sb<=sb_groups[sbgr]; sb++) 
			scfsi[R0][sb] = scfsi_dyn[R0][sb];
		    break;	    
		}	/* switch(fr_ps->header->dyn_cross[sbgr]) */
	    }
	    else if (fr_ps->config == 310 || fr_ps->config == 220)
	    {
		/* 3/1 and 2/2 */
		if(fr_ps->header->dyn_cross[sbgr] == 4) {
		    for(sb=((sbgr==0) ? 0:(sb_groups[sbgr-1]+1)); 
			    sb<=sb_groups[sbgr]; sb++) {
			scfsi[T2[sbgr]][sb] = scfsi_sum[T23][sb];
			for(sufr = 0; sufr<3; ++sufr) {
			    scalar[T2[sbgr]][sufr][sb] = scalar_sum[T23][sufr][sb];
			    for(smp = 0; smp<SCALE_BLOCK; ++smp) {
				sb_sample[T2[sbgr]][sufr][smp][sb] = 
				    sb_sample_sum[T23][sufr][smp][sb];
			    }   /* for(smp..) */
			}	    /* for(sufr.). */
		    }	    /* for(sb..) */
		} 
    
		/* scfsi_dyn in scfsi of L0 and R0 */
		/* T2 */
		switch(fr_ps->header->dyn_cross[sbgr]) {
	      case 2: case 3:
		    for(sb=((sbgr==0) ? 0:(sb_groups[sbgr-1]+1)); 
			    sb<=sb_groups[sbgr]; sb++) 
			scfsi[T2outof[sbgr]][sb] = scfsi_dyn[T2outof[sbgr]][sb];
		    break;	    
		}	/* switch(dyncr_do[sbgr]) */
		/* T3 */
		switch(fr_ps->header->dyn_cross[sbgr]) {
	      case 1: case 3:
		    for(sb=((sbgr==0) ? 0:(sb_groups[sbgr-1]+1)); 
			    sb<=sb_groups[sbgr]; sb++) 
			scfsi[T3outof[sbgr]][sb] = scfsi_dyn[T3outof[sbgr]][sb];
		    break;	    
		}	/* switch(dyncr_do[sbgr]) */
	    }
	    else if (fr_ps->config == 300 || fr_ps->config == 302 || fr_ps->config == 210)
	    {
		/* 3/0 (+2/0) and 2/1 */
		/* scfsi_dyn in scfsi of L0 and R0 */
		/* T2 */
		switch(fr_ps->header->dyn_cross[sbgr]) {
	      case 1:
		    for(sb=((sbgr==0) ? 0:(sb_groups[sbgr-1]+1)); 
			    sb<=sb_groups[sbgr]; sb++) 
			scfsi[T2outof[sbgr]][sb] = scfsi_dyn[T2outof[sbgr]][sb];
		    break;	    
		}	/* switch(dyncr_do[sbgr]) */
    
		if (fr_ps->header->dyn_second_stereo[sbgr])
		    for (sb=((sbgr==0) ? 0:(sb_groups[sbgr-1]+1)); sb<=sb_groups[sbgr]; sb++)
		    {
			scfsi[T3[sbgr]][sb] = scfsi_sum[T23][sb];
			for (sufr = 0; sufr<3; ++sufr)
			{
			    scalar[T3[sbgr]][sufr][sb] = scalar_sum[T23][sufr][sb];
			    for (smp = 0; smp<SCALE_BLOCK; ++smp)
				sb_sample[T3[sbgr]][sufr][smp][sb] = sb_sample_sum[T23][sufr][smp][sb];
			}	    /* for(sufr..) */
		    }	    /* for(sb..) */
	    }
	    else if (fr_ps->config == 202)
	    {
		if (fr_ps->header->dyn_second_stereo[sbgr])
		    for (sb=((sbgr==0) ? 0:(sb_groups[sbgr-1]+1)); sb<=sb_groups[sbgr]; sb++)
		    {
			scfsi[T2[sbgr]][sb] = scfsi_sum[T23][sb];
			for (sufr = 0; sufr<3; ++sufr)
			{
			    scalar[T2[sbgr]][sufr][sb] = scalar_sum[T23][sufr][sb];
			    for (smp = 0; smp<SCALE_BLOCK; ++smp)
				sb_sample[T2[sbgr]][sufr][smp][sb] = sb_sample_sum[T23][sufr][smp][sb];
			}	    /* for(sufr.). */
		    }	    /* for(sb..) */
	    }
	    else if (fr_ps->config == 102)
	    {
		if (fr_ps->header->dyn_second_stereo[sbgr])
		    for (sb=((sbgr==0) ? 0:(sb_groups[sbgr-1]+1)); sb<=sb_groups[sbgr]; sb++)
		    {
			scfsi[1][sb] = scfsi_sum[T23][sb];
			for (sufr = 0; sufr<3; ++sufr)
			{
			    scalar[1][sufr][sb] = scalar_sum[T23][sufr][sb];
			    for (smp = 0; smp<SCALE_BLOCK; ++smp)
				sb_sample[1][sufr][smp][sb] = sb_sample_sum[T23][sufr][smp][sb];
			}	    /* for(sufr.). */
		    }	    /* for(sb..) */
	    }
	}	/* if(fr_ps->header->dyn_cross[sbgr] == 0) */
    }		/* for(sbgr=0.. */
}

#ifdef Augmentation_7ch
void take_dyn_cross_aug (
	int		aug_dyn_cr,	
	frame_params	*fr_ps,  		    
	    /* out: int	fr_ps->header->dyn_cross_on, 
	     *	    int	fr_ps->header->dyn_cross_LR, 
    	     *	    int	fr_ps->header->dyn_cross[SBGRS],	    
	     */			    
	double		sb_sample_sum[5][3][SCALE_BLOCK][SBLIMIT], 	    
						    /* in, T23, T24, T34, T234, T56 */
	unsigned int	scalar_sum[][3][SBLIMIT], 			/* in */ 
	unsigned int	scfsi_sum[][SBLIMIT],  				/* in */
	unsigned int	scfsi_dyn[][SBLIMIT],  				/* in */
	double		sb_sample[CHANMAX3][3][SCALE_BLOCK][SBLIMIT],	/* out */
	unsigned int	scalar[][3][SBLIMIT], 				/* out */ 
	unsigned int	scfsi[][SBLIMIT]) 				/* out */
{
    int	    sbgr, sb, sufr, smp;
    layer *info  = fr_ps->header;
    int center   = info->center;
    int surround = info->surround;

    fr_ps->header->aug_dyn_cross_on = 1;

    for (sbgr = LOW_SBGR; sbgr < SBGRS; sbgr++)
    {
	if (aug_dyn_cr == -1)
	    fr_ps->header->aug_dyn_cross[sbgr] = rand () % 19;
	else
	    fr_ps->header->aug_dyn_cross[sbgr] = aug_dyn_cr;

	/* copying sums, scf, scfsi */
	
	if (fr_ps->header->aug_dyn_cross[sbgr] == 4)
	{
	    for (sb = ((sbgr==0) ? 0:(sb_groups[sbgr-1]+1)); 
		    sb <= sb_groups[sbgr]; sb++)
	    {
		scfsi[T5[sbgr]][sb] = scfsi_sum[T56][sb];
		for (sufr = 0; sufr < 3; ++sufr)
		{
		    scalar[T5[sbgr]][sufr][sb] = scalar_sum[T56][sufr][sb];
		    for (smp = 0; smp < SCALE_BLOCK; ++smp)
			sb_sample[T5[sbgr]][sufr][smp][sb] = 
			    sb_sample_sum[T56][sufr][smp][sb];
		}
	    }
	}

	/* scfsi_dyn in scfsi of MPEG2 channels */
	/* L0 */
	switch (fr_ps->header->aug_dyn_cross[sbgr])
	{
	case 5: case 6: case 7: case 8: case 9:
	      for (sb = ((sbgr==0) ? 0:(sb_groups[sbgr-1]+1)); 
		      sb <= sb_groups[sbgr]; sb++) 
		  scfsi[L0][sb] = scfsi_dyn[L0][sb];
	      break;	    
	}
	/* R0 */
	switch (fr_ps->header->aug_dyn_cross[sbgr])
	{
	case 1: case 6: case 11: case 15:
	      for (sb = ((sbgr==0) ? 0:(sb_groups[sbgr-1]+1)); 
		      sb <= sb_groups[sbgr]; sb++) 
		  scfsi[R0][sb] = scfsi_dyn[R0][sb];
	      break;	    
	}
	/* T2 */
	switch (fr_ps->header->aug_dyn_cross[sbgr])
	{
	case 2: case 7: case 10: case 11: case 12: case 13: case 16:
	      for (sb = ((sbgr==0) ? 0:(sb_groups[sbgr-1]+1)); 
		      sb <= sb_groups[sbgr]; sb++) 
		  scfsi[T2[sbgr]][sb] = scfsi_dyn[T2[sbgr]][sb];
	      break;	    
	}
	/* T3 */
	switch (fr_ps->header->aug_dyn_cross[sbgr])
	{
	case 14: case 15: case 16: case 17: case 18:
	      for (sb = ((sbgr==0) ? 0:(sb_groups[sbgr-1]+1)); 
		      sb <= sb_groups[sbgr]; sb++) 
		  scfsi[T3[sbgr]][sb] = scfsi_dyn[T3[sbgr]][sb];
	      break;	    
	}
	/* T4 */
	switch (fr_ps->header->aug_dyn_cross[sbgr])
	{
	case 3: case 8: case 13: case 17:
	      for (sb = ((sbgr==0) ? 0:(sb_groups[sbgr-1]+1)); 
		      sb <= sb_groups[sbgr]; sb++) 
		  scfsi[T4[sbgr]][sb] = scfsi_dyn[T4[sbgr]][sb];
	      break;	    
	}
    }
}
#endif

/************************************************************************
 *
 * trans_chan
 *
 * Version 1.0	    94/06/27	Andreas Silzle, IRT
 * Version 1.1	    94/10/28	Andreas Silzle, IRT
 *
 * Which channels are in the transmission channels 
 * T2[SBGRS], T3[SBGRS], T4[SBGRS], T2outof{SBGRS] per subband group.
 * T2outof[] can be changed later by fr_ps->header->dyn_cross_LR.
 *
 ************************************************************************/

void trans_chan (frame_params *fr_ps) 		    
	    /* in:  int	fr_ps->header->tc_alloc[SBGRS], 
    	     *	    int	fr_ps->numchan,	    
	     *	    int	fr_ps->header->matrix,	    0 normal -, 3 no matricing 
	     *	    int fr_ps->header->dyn_cross_LR  0 out of L0, 1 out of R0
	     */			    
/*  global parameters
 *  out:    T2[SBGRS], T3[SBGRS], T4[SBGRS], T2outof[SBGRS], T3outof[SBGRS], T4outof[SBGRS]
 */
{  
    int sbgr;

    for (sbgr = 0; sbgr < SBGRS; sbgr++)
    {
	/* 960627 FdB tca dependent on configuration */
	if (fr_ps->config == 320)
	{
	    /* 3/2 */
	    switch(fr_ps->header->tc_alloc[sbgr]) {
	  case 0:
		T2[sbgr] = C; T3[sbgr] = LS; T4[sbgr] = RS;
		if(fr_ps->header->dyn_cross_LR == 0)
		    T2outof[sbgr] = L0;
		else
		    T2outof[sbgr] = R0;
		T3outof[sbgr] = L0;
		T4outof[sbgr] = R0;
		break;
	  case 1:
		T2[sbgr] = L; T3[sbgr] = LS; T4[sbgr] = RS;
		T2outof[sbgr] = L0;
		T3outof[sbgr] = L0;
		T4outof[sbgr] = R0;
		break;
	  case 2:
		T2[sbgr] = R; T3[sbgr] = LS; T4[sbgr] = RS;
		T2outof[sbgr] = R0;
		T3outof[sbgr] = L0;
		T4outof[sbgr] = R0;
		break;
	  case 3:
		T2[sbgr] = C; T3[sbgr] = L; T4[sbgr] = RS;
		if(fr_ps->header->dyn_cross_LR == 0)
		    T2outof[sbgr] = L0;
		else
		    T2outof[sbgr] = R0;
		T3outof[sbgr] = L0;
		T4outof[sbgr] = R0;
		break;
	  case 4:
		T2[sbgr] = C; T3[sbgr] = LS; T4[sbgr] = R;
		if(fr_ps->header->dyn_cross_LR == 0)
		    T2outof[sbgr] = L0;
		else
		    T2outof[sbgr] = R0;
		T3outof[sbgr] = L0;
		T4outof[sbgr] = R0;
		break;
	  case 5:
		T2[sbgr] = C; T3[sbgr] = L; T4[sbgr] = R;
		if(fr_ps->header->dyn_cross_LR == 0)
		    T2outof[sbgr] = L0;
		else
		    T2outof[sbgr] = R0;
		T3outof[sbgr] = L0;
		T4outof[sbgr] = R0;
		break;
	  case 6:
		T2[sbgr] = R; T3[sbgr] = L; T4[sbgr] = RS;
		T2outof[sbgr] = R0;
		T3outof[sbgr] = L0;
		T4outof[sbgr] = R0;
		break;
	  case 7:
		T2[sbgr] = L; T3[sbgr] = LS; T4[sbgr] = R;
		T2outof[sbgr] = L0;
		T3outof[sbgr] = L0;
		T4outof[sbgr] = R0;
		break;
	    }	/* switch() */
	}
	else if (fr_ps->config == 310)
	{
	    /* 3/1 */
	    switch(fr_ps->header->tc_alloc[sbgr]) {
	  case 0:
		T2[sbgr] = 2;
		T3[sbgr] = 3;
		if(fr_ps->header->dyn_cross_LR == 0)
		    T2outof[sbgr] = T3outof[sbgr] = L0;
		else
		    T2outof[sbgr] = T3outof[sbgr] = R0;
		break;
	  case 1:
		T2[sbgr] = L;
		T3[sbgr] = 3;
		T2outof[sbgr] = L0;
		if(fr_ps->header->dyn_cross_LR == 0)
		    T3outof[sbgr] = L0;
		else
		    T3outof[sbgr] = R0;
		break;
	  case 2:
		T2[sbgr] = R;
		T3[sbgr] = 3;
		T2outof[sbgr] = R0;
		if(fr_ps->header->dyn_cross_LR == 0)
		    T3outof[sbgr] = L0;
		else
		    T3outof[sbgr] = R0;
		break;
	  case 3:
		T2[sbgr] = 2;
		T3[sbgr] = L;
		if(fr_ps->header->dyn_cross_LR == 0)
		    T2outof[sbgr] = L0;
		else
		    T2outof[sbgr] = R0;
		T3outof[sbgr] = L0;
		break;
	  case 4:
		T2[sbgr] = 2;
		T3[sbgr] = R;
		if(fr_ps->header->dyn_cross_LR == 0)
		    T2outof[sbgr] = L0;
		else
		    T2outof[sbgr] = R0;
		T3outof[sbgr] = R0;
		break;
	  case 5:
		T2[sbgr] = L;
		T3[sbgr] = R;
		T2outof[sbgr] = L0;
		T3outof[sbgr] = R0;
		break;
	    }	/* switch() */
	}
	else if (fr_ps->config == 220)
	{
	    /* 2/2 */
	    switch(fr_ps->header->tc_alloc[sbgr]) {
	  case 0:
		T2[sbgr] = 2; T3[sbgr] = 3;
		break;
	  case 1:
		T2[sbgr] = L; T3[sbgr] = 3;
		break;
	  case 2:
		T2[sbgr] = 2; T3[sbgr] = R;
		break;
	  case 3:
		T2[sbgr] = L; T3[sbgr] = R;
		break;
	    }	/* switch() */
	    T2outof[sbgr] = L0;
	    T3outof[sbgr] = R0;
	}
	else if (fr_ps->config == 300 || fr_ps->config == 302)
	{
	    /* 3/0 (+ 2/0) */
	    switch(fr_ps->header->tc_alloc[sbgr]) {
	  case 0:
		T2[sbgr] = 2;
		if(fr_ps->header->dyn_cross_LR == 0)
		    T2outof[sbgr] = L0;
		else
		    T2outof[sbgr] = R0;
		break;
	  case 1:
		T2[sbgr] = L;
		T2outof[sbgr] = L0;
		break;
	  case 2:
		T2[sbgr] = R;
		T2outof[sbgr] = R0;
		break;
	    }	/* switch() */
	}
	else if (fr_ps->config == 210)
	{
	    /* 2/1 */
	    switch(fr_ps->header->tc_alloc[sbgr]) {
	  case 0:
		T2[sbgr] = 2;
		if(fr_ps->header->dyn_cross_LR == 0)
		    T2outof[sbgr] = L0;
		else
		    T2outof[sbgr] = R0;
		break;
	  case 1:
		T2[sbgr] = L;
		T2outof[sbgr] = L0;
		break;
	  case 2:
		T2[sbgr] = R;
		T2outof[sbgr] = R0;
		break;
	    }	/* switch() */
	}
	if (fr_ps->config == 302)
	{
	    /* 3/0 + 2/0 */
	    T3[sbgr] = 3; 
	    T4[sbgr] = 4; 
	    T4outof[sbgr] = 3;
	}
	else if (fr_ps->config == 202)
	{
	    /* 2/0 + 2/0 */
	    T2[sbgr] = 2; 
	    T3[sbgr] = 3; 
	    T3outof[sbgr] = 2;
	}
	else if (fr_ps->config == 102)
	{
	    /* 1/0 + 2/0 */
	    T2[sbgr] = 2; 
	    T2outof[sbgr] = 1;
	}
    }	/* for(sbgr.. */
}

#ifdef Augmentation_7ch
void trans_chan_aug (frame_params *fr_ps) 		    
	    /* in:  int	fr_ps->header->tc_alloc[SBGRS], 
    	     *	    int	fr_ps->numchan,	    
	     *	    int	fr_ps->header->matrix,	    0 normal -, 3 no matricing 
	     *	    int fr_ps->header->dyn_cross_LR  0 out of L0, 1 out of R0
	     */			    
/*  global parameters
 *  out:    T2[SBGRS], T3[SBGRS], T4[SBGRS], T2outof[SBGRS], T3outof[SBGRS], T4outof[SBGRS]
 */
{  
    int sbgr;

    for (sbgr = 0; sbgr < SBGRS; sbgr++)
    {
	/* 5/2 */
	switch (fr_ps->header->aug_tc_alloc[sbgr])
	{
	case 0:
	      T5[sbgr] = LC;
	      T6[sbgr] = RC;
	      break;
	case 1:
	      T5[sbgr] = L7;
	      T6[sbgr] = RC;
	      break;
	case 2:
	      T5[sbgr] = LC;
	      T6[sbgr] = R7;
	      break;
	case 3:
	      T5[sbgr] = L7;
	      T6[sbgr] = R7;
	      break;
	case 4:
	      T5[sbgr] = LC;
	      T6[sbgr] = C7;
	      break;
	case 5:
	      T5[sbgr] = L7;
	      T6[sbgr] = C7;
	      break;
	case 6:
	      T5[sbgr] = C7;
	      T6[sbgr] = RC;
	      break;
	case 7:
	      T5[sbgr] = C7;
	      T6[sbgr] = R7;
	      break;
	}	/* switch() */
    }
}
#endif

/************************************************************************
 *
 * combine
 *
 * Version 1.0	    94/06/27	Andreas Silzle, IRT
 *
 * Add two channels into a mono channel to use it in dynamic crosstalk.
 *
 ************************************************************************/

void combine ( 
	frame_params	*fr_ps, 		    
	    /* in:  int	fr_ps->header->tc_alloc[SBGRS], 
	     *	    int fr_ps->sblimit,			
	     */
	double	sb_sample[CHANMAX3][3][SCALE_BLOCK][SBLIMIT],	/* in */
	double	sb_sample_sum[5][3][SCALE_BLOCK][SBLIMIT]) 	
		    /* out; T23, T24, T34, T234, T56 */
{  
/*  global parameters
 *  in:	T2[SBGRS], T3[SBGRS], T4[SBGRS]
 *      sb_groups[SBGRS]
 */
    int sb, sbgr, smp, sufr;

    for (sbgr=0; sbgr < SBGRS; sbgr++)
    {
	for (sb = ((sbgr==0) ? 0:(sb_groups[sbgr-1]+1)); 
		sb <= sb_groups[sbgr]; sb++)
	{
	    for (smp = 0; smp < SCALE_BLOCK; smp++)
	    {
		for (sufr = 0; sufr < 3; ++sufr)
		{
		    if (fr_ps->config == 320 || fr_ps->config == 310 || fr_ps->config == 220 || fr_ps->config == 202)
			sb_sample_sum[T23][sufr][smp][sb] = 
			    .5 * (sb_sample[T2[sbgr]][sufr][smp][sb]
				+ sb_sample[T3[sbgr]][sufr][smp][sb]);
		    else if (fr_ps->config == 102)
			sb_sample_sum[T23][sufr][smp][sb] = 
			    .5 * (sb_sample[1       ][sufr][smp][sb]
				+ sb_sample[T2[sbgr]][sufr][smp][sb]);
		    else if (fr_ps->config == 302)
			sb_sample_sum[T23][sufr][smp][sb] = 
			    .5 * (sb_sample[T3[sbgr]][sufr][smp][sb]
				+ sb_sample[T4[sbgr]][sufr][smp][sb]);
		    if (fr_ps->config == 320)
		    {
			sb_sample_sum[T24][sufr][smp][sb] = 
			    .5 * (sb_sample[T2[sbgr]][sufr][smp][sb]
				+ sb_sample[T4[sbgr]][sufr][smp][sb]);
			sb_sample_sum[T34][sufr][smp][sb] = 
			    .5 * (sb_sample[T3[sbgr]][sufr][smp][sb]
				+ sb_sample[T4[sbgr]][sufr][smp][sb]);
			sb_sample_sum[T234][sufr][smp][sb] = 
			    .333333 * (sb_sample[T2[sbgr]][sufr][smp][sb]
				+ sb_sample[T3[sbgr]][sufr][smp][sb]
				+ sb_sample[T4[sbgr]][sufr][smp][sb]);
		    }
		}   /* for(sufr.. */
	    }	    /* for(smp.. */
	}	    /* for(sb.. */
    }		    /* for(sbgr.. */	
}

#ifdef Augmentation_7ch
void combine_aug ( 
	frame_params	*fr_ps, 		    
	    /* in:  int	fr_ps->header->tc_alloc[SBGRS], 
	     *	    int fr_ps->sblimit,			
	     */
	double	sb_sample[CHANMAX3][3][SCALE_BLOCK][SBLIMIT],	/* in */
	double	sb_sample_sum[5][3][SCALE_BLOCK][SBLIMIT]) 	
		    /* out; T23, T24, T34, T234, T56 */
{  
/*  global parameters
 *  in:	T2[SBGRS], T3[SBGRS], T4[SBGRS]
 *      sb_groups[SBGRS]
 */
    int sb, sbgr, smp, sufr;

    for (sbgr = 0; sbgr < SBGRS; sbgr++)
    {
	for (sb = ((sbgr==0) ? 0:(sb_groups[sbgr-1]+1)); 
		sb <= sb_groups[sbgr]; sb++)
	{
	    for (smp = 0; smp < SCALE_BLOCK; smp++)
	    {
		for (sufr = 0; sufr < 3; ++sufr)
		{
		    sb_sample_sum[T56][sufr][smp][sb] = 
			.5 * (sb_sample[T5[sbgr]][sufr][smp][sb]
			    + sb_sample[T6[sbgr]][sufr][smp][sb]);
		}   /* for (sufr.. */
	    }	    /* for (smp.. */
	}	    /* for (sb.. */
    }		    /* for (sbgr.. */	
}
#endif

/****************************************************************************
 *
 *  void dyn_bal
 *
 *  Add the bits for scf and scfsi of the channel with dynamic crosstalk
 *  in the case of min_ch is L0 and R0.
 *
 *  07/94	Susanne Ritscher
 *  94/10/28	Andreas Silzle	    little remake
 *
 *****************************************************************************/

void dyn_bal (
	unsigned int	scfsi[CHANMAX3][SBLIMIT],   /* in */ 
	int		sbgr,			    /* in */
	frame_params	*fr_ps,
	    /* in:  fr_ps->header->dyn_cross[sbgr] */
	int		min_ch,			    /* in */
	int		min_sb,			    /* in */
	int		*seli,			    /* in, out */
	int		*scale)			    /* in, out */
{
/*  global parameters
 *  in:	T2[SBGRS], T3[SBGRS], T4[SBGRS], T2outof[SBGRS], T3outof[SBGRS], T4outof[SBGRS]
 */
    static int	sfsPerScfsi[] = { 3,2,1,2 };    /* lookup # sfs per scfsi */
    layer *info  = fr_ps->header;
    int center   = info->center;
    int surround = info->surround;

    /* 960627 FdB DynX dependent on configuration */
    if (fr_ps->config == 320)
    {
	/* 3/2 */
	switch (fr_ps->header->dyn_cross[sbgr]) {
      case 0:	
	    break;	    /* no additional scf and scfsi */
      case 1:  
	    if (min_ch == R0) {
		*seli += 2;
		*scale += 6 * sfsPerScfsi[scfsi[T4[sbgr]][min_sb]];
	    }
	    break;
      case 2:  
	    if (min_ch == L0) {
		*seli += 2;
		*scale += 6 * sfsPerScfsi[scfsi[T3[sbgr]][min_sb]];
	    }
	    break;
      case 3: 
	    if (min_ch == T2outof[sbgr] && (center != 3 || sbgr < 10)) {
		*seli += 2;
		*scale += 6 * sfsPerScfsi[scfsi[T2[sbgr]][min_sb]];
	    }
	    break;
      case 4: 
	    if (min_ch == L0) {
		*seli += 2;
		*scale += 6 * sfsPerScfsi[scfsi[T3[sbgr]][min_sb]];
	    }
	    if (min_ch == R0) {
		*seli += 2;
		*scale += 6 * sfsPerScfsi[scfsi[T4[sbgr]][min_sb]];
	    }
	    break;
      case 5:  
	    if (min_ch == T2outof[sbgr] && (center != 3 || sbgr < 10)) {
		*seli += 2;
		*scale += 6 * sfsPerScfsi[scfsi[T2[sbgr]][min_sb]];
	    }
	    if (min_ch == R0) {
		*seli += 2;
		*scale += 6 * sfsPerScfsi[scfsi[T4[sbgr]][min_sb]];
	    }
	    break;
      case 6: 
	    if (min_ch == T2outof[sbgr] && (center != 3 || sbgr < 10)) {
		*seli += 2;
		*scale += 6 * sfsPerScfsi[scfsi[T2[sbgr]][min_sb]];
	    }
	    if (min_ch == L0) {
		*seli += 2;
		*scale += 6 * sfsPerScfsi[scfsi[T3[sbgr]][min_sb]];
	    }
	    break;
      case 7: 
	    if (min_ch == T2outof[sbgr] && (center != 3 || sbgr < 10)) {
		*seli += 2;
		*scale += 6 * sfsPerScfsi[scfsi[T2[sbgr]][min_sb]];
	    }
	    if (min_ch == L0) {
		*seli += 2;
		*scale += 6 * sfsPerScfsi[scfsi[T3[sbgr]][min_sb]];
	    }
	    if (min_ch == R0) {
		*seli += 2;
		*scale += 6 * sfsPerScfsi[scfsi[T4[sbgr]][min_sb]];
	    }
	    break;
      case 8:
	    if (min_ch == T3[sbgr]) {
		*seli += 2;
		*scale += 6 * sfsPerScfsi[scfsi[T4[sbgr]][min_sb]];
	    }
	    break;
      case 9:  
	    if (min_ch == T2[sbgr]) {
		*seli += 2;
		*scale += 6 * sfsPerScfsi[scfsi[T3[sbgr]][min_sb]];
	    }
	    break;
      case 10:  
	    if (min_ch == T2[sbgr]) {
		*seli += 2;
		*scale += 6 * sfsPerScfsi[scfsi[T4[sbgr]][min_sb]];
	    }
	    break;
      case 11:  
	    if (min_ch == T2[sbgr]) {
		*seli += 2;
		*scale += 6 * sfsPerScfsi[scfsi[T3[sbgr]][min_sb]];
	    }
	    if (min_ch == R0) {
		*seli += 2;
		*scale += 6 * sfsPerScfsi[scfsi[T4[sbgr]][min_sb]];
	    }
	    break;
      case 12:  
	    if (min_ch == T2[sbgr]) {
		*seli += 2;
		*scale += 6 * sfsPerScfsi[scfsi[T4[sbgr]][min_sb]];
	    }
	    if (min_ch == L0) {
		*seli += 2;
		*scale += 6 * sfsPerScfsi[scfsi[T3[sbgr]][min_sb]];
	    }
	    break;
      case 13:  
	    if (min_ch == T2outof[sbgr] && (center != 3 || sbgr < 10)) {
		*seli += 2;
		*scale += 6 * sfsPerScfsi[scfsi[T2[sbgr]][min_sb]];
	    }
	    if (min_ch == T3[sbgr]) {
		*seli += 2;
		*scale += 6 * sfsPerScfsi[scfsi[T4[sbgr]][min_sb]];
	    }
	    break;
      case 14:  
	    if (min_ch == T2[sbgr]) {
		*seli += 4;
		*scale += 6 * sfsPerScfsi[scfsi[T3[sbgr]][min_sb]];
		*scale += 6 * sfsPerScfsi[scfsi[T4[sbgr]][min_sb]];
	    }
	    break;
      case 15: 
	    fprintf(stderr, "FORBIDDEN DYN_CROSS!!!!!\n");
	    fflush(stderr);
	    exit(0);
	    break;
	}	    /* switch.. */
    }
    else if (fr_ps->config == 310 || fr_ps->config == 220)
    {
	/* 3/1 and 2/2 */
	switch (fr_ps->header->dyn_cross[sbgr]) {
      case 0:	
	    break;	    /* no additional scf and scfsi */
      case 1:  
	    if (min_ch == T3outof[sbgr]) {
		*seli += 2;
		*scale += 6 * sfsPerScfsi[scfsi[T3[sbgr]][min_sb]];
	    }
	    break;
      case 2:  
	    if (min_ch == T2outof[sbgr] && (center != 3 || sbgr < 10)) {
		*seli += 2;
		*scale += 6 * sfsPerScfsi[scfsi[T2[sbgr]][min_sb]];
	    }
	    break;
      case 3: 
	    if (min_ch == T2outof[sbgr] && (center != 3 || sbgr < 10)) {
		*seli += 2;
		*scale += 6 * sfsPerScfsi[scfsi[T2[sbgr]][min_sb]];
	    }
	    if (min_ch == T3outof[sbgr]) {
		*seli += 2;
		*scale += 6 * sfsPerScfsi[scfsi[T3[sbgr]][min_sb]];
	    }
	    break;
      case 4:  
	    if (min_ch == T2[sbgr]) {
		*seli += 2;
		*scale += 6 * sfsPerScfsi[scfsi[T3[sbgr]][min_sb]];
	    }
	    break;
      default: 
	    fprintf(stderr, "FORBIDDEN DYN_CROSS!!!!!\n");
	    fflush(stderr);
	    exit(0);
	    break;
	}	    /* switch.. */
    }
    else if (fr_ps->config == 300 || fr_ps->config == 302 || fr_ps->config == 210)
    {
	/* 3/0 (+2/0) and 2/1 */
	switch(fr_ps->header->dyn_cross[sbgr]) {
      case 0:	
	    break;	    /* no additional scf and scfsi */
      case 1:  
	    if (min_ch == T2outof[sbgr] && (center != 3 || sbgr < 10)) {
		*seli += 2;
		*scale += 6 * sfsPerScfsi[scfsi[T2[sbgr]][min_sb]];
	    }
	    break;
      default: 
	    fprintf(stderr, "FORBIDDEN DYN_CROSS!!!!!\n");
	    fflush(stderr);
	    exit(0);
	    break;
	}	    /* switch.. */
    
	if (fr_ps->config == 302 && info->dyn_second_stereo[sbgr] && min_ch == T4outof[sbgr])
	{
	    *seli += 2;
	    *scale += 6 * sfsPerScfsi[scfsi[T4[sbgr]][min_sb]];
	}
    }
    else if (fr_ps->config == 202 && info->dyn_second_stereo[sbgr] && min_ch == T3outof[sbgr])
    {
	    *seli += 2;
	    *scale += 6 * sfsPerScfsi[scfsi[T3[sbgr]][min_sb]];
    }
    else if (fr_ps->config == 102 && info->dyn_second_stereo[sbgr] && min_ch == T2outof[sbgr])
    {
	    *seli += 2;
	    *scale += 6 * sfsPerScfsi[scfsi[T2[sbgr]][min_sb]];
    }
}


#ifdef Augmentation_7ch
void dyn_bal_aug (
	unsigned int	scfsi[CHANMAX3][SBLIMIT],   /* in */ 
	int		sbgr,			    /* in */
	frame_params	*fr_ps,			    /* in:  fr_ps->header->dyn_cross[sbgr] */
	int		min_ch,			    /* in */
	int		min_sb,			    /* in */
	int		*seli,			    /* in, out */
	int		*scale)			    /* in, out */
{
/*  global parameters
 *  in:	T2[SBGRS], T3[SBGRS], T4[SBGRS], T5[SBGRS]
 */
    static int	sfsPerScfsi[] = { 3,2,1,2 };    /* lookup # sfs per scfsi */
    int dynx_T2 = !dyn_ch (fr_ps, sbgr, 2);
    int dynx_T3 = !dyn_ch (fr_ps, sbgr, 3);
    int dynx_T4 = !dyn_ch (fr_ps, sbgr, 4);
    int T5T0 = L0, T6T0 = L0, T6T1 = R0;
    
    if (fr_ps->header->aug_tc_alloc[sbgr] == 4 || fr_ps->header->aug_tc_alloc[sbgr] == 5)
    	if (fr_ps->header->dyn_cross_LR == 0)
	    T6T1 = L0;
	else
	    T6T0 = R0;
    else if (fr_ps->header->aug_tc_alloc[sbgr] == 6 || fr_ps->header->aug_tc_alloc[sbgr] == 7)
        if (fr_ps->header->dyn_cross_LR)
	    T5T0 = R0;

    /* 5/2 */
    switch (fr_ps->header->aug_dyn_cross[sbgr])
    {
    case  0:
	break;	    /* no additional scf and scfsi */
    case  1:
	if (min_ch == T6T1)
	{
	    *seli += 2;
	    *scale += 6 * sfsPerScfsi[scfsi[T6[sbgr]][min_sb]];
	}
	break;
    case  2:
	if (min_ch == T2[sbgr] || (dynx_T2 && min_ch == T2from[sbgr]))
	{
	    *seli += 2;
	    *scale += 6 * sfsPerScfsi[scfsi[T6[sbgr]][min_sb]];
	}
	break;
    case  3:
	if (min_ch == T4[sbgr] || (dynx_T4 && min_ch == T4from[sbgr]))
	{
	    *seli += 2;
	    *scale += 6 * sfsPerScfsi[scfsi[T6[sbgr]][min_sb]];
	}
	break;
    case  4:
	if (min_ch == T5[sbgr])
	{
	    *seli += 2;
	    *scale += 6 * sfsPerScfsi[scfsi[T6[sbgr]][min_sb]];
	}
	break;
    case  5:
	if (min_ch == T5T0)
	{
	    *seli += 2;
	    *scale += 6 * sfsPerScfsi[scfsi[T5[sbgr]][min_sb]];
	}
	break;
    case  6:
	if (min_ch == T5T0)
	{
	    *seli += 2;
	    *scale += 6 * sfsPerScfsi[scfsi[T5[sbgr]][min_sb]];
	}
	else if (min_ch == T6T1)
	{
	    *seli += 2;
	    *scale += 6 * sfsPerScfsi[scfsi[T6[sbgr]][min_sb]];
	}
	break;
    case  7:
	if (min_ch == T5T0)
	{
	    *seli += 2;
	    *scale += 6 * sfsPerScfsi[scfsi[T5[sbgr]][min_sb]];
	}
	if (min_ch == T2[sbgr] || (dynx_T2 && min_ch == T2from[sbgr]))
	{
	    *seli += 2;
	    *scale += 6 * sfsPerScfsi[scfsi[T6[sbgr]][min_sb]];
	}
	break;
    case  8:
	if (min_ch == T5T0)
	{
	    *seli += 2;
	    *scale += 6 * sfsPerScfsi[scfsi[T5[sbgr]][min_sb]];
	}
	if (min_ch == T4[sbgr] || (dynx_T4 && min_ch == T4from[sbgr]))
	{
	    *seli += 2;
	    *scale += 6 * sfsPerScfsi[scfsi[T6[sbgr]][min_sb]];
	}
	break;
    case  9:
	if (min_ch == T5T0)
	{
	    *seli += 2;
	    *scale += 6 * sfsPerScfsi[scfsi[T5[sbgr]][min_sb]];
	}
	if (min_ch == T6T0)
	{
	    *seli += 2;
	    *scale += 6 * sfsPerScfsi[scfsi[T6[sbgr]][min_sb]];
	}
	break;
    case 10:
	if (min_ch == T2[sbgr] || (dynx_T2 && min_ch == T2from[sbgr]))
	{
	    *seli += 2;
	    *scale += 6 * sfsPerScfsi[scfsi[T5[sbgr]][min_sb]];
	}
	break;
    case 11:
	if (min_ch == T2[sbgr] || (dynx_T2 && min_ch == T2from[sbgr]))
	{
	    *seli += 2;
	    *scale += 6 * sfsPerScfsi[scfsi[T5[sbgr]][min_sb]];
	}
	if (min_ch == T6T1)
	{
	    *seli += 2;
	    *scale += 6 * sfsPerScfsi[scfsi[T6[sbgr]][min_sb]];
	}
	break;
    case 12:
	if (min_ch == T2[sbgr] || (dynx_T2 && min_ch == T2from[sbgr]))
	{
	    *seli += 4;
	    *scale += 6 * sfsPerScfsi[scfsi[T5[sbgr]][min_sb]];
	    *scale += 6 * sfsPerScfsi[scfsi[T6[sbgr]][min_sb]];
	}
	break;
    case 13:
	if (min_ch == T2[sbgr] || (dynx_T2 && min_ch == T2from[sbgr]))
	{
	    *seli += 2;
	    *scale += 6 * sfsPerScfsi[scfsi[T5[sbgr]][min_sb]];
	}
	if (min_ch == T4[sbgr] || (dynx_T4 && min_ch == T4from[sbgr]))
	{
	    *seli += 2;
	    *scale += 6 * sfsPerScfsi[scfsi[T6[sbgr]][min_sb]];
	}
	break;
    case 14:
	if (min_ch == T3[sbgr] || (dynx_T3 && min_ch == T3from[sbgr]))
	{
	    *seli += 2;
	    *scale += 6 * sfsPerScfsi[scfsi[T5[sbgr]][min_sb]];
	}
	break;
    case 15:
	if (min_ch == T3[sbgr] || (dynx_T3 && min_ch == T3from[sbgr]))
	{
	    *seli += 2;
	    *scale += 6 * sfsPerScfsi[scfsi[T5[sbgr]][min_sb]];
	}
	if (min_ch == T6T1)
	{
	    *seli += 2;
	    *scale += 6 * sfsPerScfsi[scfsi[T6[sbgr]][min_sb]];
	}
	break;
    case 16:
	if (min_ch == T3[sbgr] || (dynx_T3 && min_ch == T3from[sbgr]))
	{
	    *seli += 2;
	    *scale += 6 * sfsPerScfsi[scfsi[T5[sbgr]][min_sb]];
	}
	if (min_ch == T2[sbgr] || (dynx_T2 && min_ch == T2from[sbgr]))
	{
	    *seli += 2;
	    *scale += 6 * sfsPerScfsi[scfsi[T6[sbgr]][min_sb]];
	}
	break;
    case 17:
	if (min_ch == T3[sbgr] || (dynx_T3 && min_ch == T3from[sbgr]))
	{
	    *seli += 2;
	    *scale += 6 * sfsPerScfsi[scfsi[T5[sbgr]][min_sb]];
	}
	if (min_ch == T4[sbgr] || (dynx_T4 && min_ch == T4from[sbgr]))
	{
	    *seli += 2;
	    *scale += 6 * sfsPerScfsi[scfsi[T6[sbgr]][min_sb]];
	}
	break;
    case 18:
	if (min_ch == T3[sbgr] || (dynx_T3 && min_ch == T3from[sbgr]))
	{
	    *seli += 4;
	    *scale += 6 * sfsPerScfsi[scfsi[T5[sbgr]][min_sb]];
	    *scale += 6 * sfsPerScfsi[scfsi[T6[sbgr]][min_sb]];
	}
	break;
  default: 
	fprintf (stderr, "FORBIDDEN AUG_DYN_CROSS!!!!!\n");
	fflush (stderr);
	exit (1);
	break;
    }
}
#endif


/****************************************************************************
 *
 *  void choose_dyn
 *
 *  bit_alloc of the dynamic crosstalk channels must be have a value; 
 *  only for sending scf in datastream.
 *
 *  07/94	Susanne Ritscher
 *  94/10/31	Andreas Silzle	    little remake
 *  94/12/30	Andreas Silzle	    
 *		    fill only up to that subband of the basic channel
 *		    which is unequal zero.
 *
 *****************************************************************************/

void choose_dyn (
	frame_params	*fr_ps, 
	    /* in:  fr_ps->header->dyn_cross[sbgr] 
	     *	    fr_ps->alloc
	     */
	int		min_ch,				/* in */
	int		min_sb,				/* in */
	int		sbgr, 				/* in */
	unsigned int	bit_alloc[CHANMAX3][SBLIMIT])	/* out */
{
/*  global parameters
 *  in:	T2[SBGRS], T3[SBGRS], T4[SBGRS], T2outof[SBGRS]
 */
    layer *info  = fr_ps->header;
    int center   = info->center;
    int surround = info->surround;

/* 960627 FdB DynX dependent on configuration */
if (fr_ps->config == 320)
{
    /* 3/2 */
    switch(fr_ps->header->dyn_cross[sbgr]) {
  case 0:	
	break;
  case 1: 
	if(min_ch == R0 && bit_alloc[R0][min_sb] != 0)
	    bit_alloc[T4[sbgr]][min_sb] = 1;
	break;
  case 2: 
	if(min_ch == L0 && bit_alloc[L0][min_sb] != 0)
	    bit_alloc[T3[sbgr]][min_sb] = 1;
	break;
  case 3: 
	if(min_ch == T2outof[sbgr] && bit_alloc[T2outof[sbgr]][min_sb] != 0)
	       bit_alloc[T2[sbgr]][min_sb] = 1;
	break;
  case 4: 
	if(min_ch == R0 && bit_alloc[R0][min_sb] != 0)
	    bit_alloc[T4[sbgr]][min_sb] = 1;
	if(min_ch == L0 && bit_alloc[L0][min_sb] != 0)
	    bit_alloc[T3[sbgr]][min_sb] = 1;
	break;
  case 5: 
	if(min_ch == T2outof[sbgr] && bit_alloc[T2outof[sbgr]][min_sb] != 0) 
	    bit_alloc[T2[sbgr]][min_sb] = 1;
	if(min_ch == R0 && bit_alloc[R0][min_sb] != 0) 
	    bit_alloc[T4[sbgr]][min_sb] = 1;
	break;
  case 6: 
	if(min_ch == T2outof[sbgr] && bit_alloc[T2outof[sbgr]][min_sb] != 0) 
	    bit_alloc[T2[sbgr]][min_sb] = 1;
	if(min_ch == L0 && bit_alloc[L0][min_sb] != 0) 
	    bit_alloc[T3[sbgr]][min_sb] = 1;
	break;
  case 7: 
	if(min_ch == T2outof[sbgr] && bit_alloc[T2outof[sbgr]][min_sb] != 0) 
	    bit_alloc[T2[sbgr]][min_sb] = 1;
	if(min_ch == L0 && bit_alloc[L0][min_sb] != 0) 
	    bit_alloc[T3[sbgr]][min_sb] = 1;
	if(min_ch == R0 && bit_alloc[R0][min_sb] != 0) 
	    bit_alloc[T4[sbgr]][min_sb] = 1;
	break;
  case 8: 
	if(min_ch == T3[sbgr] && bit_alloc[T3[sbgr]][min_sb] != 0)
	    bit_alloc[T4[sbgr]][min_sb] = 1;
	break;
  case 9: 
	if(min_ch == T2[sbgr] && bit_alloc[T2[sbgr]][min_sb] != 0)
	    bit_alloc[T3[sbgr]][min_sb] = 1;
	break;
 case 10: 
	if(min_ch == T2[sbgr] && bit_alloc[T2[sbgr]][min_sb] != 0)
	    bit_alloc[T4[sbgr]][min_sb] = 1;
	break;
 case 11: 
	if(min_ch == T2[sbgr] && bit_alloc[T2[sbgr]][min_sb] != 0)
	    bit_alloc[T3[sbgr]][min_sb] = 1;
	if(min_ch == R0 && bit_alloc[R0][min_sb] != 0)
	    bit_alloc[T4[sbgr]][min_sb] = 1;
	break;
 case 12: 
	if(min_ch == T2[sbgr] && bit_alloc[T2[sbgr]][min_sb] != 0)
	    bit_alloc[T4[sbgr]][min_sb] = 1;
	if(min_ch == L0 && bit_alloc[L0][min_sb] != 0)
	    bit_alloc[T3[sbgr]][min_sb] = 1;
	break;
 case 13: 
	if(min_ch == T2outof[sbgr] && bit_alloc[T2outof[sbgr]][min_sb] != 0) 
	    bit_alloc[T2[sbgr]][min_sb] = 1;
	if(min_ch == T3[sbgr] && bit_alloc[T3[sbgr]][min_sb] != 0)
	    bit_alloc[T4[sbgr]][min_sb] = 1;
	break;
 case 14: 
	if(min_ch == T2[sbgr] && bit_alloc[T2[sbgr]][min_sb] != 0) {
	    bit_alloc[T3[sbgr]][min_sb] = 1;
	    bit_alloc[T4[sbgr]][min_sb] = 1;
	}
	break;
 case 15: 
	fprintf(stderr, "FORBIDDEN DYN_CROSS!!!!!\n");
	fflush(stderr);
	exit(0);
	break;
    }	    /* switch.. */
}
else if (fr_ps->config == 310 || fr_ps->config == 220)
{
    /* 3/1 and 2/2 */
    switch(fr_ps->header->dyn_cross[sbgr]) {
  case 0:	
	break;
  case 1: 
	if(min_ch == T3outof[sbgr] && bit_alloc[T3outof[sbgr]][min_sb] != 0)
	    bit_alloc[T3[sbgr]][min_sb] = 1;
	break;
  case 2: 
	if(min_ch == T2outof[sbgr] && bit_alloc[T2outof[sbgr]][min_sb] != 0)
	    bit_alloc[T2[sbgr]][min_sb] = 1;
	break;
  case 3: 
	if(min_ch == T2outof[sbgr] && bit_alloc[T2outof[sbgr]][min_sb] != 0) 
	    bit_alloc[T2[sbgr]][min_sb] = 1;
	if(min_ch == T3outof[sbgr] && bit_alloc[T3outof[sbgr]][min_sb] != 0) 
	    bit_alloc[T3[sbgr]][min_sb] = 1;
	break;
  case 4: 
	if(min_ch == T2[sbgr] && bit_alloc[T2[sbgr]][min_sb] != 0)
	    bit_alloc[T3[sbgr]][min_sb] = 1;
	break;
 default: 
	fprintf(stderr, "FORBIDDEN DYN_CROSS!!!!!\n");
	fflush(stderr);
	exit(0);
	break;
    }	    /* switch.. */
}
else if (fr_ps->config == 300 || fr_ps->config == 302 || fr_ps->config == 210)
{
    /* 3/0 (+2/0) and 2/1 */
    switch(fr_ps->header->dyn_cross[sbgr]) {
  case 0:	
	break;
  case 1: 
	if(min_ch == T2outof[sbgr] && bit_alloc[T2outof[sbgr]][min_sb] != 0)
	    bit_alloc[T2[sbgr]][min_sb] = 1;
	break;
 default: 
	fprintf(stderr, "FORBIDDEN DYN_CROSS!!!!!\n");
	fflush(stderr);
	exit(0);
	break;
    }	    /* switch.. */
    if (fr_ps->header->dyn_second_stereo[sbgr])
	if(min_ch == T4outof[sbgr] && bit_alloc[T4outof[sbgr]][min_sb] != 0)
	    bit_alloc[T4[sbgr]][min_sb] = 1;
}
else if (fr_ps->config == 202)
{
    if (fr_ps->header->dyn_second_stereo[sbgr])
	if(min_ch == T3outof[sbgr] && bit_alloc[T3outof[sbgr]][min_sb] != 0)
	    bit_alloc[T3[sbgr]][min_sb] = 1;
}
else if (fr_ps->config == 102)
{
    if (fr_ps->header->dyn_second_stereo[sbgr])
	if(min_ch == T2outof[sbgr] && bit_alloc[T2outof[sbgr]][min_sb] != 0)
	    bit_alloc[T2[sbgr]][min_sb] = 1;
}
}

#ifdef Augmentation_7ch
void choose_dyn_aug (
	frame_params	*fr_ps, 
	    /* in:  fr_ps->header->dyn_cross[sbgr] 
	     *	    fr_ps->alloc
	     */
	int		min_ch,				/* in */
	int		min_sb,				/* in */
	int		sbgr, 				/* in */
	unsigned int	bit_alloc[CHANMAX3][SBLIMIT])	/* out */
{
/*  global parameters
 *  in:	T2[SBGRS], T3[SBGRS], T4[SBGRS], T2outof[SBGRS]
 */
    layer *info  = fr_ps->header;
    int dynx_T2 = !dyn_ch (fr_ps, sbgr, 2);
    int dynx_T3 = !dyn_ch (fr_ps, sbgr, 3);
    int dynx_T4 = !dyn_ch (fr_ps, sbgr, 4);
    int T5T0 = L0, T6T0 = L0, T6T1 = R0;
    
    if (info->aug_tc_alloc[sbgr] == 4 || info->aug_tc_alloc[sbgr] == 5)
    	if (fr_ps->header->dyn_cross_LR == 0)
	    T6T1 = L0;
	else
	    T6T0 = R0;
    else if (info->aug_tc_alloc[sbgr] == 6 || info->aug_tc_alloc[sbgr] == 7)
        if (info->dyn_cross_LR)
	    T5T0 = R0;

    /* 5/2 */
    switch (fr_ps->header->aug_dyn_cross[sbgr])
    {
    case 0:	
	  break;
    case 1: 
	  if (min_ch == T6T1 && bit_alloc[T6T1][min_sb] != 0)
	      bit_alloc[T6[sbgr]][min_sb] = 1;
	  break;
    case 2: 
	  if ((min_ch == T2[sbgr] && bit_alloc[T2[sbgr]][min_sb] != 0) ||
	      (dynx_T2 && min_ch == T2from[sbgr] && bit_alloc[T2from[sbgr]][min_sb] != 0))
	      bit_alloc[T6[sbgr]][min_sb] = 1;
	  break;
    case 3: 
	  if ((min_ch == T4[sbgr] && bit_alloc[T4[sbgr]][min_sb] != 0) ||
	      (dynx_T4 && min_ch == T4from[sbgr] && bit_alloc[T4from[sbgr]][min_sb] != 0))
	      bit_alloc[T6[sbgr]][min_sb] = 1;
	  break;
    case 4: 
	  if (min_ch == T5[sbgr] && bit_alloc[T5[sbgr]][min_sb] != 0)
	      bit_alloc[T6[sbgr]][min_sb] = 1;
	  break;
    case 5: 
	  if (min_ch == T5T0 && bit_alloc[T5T0][min_sb] != 0) 
	      bit_alloc[T5[sbgr]][min_sb] = 1;
	  break;
    case 6: 
	  if (min_ch == T5T0 && bit_alloc[T5T0][min_sb] != 0) 
	      bit_alloc[T5[sbgr]][min_sb] = 1;
	  if (min_ch == T6T1 && bit_alloc[T6T1][min_sb] != 0) 
	      bit_alloc[T6[sbgr]][min_sb] = 1;
	  break;
    case 7: 
	  if (min_ch == T5T0 && bit_alloc[T5T0][min_sb] != 0) 
	      bit_alloc[T5[sbgr]][min_sb] = 1;
	  if ((min_ch == T2[sbgr] && bit_alloc[T2[sbgr]][min_sb] != 0) ||
	      (dynx_T2 && min_ch == T2from[sbgr] && bit_alloc[T2from[sbgr]][min_sb] != 0))
	      bit_alloc[T6[sbgr]][min_sb] = 1;
	  break;
    case 8: 
	  if (min_ch == T5T0 && bit_alloc[T5T0][min_sb] != 0) 
	      bit_alloc[T5[sbgr]][min_sb] = 1;
	  if ((min_ch == T4[sbgr] && bit_alloc[T4[sbgr]][min_sb] != 0) ||
	      (dynx_T4 && min_ch == T4from[sbgr] && bit_alloc[T4from[sbgr]][min_sb] != 0))
	      bit_alloc[T6[sbgr]][min_sb] = 1;
	  break;
    case 9: 
	  if (min_ch == T5T0 && bit_alloc[T5T0][min_sb] != 0)
	      bit_alloc[T5[sbgr]][min_sb] = 1;
	  if (min_ch == T6T0 && bit_alloc[T6T0][min_sb] != 0)
	      bit_alloc[T6[sbgr]][min_sb] = 1;
	  break;
   case 10: 
	  if ((min_ch == T2[sbgr] && bit_alloc[T2[sbgr]][min_sb] != 0) ||
	      (dynx_T2 && min_ch == T2from[sbgr] && bit_alloc[T2from[sbgr]][min_sb] != 0))
	      bit_alloc[T5[sbgr]][min_sb] = 1;
	  break;
   case 11: 
	  if ((min_ch == T2[sbgr] && bit_alloc[T2[sbgr]][min_sb] != 0) ||
	      (dynx_T2 && min_ch == T2from[sbgr] && bit_alloc[T2from[sbgr]][min_sb] != 0))
	      bit_alloc[T5[sbgr]][min_sb] = 1;
	  if (min_ch == T6T1 && bit_alloc[T6T1][min_sb] != 0)
	      bit_alloc[T6[sbgr]][min_sb] = 1;
	  break;
   case 12: 
	  if ((min_ch == T2[sbgr] && bit_alloc[T2[sbgr]][min_sb] != 0) ||
	      (dynx_T2 && min_ch == T2from[sbgr] && bit_alloc[T2from[sbgr]][min_sb] != 0))
	      bit_alloc[T5[sbgr]][min_sb] = bit_alloc[T6[sbgr]][min_sb] = 1;
	  break;
   case 13: 
	  if ((min_ch == T2[sbgr] && bit_alloc[T2[sbgr]][min_sb] != 0) ||
	      (dynx_T2 && min_ch == T2from[sbgr] && bit_alloc[T2from[sbgr]][min_sb] != 0))
	      bit_alloc[T5[sbgr]][min_sb] = 1;
	  if ((min_ch == T4[sbgr] && bit_alloc[T4[sbgr]][min_sb] != 0) ||
	      (dynx_T4 && min_ch == T4from[sbgr] && bit_alloc[T4from[sbgr]][min_sb] != 0))
	      bit_alloc[T6[sbgr]][min_sb] = 1;
	  break;
   case 14: 
	  if ((min_ch == T3[sbgr] && bit_alloc[T3[sbgr]][min_sb] != 0) ||
	      (dynx_T3 && min_ch == T3from[sbgr] && bit_alloc[T3from[sbgr]][min_sb] != 0))
	      bit_alloc[T5[sbgr]][min_sb] = 1;
	  break;
   case 15: 
	  if ((min_ch == T3[sbgr] && bit_alloc[T3[sbgr]][min_sb] != 0) ||
	      (dynx_T3 && min_ch == T3from[sbgr] && bit_alloc[T3from[sbgr]][min_sb] != 0))
	      bit_alloc[T5[sbgr]][min_sb] = 1;
	  if (min_ch == T6T1 && bit_alloc[T6T1][min_sb] != 0)
	      bit_alloc[T6[sbgr]][min_sb] = 1;
	  break;
   case 16: 
	  if ((min_ch == T3[sbgr] && bit_alloc[T3[sbgr]][min_sb] != 0) ||
	      (dynx_T3 && min_ch == T3from[sbgr] && bit_alloc[T3from[sbgr]][min_sb] != 0))
	      bit_alloc[T5[sbgr]][min_sb] = 1;
	  if ((min_ch == T2[sbgr] && bit_alloc[T2[sbgr]][min_sb] != 0) ||
	      (dynx_T2 && min_ch == T2from[sbgr] && bit_alloc[T2from[sbgr]][min_sb] != 0))
	      bit_alloc[T6[sbgr]][min_sb] = 1;
	  break;
   case 17: 
	  if ((min_ch == T3[sbgr] && bit_alloc[T3[sbgr]][min_sb] != 0) ||
	      (dynx_T3 && min_ch == T3from[sbgr] && bit_alloc[T3from[sbgr]][min_sb] != 0))
	      bit_alloc[T5[sbgr]][min_sb] = 1;
	  if ((min_ch == T4[sbgr] && bit_alloc[T4[sbgr]][min_sb] != 0) ||
	      (dynx_T4 && min_ch == T4from[sbgr] && bit_alloc[T4from[sbgr]][min_sb] != 0))
	      bit_alloc[T6[sbgr]][min_sb] = 1;
	  break;
   case 18:
	  if ((min_ch == T3[sbgr] && bit_alloc[T3[sbgr]][min_sb] != 0) ||
	      (dynx_T3 && min_ch == T3from[sbgr] && bit_alloc[T3from[sbgr]][min_sb] != 0))
	      bit_alloc[T5[sbgr]][min_sb] = bit_alloc[T6[sbgr]][min_sb] = 1;
	  break;
   default: 
	  fprintf (stderr, "FORBIDDEN AUG_DYN_CROSS!!!!!\n");
	  fflush (stderr);
	  exit (1);
	  break;
    }
}
#endif


/************************************************************************
 *
 * scfsi_calc_dyn    
 *
 * Version 1.0	    94/07/07	Andreas Silzle, IRT
 *
 * Calculating the scale factor select information of one channel 
 * without changing the scale factors.
 *
 ************************************************************************/

void scfsi_calc_dyn (
	unsigned int	scalar_dyn[][3][SBLIMIT],   /* in/out */
	int		ch, 			    /* in */
	int		sblimit,		    /* in */ 
	unsigned int	scfsi_dyn[][SBLIMIT]) 	    /* out */
{
    int	    sb;
    
    for (sb = 0; sb < sblimit; sb++)
    {
	/* If the subbands are coded only with SCF's, they must be accurate. */
	/* 1993-02-19 and 1994-03-17 shn */
	/* 4 patterns are possible: 0: ABC, 1: AAB, 2: AAA, 3: ABB */
	if ((scalar_dyn[ch][0][sb] != scalar_dyn[ch][1][sb]) &&
	    (scalar_dyn[ch][1][sb] != scalar_dyn[ch][2][sb]))
	    scfsi_dyn[ch][sb] = 0;
	else
	if ((scalar_dyn[ch][0][sb] == scalar_dyn[ch][1][sb]) &&
	    (scalar_dyn[ch][1][sb] != scalar_dyn[ch][2][sb]))
	    scfsi_dyn[ch][sb] = 1;
	else
	if ((scalar_dyn[ch][0][sb] == scalar_dyn[ch][1][sb]) &&
	    (scalar_dyn[ch][1][sb] == scalar_dyn[ch][2][sb]))
	    scfsi_dyn[ch][sb] = 2;
	else
	    scfsi_dyn[ch][sb] = 3;
    }	/* for (sb.. */
}

/************************************************************************
 *
 * scfsi_calc    
 *
 * Version 1.0	    94/07/07	Andreas Silzle, IRT
 *
 * Calculating the scale factor select information of one channel. 
 *
 * route: II_transmission_pattern (Layer II only)
 *
 * PURPOSE:For a given subband, determines whether to send 1, 2, or
 * all 3 of the scalefactors, and fills in the scalefactor
 * select information accordingly
 *
 * SEMANTICS:  The subbands and channels are classified based on how much
 * the scalefactors changes over its three values (corresponding
 * to the 3 sets of 12 samples per subband).  The classification
 * will send 1 or 2 scalefactors instead of three if the scalefactors
 * do not change much.  The scalefactor select information,
 * #scfsi#, is filled in accordingly.
 *
 ************************************************************************/

void scfsi_calc (
	unsigned int	scalar[][3][SBLIMIT], 	/* in/out */
	int		ch, 			/* in */
	int		sblimit,		/* in */ 
	unsigned int	scfsi[][SBLIMIT]) 	/* out */
{
    int dscf[2];
    int class[2], sb, j;
    static int pattern[5][5] = {
	0x123, 0x122, 0x122, 0x133, 0x123,
	0x113, 0x111, 0x111, 0x444, 0x113,
	0x111, 0x111, 0x111, 0x333, 0x113,
	0x222, 0x222, 0x222, 0x333, 0x123,
	0x123, 0x122, 0x122, 0x133, 0x123    };

    for (sb = 0; sb < sblimit; sb++)
    {
	dscf[0] = (scalar[ch][0][sb] - scalar[ch][1][sb]);
	dscf[1] = (scalar[ch][1][sb] - scalar[ch][2][sb]);
	for (j=0; j<2; j++)
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
	case 0x123 :
	    scfsi[ch][sb] = 0;
	    break;
	case 0x122 :
	    scfsi[ch][sb] = 3;
	    scalar[ch][2][sb] = scalar[ch][1][sb];
	    break;
	case 0x133 :
	    scfsi[ch][sb] = 3;
	    scalar[ch][1][sb] = scalar[ch][2][sb];
	    break;
	case 0x113 :
	    scfsi[ch][sb] = 1;
	    scalar[ch][1][sb] = scalar[ch][0][sb];
	    break;
	case 0x111 :
	    scfsi[ch][sb] = 2;
	    scalar[ch][1][sb] = scalar[ch][2][sb] = scalar[ch][0][sb];
	    break;
	case 0x222 :
	    scfsi[ch][sb] = 2;
	    scalar[ch][0][sb] = scalar[ch][2][sb] = scalar[ch][1][sb];
	    break;
	case 0x333 :
	    scfsi[ch][sb] = 2;
	    scalar[ch][0][sb] = scalar[ch][1][sb] = scalar[ch][2][sb];
	    break;
	case 0x444 :
	    scfsi[ch][sb] = 2;
	    if (scalar[ch][0][sb] > scalar[ch][2][sb])
		scalar[ch][0][sb] = scalar[ch][2][sb];
	    scalar[ch][1][sb] = scalar[ch][2][sb] = scalar[ch][0][sb];
	}	/* switch */
    }		/* for (sb.. */
}

/************************************************************************
 *
 * scf_calc    
 *
 * Calculating the scale factor for one channel.
 *
 * Version 1.0	    94/07/07	Andreas Silzle, IRT
 *
 * route: II_scale_factor_calc() 
 *
 * PURPOSE:For each subband, calculate the scale factor for each set
 * of the 12 subband samples
 *
 * SEMANTICS:  Pick the scalefactor #multiple[]# just larger than the
 * absolute value of the peak subband sample of 12 samples,
 * and store the corresponding scalefactor index in #scalar#.
 *
 * Layer II has three sets of 12-subband samples for a given
 * subband.
 *
 ************************************************************************/

void scf_calc (
	double		sample[][3][SCALE_BLOCK][SBLIMIT], /* in */
	int		sblimit,		/* in */
	int		ch,			/* in */
	unsigned int	scf[][3][SBLIMIT])	/* out */ 
{
/* global variables: 
 * double multiple[64] 
 */
    int	    sb, smp, sufr, r;
    double  s[SBLIMIT];

    for (sufr=0; sufr<3; sufr++)
    {
	for (sb=0; sb<sblimit; sb++)
	    for (smp=1, s[sb] = mod(sample[ch][sufr][0][sb]); 
		    smp<SCALE_BLOCK; smp++)
		if (mod(sample[ch][sufr][smp][sb]) > s[sb])
		    s[sb] = mod(sample[ch][sufr][smp][sb]);

	for (sb=0; sb<sblimit; sb++)
	    for (r=SCALE_RANGE-1, scf[ch][sufr][sb]=0; r>=0; r--)
		if (s[sb] < multiple[r])
		{                       /* <= changed to <, 1992-11-06 shn*/
		    scf[ch][sufr][sb] = r;
		    break;
		}
    }   /* for (sufr.. */
}

