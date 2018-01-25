/**********************************************************************
 * ISO MPEG Audio Subgroup Software Simulation Group (1996)
 * ISO 13818-3 MPEG-2 Audio Multichannel Encoder
 *
 * $Id: tonal.c 1.8 1996/02/12 07:13:35 rowlands Exp $
 *
 * $Log: tonal.c $
 * Revision 1.8  1996/02/12 07:13:35  rowlands
 * Release following Munich meeting
 *
 * Revision 1.5.2.1  1995/11/06  04:19:12  rowlands
 * Received from Uwe Felderhoff (IRT)
 *
 * Revision 1.7  1995/08/14  08:06:37  tenkate
 * ML-LSF added Warner ten Kate 7/8/95 (Philips)
 * II_psycho_one() split into II_psycho_one() for audio data and
 * II_psycho_one_ml() for MultiLingual data.
 * Variables crit_band, cbound and sub_size has been made local to
 * these functions.
 * Tables "cb" and "th" are copied from LSF-directory.
 * ltg has found its ml counterpart.
 *
 * Revision 1.6  1995/07/31  07:47:50  tenkate
 * bugs correction (if center==1 in Psycho_One), 25/07/95 WtK
 *
 * Revision 1.4.2.1  1995/06/16  03:46:42  rowlands
 * Input from Susanne Ritscher (IRT)
 *
 **********************************************************************/

/**********************************************************************
 *   date   programmers         comment                               *
 * 2/25/91  Douglas Wong        start of version 1.1 records          *
 * 3/06/91  Douglas Wong        rename: setup.h to endef.h            *
 *                              updated I_psycho_one and II_psycho_one*
 * 3/11/91  W. J. Carter        Added Douglas Wong's updates dated    *
 *                              3/9/91 for I_Psycho_One() and for     *
 *                              II_Psycho_One().                      *
 * 5/10/91  W. Joseph Carter    Ported to Macintosh and Unix.         *
 *                              Located and fixed numerous software   *
 *                              bugs and table data errors.           *
 * 6/11/91  Davis Pan           corrected several bugs                *
 *                              based on comments from H. Fuchs       *
 * 01jul91  dpwe (Aware Inc.)   Made pow() args float                 *
 *                              Removed logical bug in I_tonal_label: *
 *                              Sometimes *tone returned == STOP      *
 * 7/10/91  Earle Jennings      no change necessary in port to MsDos  *
 * 11sep91  dpwe@aware.com      Subtracted 90.3dB from II_f_f_t peaks *
 * 10/1/91  Peter W. Farrett    Updated II_Psycho_One(),I_Psycho_One()*
 *				to include comments.		      *
 *11/29/91  Masahiro Iwadare    Bug fix regarding POWERNORM           *
 *                              fixed several other miscellaneous bugs*
 * 2/11/92  W. Joseph Carter    Ported new code to Macintosh.  Most   *
 *                              important fixes involved changing     *
 *                              16-bit ints to long or unsigned in    *
 *                              bit alloc routines for quant of 65535 *
 *                              and passing proper function args.     *
 *                              Removed "Other Joint Stereo" option   *
 *                              and made bitrate be total channel     *
 *                              bitrate, irrespective of the mode.    *
 *                              Fixed many small bugs & reorganized.  *
 * 2/12/92  Masahiro Iwadare    Fixed some potential bugs in          *
 *          Davis Pan           subsampling()                         *
 * 2/25/92  Masahiro Iwadare    Fixed some more potential bugs        *
 * 92-11-06 Soren H. Nielsen	Corrected power calculation in I_ and *
 *				II_f_f_t.		              *	
 **********************************************************************
 *                                                                    *
 *                                                                    *
 *  MPEG/audio Phase 2 coding/decoding multichannel                   *
 *                                                                    *
 *  7/27/93        Susanne Ritscher,  IRT Munich                      *
 *  8/27/93        Susanne Ritscher, IRT Munich                       *
 *                 Channel-Switching is working                       *
 *  9/1/93         Susanne Ritscher,  IRT Munich                      *
 *                 all channels normalized                            *
 *                                                                    *
 *  9/20/93        channel-switching is only performed at a           *
 *                 certain limit of TC_ALLOC dB, which is included    *
 *                 in encoder.h                                       *
 *                                                                    *
 * 10/18/93        seperated smr and ltmin                            *
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
 **********************************************************************/
#define TONAL_WIDTH 0.1 /* When more than one tonal component is within
                           this width in Bark, the weaker one(s) are
                           eliminated */


#include "common.h"
#include "encoder.h"

/**********************************************************************/
/*
/*        This module implements the psychoacoustic model I for the
/* MPEG encoder layer II. It uses simplified tonal and noise masking
/* threshold analysis to generate SMR for the encoder bit allocation
/* routine.
/*
/**********************************************************************/

int read_crit_band(int lay, int freq) 
{
 int crit_band;
 FILE *fp;
 char r[16], t[80];

 strcpy(r, "2cb1");
 r[0] = (char) lay + '0';
 r[3] = (char) freq + '0';
 if( !(fp = OpenTableFile(r)) ){       /* check boundary values */
    printf("Please check %s boundary table\n",r);
    exit(0);
 }
 fgets(t,80,fp);
 sscanf(t,"%d\n",&crit_band);
 fclose(fp);
 return(crit_band);
}        

void read_cbound(int lay, int freq, int crit_band, int *cbound) 
 /* this function reads in critical band boundaries */
{
 int i,j,k;
 FILE *fp;
 char r[16], t[80];

 strcpy(r, "2cb1");
 r[0] = (char) lay + '0';
 r[3] = (char) freq + '0';
 if( !(fp = OpenTableFile(r)) ){       /* check boundary values */
    printf("Please check %s boundary table\n",r);
    exit(0);
 }
 fgets(t,80,fp);               /* skip input for critical bands */
 sscanf(t,"%d\n",&k);
 for(i=0;i<crit_band;i++){ 
    fgets(t,80,fp);
    sscanf(t,"%d %d\n",&j, &k);
    if(i==j) cbound[j] = k;
    else {                     /* error */
       printf("Please check index %d in cbound table %s\n",i,r);
       exit(0);
    }
 }
 fclose(fp);
}        

void read_freq_band(int *sub_size, g_ptr *ltg, int lay, int freq) 
  /* this function reads in frequency bands and bark values  */
{
 int i,j, k;
 double a,b,c;
 FILE *fp;
 char r[16], t[80];

 strcpy(r, "2th1");
 r[0] = (char) lay + '0';
 r[3] = (char) freq + '0';
 if( !(fp = OpenTableFile(r)) ){   /* check freq. values  */
    printf("Please check frequency and cband table %s\n",r);
    exit(0);
 }
 fgets(t,80,fp);              /* read input for freq. subbands */
 sscanf(t,"%d\n",sub_size);
 *ltg = (g_ptr) mem_alloc (sizeof(g_thres) * (*sub_size), "ltg");
 (*ltg)[0].line = 0;          /* initialize global masking threshold */
 (*ltg)[0].bark = 0;
 (*ltg)[0].hear = 0;
 for(i=1;i<(*sub_size);i++){    /* continue to read freq. subband */
    fgets(t,80,fp);          /* and assign                     */
    sscanf(t,"%d %d %lf %lf\n",&j, &k, &b, &c);
    if(i == j){
       (*ltg)[j].line = k;
       (*ltg)[j].bark = b;
       (*ltg)[j].hear = c;
    }
    else {                   /* error */
       printf("Please check index %d in freq-cb table %s\n",i,r);
       exit(0);
    }
 }
 fclose(fp);
}

void make_map (int sub_size, mask *power, g_thres *ltg)
/* this function calculates the global masking threshold */
{
    int i, j;
   
    for (i = 1; i < sub_size; i++)
	for (j = ltg[i-1].line; j <= ltg[i].line; j++)
	    power[j].map = i;
}

double add_db(double a, double b)
{
 a = pow(10.0,a/10.0);
 b = pow(10.0,b/10.0);
 return 10 * log10(a+b);
}

/****************************************************************/
/*
/*        Fast Fourier transform of the input samples.
/*
/****************************************************************/

void II_f_f_t (double *sample, mask *power) /* this function calculates an */
					    /* FFT analysis for the freq.  */
					    /* domain                      */
{
    int i,j,k,ll,l=0;
    int ip, le, le1;
    double t_r, t_i, u_r, u_i;
    static int M, MM1, init = 0, N, NV2, NM1;
    double *x_r, *x_i, *energy;
    static int *rev;
    static double *w_r, *w_i;

    x_r = (double *) mem_alloc (sizeof (DFFT), "x_r");
    x_i = (double *) mem_alloc (sizeof (DFFT), "x_i");
    energy = (double *) mem_alloc (sizeof (DFFT), "energy");

    if (!init)
    {
	rev = (int *) mem_alloc (sizeof (IFFT), "rev");
	w_r = (double *) mem_alloc (sizeof (D10), "w_r");
	w_i = (double *) mem_alloc (sizeof (D10), "w_i");
	M = 10;
	MM1 = 9;
	N = FFT_SIZE;
	NV2 = FFT_SIZE >> 1;
	NM1 = FFT_SIZE - 1;
	for (ll = 0; ll < M; ll++)
	{
	    le = 1 << (M-ll);
	    le1 = le >> 1;
	    w_r[ll] = cos (PI / le1);
	    w_i[ll] = -sin (PI / le1);
	}
	for (i = 0; i < FFT_SIZE; rev[i] = l, i++)
	    for (j = 0, l = 0; j < 10; j++)
	    {
		k = (i >> j) & 1;
		l |= (k << 9 - j);                
	    }
	init = 1;
    }

    for (i = 0; i < FFT_SIZE; i++)
    {
       x_r[i] = sample[i];
       x_i[i] = energy[i] = 0;
    }
/*
    memcpy ((char *) x_r, (char *) sample, sizeof (double) * FFT_SIZE);
*/
    for (ll = 0; ll < MM1; ll++)
    {
	le = 1 << (M - ll);
	le1 = le >> 1;
	u_r = 1;
	u_i = 0;
	for (j = 0; j < le1; j++)
	{
	    for (i = j; i < N; i += le)
	    {
		ip = i + le1;
		t_r = x_r[i] + x_r[ip];
		t_i = x_i[i] + x_i[ip];
		x_r[ip] = x_r[i] - x_r[ip];
		x_i[ip] = x_i[i] - x_i[ip];
		x_r[i] = t_r;
		x_i[i] = t_i;
		t_r = x_r[ip];
		x_r[ip] = x_r[ip] * u_r - x_i[ip] * u_i;
		x_i[ip] = x_i[ip] * u_r + t_r * u_i;
	    }
	    t_r = u_r;
	    u_r = u_r * w_r[ll] - u_i * w_i[ll];
	    u_i = u_i * w_r[ll] + t_r * w_i[ll];
	}
    }
    for (i = 0; i < N; i += 2)
    {
	ip = i + 1;
	t_r = x_r[i] + x_r[ip];
	t_i = x_i[i] + x_i[ip];
	x_r[ip] = x_r[i] - x_r[ip];
	x_i[ip] = x_i[i] - x_i[ip];
	x_r[i] = t_r;
	x_i[i] = t_i;
	energy[i] = x_r[i] * x_r[i] + x_i[i] * x_i[i];
    }
    for (i = 0; i < FFT_SIZE; i++)
	if (i < rev[i])
	{
	    t_r = energy[i];
	    energy[i] = energy[rev[i]];
	    energy[rev[i]] = t_r;
	}
    for (i = 0; i < HAN_SIZE; i++)
    {
	/* calculate power density spectrum */
	if (energy[i] < 1E-20)
	    energy[i] = 1E-20;
	/* power calculation corrected with a factor 4, both positive
	   and negative frequencies exist, 1992-11-06 shn */
	power[i].x = 10 * log10 (energy[i] * 4.0) + POWERNORM;
	power[i].next = STOP;
	power[i].type = FALSE;
    }

    mem_free ((void **) &x_r);
    mem_free ((void **) &x_i);
    mem_free ((void **) &energy);

}

/****************************************************************/
/*
/*         Window the incoming audio signal.
/*
/****************************************************************/

void II_hann_win (double *sample)   /* this function calculates a  */
				    /* Hann window for PCM (input) */
{				    /* samples for a 1024-pt. FFT  */
    register int i;
    register double sqrt_8_over_3;
    static int init = 0;
    static double *window;
    
    if (!init)
    {
	/* calculate window function for the Fourier transform */
	window = (double *) mem_alloc (sizeof (DFFT), "window");
	sqrt_8_over_3 = pow (8.0 / 3.0, 0.5);
	for (i = 0; i < FFT_SIZE; i++)
	{
	    /* Hann window formula */
	    window[i] = sqrt_8_over_3 * 0.5 * (1 - cos (2.0*PI*i / (FFT_SIZE-1))) / FFT_SIZE;
	}
	init = 1;
    }

    for (i = 0; i < FFT_SIZE; i++)
	sample[i] *= window[i];
}

/*******************************************************************/
/*
/*        This function finds the maximum spectral component in each
/* subband and return them to the encoder for time-domain threshold
/* determination.
/*
/*******************************************************************/

void II_pick_max (mask *power, double *spike)
{
 double max;
 int i,j;

 for(i=0;i<HAN_SIZE;spike[i>>4] = max, i+=16)      /* calculate the      */
 for(j=0, max = DBMIN;j<16;j++)                    /* maximum spectral   */
    max = (max>power[i+j].x) ? max : power[i+j].x; /* component in each  */
}                                                  /* subband from bound */
                                                   /* 4-16               */

/****************************************************************/
/*
/*        This function labels the tonal component in the power
/* spectrum.
/*
/****************************************************************/

void II_tonal_label(mask *power, int *tone) /* this function extracts (tonal) */
					    /* sinusoidals from the spectrum  */
{
 int i,j, last = LAST, first, run, last_but_one = LAST; /* dpwe */
 double max;

 *tone = LAST;
 for(i=2;i<HAN_SIZE-12;i++){
    if(power[i].x>power[i-1].x && power[i].x>=power[i+1].x){
       power[i].type = TONE;
       power[i].next = LAST;
       if(last != LAST) power[last].next = i;
       else first = *tone = i;
       last = i;
    }
 }
 last = LAST;
 first = *tone;
 *tone = LAST;
 while(first != LAST){               /* the conditions for the tonal          */
    if(first<2 || first>499) run = 0;/* otherwise k+/-j will be out of bounds */
    else if(first<62) run = 2;       /* components in layer II, which         */
    else if(first<126) run = 3;      /* are the boundaries for calc.          */
    else if(first<254) run = 6;      /* the tonal components                  */
    else run = 12;
    max = power[first].x - 7;        /* after calculation of tonal   */
    for(j=2;j<=run;j++)              /* components, set to local max */
       if(max < power[first-j].x || max < power[first+j].x){
          power[first].type = FALSE;
          break;
       }
    if(power[first].type == TONE){   /* extract tonal components */
       int help=first;
       if(*tone==LAST) *tone = first;
       while((power[help].next!=LAST)&&(power[help].next-first)<=run)
          help=power[help].next;
       help=power[help].next;
       power[first].next=help;
       if((first-last)<=run){
          if(last_but_one != LAST) power[last_but_one].next=first;
       }
       if(first>1 && first<255){     /* calculate the sum of the */
          double tmp;                /* powers of the components */
          tmp = add_db(power[first-1].x, power[first+1].x);
          power[first].x = add_db(power[first].x, tmp);
       }
       for(j=1;j<=run;j++){
          power[first-j].x = power[first+j].x = DBMIN;
          power[first-j].next = power[first+j].next = STOP;
          power[first-j].type = power[first+j].type = FALSE;
       }
       last_but_one=last;
       last = first;
       first = power[first].next;
    }
    else {
       int ll;
       if(last == LAST); /* *tone = power[first].next; dpwe */
       else power[last].next = power[first].next;
       ll = first;
       first = power[first].next;
       power[ll].next = STOP;
    }
 }
}

/****************************************************************/
/*
/*        This function groups all the remaining non-tonal
/* spectral lines into critical band where they are replaced by
/* one single line.
/*
/****************************************************************/
        
void noise_label(int crit_band, int *cbound, mask *power, int *noise, g_thres *ltg)
{
 int i,j, centre, last = LAST;
 double index, weight, sum;
                              /* calculate the remaining spectral */
 for(i=0;i<crit_band-1;i++){  /* lines for non-tonal components   */
     for(j=cbound[i],weight = 0.0,sum = DBMIN;j<cbound[i+1];j++){
        if(power[j].type != TONE){
           if(power[j].x != DBMIN){
              sum = add_db(power[j].x,sum);
              weight += pow(10.0, power[j].x/10.0) * (ltg[power[j].map].bark-i);
              power[j].x = DBMIN;
           }
        }   /* check to see if the spectral line is low dB, and if  */
     }      /* so replace the center of the critical band, which is */
            /* the center freq. of the noise component              */
     if(sum <= DBMIN)  centre = (cbound[i+1]+cbound[i]) /2;
     else {
        index = weight/pow(10.0,sum/10.0);
        centre = cbound[i] + (int) (index * (double) (cbound[i+1]-cbound[i]) );
     }     /* locate next non-tonal component until finished; */
           /* add to list of non-tonal components             */
     if(power[centre].type == TONE) centre++;
     if(last == LAST) *noise = centre;
     else {
        power[centre].next = LAST;
        power[last].next = centre;
     }
     power[centre].x = sum;
     power[centre].type = NOISE;        
     last = centre;
 }        
}

/****************************************************************/
/*
/*        This function reduces the number of noise and tonal
/* component for further threshold analysis.
/*
/****************************************************************/

void subsampling (mask *power, g_thres *ltg, int *tone, int *noise)
{
    int i, old;
   
    i = *tone;
    old = STOP;
    /* calculate tonal components for */
    while (i != LAST)
    {
	/* reduction of spectral lines    */
	if (power[i].x < ltg[power[i].map].hear)
	{
	    power[i].type = FALSE;
	    power[i].x = DBMIN;
	    if (old == STOP)
		*tone = power[i].next;
	    else
		power[old].next = power[i].next;
	}
	else
	    old = i;
	i = power[i].next;
    }
    i = *noise;
    old = STOP;
    /* calculate non-tonal components for */
    while (i != LAST)
    {
	/* reduction of spectral lines        */
        if (power[i].x < ltg[power[i].map].hear)
	{
	    power[i].type = FALSE;
	    power[i].x = DBMIN;
	    if (old == STOP)
		*noise = power[i].next;
	    else
		power[old].next = power[i].next;
	}
	else
	    old = i;
	i = power[i].next;
    }
    i = *tone;
    old = STOP;
    while (i != LAST)
    {
	/* if more than one */
	if (power[i].next == LAST)
	    break;             /* tonal component  */
	if (ltg[power[power[i].next].map].bark -     /* is less than .5  */
	    ltg[power[i].map].bark < TONAL_WIDTH)
	{
	    /* bark, take the   */
	    if (power[power[i].next].x > power[i].x)
	    {
		/* maximum          */
		if (old == STOP)
		    *tone = power[i].next;
		else
		    power[old].next = power[i].next;
		power[i].type = FALSE;
		power[i].x = DBMIN;
	    }
	    else
	    {
		power[power[i].next].type = FALSE;
		power[power[i].next].x = DBMIN;
		power[i].next = power[power[i].next].next;
	    }
	}
	else
	    old = i;
	i = power[i].next;
    }
}


/* ----------------------------------------------------------------------------
The masking function parameters are here set according to the parameters in
the IRT real time implementation. The constant definitions are for convenience.
1993-07-23 shn
---------------------------------------------------------------------------- */

#define AV_TONAL_K    -9.0 /* Masking index, tonal, constant part  [dB] */
#define AV_NOISE_K    -5.0 /* Masking index, noisy, constant part  [dB] */
#define AV_TONAL_DZ   -0.3 /* Masking index, tonal, CBR dependence [dB/Bark] */
#define AV_NOISE_DZ   -0.3 /* Masking index, noisy, CBR dependence [dB/Bark] */

#define LOW_LIM_1     -1.0 /* 1st lower slope from 0         to LOW_LIM_1 [Bark] */
#define LOW_LIM_2     -3.0 /* 2nd lower slope from LOW_LIM_1 to LOW_LIM_2 [Bark] */

#define LOW_DZ_K_1     6.0 /* 1st lower slope, constant part [dB/Bark] */
#define LOW_DZ_SPL_1   0.4 /* 1st lower slope, SPL dependence [dB/(Bark*dB)] */
#define LOW_DZ_MIN_1  17.0 /* 1st lower slope, minimum value [dB/Bark] */
#define LOW_DZ_2      17.0 /* 2nd lower slope [dB/Bark] */

#define UP_LIM_1       1.0 /* 1st upper slope from 0        to UP_LIM_1 [Bark] */
#define UP_LIM_2       8.0 /* 2nd upper slope from UP_LIM_1 to UP_LIM_2 [Bark] */

#define UP_DZ_1      -18.0 /* 1st upper slope, constant part [dB/Bark] */
#define UP_SPL_1       0.0 /* 1st upper slope, SPL dependence [dB/(Bark*dB)] */
#define UP_DZ_2      -17.0 /* 2nd upper slope, constant part [dB/Bark] */
#define UP_SPL_2      -0.1 /* 2nd upper slope, SPL dependence [dB/(Bark*dB)] */

#define H_THR_OFFSET -12.0 /* Hearing threshold offset [dB] */
#define H_THR_OS_BR    0 /*96*/   /* Minimum datarate for offset, [kbit/s per channel] */

#define MASK_ADD       2.0 /* Addition of maskers	                [dB] */
#define QUIET_ADD      3.0 /* Addition of masker and threshold in quiet [dB] */

/****************************************************************
*
*        This function calculates the individual threshold and
* sum with the quiet threshold to find the global threshold.
*
****************************************************************/

void threshold(int sub_size, mask *power, g_thres *ltg, int *tone, int *noise, int bit_rate)
{
  int k, t;
  double z, dz, spl, vf, tmps;

  for (k=1; k<sub_size; k++) /* Target frequencies */
  {
    ltg[k].x = DBMIN;
    t = *tone;          /* calculate individual masking threshold  */
    while(t != LAST)    /* for tonal components, t,  to find LTG   */
    {
      z   = ltg[power[t].map].bark; /* critical band rate of masker */
      dz  = ltg[k].bark - z ;       /* distance of bark value*/
      spl = power[t].x;             /* sound pressure level of masker */

      if (dz >= LOW_LIM_2 && dz <  UP_LIM_2)
      {
        tmps = spl + AV_TONAL_K + AV_TONAL_DZ * z;

        /* masking function for lower & upper slopes */
        if (LOW_LIM_2<=dz && dz<LOW_LIM_1)
          if (LOW_DZ_SPL_1 * spl + LOW_DZ_K_1 > LOW_DZ_MIN_1)
            vf = LOW_DZ_2 * (dz - LOW_LIM_1) + 
                (LOW_DZ_SPL_1 * spl + LOW_DZ_K_1) * LOW_LIM_1;
          else
            vf = LOW_DZ_2 * (dz - LOW_LIM_1) + LOW_DZ_MIN_1 * LOW_LIM_1;
        else if (LOW_LIM_1<=dz && dz<0)
          if (LOW_DZ_SPL_1 * spl + LOW_DZ_K_1 > LOW_DZ_MIN_1)
            vf = (LOW_DZ_SPL_1 * spl + LOW_DZ_K_1) * dz;
          else
            vf = LOW_DZ_MIN_1 * dz;
        else if (0<=dz && dz<UP_LIM_1)
          vf = (UP_DZ_1 * dz);
        else if (UP_LIM_1<=dz && dz<UP_LIM_2)
          vf = (dz - UP_LIM_1) * (UP_DZ_2 - UP_SPL_2 * spl) +
               UP_DZ_1 * UP_LIM_1;
        tmps += vf;        
        ltg[k].x = non_lin_add(ltg[k].x, tmps, MASK_ADD);
      }
      t = power[t].next;
    } /* while */

    t = *noise;        /* calculate individual masking threshold   */
    while(t != LAST)   /* for non-tonal components, t, to find LTG */
    {
      z   = ltg[power[t].map].bark; /* critical band rate of masker */
      dz  = ltg[k].bark - z ;       /* distance of bark value*/
      spl = power[t].x;             /* sound pressure level of masker */

      if (dz >= LOW_LIM_2 && dz <  UP_LIM_2)
      {
        tmps = spl + AV_NOISE_K + AV_NOISE_DZ * z;

        /* masking function for lower & upper slopes */
        if (LOW_LIM_2<=dz && dz<LOW_LIM_1)
          if (LOW_DZ_SPL_1 * spl + LOW_DZ_K_1 > LOW_DZ_MIN_1)
            vf = LOW_DZ_2 * (dz - LOW_LIM_1) + 
                (LOW_DZ_SPL_1 * spl + LOW_DZ_K_1) * LOW_LIM_1;
          else
            vf = LOW_DZ_2 * (dz - LOW_LIM_1) + LOW_DZ_MIN_1 * LOW_LIM_1;
        else if (LOW_LIM_1<=dz && dz<0)
          if (LOW_DZ_SPL_1 * spl + LOW_DZ_K_1 > LOW_DZ_MIN_1)
            vf = (LOW_DZ_SPL_1 * spl + LOW_DZ_K_1) * dz;
          else
            vf = LOW_DZ_MIN_1 * dz;
        else if (0<=dz && dz<UP_LIM_1)
          vf = (UP_DZ_1 * dz);
        else if (UP_LIM_1<=dz && dz<UP_LIM_2)
          vf = (dz - UP_LIM_1) * (UP_DZ_2 - UP_SPL_2 * spl) +
               UP_DZ_1 * UP_LIM_1;
       tmps += vf;        
       ltg[k].x = non_lin_add(ltg[k].x, tmps, MASK_ADD);
     }
      t = power[t].next;
    }

    if (bit_rate < H_THR_OS_BR)
      ltg[k].x = non_lin_add(ltg[k].hear, ltg[k].x, QUIET_ADD);
    else
      ltg[k].x = non_lin_add(ltg[k].hear + H_THR_OFFSET, ltg[k].x, QUIET_ADD);
 
  } /* for */
  fflush(stderr);
}



/* --------------------------------------------------------------
non_lin_add
A flexible addition function for levels.
Input: a,b: the levels to be added.
	 c: the number of dB increase when a and b are equal.
Common values for c are 3.01 (power addition)
		    and 6.02 (voltage addition).
10.0/(10*log10(2)) = 3.3219
Function added 1993-04-14 Soren H. Nielsen
-------------------------------------------------------------- */
double non_lin_add(double a, double b, double c)
{
  c *= 3.3219;
  a = pow(10.0, a/c); b = pow(10.0, b/c);
  return(c*log10(a+b));
}


/****************************************************************/
/*
/*        This function finds the minimum masking threshold and
/* return the value to the encoder.
/*
/****************************************************************/

void II_minimum_mask(int sub_size, g_thres *ltg, double *ltmin, int sblimit)
{
 double min;
 int i,j;
 

 j=1;
 for(i=0;i<sblimit;i++)
    if(j>=sub_size-1)                   /* check subband limit, and       */
       ltmin[i] = ltg[sub_size-1].hear; /* calculate the minimum masking  */
    else {                              /* level of LTMIN for each subband*/
       min = ltg[j].x;
       while(ltg[j].line>>4 == i && j < sub_size){
       if(min>ltg[j].x)  min = ltg[j].x;
       j++;
    }
    ltmin[i] = min;
 }
}

/*****************************************************************/
/*
/*        This procedure is called in musicin to pick out the
/* smaller of the scalefactor or threshold.
/*
/*****************************************************************/

void II_smr (double *ltmin, double *smr, double *spike, double *scale, int sblimit, int l, int m)
{
    int i,j;
    double max;
		   
    for (i = l; i < m; i++)
    {
	/* determine the signal   */
	max = 20 * log10 (scale[i] * 32768) - 10;   /* level for each subband */
	if (spike[i] > max)
	    max = spike[i];			    /* for the maximum scale  */
	max -= ltmin[i];			    /* factors                */
	smr[i] = max;
    }
}
        
/****************************************************************/
/*
/*        This procedure calls all the necessary functions to
/* complete the psychoacoustic analysis.
/*
/****************************************************************/

void II_Psycho_One (double (*buffer)[1152],
		    double (*scale)[32],
		    double (*ltmin)[32],
		    frame_params *fr_ps,
		    double (*smr)[32],
		    double (*spiki)[32],
		    int aiff)
{
    layer *info = fr_ps->header;
    int   stereo = fr_ps->stereo;
    int   stereomc = fr_ps->stereomc;	
    int   stereoaug = fr_ps->stereoaug;	
    int   sblimit = fr_ps->sblimit;
    int k,i, tone=0, noise=0;
    static char init = 0;
    static int 	off[12] = {256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256}; /* max 7 MC channels + 5 compatible channels */
    double *sample;
    DSBL *spike;
    static D1408 *fft_buf;
    static mask_ptr power;
    static g_ptr ltg;
    int j, l, z, q;
    static int crit_band;
    static int *cbound;
    static int sub_size;
   
    sample = (double *) mem_alloc (sizeof (DFFT), "sample");
    spike = (DSBL *) mem_alloc (sizeof (D12SBL), "spike");
   
    if (!init)
    {
	/* bands, bark values, and mapping */
	/* changed 5 to 7 for matricing 8/10/93,SR */
	/* changed 7 to 12 for aug matricing 8/11/96,FdB */
	fft_buf = (D1408 *) mem_alloc ((long) sizeof (D1408) * 12, "fft_buf");
    	power = (mask_ptr) mem_alloc (sizeof (mask) * HAN_SIZE, "power");
	/* call functions for critical boundaries, freq. */
	crit_band = read_crit_band (info->lay, info->sampling_frequency);
	cbound = (int *) mem_alloc (sizeof (int) * crit_band, "cbound");
	read_cbound (info->lay, info->sampling_frequency, crit_band, cbound);
	read_freq_band (&sub_size, &ltg, info->lay, info->sampling_frequency);
	make_map (sub_size, power, ltg);
	for (i = 0; i < 1408; i++)
	    fft_buf[0][i] = fft_buf[1][i] = fft_buf[2][i] =
	    fft_buf[3][i] = fft_buf[4][i] = fft_buf[5][i] =
	    fft_buf[6][i] = fft_buf[7][i] = fft_buf[8][i] =
	    fft_buf[9][i] = fft_buf[10][i] = fft_buf[11][i] = 0;
	init = 1;
    }
 
    if (aiff != 1)
    {
	j = 0;
	l = 2;
    }
    else
    {    
	j = 0;
	if (stereoaug == 2) l = 12; /*fr_ps->stereo + fr_ps->stereomc + fr_ps->stereoaug + 5 compatible */
	else                l =  7; /*fr_ps->stereo + fr_ps->stereomc + 2 compatible */
    } 

    for (k = j; k < l; k++)
    {
        for (i = 0; i < 1152; i++)
	    fft_buf[k][(i + off[k]) % 1408] = (double) buffer[k][i] / SCALE;
	for (i = 0; i < FFT_SIZE; i++)
	    sample[i] = fft_buf[k][(i + 1216 + off[k]) % 1408];
   
	off[k] += 1152;
	off[k] %= 1408;
	/* call functions for windowing PCM samples,*/
	II_hann_win (sample);		/* location of spectral components in each  */
	for (i = 0; i < HAN_SIZE; i++)
	    power[i].x = DBMIN;		/* subband with labeling */
	II_f_f_t (sample, power);	/* locate remaining non- */
       
	if (fr_ps->header->center == 3 && k == 2)
	{
	    /* set center to 0, 9/2/93,SR*/
	    /* add to Left and Right ? WtK */
	    for (z = 184; z < HAN_SIZE; z++)
		power[z].x = -103.670;    /* DBMIN + 96.330; */
	}
	II_pick_max (power, spike[k]); /* tonal sinusoidals,   */

#ifdef PRINTOUT
	if (verbosity >= 3)
	{
	    printf ("\nChannel %d", k);
	    printf ("\nSignal value per subband, from the FFT:\n");
	    for (i = 0; i < sblimit; i++)
		printf ("%5.1f dB  ", spike[k][i]);
	    printf ("\nMax. signal peak per subband, SCF SPL:\n");
	    for (i = 0; i < sblimit; i++)   /* from [II_smr] determine the SCF SPL */
		printf ("%5.1f dB  ", 20 * log10(scale[k][i] * 32768)); 
	    fflush (stdout);
	}
#endif

	II_tonal_label (power, &tone);                /* reduce noise & tonal components , find */
	noise_label (crit_band, cbound, power, &noise, ltg); 

#ifdef PRINTOUT
	if (verbosity >= 3)
	{
	    printf ("\nMaskers before sorting, FFT based levels:\n");
	    for (i = 0; i < 511; i++)
	    {
		if ((power[i].type == NOISE) && (power[i].x > -200))
		    printf ("N:%3u %5.1f dB  ", i, power[i].x);
		if ((power[i].type == TONE) && (power[i].x > -200))
		    printf ("T:%3u %5.1f dB  ", i, power[i].x);
	    }
	    printf ("tone = %d noise = %d \n", tone, noise);
	    fflush (stdout);
	}
#endif

	subsampling (power, ltg, &tone, &noise);      /* global & minimal     */
    
#ifdef PRINTOUT
	if (verbosity >= 3)
	{
	    printf ("\nMaskers after sorting:\n");
	    for (i = 0; i < 511; i++)
	    {
		if ((power[i].type == NOISE) && (power[i].x > -200))
		    printf ("N:%3u %5.1f dB  ", i, power[i].x);
		if ((power[i].type == TONE) && (power[i].x > -200))
		    printf ("T:%3u %5.1f dB  ", i, power[i].x);
	    }
	    fflush (stdout);
	}
#endif

	threshold (sub_size, power, ltg, &tone, &noise,
		   bitrate[info->lay-1][info->bitrate_index] / (stereo+stereomc+stereoaug)); /*to-mask ratio*//* 21/03/1995 JMZ BUG ???!!!???*/
	II_minimum_mask (sub_size, ltg, &ltmin[k][0], sblimit);
    
#ifdef PRINTOUT
	if (verbosity >= 3)
	{
	    printf ("\nMinimum masking threshold:\n");
	    for (i = 0; i < sblimit; i++)
		printf ("%5.1f dB  ", ltmin[k][i]);
	    fflush (stdout);
	}
#endif

	for (i = 0; i < SBLIMIT; i++)
	    spiki[k][i] = spike[k][i];
     
	i = 0;
	q = sblimit;
	II_smr (ltmin[k], smr[k], spike[k], scale[k], sblimit, i, q);
    }
    
    mem_free ((void **) &sample);
    mem_free ((void **) &spike); 
} /*II_Psycho_One*/

void II_Psycho_One_ml (
	double (*buffer)[1152],
	double (*scale)[32],
	double (*ltmin)[32],
	frame_params *fr_ps,
	double (*smr)[32],
	double (*spiki)[32]
)
{
    layer *info = fr_ps->header;
    int	n_ml_ch   = info->multiling_ch;
    int sblimit_ml = fr_ps->sblimit_ml;
    int k,i, tone=0, noise=0;
    static char init = 0;
    static int off[7] = {256, 256, 256, 256, 256, 256, 256};   /* max 7 ML channels */
    double *sample;
    DSBL *spike;
    static D1408 *fft_buf;
    static mask_ptr power;
    static g_ptr ltg_ml;
    static int crit_band_ml;
    static int *cbound_ml;
    static int sub_size_ml;
    int j, l, z, q;


    sample = (double *) mem_alloc (sizeof (DFFT), "sample");
    spike = (DSBL *) mem_alloc (sizeof (D7SBL), "spike");
   
    if (!init)
    {
	/* bands, bark values, and mapping */
	fft_buf = (D1408 *) mem_alloc ((long) sizeof (D1408) * 7, "fft_buf");
	power = (mask_ptr) mem_alloc (sizeof (mask) * HAN_SIZE, "power");
	/* call functions for critical boundaries, freq. */
	if (info->multiling_fs == 0)
	{
	    crit_band_ml = read_crit_band (info->lay, info->sampling_frequency);
	    cbound_ml = (int *) mem_alloc (sizeof (int) * crit_band_ml, "cbound_ml");
	    read_cbound (info->lay, info->sampling_frequency, crit_band_ml, cbound_ml);
	    read_freq_band (&sub_size_ml, &ltg_ml, info->lay, info->sampling_frequency);
	    /* values are equal to those for the mc audio data */
	}
	else
	{
	    crit_band_ml = read_crit_band (info->lay, info->sampling_frequency + 4);
	    cbound_ml = (int *) mem_alloc (sizeof (int) * crit_band_ml, "cbound_ml");
	    read_cbound (info->lay, info->sampling_frequency + 4, crit_band_ml, cbound_ml);
	    read_freq_band (&sub_size_ml, &ltg_ml, info->lay, info->sampling_frequency + 4);
	}
	make_map (sub_size_ml, power, ltg_ml);
	for (i = 0; i < 1408; i++)
	    fft_buf[0][i] = fft_buf[1][i] = fft_buf[2][i] =     
	    fft_buf[3][i] = fft_buf[4][i] = fft_buf[5][i] = fft_buf[6][i] = 0;
	init = 1;
    }
 
    if (n_ml_ch > 0) 
    {
	for (k = 0; k < n_ml_ch; k++)
	{ 
	    for (i = 0; i < 1152; i++)
		fft_buf[k][(i+off[k]) % 1408]= (double) buffer[7+k][i] / SCALE;
	    for (i = 0; i < FFT_SIZE; i++)
		sample[i] = fft_buf[k][(i+1216+off[k]) % 1408];
    
	    off[k] += 1152;
	    off[k] %= 1408;

	    /* call functions for windowing PCM samples,*/
	    II_hann_win (sample);		/* location of spectral components in each  */
	    for (i = 0; i < HAN_SIZE; i++)
		power[i].x = DBMIN;		/* subband with labeling */
	    II_f_f_t (sample, power);		/* locate remaining non- */
	    
	    II_pick_max (power, spike[k]);
						/* tonal sinusoidals,   */
	
	    II_tonal_label (power, &tone);	/* reduce noise & tonal components , find */
	    noise_label (crit_band_ml, cbound_ml, power, &noise, ltg_ml); 
	
	    subsampling (power, ltg_ml, &tone, &noise);	/* global & minimal     */
	    
	    threshold (sub_size_ml, power, ltg_ml, &tone, &noise,
		       bitrate[info->lay-1][info->bitrate_index] / 2);	/* to-mask ratio */
									/* threshold, and sgnl- */
	    /* fprintf(stderr,  "sblimit_ml : %d\n",  sblimit_ml); fflush(stderr); */
	    II_minimum_mask (sub_size_ml, ltg_ml, &ltmin[7+k][0], sblimit_ml);
	    
	    for (i = 0; i < SBLIMIT; i++)
		spiki[7+k][i] = spike[k][i];
	 
	    i = 0;
	    q = sblimit_ml;
	    II_smr (ltmin[7+k], smr[7+k], spike[k], scale[7+k], sblimit_ml, i, q);
	
	} /* k-loop*/
    } /*n_ml_ch>0*/

    mem_free ((void **) &sample);
    mem_free ((void **) &spike); 
} /* II_psycho_One_ml */

/**********************************************************************/
/*
/*        This module implements the psychoacoustic model I for the
/* MPEG encoder layer I. It uses simplified tonal and noise masking
/* threshold analysis to generate SMR for the encoder bit allocation
/* routine.
/*
/**********************************************************************/

/****************************************************************/
/*
/*        Fast Fourier transform of the input samples.
/*
/****************************************************************/

void I_f_f_t (double *sample, mask *power)  /* this function calculates */
					    /* an FFT analysis for the  */
					    /* freq. domain             */
{
 int i,j,k,ll,l=0;
 int ip, le, le1;
 double t_r, t_i, u_r, u_i;
 static int M, MM1, init = 0, N, NV2, NM1;
 double *x_r, *x_i, *energy;
 static int *rev;
 static double *w_r, *w_i;

 x_r = (double *) mem_alloc(sizeof(DFFT2), "x_r");
 x_i = (double *) mem_alloc(sizeof(DFFT2), "x_i");
 energy = (double *) mem_alloc(sizeof(DFFT2), "energy");
 for(i=0;i<FFT_SIZE/2;i++) x_r[i] = x_i[i] = energy[i] = 0;
 if(!init){
    rev = (int *) mem_alloc(sizeof(IFFT2), "rev");
    w_r = (double *) mem_alloc(sizeof(D9), "w_r");
    w_i = (double *) mem_alloc(sizeof(D9), "w_i");
    M = 9;
    MM1 = 8;
    N = FFT_SIZE/2;
    NV2 = FFT_SIZE/2 >> 1;
    NM1 = FFT_SIZE/2 - 1;
    for(ll=0;ll<M;ll++){
       le = 1 << (M-ll);
       le1 = le >> 1;
       w_r[ll] = cos(PI/le1);
       w_i[ll] = -sin(PI/le1);
    }
    for(i=0;i<FFT_SIZE/2;rev[i] = l,i++) for(j=0,l=0;j<9;j++){
       k=(i>>j) & 1;
       l |= (k<<8-j);                
    }
    init = 1;
 }
 memcpy( (char *) x_r, (char *) sample, sizeof(double) * FFT_SIZE/2);
 for(ll=0;ll<MM1;ll++){
    le = 1 << (M-ll);
    le1 = le >> 1;
    u_r = 1;
    u_i = 0;
    for(j=0;j<le1;j++){
       for(i=j;i<N;i+=le){
          ip = i + le1;
          t_r = x_r[i] + x_r[ip];
          t_i = x_i[i] + x_i[ip];
          x_r[ip] = x_r[i] - x_r[ip];
          x_i[ip] = x_i[i] - x_i[ip];
          x_r[i] = t_r;
          x_i[i] = t_i;
          t_r = x_r[ip];
          x_r[ip] = x_r[ip] * u_r - x_i[ip] * u_i;
          x_i[ip] = x_i[ip] * u_r + t_r * u_i;
       }
       t_r = u_r;
       u_r = u_r * w_r[ll] - u_i * w_i[ll];
       u_i = u_i * w_r[ll] + t_r * w_i[ll];
    }
 }
 for(i=0;i<N;i+=2){
    ip = i + 1;
    t_r = x_r[i] + x_r[ip];
    t_i = x_i[i] + x_i[ip];
    x_r[ip] = x_r[i] - x_r[ip];
    x_i[ip] = x_i[i] - x_i[ip];
    x_r[i] = t_r;
    x_i[i] = t_i;
    energy[i] = x_r[i] * x_r[i] + x_i[i] * x_i[i];
 }
 for(i=0;i<FFT_SIZE/2;i++) if(i<rev[i]){
    t_r = energy[i];
    energy[i] = energy[rev[i]];
    energy[rev[i]] = t_r;
 }
 for(i=0;i<HAN_SIZE/2;i++){                     /* calculate power  */
    if(energy[i] < 1E-20) energy[i] = 1E-20;    /* density spectrum */
       /* power calculation corrected with a factor 4, both positive
	  and negative frequencies exist, 1992-11-06 shn */
       power[i].x = 10 * log10(energy[i]*4) + POWERNORM;
       power[i].next = STOP;
       power[i].type = FALSE;
 }
    mem_free ((void **) &x_r);
    mem_free ((void **) &x_i);
    mem_free ((void **) &energy);
}

/****************************************************************/
/*
/*         Window the incoming audio signal.
/*
/****************************************************************/

void I_hann_win (double *sample)    /* this function calculates a  */
				    /* Hann window for PCM (input) */
{				    /* samples for a 512-pt. FFT   */
 register int i;
 register double sqrt_8_over_3;
 static int init = 0;
 static double *window;

 if(!init){  /* calculate window function for the Fourier transform */
    window = (double *) mem_alloc (sizeof (DFFT2), "window");
    sqrt_8_over_3 = pow(8.0/3.0, 0.5);
    for(i=0;i<FFT_SIZE/2;i++){
      /* Hann window formula */
      window[i]=sqrt_8_over_3*0.5*(1-cos(2.0*PI*i/(FFT_SIZE/2-1)))/(FFT_SIZE/2);
    }
    init = 1;
 }
 for(i=0;i<FFT_SIZE/2;i++) sample[i] *= window[i];
}

/*******************************************************************/
/*
/*        This function finds the maximum spectral component in each
/* subband and return them to the encoder for time-domain threshold
/* determination.
/*
/*******************************************************************/

void I_pick_max (mask *power, double *spike)
{
 double max;
 int i,j;

 /* calculate the spectral component in each subband */
 for(i=0;i<HAN_SIZE/2;spike[i>>3] = max, i+=8)
    for(j=0, max = DBMIN;j<8;j++) max = (max>power[i+j].x) ? max : power[i+j].x;
}

/****************************************************************/
/*
/*        This function labels the tonal component in the power
/* spectrum.
/*
/****************************************************************/

void I_tonal_label (mask *power, int *tone) /* this function extracts   */
					    /* (tonal) sinusoidals from */
					    /* the spectrum             */
{
 int i,j, last = LAST, first, run;
 double max;
 int last_but_one=LAST;

 *tone = LAST;
 for(i=2;i<HAN_SIZE/2-6;i++){
    if(power[i].x>power[i-1].x && power[i].x>=power[i+1].x){
       power[i].type = TONE;
       power[i].next = LAST;
       if(last != LAST) power[last].next = i;
       else first = *tone = i;
       last = i;
    }
 }
 last = LAST;
 first = *tone;
 *tone = LAST;
 while(first != LAST){                /* conditions for the tonal     */
    if(first<2 || first>249) run = 0; /* otherwise k+/-j will be out of bounds*/
    else if(first<62) run = 2;        /* components in layer II, which */
    else if(first<126) run = 3;       /* are the boundaries for calc.   */
    else run = 6;                     /* the tonal components          */
    max = power[first].x - 7;
    for(j=2;j<=run;j++)  /* after calc. of tonal components, set to loc.*/
       if(max < power[first-j].x || max < power[first+j].x){   /* max   */
          power[first].type = FALSE;
          break;
       }
    if(power[first].type == TONE){    /* extract tonal components */
       int help=first;
       if(*tone == LAST) *tone = first;
       while((power[help].next!=LAST)&&(power[help].next-first)<=run)
          help=power[help].next;
       help=power[help].next;
       power[first].next=help;
       if((first-last)<=run){
          if(last_but_one != LAST) power[last_but_one].next=first;
       }
       if(first>1 && first<255){     /* calculate the sum of the */
          double tmp;                /* powers of the components */
          tmp = add_db(power[first-1].x, power[first+1].x);
          power[first].x = add_db(power[first].x, tmp);
       }
       for(j=1;j<=run;j++){
          power[first-j].x = power[first+j].x = DBMIN;
          power[first-j].next = power[first+j].next = STOP; /*dpwe: 2nd was .x*/
          power[first-j].type = power[first+j].type = FALSE;
       }
       last_but_one=last;
       last = first;
       first = power[first].next;
    }
    else {
       int ll;
       if(last == LAST) ; /* *tone = power[first].next; dpwe */
       else power[last].next = power[first].next;
       ll = first;
       first = power[first].next;
       power[ll].next = STOP;
    }
 }
}                        
                                
/****************************************************************/
/*
/*        This function finds the minimum masking threshold and
/* return the value to the encoder.
/*
/****************************************************************/

void I_minimum_mask(int sub_size, g_thres *ltg, double *ltmin)
{
 double min;
 int i,j;

 j=1;
 for(i=0;i<SBLIMIT;i++)
    if(j>=sub_size-1)                   /* check subband limit, and       */
       ltmin[i] = ltg[sub_size-1].hear; /* calculate the minimum masking  */
    else {                              /* level of LTMIN for each subband*/
       min = ltg[j].x;
       while(ltg[j].line>>3 == i && j < sub_size){
          if (min>ltg[j].x)  min = ltg[j].x;
          j++;
       }
       ltmin[i] = min;
    }
}

/*****************************************************************/
/*
/*        This procedure is called in musicin to pick out the
/* smaller of the scalefactor or threshold.
/*
/*****************************************************************/

void I_smr (double *ltmin, double *spike, double *scale)
{
 int i,j;
 double max;
                
 for(i=0;i<SBLIMIT;i++){                      /* determine the signal   */
    max = 20 * log10(scale[i] * 32768) - 10;  /* level for each subband */
    if(spike[i]>max) max = spike[i];          /* for the scalefactor    */
    max -= ltmin[i];
    ltmin[i] = max;
 }
}
        
/****************************************************************/
/*
/*        This procedure calls all the necessary functions to
/* complete the psychoacoustic analysis.
/*
/****************************************************************/

void
I_Psycho_One(
	double (*buffer)[1152],
	double (*scale)[32],
	double (*ltmin)[32],
	frame_params *fr_ps
) {
 int stereo = fr_ps->stereo;
 the_layer info = fr_ps->header;
 int k,i, tone=0, noise=0;
 static char init = 0;
 static int off[2] = {256,256};
 double *sample;
 DSBL *spike;
 static D640 *fft_buf;
 static mask_ptr power;
 static g_ptr ltg;
 static int crit_band;
 static int *cbound;
 static int sub_size;

 sample = (double *) mem_alloc(sizeof(DFFT2), "sample");
 spike = (DSBL *) mem_alloc(sizeof(D2SBL), "spike");
            /* call functions for critical boundaries, freq. */
 if(!init){ /* bands, bark values, and mapping              */
    fft_buf = (D640 *) mem_alloc(sizeof(D640) * 2, "fft_buf");
    power = (mask_ptr) mem_alloc(sizeof(mask) * HAN_SIZE/2, "power");
    crit_band = read_crit_band(info->lay,info->sampling_frequency);
    cbound = (int *) mem_alloc(sizeof(int) * crit_band, "cbound");
    read_cbound(info->lay,info->sampling_frequency,crit_band,cbound);
    read_freq_band(&sub_size,&ltg,info->lay,info->sampling_frequency);
    make_map(sub_size,power,ltg);
    for(i=0;i<640;i++) fft_buf[0][i] = fft_buf[1][i] = 0;
    init = 1;
 }
 for(k=0;k<stereo;k++){    /* check PCM input for a block of */
    for(i=0;i<384;i++)     /* 384 samples for a 512-pt. FFT  */
       fft_buf[k][(i+off[k])%640]= (double) buffer[k][i]/SCALE;
    for(i=0;i<FFT_SIZE/2;i++)
       sample[i] = fft_buf[k][(i+448+off[k])%640];
    off[k] += 384;
    off[k] %= 640;
                        /* call functions for windowing PCM samples,   */
    I_hann_win(sample); /* location of spectral components in each     */
    for(i=0;i<HAN_SIZE/2;i++) power[i].x = DBMIN;   /* subband with    */
    I_f_f_t(sample, power);              /* labeling, locate remaining */
    I_pick_max(power, &spike[k][0]);     /* non-tonal sinusoidals,     */
    I_tonal_label(power, &tone);         /* reduce noise & tonal com., */
    noise_label(crit_band,cbound,power, &noise, ltg);     /* find global & minimal      */
    subsampling (power, ltg, &tone, &noise);  /* threshold, and sgnl-   */
    threshold(sub_size,power, ltg, &tone, &noise,     /* to-mask ratio          */
      bitrate[info->lay-1][info->bitrate_index]/stereo);
    I_minimum_mask(sub_size,ltg, &ltmin[k][0]);
    I_smr(&ltmin[k][0], &spike[k][0], &scale[k][0]);        
 }
    mem_free ((void **) &sample);
    mem_free ((void **) &spike);
}
