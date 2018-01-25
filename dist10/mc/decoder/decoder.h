/**********************************************************************
 * ISO MPEG Audio Subgroup Software Simulation Group (1996)
 * ISO 13818-3 MPEG-2 Audio Multichannel Decoder
 *
 * $Id: decoder.h 1.11 1996/04/18 05:37:23 rowlands Exp $
 *
 * $Log: decoder.h $
 * Revision 1.11  1996/04/18 05:37:23  rowlands
 * Release following Florence meeting
 *
 * Revision 1.10  1996/02/12 07:13:06  rowlands
 * Release following Munich meeting
 *
 * Revision 1.9.1.1  1996/01/20  17:28:45  rowlands
 * Received from Ralf Schwalbe (Telekom FTZ) - includes prediction
 *
 * Revision 1.7.1.3  1995/08/14  08:13:06  tenkate
 * ML-LSF added Warner ten Kate 7/8/95 (Philips)
 *
 * Revision 1.7.1.1  1995/07/14  06:12:46  rowlands
 * Updated dynamic crosstalk from FTZ: revision FTZ_03
 *
 * Revision 1.3.3.1  1995/06/16  08:00:46  rowlands
 * Input from Sang Wook Kim (Samsung AIT)
 *
 * Revision 1.3.1.1  1995/06/14  04:35:34  rowlands
 * Added declaration for dematrix procedure 2 routines.
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
 *		  						      *
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
 *		   ancillary data is working			      *
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

#define  DFLT_IPEXT_MPG	   ".mpg" 	/* default input for base file */
#define  DFLT_IPEXT_EXT	   ".ext" 	/* default input for extension file */
#define  DFLT_OPEXT_DEC	   ".dec"  	/* default output file name extension */
#define  DFLT_OPEXT_LFE	   "_lfe.dec" 	/* 10/31/95 R.S. output for lfe pcm-file */
#define  DFLT_OPEXT_ML	   ".ml" 	/* default MultiLingual output file name extension */
#define	 FILTYP_DEC_AIFF   "AIFF"	/* '-> " . 7/13/92. sr */
#define	 FILTYP_DEC_BNRY   "TEXT"	/* '-> " . 7/13/92. sr */
#define	 CREATR_DEC_AIFF   "Sd2a"	/* '-> " . 7/13/92. sr */
#define	 CREATR_DEC_BNRY   "????"	/* '-> " . 7/13/92. sr */

#define   SYNC_WORD         (long) 0xfff
#define   SYNC_WORD_LNGTH   12

#define   MUTE              0
#define   STEP		    0.03125	 /* 13.10.93 R.S. step for prediction */
#define   PREDDEL           (2+7)        /* max. delay in prediction, HP 08-nov-94 */

#define   EXTENSION_CRC_CHECK
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
extern void   decode_info(Bit_stream*, frame_params*);
extern void   mc_header(Bit_stream*, frame_params*);
extern void   mc_composite_status_info(Bit_stream*, frame_params*, int, int);
#ifdef Augmentation_7ch
extern void   mc_aug_composite_status_info(Bit_stream*, frame_params*, int, int);
#endif
extern void   I_decode_bitalloc(Bit_stream*, frame_params*, unsigned int[7][SBLIMIT], int);
extern void   II_decode_bitalloc(Bit_stream*, frame_params*, unsigned int[7][SBLIMIT], int);
extern void   II_decode_bitalloc_mc(Bit_stream*, frame_params*, unsigned int[7][SBLIMIT], int*, int*, int);
#ifdef Augmentation_7ch
extern void   II_decode_bitalloc_aug(Bit_stream*, frame_params*, unsigned int[7][SBLIMIT], int*, int*, int);
#endif
extern void   II_decode_bitalloc_ml(Bit_stream*, frame_params*, unsigned int[7][SBLIMIT], int*);
extern void   I_decode_scale(Bit_stream*, frame_params*, unsigned int[7][SBLIMIT],
					   unsigned int[7][3][SBLIMIT],
					   int, int);
extern void   II_decode_scale(Bit_stream*, frame_params*, unsigned int[7][SBLIMIT],
					   unsigned int[7][SBLIMIT], unsigned int[7][3][SBLIMIT],
					   int*, int*, int);
extern void   II_decode_scale_ml(Bit_stream*, frame_params*, unsigned int[7][SBLIMIT],
					   unsigned int[7][SBLIMIT], unsigned int[7][3][SBLIMIT],
					   int*);
extern void   I_buffer_sample(Bit_stream*, frame_params*, unsigned int[7][3][SBLIMIT],
					   unsigned int[7][SBLIMIT]);
extern void   II_buffer_sample(Bit_stream*, frame_params*, unsigned int[7][3][SBLIMIT],
					   unsigned int[7][SBLIMIT]);
extern void   II_buffer_sample_mc(Bit_stream*, frame_params*, unsigned int[7][3][SBLIMIT],
					   unsigned int[7][SBLIMIT], int, int, int);
#ifdef Augmentation_7ch
extern void   II_buffer_sample_aug(Bit_stream*, frame_params*, unsigned int[7][3][SBLIMIT],
					   unsigned int[7][SBLIMIT], int);
#endif
extern void   II_buffer_sample_ml(Bit_stream*, frame_params*, unsigned int[7][3][SBLIMIT],
					   unsigned int[7][SBLIMIT], int*);
extern void   I_dequantize_sample(unsigned int[7][3][SBLIMIT], int,
					   unsigned int[7][SBLIMIT], double[7][SBLIMIT][3][12],
					   frame_params*, int*);
extern void   II_dequantize_sample(unsigned int[7][3][SBLIMIT],
					   unsigned int[7][SBLIMIT], double[7][SBLIMIT][3][12],
					   frame_params*, int*);
extern void   II_dequantize_sample_mc(unsigned int[7][3][SBLIMIT],
					   unsigned int[7][SBLIMIT], double[7][SBLIMIT][3][12],
					   frame_params*, int, int, int*);
#ifdef Augmentation_7ch
extern void   II_dequantize_sample_aug(unsigned int[7][3][SBLIMIT],
					   unsigned int[7][SBLIMIT], double[7][SBLIMIT][3][12],
					   frame_params*, int*);
#endif
extern void   II_dequantize_sample_ml(unsigned int[7][3][SBLIMIT],
					   unsigned int[7][SBLIMIT], double[7][SBLIMIT][3][12],
					   frame_params*, int*, int*);
extern void   II_lfe_calc(frame_params*);
extern void   I_denormalize_sample(double[7][SBLIMIT][3][12],
					   unsigned int[7][3][SBLIMIT], int, frame_params*, int*);
extern void   II_denormalize_sample(double[7][SBLIMIT][3][12],
					   unsigned int[7][3][SBLIMIT], frame_params*, int, int*);
extern void   II_denormalize_sample_mc(double[7][SBLIMIT][3][12],
					   unsigned int[7][3][SBLIMIT], frame_params*, int, int, int, int*);
#ifdef Augmentation_7ch
extern void   II_denormalize_sample_aug(double[7][SBLIMIT][3][12],
					   unsigned int[7][3][SBLIMIT], frame_params*, int, int*);
#endif
extern void   II_denormalize_sample_ml(double[7][SBLIMIT][3][12],
					   unsigned int[7][3][SBLIMIT], frame_params*, int, int*, int*);
extern int    SubBandSynthesis(double*, int, long*);
extern int    SubBandSynthesis_ml(double*, int, long*);
extern void   out_fifo(long[7][3][SBLIMIT], int, frame_params*, int, FILE*, unsigned long*,int);
extern void   out_fifo_ml(long[7][3][SBLIMIT], int, frame_params*, int, FILE*, unsigned long*);
extern void   buffer_CRC(Bit_stream*, unsigned int*);
extern void   recover_CRC_error(long[7][3][SBLIMIT], int, frame_params*,FILE*,unsigned long*,int);
extern void   dematricing_mc(double[7][SBLIMIT][3][12], frame_params*, double[2][8][36+PREDDEL]);
#ifdef Augmentation_7ch
extern void   dematricing_aug(double[7][SBLIMIT][3][12], frame_params*);
#endif
extern void   denormalizing_mc(double[7][SBLIMIT][3][12], frame_params*, int); 
#ifdef Augmentation_7ch
extern void   denormalizing_aug(double[7][SBLIMIT][3][12], frame_params*); 
#endif
#else
extern void   decode_info();
extern void   mc_header();
extern void   mc_composite_status_info();
#ifdef Augmentation_7ch
extern void   mc_aug_composite_status_info();
#endif
extern void   I_decode_bitalloc();
extern void   II_decode_bitalloc();
extern void   II_decode_bitalloc_mc();
#ifdef Augmentation_7ch
extern void   II_decode_bitalloc_aug();
#endif
extern void   II_decode_bitalloc_ml();
extern void   I_decode_scale();
extern void   II_decode_scale();
extern void   II_decode_scale_ml();
extern void   I_buffer_sample();
extern void   II_buffer_sample();
extern void   II_buffer_sample_mc();
#ifdef Augmentation_7ch
extern void   II_buffer_sample_aug();
#endif
extern void   II_buffer_sample_ml();
extern void   I_dequantize_sample();
extern void   II_dequantize_sample();
extern void   II_dequantize_sample_mc();
#ifdef Augmentation_7ch
extern void   II_dequantize_sample_aug();
#endif
extern void   II_dequantize_sample_ml();
extern void   II_lfe_calc();
extern void   I_denormalize_sample();
extern void   II_denormalize_sample();
extern void   II_denormalize_sample_mc();
#ifdef Augmentation_7ch
extern void   II_denormalize_sample_aug();
#endif
extern void   II_denormalize_sample_ml();
extern int    SubBandSynthesis();
extern int    SubBandSynthesis_ml();
extern void   out_fifo();
extern void   out_fifo_ml();
extern void   buffer_CRC();
extern void   recover_CRC_error();
extern void   dematricing_mc();
#ifdef Augmentation_7ch
extern void   dematricing_aug();
#endif
extern void   denormalizing_mc(); 
#ifdef Augmentation_7ch
extern void   denormalizing_aug(); 
#endif
#endif
