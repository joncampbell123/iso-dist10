/**********************************************************************
 * Copyright (c) 1995 MPEG/audio software simulation group
 * All Rights Reserved
 * $Id: decoder.h,v 1.1 1996/04/18 03:39:59 rowlands Exp $
 *
 * MPEG/audio coding/decoding software, work in progress
 *   NOT for public distribution until verified and approved by the
 *   MPEG/audio committee.
 *
 * $Log: decoder.h,v $
 * Revision 1.1  1996/04/18 03:39:59  rowlands
 * Initial revision
 *
 * Revision 1.3  1995/04/06  05:01:42  rowlands
 * Ran decoder through protoize to convert to ANSI C.
 *
 * Revision 1.2  1995/04/06  04:56:00  rowlands
 * Added header, RCS info and cleaned up prototypes in initial revision.
 *
 **********************************************************************/

/**********************************************************************
 * VERSION 2.5                                                        *
 *   changes made since last update:                                  *
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
 *  Version 1.0 Shareware                                             *
 *                                                                    *
 *  07/12/94       Ralf Schwalbe,  Telekom FTZ Berlin                 *
 *                 Tel: +49 30 6708 2406                              *
 *                 Fax: +49 30 6774 539                               *
 *								      *
 *  04/11/94	   Ralf Schwalbe. Telekom FTZ Berlin                  *
 *                  - decoding extension bitstream                    *
 *                  - some new subroutines, globale variables and     *
 *                    structures (important to handle the ext. bitst.)*
 *                  - changed all functions to ANSI-C funktion header *
 *		    - corrected some bugs to decode bitstreams > 512kB*
 *								      *
 *  Version 1.1 Shareware                                             *
 *                                                                    *
 *  07/12/94       Ralf Schwalbe,  Telekom FTZ Berlin                 *
 *                 Tel: +49 30 6708 2406                              *
 *                 Fax: +49 30 6774 539                               *
 *								      *
 **********************************************************************/

/***********************************************************************
*
*  Decoder Include Files
*
***********************************************************************/

/***********************************************************************
*
*  Decoder Definitions
*
***********************************************************************/

#define  DFLT_OPEXT        ".dec"  	/* default output file name extension */
#define  DFLT_OPEXT_EXT	   ".ext" 	/* default input for extension file */
#define	 FILTYP_DEC_AIFF   "AIFF"	/* '-> " . 7/13/92. sr */
#define	 FILTYP_DEC_BNRY   "TEXT"	/* '-> " . 7/13/92. sr */
#define	 CREATR_DEC_AIFF   "Sd2a"	/* '-> " . 7/13/92. sr */
#define	 CREATR_DEC_BNRY   "????"	/* '-> " . 7/13/92. sr */

#define   SYNC_WORD         (long) 0xfff
#define   SYNC_WORD_LNGTH   12

#define   MUTE              0
#define   STEP		    0.03125	 /* 13.10.93 R.S. step for prediction */
#define   PREDDEL           (2+7)        /* max. delay in prediction, HP 08-nov-94 */

/***********************************************************************
*
*  Decoder Type Definitions
*
***********************************************************************/

/***********************************************************************
*
*  Decoder Variable External Declarations
*
***********************************************************************/
extern int bits_in_frame;
extern unsigned long frameNum;
extern unsigned int ext_length;
 
/***********************************************************************
*
*  Decoder Function Prototype Declarations
*
***********************************************************************/

/* The following functions are in the file "musicout.c" */

#ifdef   PROTO_ARGS
extern void   usage(void);
#else
extern void   usage();
#endif

/* The following functions are in the file "decode.c" */

#ifdef   PROTO_ARGS
extern void   decode_info(Bit_stream_struc*, frame_params*);
extern void   II_decode_bitalloc(Bit_stream_struc*, unsigned int[5][SBLIMIT],
					   frame_params*, int*, int*);
extern void   II_decode_scale(Bit_stream_struc*, unsigned int[5][SBLIMIT],
					   unsigned int[5][SBLIMIT], unsigned int[5][3][SBLIMIT],
					   frame_params*, int*, int*);
extern void   II_buffer_sample(Bit_stream_struc*, unsigned int[5][3][SBLIMIT],
					   unsigned int[5][SBLIMIT], frame_params*);
extern void   II_buffer_samplemc(Bit_stream_struc*, unsigned int[5][3][SBLIMIT],
					   unsigned int[5][SBLIMIT], frame_params*, int*);
extern void   read_quantizer_table(double[17], double[17]);
extern void   II_dequantize_sample(unsigned int[5][3][SBLIMIT],
					   unsigned int[5][SBLIMIT], double[5][SBLIMIT][3][12],
					   frame_params*, int*);
extern void   II_dequantize_samplemc(unsigned int[5][3][SBLIMIT],
					   unsigned int[5][SBLIMIT], double[5][SBLIMIT][3][12],
					   frame_params*, int*, int*);
extern void   read_scale_factor(double[SCALE_RANGE]);
extern void   II_denormalize_sample(double[5][SBLIMIT][3][12],
					   unsigned int[5][3][SBLIMIT], frame_params*, int, int*);
extern void   II_denormalize_samplemc(double[5][SBLIMIT][3][12],
					   unsigned int[5][3][SBLIMIT], frame_params*, int, int*,
					   int*);
extern void   create_syn_filter(double[64][SBLIMIT]);
extern int    SubBandSynthesis (double*, int, long*);
extern void   read_syn_window(double[HAN_SIZE]);
extern void   window_sample(double*, double*);
extern void   out_fifo(long[5][3][SBLIMIT], int, frame_params*, int, FILE*,
					   unsigned long*);
extern void   buffer_CRC(Bit_stream_struc*, unsigned int*);
extern void   recover_CRC_error(long[5][3][SBLIMIT], int, frame_params*,
					   FILE*, unsigned long*);
extern void   mc_header(Bit_stream_struc*, frame_params*);
extern int    mc_ext_header(void);
extern int    mc_ext_error_check(void);
extern void   mc_composite_status_info(Bit_stream_struc*, frame_params*);

extern
void
dematricing(
	double (*pcm_sample)[SBLIMIT][3][12],
	frame_params *fr_ps,
	double (*pred_buf)[8][36+PREDDEL]
);

#else
extern void   decode_info();
extern void   II_decode_bitalloc();
extern void   II_decode_scale();
extern void   II_buffer_sample();
extern void   read_quantizer_table();
extern void   II_dequantize_sample();
extern void   read_scale_factor();
extern void   II_denormalize_sample();
extern void   create_syn_filter();
extern int    SubBandSynthesis ();
extern void   read_syn_window();
extern void   window_sample();
extern void   out_fifo();
extern int    mc_ext_header();
extern int    mc_ext_error_check();
extern void   buffer_CRC();
extern void   recover_CRC_error();
#endif
