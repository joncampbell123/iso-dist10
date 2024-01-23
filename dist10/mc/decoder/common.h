/**********************************************************************
 * ISO MPEG Audio Subgroup Software Simulation Group (1996)
 * ISO 13818-3 MPEG-2 Audio Multichannel Decoder
 *
 * $Id: common.h 2.2 1997/01/17 04:31:07 rowlands Exp $
 *
 * $Log: common.h $
 * Revision 2.2  1997/01/17 04:31:07  rowlands
 * Updated pagkage revision number
 *
 * Revision 2.1  1996/04/18 05:37:23  rowlands
 * Release following Florence meeting
 *
 * Revision 2.0  1996/02/12 07:19:57  rowlands
 * Release following Munich meeting
 *
 * Revision 1.9.1.1  1996/01/20  17:28:45  rowlands
 * Received from Ralf Schwalbe (Telekom FTZ) - includes prediction
 *
 * Revision 1.7.1.3  1995/08/14  08:11:37  tenkate
 * ML-LSF added WtK 7/8/95
 * struct frame_params extended.
 *
 * Revision 1.7.1.1  1995/07/14  06:12:46  rowlands
 * Updated dynamic crosstalk from FTZ: revision FTZ_03
 *
 * Revision 1.6  1995/06/22  01:23:00  rowlands
 * Merged FTZ dynamic crosstalk
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
 *								      *
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

#define VERSION "$Revision: 2.2 $"

/***********************************************************************
*
*  Global Conditional Compile Switches
*
***********************************************************************/
/*#define PRINTOUT*/

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
#include <stdint.h>
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

/***********************************************************************
*
*  Global Definitions
*
***********************************************************************/
extern int pred_coef_table[6][16];  /* def. in decode.c */
extern double S_freq;
extern int Bitrate, Frame_Bits;
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

	unsigned int	lfe_alloc;
	unsigned int	lfe_scf;
	unsigned int	lfe_spl[12];
	double		lfe_spl_fraction[12]; 

	int audio_mix;		   /* large or small room */
	int no_of_multi_lingual_ch;/* number of multilingual channels */
	int multi_lingual_fs;	   /* fs of main channels and ML-fs are the same*/
	int multi_lingual_layer;   /* Layer IIML or Layer II ML is used*/
	int mc_prediction[8];	   /* prediction bit *//* 28.9. R.S. changed name and length like DIS*/
	int mc_pred_coeff[8][6][3];/* bits for prediction coefficient */
	int tc_alloc[12];	   /* transmission channel allocation in each sbgroup*/
	int mc_predsi[8][6];	   /* predictor select information*/
	int mc_delay_comp[8][6];   /* 3 bit for delay compensations factor */
	int ext_bit_stream_present;
	int copyright_ident_bit;   /* additional copyright bits */
	int copyright_ident_start; /*                           */
	int n_ad_bytes;	
	int mc_prediction_on;	   
	int tc_sbgr_select;
	int dyn_cross_on;
	int dyn_cross_LR;
	int dyn_cross_mode[12];
	int dyn_second_stereo[12]; /* 960816 FdB changed from scalar to array */
	int tc_allocation;
	int ext_syncword;         /* 12 bits */
	int ext_crc_check;        /* 16 bits */
	int ext_length;           /* 11 bits *//* Important!! in bits per frame */
	int reserved_bit;
unsigned int ext_crc_bits[15];
#ifdef Augmentation_7ch
	int aug_mtx_proc;
	int aug_dyn_cross_on;
	int aug_future_ext;
	int tc_aug_alloc[12];	   /* transmission channel allocation in each sbgroup */
	int dyn_cross_aug_mode[12];
#endif
} layer, *the_layer;

/* "bit_stream.h" Type Definitions */

typedef struct  bit_stream {
    FILE        *pt;            /* pointer to bit stream device */
    unsigned char *bits;        /* bit stream bit buffer */
    int         header_size;	/* header of bitstream (in number of bytes) */
    long        totbits;        /* bit counter of bit stream */
    long        curpos;         /* bit pointer of bit stream */
    int         mode;           /* bit stream open in read or write mode */
    int         eobs;           /* end of bit stream flag */
    char        format;		/* format of file in rd mode (BINARY/ASCII) */
} Bit_stream;

/* Parent Structure Interpreting some Frame Parameters in Header */

typedef struct {
	layer		*header;	/* raw header information */
	Bit_stream	*bs_mpg;
	Bit_stream	*bs_ext;
	Bit_stream	*bs_mc;
	int		actual_mode;    /* when writing IS, may forget if 0 chs */
	al_table	*alloc;         /* bit allocation table read in */
	al_table	*alloc_mc;      /* MC bit allocation table read in */
	al_table	*alloc_ml;      /* ML bit allocation table read in */
	int		tab_num; 	/* number of table as loaded */
	int		tab_num_mc; 	/* number of MC-table as loaded */
	int		tab_num_ml;     /* number of ML-table as loaded */
	int		stereo;         /* 1 for mono, 2 for stereo */
	int		mc_channel;
	int		jsbound; 	/* first band of joint stereo coding */
	double		mnr_min;	/* mnr for dynamic bitallocation */
	int		sblimit;	/* total number of sub bands */
	int		sblimit_mc;     /* total number of MC sub bands */
	int		sblimit_ml;     /* total number of ML sub bands */
	int		alloc_bits;	/* to read the right length of tc_alloc field */  
	int		dyn_cross_bits; /* to read the right length of dyn_cross field */
	int		pred_mode;      /* entry for prediction table */
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
				int32_t    ckSize;
} ChunkHeader;

typedef struct  Chunk_struct {
				ID      ckID;
				int32_t    ckSize;
				ID      formType;
} Chunk;

typedef struct  CommonChunk_struct {
				ID              ckID;
				int32_t            ckSize;
				int16_t           numChannels;
				uint32_t   numSampleFrames;
				int16_t           sampleSize;
				char            sampleRate[10];
} CommonChunk;

typedef struct  SoundDataChunk_struct {
				ID              ckID;
				int32_t            ckSize;
				uint32_t   offset;
				uint32_t   blockSize;
} SoundDataChunk;

typedef struct  blockAlign_struct {
				uint32_t   offset;
				uint32_t   blockSize;
} blockAlign;

typedef struct  IFF_AIFF_struct {
				int16_t           numChannels;
                uint32_t   numSampleFrames;
                int16_t           sampleSize;
		double          sampleRate;
                ID/*char**/     sampleType;/*must be allocated 21.6.93 SR*/
                blockAlign      blkAlgn;
} IFF_AIFF;

/***********************************************************************
*
*  Global Variable External Declarations
*
***********************************************************************/
extern layer    info;
extern char     *mode_names[4];
extern char     *layer_names[3];
extern double   s_freq[4];
extern int	bitrate[3][15];
extern double	multiple[64];
extern int      sb_groups[12];
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
extern void	      mc_pick_table(frame_params*);
extern int            js_bound(int, int);
extern void           hdr_to_frps(frame_params*);
extern void	      mc_hdr_to_frps(frame_params*);
extern void           WriteHdr(frame_params*, FILE*);
extern void           *mem_alloc(unsigned long, char*);
extern void           mem_free(void**);
extern void           double_to_extended(double*, char[10]);
extern void           extended_to_double(char[10], double*);
extern int	      aiff_read_headers(FILE*, IFF_AIFF*, int*);
extern int	      aiff_seek_to_sound_data(FILE*);
extern int            aiff_write_headers(FILE*, IFF_AIFF*);
extern int            open_bit_stream_r(Bit_stream*, char*, int);
extern void           close_bit_stream_r(Bit_stream*);
extern unsigned int   get1bit(Bit_stream*);
extern unsigned long  getbits(Bit_stream*, int);
extern void	      program_information(void);
extern int            end_bs(Bit_stream*);
extern int            seek_sync_mpg(Bit_stream*);
extern int            seek_sync_ext(Bit_stream*, frame_params*);
extern void           update_CRC(unsigned int, unsigned int, unsigned int*);
extern void           I_CRC_calc(frame_params*, unsigned int[2][SBLIMIT],
						unsigned int*);
extern void           II_CRC_calc(frame_params*, unsigned int[7][SBLIMIT],
						unsigned int[7][SBLIMIT], unsigned int*);
extern void           mc_error_check(frame_params*, unsigned int[7][SBLIMIT],
						unsigned int[7][SBLIMIT], unsigned int*, int, int);
#ifdef Augmentation_7ch
extern void           mc_aug_error_check(frame_params*, unsigned int[7][SBLIMIT],
						unsigned int[7][SBLIMIT], unsigned int*);
#endif
extern int	      mc_ext_error_check(frame_params*, int, int);

#ifdef  MACINTOSH
extern void           set_mac_file_attr(char[MAX_NAME_SIZE], short, OsType,
						OsType);
#endif

#else
extern FILE           *OpenTableFile();
extern int            read_bit_alloc();
extern int            pick_table();
extern int            mc_pick_table();
extern int            js_bound();
extern void           hdr_to_frps();
extern void           mc_hdr_to_frps();
extern void           WriteHdr();
extern void           *mem_alloc();
extern void           mem_free();
extern void           double_to_extended();
extern void           extended_to_double();
extern int            aiff_read_headers();
extern int            aiff_seek_to_sound_data();
extern int            aiff_write_headers();
extern int            open_bit_stream_r();
extern void           close_bit_stream_r();
extern unsigned int   get1bit();
extern unsigned long  getbits();
extern void	      program_information();
extern int            end_bs();
extern int            seek_sync_mpg();
extern int            seek_sync_ext();
extern void           update_CRC();
extern void           I_CRC_calc();
extern void           II_CRC_calc();
extern void   	      mc_error_check();
#ifdef Augmentation_7ch
extern void   	      mc_aug_error_check();
#endif
extern void   	      mc_ext_check();

#ifdef  MACINTOSH
extern void           set_mac_file_attr();
#endif

#endif
