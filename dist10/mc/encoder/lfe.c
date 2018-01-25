/**********************************************************************
 * ISO MPEG Audio Subgroup Software Simulation Group (1996)
 * ISO 13818-3 MPEG-2 Audio Multichannel Encoder
 *
 * $Id: lfe.c,v 1.4 1996/02/12 07:13:35 rowlands Exp $
 *
 * $Log: lfe.c,v $
 * Revision 1.4  1996/02/12 07:13:35  rowlands
 * Release following Munich meeting
 *
 * Revision 1.1  1995/11/06  04:19:12  rowlands
 * Received from Uwe Felderhoff (IRT)
 *
 **********************************************************************/

#include "common.h"
#include "encoder.h"

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
 
/************************************************************************
/*
/* encode_lfe  
/*
/************************************************************************/

void lf_scalefactor(
		    double lfe_sample[12], 
		    unsigned int *lfe_scalar)
{
  int  j;
  double s;

    for(j=1, s = mod(lfe_sample[0]); j<12; j++)
      if (mod(lfe_sample[j]) > s)
         s = mod(lfe_sample[j]);

    for(j=SCALE_RANGE-1, *lfe_scalar = 0; j>=0; j--)
      if (s <= multiple[j]){
         *lfe_scalar = j;
         break;
      }
   
}


void II_encode_lfe_scale(
			unsigned int lfe_scalar, 
			Bit_stream_struc *bs)
{
      putbits(bs,lfe_scalar,6);
}


void lfe_allocation(
		    unsigned int *lfe_alloc, 
		    int *adb)                   /* 70 */
{
    int nbal, bspl, bscf;

 /*   *lfe_alloc = 4          /* index (Lay II bit alloc tabel) */
    nbal = 4;                 /* 4 bits  (nbal) */
    bscf = 6;                 /* bits for scale factor */
    bspl = 12 * 5;            /* bits used for quantization*/
    *adb -= nbal + bscf + bspl;
}


void II_lfe_quantisation(
			unsigned int lfe_scalar, 
			double lfe_samples[12], 
			unsigned int lfe_alloc, 
			unsigned int sbband[12], 
			frame_params *fr_ps)
{
   int i, j, s, qnt, n, sig;
   unsigned int stps;
   double d;
   al_table *alloc = fr_ps->alloc;
   for (s=0;s<12;s++)
   {
          d = lfe_samples[s] / multiple[lfe_scalar];
          if (mod(d) > 1.0)
            printf("In LFE, not scaled properly  %d  %12.10e\n",s,d);
          qnt = (*alloc)[0][lfe_alloc].quant;
          d = d * a[qnt] + b[qnt];
          /* extract MSB N-1 bits from the floating point sample */
          if (d >= 0) sig = 1;
          else { sig = 0; d += 1.0; }
          n = 0;
#ifndef MS_DOS
          stps = (*alloc)[0][lfe_alloc].steps;
          while ((1L<<n) < stps) n++;
#else
          while  ( ( (unsigned long)(1L<<(long)n) <
                   ((unsigned long) ((*alloc)[i][bit_alloc[k][i]].steps)
                   & 0xffff)
                   ) && ( n< 16)
                  ) n++;
#endif
          n--;
          sbband[s] = (unsigned int) (d * (double) (1L<<n));
          /* tag the inverted sign bit to sbband at position N */
          /* The bit inversion is a must for grouping with 3,5,9 steps
             so it is done for all subbands */
          if (sig) sbband[s] |= 1<<n;
        }
}



void II_encode_lfe_alloc(
			unsigned int lfe_alloc, 
			frame_params *fr_ps, 
			Bit_stream_struc *bs)
{
    al_table *alloc = fr_ps->alloc;
    putbits(bs, lfe_alloc, (*alloc)[0][0].bits);
}


void II_lfe_sample_encoding(
			    unsigned int lfe_sbband[12], 
			    unsigned int lfe_alloc, 
			    frame_params *fr_ps, 
			    Bit_stream_struc *bs)
{
   unsigned int s, j;
   al_table *alloc = fr_ps->alloc;

   for (s=0;s<12;s++)
     if ((*alloc)[0][lfe_alloc].group == 3){
       putbits(bs, lfe_sbband[s], (*alloc)[0][lfe_alloc].bits);
     }
}


/***********************************************************************
 *
 *
 * filter()
 *
 * A lf-filter to produce the LFE signal
 *
 **********************************************************************/

 void lfe_filter(
		double  **buffer, 
		double lfe_sample[12])
 {
    static double c;
    static double d[8];
    static double w1[8], w2[8], v0[8], v1[8], v2[8];
    static double a0[8], a1[8], a2[8], b0[8], b1[8], b2[8];
    static double A0[8], B0[8], B1[8], B2[8];
    static int fa, fg;
    static int init = 0;
    int i, j, k;
    double sambuf[1152];

    if(init == 0){
      for(i=0;i<8;i++)
        w1[i] = w2[i] =
        v0[i] = v1[i] = v2[i] = 0.0;



      fg = 125;                  /* cutoff-frequency */
      fa = 48000;                /* sampling-frequency */

      /* filter coefficients */

      for (i = 0; i < 8; i++){
        A0[i]=B0[i]=B2[i]=1.0;
        B1[i]=2*cos((2*i+1)*PI/(4*8));
      }

      c = 1/tan(PI * fg /fa);

      for (i = 0; i < 8; i++){
        d[i] = (B0[i] + (B1[i] * c) + (B2[i] * c * c));
        a0[i] = A0[i] / d[i];
        a1[i] = 2 * A0[i] / d[i];
        a2[i] = A0[i] / d[i];
        b0[i] = 1;
        b1[i] = 2 * (B0[i] - (B2[i] * c * c)) / d[i];
        b2[i] = (B0[i] - (B1[i] * c) + (B2[i] * c * c)) / d[i];
      }
      init = 1;
    }
    j = 0;
    for(i = 0; i < 1152; i++){
      sambuf[i] = **buffer ;
      for(k = 0; k < 8; k++){
        v0[k] = sambuf[i];
        sambuf[i] = v0[k] * a0[k] + v1[k] * a1[k] + v2[k] * a2[k] - w1[k] * b1[k
] - w2[k] * b2[k];
        v2[k] = v1[k]; v1[k] = v0[k];
        w2[k] = w1[k]; w1[k] = sambuf[i];
      }
      (*buffer)++;
    }
    i = 0;
    for (j = 0; j < 12; j++){
      lfe_sample[j] = sambuf[i]/SCALE;
      i += 96;
    }
 }






