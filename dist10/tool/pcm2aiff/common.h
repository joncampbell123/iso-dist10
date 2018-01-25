/**********************************************************************
 * Copyright (c) 1995 MPEG/audio software simulation group
 * All Rights Reserved
 * $Id: common.h,v 1.1 1996/04/18 03:39:59 rowlands Exp $
 *
 * MPEG/audio coding/decoding software, work in progress
 *   NOT for public distribution until verified and approved by the
 *   MPEG/audio committee.
 *
 * $Log: common.h,v $
 * Revision 1.1  1996/04/18 03:39:59  rowlands
 * Initial revision
 *
 * Revision 1.4  1995/04/06  05:49:24  rowlands
 * Moved machine #define to makefile
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

#define VERSION "1.1"  /* R.S for Shareware */


/***********************************************************************
*
*  Global Conditional Compile Switches
*
***********************************************************************/
/*#define PRINTOUT*/
/*#define UNIX	        */ 	 /* Unix conditional compile switch */
/*#define MACINTOSH	*/	 /* Macintosh conditional compile switch */
/*#define MS_DOS	*/	 /* IBM PC conditional compile switch
				   Microsoft C ver. 6.0 */


#ifdef  UNIX
#define         TABLES_PATH     "tables/"  /* to find data files */
/* name of environment variable holding path of table files */
#define         MPEGTABENV      "MPEGTABLES"
#define         PATH_SEPARATOR  "/"        /* how to build paths */
#define      PROTO_ARGS                 /* unix gcc uses arg. prototypes */
#endif  /* UNIX */

#ifdef  MACINTOSH
/* #define      TABLES_PATH ":tables:"  /* where to find data files */
#define         PROTO_ARGS              /* Mac uses argument prototypes */
#endif  /* MACINTOSH */

#ifdef  MS_DOS
#define         PROTO_ARGS  /* DOS uses argument prototypes */
#endif  /* MS_DOS */


/* MS_DOS and VMS do not define TABLES_PATH, so OpenTableFile will default
   to finding the data files in the default directory */

/***********************************************************************
*
*  Global Include Files
*
***********************************************************************/

#include        <stdio.h>
#include        <string.h>
#include        <math.h>

#ifdef  UNIX
#include        <unistd.h>	 /* removed 92-08-05 shn */
#include	<stdlib.h>       /* put in 92-08-05 shn */
#endif  /* UNIX */

#ifdef  MACINTOSH
#include        <stdlib.h>
#include        <console.h>
#endif  /* MACINTOSH */

#ifdef  MS_DOS
/* #include	<alloc.h> */ /* removed 92-07-08 sr */
#include	<malloc.h>   /* put in 92-07-08 sr */
/* #include	<mem.h> */   /* removed 92-07-08 sr */
#include <stdlib.h>
#endif  /* MS_DOS */


#define	SEEK_SET	0
#define	SEEK_CUR	1
#define	SEEK_END	2


/***********************************************************************
*
*  Global Definitions
*
***********************************************************************/
extern int pred_coef_table[8][14];  /* def. in decode.c */
extern int read_ext_header,ext_error;
extern unsigned long frameNum,ext_total;
/* General Definitions */

#ifdef  MS_DOS
#define         FLOAT                   double
#else
#define         FLOAT                   float
#endif

#define         FALSE                   0
#define         TRUE                    (!FALSE)
#define         NULL_CHAR               '\0'

#define         MAX_U_32_NUM            0xFFFFFFFF
#define         PI                      3.14159265358979
#define         PI4                     PI/4
#define         PI64                    PI/64
#define         LN_TO_LOG10             0.2302585093

#define         VOL_REF_NUM             0
#define         MPEG_AUDIO_ID           1
#define         MAC_WINDOW_SIZE         24

#define         MONO                    1
#define         STEREO                  2
#define         BITS_IN_A_BYTE          8
#define         WORD                    16
#define         MAX_NAME_SIZE           81
#define         SBLIMIT                 32
#define         FFT_SIZE                1024
#define         HAN_SIZE                512
#define         SCALE_BLOCK             12
#define         SCALE_RANGE             64
#define         SCALE                   32768.0
#define         CRC16_POLYNOMIAL        0x8005

/* Sync - Word for multichannel extern bitstream */
#define		EXT_SYNCWORD	        0x7ff

/* MPEG Header Definitions - Mode Values */

#define         MPG_MD_STEREO           0
#define         MPG_MD_JOINT_STEREO     1
#define         MPG_MD_DUAL_CHANNEL     2
#define		MPG_MD_MONO		3
#define		MPG_MD_NONE		4

/* Multi-channel Definitions - Mode Values */

#define		MPG_MC_STEREO 	       0
#define		MPG_MC_NONE	       6

/* AIFF Definitions */

#ifndef	MS_DOS
#define         IFF_ID_FORM             "FORM" /* HP400 unix v8.0: double qoutes 1992-07-24 shn */
#define         IFF_ID_AIFF             "AIFF"
#define         IFF_ID_COMM             "COMM"
#define         IFF_ID_SSND             "SSND"
#define         IFF_ID_MPEG             "MPEG"
#else
#define         IFF_ID_FORM             "FORM"
#define         IFF_ID_AIFF             "AIFF"
#define         IFF_ID_COMM             "COMM"
#define         IFF_ID_SSND             "SSND"
#define         IFF_ID_MPEG             "MPEG"
#endif

/* "bit_stream.h" Definitions */

#define         MINIMUM         4    /* Minimum size of the buffer in bytes */
#define         MAX_LENGTH      32   /* Maximum length of word written or
										read from bit stream */
#define         READ_MODE       0
#define         WRITE_MODE      1
#define         ALIGNING        8
#define         BINARY          0
#define         ASCII           1
#define         BS_FORMAT       ASCII /* BINARY or ASCII = 2x bytes */
#define         BUFFER_SIZE     4096

#define         MIN(A, B)       ((A) < (B) ? (A) : (B))
#define         MAX(A, B)       ((A) > (B) ? (A) : (B))

/***********************************************************************
*
*  Global Type Definitions
*
***********************************************************************/

/* Structure for Reading Layer II Allocation Tables from File */

typedef struct {
		unsigned int    steps;
		unsigned int    bits;
		unsigned int    group;
		unsigned int    quant;
} sb_alloc, *alloc_ptr;

typedef sb_alloc        al_table[SBLIMIT][16];

/* Header Information Structure */


typedef struct {
	int version;
	int lay;
	int error_protection;
	int bitrate_index;
	int bitrate_index1;
	int bitrate_index2;
	int sampling_frequency;
	int padding;
	int extension;
	int mode;
	int mode_ext;
	int copyright;
	int original;
	int emphasis;
	int center;		   /* center present */
	int surround; 		   /* surrounds present*/
	int dematrix_procedure;	   /* type of dematricing */
	int lfe;		   /* low freequency effect channel*/
	int audio_mix;		   /* large or small room */
	int no_of_multi_lingual_ch;/* number of multilingual channels */
	int multi_lingual_fs;	   /* fs of main channels and ML-fs are the same*/
	int multi_lingual_layer;   /* Layer IIML or Layer II ML is used*/
	int mc_prediction[8];	   /* prediction bit *//* 28.9. R.S. changed name and length like DIS*/
	int mc_pred_coeff[8][4][3];/* bits for prediction coefficient */
	int tc_alloc[12];	   /* transmission channel allocation in each sbgroup*/
	int mc_predsi[8][4];	   /* predictor select information*/
	int mc_delay_comp[8][4];   /* 3 bit for delay compensations factor */
	int ext_bit_stream_present;
	int copyright_ident_bit;   /* additional copyright bits */
	int copyright_ident_start; /*                           */
	int n_ad_bytes;	
	int ad_bytes_crc_info;	   /* for MC-CRC -check 7.12.94 R.S */
	int mc_prediction_on;	   
	int tc_sbgr_select;
	int dyn_cross_on;
	int dyn_cross_LR;
	int dyn_cross_mode[12];
	int dyn_second_stereo;
	int tc_allocation;
	int ext_syncword;         /* 12 bits */
	int ext_crc_check;        /* 16 bits */
	int ext_length;           /* 11 bits *//* Important!! in bits per frame */
	int reserved_bit;
unsigned int ext_crc_bits[15];
} layer, *the_layer;

/* Parent Structure Interpreting some Frame Parameters in Header */

typedef struct {
	layer	*header;        /* raw header information */
	int		actual_mode;    /* when writing IS, may forget if 0 chs */
	al_table	*alloc;     /* bit allocation table read in */
	int		tab_num; 	    /* number of table as loaded */
	int		stereo; 		/* 1 for mono, 2 for stereo */
	int		mc_channel;
	int		jsbound; 		/* first band of joint stereo coding */
	double	mnr_min;		/* mnr for dynamic bitallocation */
	int		sblimit;	    /* total number of sub bands */
} frame_params;

/* Double and SANE Floating Point Type Definitions */

typedef struct  IEEE_DBL_struct {
				unsigned long   hi;
				unsigned long   lo;
} IEEE_DBL;

typedef struct  SANE_EXT_struct {
				unsigned long   l1;
				unsigned long   l2;
				unsigned short  s1;
} SANE_EXT;

/* AIFF Type Definitions */

typedef char	 ID[4];

typedef struct  identifier_struct{
		 ID name;
		 long ck_length;
}identifier;


typedef struct  ChunkHeader_struct {
				ID      ckID;
				long    ckSize;
} ChunkHeader;

typedef struct  Chunk_struct {
				ID      ckID;
				long    ckSize;
				ID      formType;
} Chunk;

typedef struct  CommonChunk_struct {
				ID              ckID;
				long            ckSize;
				short           numChannels;
				unsigned long   numSampleFrames;
				short           sampleSize;
				char            sampleRate[10];
} CommonChunk;

typedef struct  SoundDataChunk_struct {
				ID              ckID;
				long            ckSize;
				unsigned long   offset;
				unsigned long   blockSize;
} SoundDataChunk;

typedef struct  blockAlign_struct {
				unsigned long   offset;
				unsigned long   blockSize;
} blockAlign;

typedef struct  IFF_AIFF_struct {
				short           numChannels;
                unsigned long   numSampleFrames;
                short           sampleSize;
		double          sampleRate;
                ID/*char**/     sampleType;/*must be allocated 21.6.93 SR*/
                blockAlign      blkAlgn;
} IFF_AIFF;

/* "bit_stream.h" Type Definitions */

typedef struct  bit_stream_struc {
    FILE        *pt;            /* pointer to bit stream device */
    unsigned char *buf;         /* bit stream buffer */
    int         buf_size;       /* size of buffer (in number of bytes) */
    long        totbit;         /* bit counter of bit stream */
    int         buf_byte_idx;   /* pointer to top byte in buffer */
    int         buf_bit_idx;    /* pointer to top bit of top byte in buffer */
    int         mode;           /* bit stream open in read or write mode */
    int         eob;            /* end of buffer index */
    int         eobs;           /* end of bit stream flag */
    char        format;
/* format of file in rd mode (BINARY/ASCII) */
    } Bit_stream_struc;

/***********************************************************************
*
*  Global Variable External Declarations
*
***********************************************************************/
extern Bit_stream_struc  bs_ext;
extern layer    info;
extern int	read_from_ext;
extern char     *mode_names[4];
extern char     *layer_names[3];
extern double   s_freq[4];
extern int	bitrate[3][15];
extern double	multiple[64];
extern int      sb_groups[12];
extern unsigned char bs_tmp_buf[50];
/***********************************************************************
*
*  Global Function Prototype Declarations
*
***********************************************************************/

/* The following functions are in the file "common.c" */

#ifdef  PROTO_ARGS
extern FILE           *OpenTableFile(char*);
extern int            read_bit_alloc(int, al_table*);
extern int            pick_table(frame_params*);
extern int            js_bound(int, int);
extern void           hdr_to_frps(frame_params*);
extern void           WriteHdr(frame_params*, FILE*);
extern void           WriteBitAlloc(unsigned int[2][SBLIMIT], frame_params*,
								FILE*);
extern void           WriteScale(unsigned int[2][SBLIMIT],
                        unsigned int[2][SBLIMIT], unsigned int[2][3][SBLIMIT],
						frame_params*, FILE*);
extern void           WriteSamples(int, unsigned int/*far*/[SBLIMIT],
						unsigned int[SBLIMIT], frame_params*, FILE*);
extern int            NumericQ(char*);
extern int            BitrateIndex(int, int);
extern int            SmpFrqIndex(long);
extern int            memcheck(char*, int, int);
extern void           *mem_alloc(unsigned long, char*);
extern void           mem_free(void**);
extern void           double_to_extended(double*, char[10]);
extern void           extended_to_double(char[10], double*);
extern int	      aiff_read_headers(FILE*, IFF_AIFF*, int*);
extern int	      aiff_seek_to_sound_data(FILE*);
extern int            aiff_write_headers(FILE*, IFF_AIFF*);
extern void           refill_buffer(Bit_stream_struc*);
extern void           empty_buffer(Bit_stream_struc*, int);
extern void           open_bit_stream_w(Bit_stream_struc*, char*, int);
extern int            open_bit_stream_r(Bit_stream_struc*, char*, int);
extern void           close_bit_stream_r(Bit_stream_struc*);
extern void           close_bit_stream_w(Bit_stream_struc*);
extern void           alloc_buffer(Bit_stream_struc*, int);
extern void           desalloc_buffer(Bit_stream_struc*);
extern void           back_track_buffer(Bit_stream_struc*, int);
extern void 	      skipbits(Bit_stream_struc *, unsigned int);
extern void 	      skip_ext_bits(Bit_stream_struc *, unsigned int);
extern unsigned int   get1bit(Bit_stream_struc*);
extern unsigned int   local_get1bit(Bit_stream_struc*);
extern void           put1bit(Bit_stream_struc*, int);
extern unsigned long  look_ahead(Bit_stream_struc*, int);
extern unsigned long  getbits(Bit_stream_struc*, int);
extern unsigned int   local_getbits(Bit_stream_struc*, int);
extern void           putbits(Bit_stream_struc*, unsigned int, int);
extern void           byte_ali_putbits(Bit_stream_struc*, unsigned int, int);
extern unsigned long  byte_ali_getbits(Bit_stream_struc*, int);
extern unsigned long  sstell(Bit_stream_struc*);
extern int            end_bs(Bit_stream_struc*);
extern int            seek_sync(Bit_stream_struc*, long, int);
extern void           I_CRC_calc(frame_params*, unsigned int[2][SBLIMIT],
						unsigned int*);
extern void           II_CRC_calc(frame_params*, unsigned int[5][SBLIMIT],
						unsigned int[5][SBLIMIT], unsigned int*);
extern void           mc_error_check(frame_params*, unsigned int[7][SBLIMIT],
						unsigned int[7][SBLIMIT], unsigned int*);
extern int	      mc_ext_error_check(void);
extern void           update_CRC(unsigned int, unsigned int, unsigned int*);
extern void           read_absthr(FLOAT*, long);
extern void	      program_information(void);
extern int 	      js_bound1(int, int);	/* other bounderies for multichannel*/

#ifdef  MACINTOSH
extern void           set_mac_file_attr(char[MAX_NAME_SIZE], short, OsType,
						OsType);
#endif

#ifdef  MS_DOS
extern char           *new_ext(char *filename, char *extname); /* 92-08-19 shn */
#endif

#else
extern FILE           *OpenTableFile();
extern int            read_bit_alloc();
extern int            pick_table();
extern int            js_bound();
extern void           hdr_to_frps();
extern void           WriteHdr();
extern void           WriteBitAlloc();
extern void           WriteScale();
extern void           WriteSamples();
extern int            NumericQ();
extern int            BitrateIndex();
extern int            SmpFrqIndex();
extern int            memcheck();
extern void           *mem_alloc();
extern void           mem_free();
extern void           double_to_extended();
extern void           extended_to_double();
extern int            aiff_read_headers();
extern int            aiff_seek_to_sound_data();
extern int            aiff_write_headers();
extern void           refill_buffer();
extern void           empty_buffer();
extern void           open_bit_stream_w();
extern int            open_bit_stream_r();
extern void           close_bit_stream_r();
extern void           close_bit_stream_w();
extern void           alloc_buffer();
extern void           desalloc_buffer();
extern void           back_track_buffer();
extern int            mc_ext_header();
extern int    	      mc_ext_error_check();
extern unsigned int   get1bit();
extern void           put1bit();
extern unsigned long  look_ahead();
extern unsigned long  getbits();
extern void           putbits();
extern void           byte_ali_putbits();
extern unsigned long  byte_ali_getbits();
extern unsigned long  sstell();
extern int            end_bs();
extern int            seek_sync();
extern void           I_CRC_calc();
extern void           II_CRC_calc();
extern void           update_CRC();
extern void           read_absthr();
extern void	      program_information();

#ifdef  MSDOS
extern char           *new_ext(); /* 92-08-19 shn */
#endif

#endif

#ifdef CONCAT
Bit_stream_struc   bs_concat;	/*23/03/1995 JMZ Concat */
#endif CONCAT
