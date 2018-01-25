/**********************************************************************
 * ISO MPEG Audio Subgroup Software Simulation Group (1996)
 * ISO 13818-3 MPEG-2 Audio Multichannel Encoder
 *
 * $Id: subs.c 1.6 1996/02/12 07:13:35 rowlands Exp $
 *
 * $Log: subs.c $
 * Revision 1.6  1996/02/12 07:13:35  rowlands
 * Release following Munich meeting
 *
 * Revision 1.3.2.1  1995/06/16  03:46:42  rowlands
 * Input from Susanne Ritscher (IRT)
 *
 **********************************************************************/

/**********************************************************************
 *   date   programmers         comment                               *
 * 2/25/91  Davis Pan           start of version 1.0 records          *
 * 5/10/91  W. Joseph Carter    Ported to Macintosh and Unix.         *
 * 7/10/91  Earle Jennings      Ported to MsDos from Macintosh        *
 *                              Replacement of one float with FLOAT   *
 * 2/11/92  W. Joseph Carter    Added type casting to memset() args.  *
 **********************************************************************
 *                                                                    *
 *                                                                    *
 *  MPEG/audio Phase 2 coding/decoding multichannel                   *
 *                                                                    *
 *  7/27/93        Susanne Ritscher,  IRT Munich                      *
 *  8/27/93        Susanne Ritscher, IRT Munich                       *
 *                 Channel-Switching is working                       *
 *                                                                    *
 *  9/1/93         Susanne Ritscher,  IRT Munich                      *
 *                 all channels normalized                            *
 *  9/20/93        channel-switching is only performed at a           *
 *                 certain limit of TC_ALLOC dB, which is included    *
 *                 in encoder.h                                       *
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

#include "common.h"
#include "encoder.h"

/*****************************************************************************
 ************************** Start of Subroutines *****************************
 *****************************************************************************/

/*****************************************************************************
 * FFT computes fast fourier transform of BLKSIZE samples of data            *
 *   uses decimation-in-frequency algorithm described in "Digital            *
 *   Signal Processing" by Oppenheim and Schafer, refer to pages 304         *
 *   (flow graph) and 330-332 (Fortran program in problem 5)                 *
 *   to get the inverse fft, change line 20 from                             *
 *                 w_imag[L] = -sin(PI/le1);                                 *
 *                          to                                               *
 *                 w_imag[L] = sin(PI/le1);                                  *
 *                                                                           *
 *   required constants:                                                     *
 *         #define      PI          3.14159265358979                         *
 *         #define      BLKSIZE     1024                                     *
 *         #define      LOGBLKSIZE  10                                       *
 *                                                                           *
 *****************************************************************************/

void fft(float *x_real, float *x_imag, float *energy, float *phi)
{
 static int     M, MM1;
 static int     init=0, N, NV2, NM1;
 static double  w_real[LOGBLKSIZE], w_imag[LOGBLKSIZE];
 int            i,j,k,ll;
 int            ip, le,le1;
 double         t_real, t_imag, u_real, u_imag;

 if(init==0) {
    memset((char *) w_real, 0, sizeof(w_real));  /* preset statics to 0 */
    memset((char *) w_imag, 0, sizeof(w_imag));  /* preset statics to 0 */
    M = LOGBLKSIZE;
    MM1 = LOGBLKSIZE-1;
    N = BLKSIZE;
    NV2 = BLKSIZE >> 1;
    NM1 = BLKSIZE - 1;
    for(ll=0; ll<M; ll++){
       le = 1 << (M-ll);
       le1 = le >> 1;
       w_real[ll] = cos(PI/le1);
       w_imag[ll] = -sin(PI/le1);
    }          
    init++;
 }
 for(ll=0; ll<MM1; ll++){
    le = 1 << (M-ll);
    le1 = le >> 1;
    u_real = 1;
    u_imag = 0;
    for(j=0; j<le1; j++){
       for(i=j; i<N; i+=le){
          ip = i + le1;
          t_real = x_real[i] + x_real[ip];
          t_imag = x_imag[i] + x_imag[ip];
          x_real[ip] = x_real[i] - x_real[ip];
          x_imag[ip] = x_imag[i] - x_imag[ip];
          x_real[i] = t_real;
          x_imag[i] = t_imag;
          t_real = x_real[ip];
          x_real[ip] = x_real[ip]*u_real - x_imag[ip]*u_imag;
          x_imag[ip] = x_imag[ip]*u_real + t_real*u_imag;
       }
       t_real = u_real;
       u_real = u_real*w_real[ll] - u_imag*w_imag[ll];
       u_imag = u_imag*w_real[ll] + t_real*w_imag[ll];
    }
 }
 /* special case: ll = M-1; all Wn = 1 */
 for(i=0; i<N; i+=2){
    ip = i + 1;
    t_real = x_real[i] + x_real[ip];
    t_imag = x_imag[i] + x_imag[ip];
    x_real[ip] = x_real[i] - x_real[ip];
    x_imag[ip] = x_imag[i] - x_imag[ip];
    x_real[i] = t_real;
    x_imag[i] = t_imag;
    energy[i] = x_real[i]*x_real[i] + x_imag[i]*x_imag[i];
    if(energy[i] <= 0.0005){phi[i] = 0;energy[i] = 0.0005;}
    else phi[i] = atan2((double) x_imag[i],(double) x_real[i]);
    energy[ip] = x_real[ip]*x_real[ip] + x_imag[ip]*x_imag[ip];
    if(energy[ip] == 0)phi[ip] = 0;
    else phi[ip] = atan2((double) x_imag[ip],(double) x_real[ip]);
 }
 /* this section reorders the data to the correct ordering */
 j = 0;
 for(i=0; i<NM1; i++){
    if(i<j){
/* use this section only if you need the FFT in complex number form *
 * (and in the correct ordering)                                    */
       t_real = x_real[j];
       t_imag = x_imag[j];
       x_real[j] = x_real[i];
       x_imag[j] = x_imag[i];
       x_real[i] = t_real;
       x_imag[i] = t_imag;
/* reorder the energy and phase, phi                                        */
       t_real = energy[j];
       energy[j] = energy[i];
       energy[i] = t_real;
       t_real = phi[j];
       phi[j] = phi[i];
       phi[i] = t_real;
    }
    k=NV2;
    while(k<=j){
       j = j-k;
       k = k >> 1;
    }
    j = j+k;
 }
}
