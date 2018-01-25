/**********************************************************************
 * ISO MPEG Audio Subgroup Software Simulation Group (1996)
 * ISO 13818-3 MPEG-2 Audio Multichannel Encoder
 *
 * $Id: encoder.h 1.9 1996/02/12 07:13:35 rowlands Exp $
 *
 * $Log: encoder.h $
 * Revision 1.9  1996/02/12 07:13:35  rowlands
 * Release following Munich meeting
 *
 * Revision 1.8  1996/02/12 05:51:17  rowlands
 * Added verbosity switch to control text output.
 * Removed reference to unused file "fp100".
 *
 * Revision 1.6.2.1  1995/11/06  04:19:12  rowlands
 * Received from Uwe Felderhoff (IRT)
 *
 * Revision 1.7  1995/08/14  08:02:08  tenkate
 * ML-LSF added Warner ten Kate 7/8/95 (Philips)
 *
 * Revision 1.4.3.1  1995/06/16  08:25:11  rowlands
 * Input from Sang Wook Kim (Samsung AIT)
 *
 * Revision 1.4.2.1  1995/06/16  03:46:42  rowlands
 * Input from Susanne Ritscher (IRT)
 *
 * Revision 1.4.1.2  1995/06/16  02:50:56  rowlands
 * Added dematrix procedure 2 procedure prototypes
 *
 **********************************************************************/

/**********************************************************************
 *   date   programmers         comment                               *
 * 2/25/91  Doulas Wong,        start of version 1.0 records          *
 *          Davis Pan                                                 *
 * 5/10/91  W. Joseph Carter    Reorganized & renamed all ".h" files  *
 *                              into "common.h" and "encoder.h".      *
 *                              Ported to Macintosh and Unix.         *
 *                              Added function prototypes for more    *
 *                              rigorous type checking.               *
 * 27jun91  dpwe (Aware)        moved "alloc_*" types, pros to common *
 *                              Use ifdef PROTO_ARGS for prototypes   *
 *                              prototypes reflect frame_params struct*
 * 7/10/91  Earle Jennings      Conversion of all floats to FLOAT     *
 * 10/3/91  Don H. Lee          implemented CRC-16 error protection   *
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
 *                              Modified some function prototypes.    *
 * 13jul92  Susanne Ritscher    MS-DOS, MSC 6.0 port fix.             *
 * 92-11-06 Soren H. Nielsen	  Changed POWERNORM to 96 dB in order *
 * 			        to get FFT levels conforming to ISO.  *
 *  dec 92 Susanne Ritscher     Changed to multi channel with several *
 * 				options                               *
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
 *  Version 2.0                                                       *
 *                                                                    *
 *  01/28/97       Frans de Bont, Philips Sound & Vision, Eindhoven   *
 *		    - dynamic crosstalk working for all configurations*
 *		    - prediction working for all configurations	      *
 *		    - extension bitstream fixed			      *
 *		    - fully compliant to DIS 13818-3.2                *
 *                                                                    *				      *		    
 **********************************************************************/
/**********************************************************************
 *                                                                    *
 *  06/06/95       Yeon Bae Thomas Kim, Samsung AIT                   *
 *                 ancillary data is working                          *
 *                                                                    *
 *  06/06/95       Sang Wook Kim, Samsung AIT                         *
 *                 corrected some bugs                                *
 *                                                                    *
 **********************************************************************/
/***********************************************************************
*
*  Encoder Include Files
*
***********************************************************************/

/***********************************************************************
*
*  Encoder Definitions
*
***********************************************************************/

/* General Definitions */

/* Default Input Arguments (for command line control) */

#define DFLT_LAY	2	/* default encoding layer is II */
#define DFLT_MOD	'r'	/* default mode is stereo front channels */
#define DFLT_PSY	1	/* default psych model is 1 */
#define DFLT_SFQ        48     	/* default input sampling rate is 44.1 kHz, now 48kHz 28.6.93 SR */
#define DFLT_BRT        384    	/* default total output bitrate is 384 kbps */
#define DFLT_EMP        'n'    	/* default de-emphasis is none */
#define DFLT_EXT        ".mpg" 	/* default output file extension */
#define DFLT_EXT_EXT    ".ext" 	/* default output file extension of extension bit stream*/
#define DFLT_ML_EXT     ".ml"  	/* default extension for MultiLingual file */
#define	DFLT_VERB	1	/* default level of verbosity */
#define DFLT_NADB	2	/* default number of ancillary data bytes (for DVD !!!) */

#define FILETYPE_ENCODE 'TEXT'
#define CREATOR_ENCODE  'MpgD'

#define TC_ALLOC        0.0

/* This is the smallest MNR a subband can have before it is counted
   as 'noisy' by the logic which chooses the number of JS subbands */
/* Now optionally in fr_ps */

/*#define NOISY_MIN_MNR   0.0*/

/* Psychacoustic Model 1 Definitions */

#define CB_FRACTION     0.33
#define MAX_SNR         1000
#define NOISE           10
#define TONE            20
#define DBMIN           -200.0
#define LAST            -1
#define STOP            -100
#define POWERNORM       90.3090
			/*96.0*/
			/* Full amplitude, 32767, should correspond to
			   96 dB, 1992-11-06 Soren H. Nielsen */
		/* 90.3090  = 20 * log10(32768) to normalize */
                                /* max output power to 96 dB per spec */

/* Psychoacoustic Model 2 Definitions */

#define LOGBLKSIZE      10
#define BLKSIZE         1024
#define HBLKSIZE        513
#define CBANDS          63
#define LXMIN           32.0

/***********************************************************************
*
*  Encoder Type Definitions
*
***********************************************************************/

/* Psychoacoustic Model 1 Type Definitions */

typedef int        IFFT2[FFT_SIZE/2];
typedef int        IFFT[FFT_SIZE];
typedef double     D9[9];
typedef double     D10[10];
typedef double     D640[640];
typedef double     D1408[1408];
typedef double     DFFT2[FFT_SIZE/2];
typedef double     DFFT[FFT_SIZE];
typedef double     DSBL[SBLIMIT];
typedef double     D2SBL[2][SBLIMIT];
typedef double     D5SBL[5][SBLIMIT];
typedef double     D7SBL[7][SBLIMIT];  /*added because of 7 channels, 8/10/93, SR*/
typedef double     D12SBL[12][SBLIMIT];  /*added because of 12 channels, 8/10/93, SR*/

typedef struct {
        int        line;
        double     bark, hear, x;
} g_thres, *g_ptr;

typedef struct {
        double     x;
        int        type, next, map;
} mask, *mask_ptr;

/* Psychoacoustic Model 2 Type Definitions */

typedef int        ICB[CBANDS];
typedef int        IHBLK[HBLKSIZE];
typedef FLOAT      F32[32];
typedef FLOAT      F2_32[2][32];
typedef FLOAT      FCB[CBANDS];
typedef FLOAT      FCBCB[CBANDS][CBANDS];
typedef FLOAT      FBLK[BLKSIZE];
typedef FLOAT      FHBLK[HBLKSIZE];
typedef FLOAT      F2HBLK[2][HBLKSIZE];
typedef FLOAT      F22HBLK[2][2][HBLKSIZE];
typedef double     DCB[CBANDS];

/***********************************************************************
*
*  Encoder Variable External Declarations
*
***********************************************************************/

#ifdef MS_DOS
/* extern unsigned _stklen = 16384; */ /* removed, 92-07-13 sr */
#endif

/***********************************************************************
*
*  Encoder Function Prototype Declarations
*
***********************************************************************/

/* The following functions are in the file "musicin.c" */

extern
void
obtain_parameters(
	frame_params		*fr_ps,
	int			*psy,
	long unsigned int	*num_samples,
	long unsigned int	*num_samples_ml,
	char			*original_file_name,
	char			*encoded_file_name,
	IFF_AIFF		*pcm_aiff_data,
	IFF_AIFF		*pcm_aiff_data_ml,
	int			*aiff,
	int			*byte_per_sample,
	int 			*mat_mode,
	int 			*aug_mat_mode,
	int			*cha_sw,
	int			*aug_cha_sw,
	char			*encoded_file_name_ext,
	int			*bit_rate,
	char			*encoded_file_name_mpg,
	int			*dyn_cross,
	int			*dyn_cross_LR,
	int			*aug_dyn_cross,
	int                     *ancillaryUse,
	int			*verbosity
);

extern
void
parse_args(
	int			argc,
	char			**argv,
	frame_params		*fr_ps,
	int			*psy,
	long unsigned int	*num_samples,
	long unsigned int	*num_samples_ml,
	char			*original_file_name,
	char			*encoded_file_name,
	IFF_AIFF		*pcm_aiff_data,
	IFF_AIFF		*pcm_aiff_data_ml,
	int			*aiff,
	int			*byte_per_sample,
	int 			*mat_mode,
	int 			*aug_mat_mode,
	int			*cha_sw,
	int			*aug_cha_sw,
	char			*encoded_file_name_ext,
	int			*bit_rate,
	char			*encoded_file_name_mpg,
	int			*dyn_cross,
	int			*dyn_cross_LR,
	int			*aug_dyn_cross,
	int                     *ancillaryUse,
	int			*verbosity
);

extern void   print_config(frame_params *fr_ps, int *psy, long unsigned int *num_samples, char *inPath, char *outPath, int *aiff);


static void   usage();
extern void   aiff_check(char *file_name, IFF_AIFF *pcm_aiff_data);


/* The following functions are in the file "encode.c" */

extern unsigned long  read_samples (FILE *musicin,
				    long int *sample_buffer,
				    long unsigned int num_samples,
				    long unsigned int frame_size,
				    int *byte_per_sample,
				    int *aiff);

extern
unsigned long
get_audio (
	FILE		*musicin,
	double		(*buffer)[1152],
	long unsigned int num_samples,
	int		stereo,
	IFF_AIFF	*aiff_ptr,
	int		stereomc,
	int		stereoaug,
	frame_params	*fr_ps,
	int		*aiff,
	int		*byte_per_sample,
	double		(*buffer_matr)[1152]
);

extern void 	   normalizing (double (*sb_sample)[3][12][SBLIMIT], frame_params *fr_ps);
#ifdef Augmentation_7ch
extern void 	   normalizing_aug (double (*sb_sample)[3][12][SBLIMIT], frame_params *fr_ps);
#endif
extern void 	   matricing (double (*sb_sample)[3][12][SBLIMIT], frame_params *fr_ps);
#ifdef Augmentation_7ch
extern void 	   matricing_aug (double (*sb_sample)[3][12][SBLIMIT], frame_params *fr_ps);
#endif
extern void        read_ana_window (double *ana_win);
extern void        window_subband (double **buffer, double *z, int k);
extern void        create_ana_filter (double (*filter)[64]);
extern void        filter_subband (double *z, double *s);
extern void        encode_info (frame_params *fr_ps, Bit_stream_struc *bs);
extern void        encode_info_mc1 (frame_params *fr_ps, Bit_stream_struc *bs);
extern void        encode_info_mc2 (frame_params *fr_ps, Bit_stream_struc *bs);
#ifdef Augmentation_7ch
extern void        encode_info_aug (frame_params *fr_ps, Bit_stream_struc *bs);
#endif
extern void        encode_info_ext1 (frame_params *fr_ps, Bit_stream_struc *bs_ext);
extern void        encode_info_ext2 (frame_params *fr_ps, Bit_stream_struc *bs_ext, unsigned int crc);
extern double      mod (double a);
extern void        I_combine_LR (double (*sb_sample)[3][12][SBLIMIT], double (*joint_sample)[3][12][SBLIMIT]);
extern void 	   II_combine_LR (double (*sb_sample)[3][12][SBLIMIT], double (*joint_sample)[3][12][SBLIMIT],
					    int sblimit);
extern void        I_scale_factor_calc (double (*sb_sample)[3][12][SBLIMIT], unsigned int (*scalar)[3][SBLIMIT],
					    int stereo);
extern void 	   II_scale_factor_calc (frame_params *fr_ps, double (*sb_sample)[3][12][SBLIMIT],
					unsigned int (*scalar)[3][SBLIMIT], int sblimit, int l, int m);
extern void 	   II_scale_factor_calc1 (double (*sb_sample)[3][12][SBLIMIT], unsigned int (*scalar)[3][SBLIMIT],
					int sblimit, int dim);
extern void        pick_scale (unsigned int (*scalar)[3][SBLIMIT], frame_params *fr_ps, double (*max_sc)[SBLIMIT],
					int cha_sw, int aug_cha_sw, int aiff);
extern void        put_scale (unsigned int (*scalar)[3][SBLIMIT], frame_params *fr_ps, double (*max_sc)[SBLIMIT]);
extern void        II_transmission_pattern (unsigned int (*scalar)[3][SBLIMIT], unsigned int (*scfsi)[SBLIMIT],
					frame_params *fr_ps);

extern void II_encode_scale (unsigned int (*bit_alloc)[SBLIMIT],
			     unsigned int (*scfsi)[SBLIMIT],
			     unsigned int (*scalar)[3][SBLIMIT],
			     unsigned int lfe_alloc, 
			     unsigned int lfe_scalar, 
			     frame_params *fr_ps,
			     Bit_stream_struc *bs,
			     int *l,
			     int *z);

extern void        I_encode_scale (unsigned int (*scalar)[3][SBLIMIT], unsigned int (*bit_alloc)[SBLIMIT],
				    frame_params *fr_ps, Bit_stream_struc *bs);
extern int         II_bits_for_nonoise (double (*perm_smr)[SBLIMIT], unsigned int (*scfsi)[SBLIMIT],
				    frame_params *fr_ps, int a, int b, int *aiff);

extern void	   II_main_bit_allocation (double (*perm_smr)[SBLIMIT],
					   double (*ltmin)[SBLIMIT],
					   unsigned int (*scfsi)[SBLIMIT],
					   unsigned int (*bit_alloc)[SBLIMIT],
					   int *adb,
					   frame_params *fr_ps,
					   int *aiff,
					   double (*sb_sample)[3][12][SBLIMIT],
					   unsigned int (*scalar)[3][SBLIMIT],
					   double (*max_sc)[SBLIMIT],
					   double (*buffer)[1152],
					   double (*spiki)[SBLIMIT],
					   double (*joint_sample)[3][12][SBLIMIT],
					   unsigned int (*j_scale)[3][SBLIMIT], 
					   int dyn_cr, 
					   int aug_dyn_cr, 
					   unsigned int (*scfsi_dyn)[SBLIMIT], 
					   unsigned int (*scalar_dyn)[3][SBLIMIT]);

extern int         II_a_bit_allocation (double (*perm_smr)[SBLIMIT],
					unsigned int (*scfsi)[SBLIMIT],
					unsigned int (*bit_alloc)[SBLIMIT],
					int *adb,
					frame_params *fr_ps,
					int *aiff);

extern int         I_bits_for_nonoise (double (*perm_smr)[SBLIMIT], frame_params *fr_ps);
extern void        I_main_bit_allocation (double (*perm_smr)[SBLIMIT], unsigned int (*bit_alloc)[SBLIMIT],
					  int *adb, frame_params *fr_ps);
extern int         I_a_bit_allocation (double (*perm_smr)[SBLIMIT], unsigned int (*bit_alloc)[SBLIMIT],
				       int *adb, frame_params *fr_ps);
extern void        I_subband_quantization (unsigned int (*scalar)[3][SBLIMIT],
					   double (*sb_samples)[3][12][SBLIMIT],
					   unsigned int (*j_scale)[3][SBLIMIT],
					   double (*j_samps)[3][12][SBLIMIT],
					   unsigned int (*bit_alloc)[SBLIMIT],
					   unsigned int (*sbband)[3][12][SBLIMIT],
					   frame_params *fr_ps);
extern void        II_subband_quantization (unsigned int (*scalar)[3][SBLIMIT],
					    double (*sb_samples)[3][12][SBLIMIT],
					    unsigned int (*j_scale)[3][SBLIMIT],
					    double (*j_samps)[3][12][SBLIMIT],
					    unsigned int (*bit_alloc)[SBLIMIT],
					    unsigned int (*sbband)[3][12][SBLIMIT],
					    frame_params *fr_ps);
extern void        II_subband_quantization_mc (unsigned int (*scalar)[3][SBLIMIT],
					       double (*sb_samples)[3][12][SBLIMIT],
					       unsigned int (*j_scale)[3][SBLIMIT],
					       double (*j_samps)[3][12][SBLIMIT],
					       unsigned int (*bit_alloc)[SBLIMIT],
					       unsigned int (*sbband)[3][12][SBLIMIT],
					       frame_params *fr_ps);
#ifdef Augmentation_7ch
extern void        II_subband_quantization_aug (unsigned int (*scalar)[3][SBLIMIT],
						double (*sb_samples)[3][12][SBLIMIT],
						unsigned int (*j_scale)[3][SBLIMIT],
						double (*j_samps)[3][12][SBLIMIT],
						unsigned int (*bit_alloc)[SBLIMIT],
						unsigned int (*sbband)[3][12][SBLIMIT],
						frame_params *fr_ps);
#endif
extern void        I_encode_bit_alloc (unsigned int (*bit_alloc)[SBLIMIT], frame_params *fr_ps, Bit_stream_struc *bs);
extern void        II_encode_bit_alloc (unsigned int (*bit_alloc)[SBLIMIT], frame_params *fr_ps, Bit_stream_struc *bs);
extern void        II_encode_bit_alloc_mc (unsigned int (*bit_alloc)[SBLIMIT], frame_params *fr_ps, Bit_stream_struc *bs);
#ifdef Augmentation_7ch
extern void        II_encode_bit_alloc_aug (unsigned int (*bit_alloc)[SBLIMIT], frame_params *fr_ps, Bit_stream_struc *bs);
#endif
extern void        I_sample_encoding (unsigned int (*sbband)[3][12][SBLIMIT],
				      unsigned int (*bit_alloc)[SBLIMIT],
				      frame_params *fr_ps,
				      Bit_stream_struc *bs);
extern void        II_sample_encoding (unsigned int (*sbband)[3][12][SBLIMIT],
				       unsigned int (*bit_alloc)[SBLIMIT],
				       frame_params *fr_ps,
				       Bit_stream_struc *bs);
extern void        II_sample_encoding_mc (unsigned int (*sbband)[3][12][SBLIMIT],
					  unsigned int lfe_sbband[12],
					  unsigned int (*bit_alloc)[SBLIMIT],
					  unsigned int lfe_alloc,
					  frame_params *fr_ps,
					  Bit_stream_struc *bs);
#ifdef Augmentation_7ch
extern void        II_sample_encoding_aug (unsigned int (*sbband)[3][12][SBLIMIT],
					   unsigned int (*bit_alloc)[SBLIMIT],
					   frame_params *fr_ps,
					   Bit_stream_struc *bs);
#endif
extern void        encode_CRC (unsigned int crc, Bit_stream_struc *bs);
extern void	   ancillary_encode (frame_params*,Bit_stream_struc*,int);

extern void	   matricing_fft (double (*buffer)[1152], double (*buffer_matr)[1152], frame_params *fr_ps);
#ifdef Augmentation_7ch
extern void	   matricing_aug_fft (double (*buffer)[1152], double (*buffer_matr)[1152], frame_params *fr_ps);

#endif
extern void        tc_alloc (frame_params *fr_ps, double (*max_sc)[SBLIMIT]);
extern int         II_bits_for_indi (double (*perm_smr)[SBLIMIT], unsigned int (*scfsi)[SBLIMIT], frame_params *fr_ps, int *a, int *b, int *aiff);
extern int         required_bits ();
extern int         max_alloc ();
 

/* The following functions are in the file "tonal.c" */

extern void        read_cbound(int lay, int freq, int crit_band, int *cbound);
extern void        read_freq_band(int *sub_size, g_ptr *ltg, int lay, int freq);
extern void        make_map(int sub_size, mask *power, g_thres *ltg);
extern double      add_db(double a, double b);
extern void        II_f_f_t(double *sample, mask *power);
extern void        II_hann_win(double *sample);
extern void        II_pick_max(mask *power, double *spike);
extern void        II_tonal_label(mask *power, int *tone);
extern void        noise_label(int crit_band, int *cbound, mask *power, int *noise, g_thres *ltg);
extern void        subsampling(mask *power, g_thres *ltg, int *tone, int *noise);
extern void        threshold(int sub_size, mask *power, g_thres *ltg, int *tone, int *noise, int bit_rate);
extern void        II_minimum_mask(int sub_size, g_thres *ltg, double *ltmin, int sblimit);
extern void        II_smr(double *ltmin, double *smr, double *spike, double *scale, int sblimit, int l, int m);
extern void        II_Psycho_One(double (*buffer)[1152], double (*scale)[SBLIMIT], double (*ltmin)[SBLIMIT], frame_params *fr_ps, double (*smr)[SBLIMIT], double (*spiki)[SBLIMIT], int aiff);
extern void        I_f_f_t(double *sample, mask *power);
extern void        I_hann_win(double *sample);
extern void        I_pick_max(mask *power, double *spike);
extern void        I_tonal_label(mask *power, int *tone);
extern void        I_minimum_mask(int sub_size, g_thres *ltg, double *ltmin);
extern void        I_smr(double *ltmin, double *spike, double *scale);
extern void        I_Psycho_One(double (*buffer)[1152], double (*scale)[SBLIMIT], double (*ltmin)[SBLIMIT], frame_params *fr_ps);
extern double      non_lin_add(double a, double b, double c);


/* The following functions are in the file "psy.c" */

extern void        psycho_anal(double *buffer, short int *savebuf, int chn, int lay, float *snrSBLIMIT, double sfreq);
extern void        read_absthr(float *absthr, long int table);


/* The following functions are in the file "subs.c" */

extern void        fft(float *x_real, float *x_imag, float *energy, float *phi);


/* The following functions are in the file "predisto.c" */

void matri (
	double (*sbs_sample)[3][12][SBLIMIT],
	frame_params *fr_ps,
	double (*sb_sample)[3][12][SBLIMIT],
	unsigned int (*scalar)[3][SBLIMIT],
	int sblimit,
	unsigned int (*scfsi)[SBLIMIT],
	unsigned int (*scfsi_dyn)[SBLIMIT],
	unsigned int (*bit_alloc)[SBLIMIT],
	unsigned int (*subband)[3][12][SBLIMIT]
);

void trans_pattern (
	unsigned int (*scalar)[3][SBLIMIT],
	unsigned int (*scfsi)[SBLIMIT],
	frame_params *fr_ps,
	unsigned int (*scfsi_dyn)[SBLIMIT]
);

void pre_quant (
	double (*sbs_sample)[3][12][SBLIMIT],
	int l,
	int m,
	double (*sb_sample)[3][12][SBLIMIT],
	unsigned int (*scalar)[3][SBLIMIT],
	unsigned int (*bit_alloc)[SBLIMIT],
	unsigned int (*subband)[3][12][SBLIMIT],
	frame_params *fr_ps
);

void II_dequantize_sample (
	unsigned int (*sample)[3][SBLIMIT],
	unsigned int (*bit_alloc)[SBLIMIT],
	double (*fraction)[3][12][SBLIMIT],
	frame_params *fr_ps,
	int l,
	int m,
	int z
);

void II_denormalize_sample (
	double (*fraction)[3][12][SBLIMIT],
	unsigned int (*scale_index)[3][SBLIMIT],
	frame_params *fr_ps,
	int x,
	int l,
	int m,
	int z
);

void scale_factor_calc (
	unsigned int (*scalar)[3][SBLIMIT],
	int sblimit,
	int l,
	int m,
	int z,
	double (*sb_sample)[3][12][SBLIMIT]
);

void predistortion (
		    double (*sb_sample)[3][12][32], 
		    unsigned int (*scalar)[3][32], 
		    unsigned int (*bit_alloc)[32], 
		    unsigned int (*subband)[3][12][32], 
		    frame_params *fr_ps, 
		    double (*perm_smr)[32], 
		    unsigned int (*scfsi)[32], 
		    int *adb, 
		    unsigned int (*scfsi_dyn)[32]
		    );

/* The following functions are in the file "dyn_cross.c" */

void take_dyn_cross (
	int		dyn_cr,	
	frame_params	*fr_ps,  		    
	double		sb_sample_sum[5][3][SCALE_BLOCK][SBLIMIT], 	    
    	unsigned int	scalar_sum[][3][SBLIMIT], 			
	unsigned int	scfsi_sum[][SBLIMIT],  				
	unsigned int	scfsi_dyn[][SBLIMIT],  				
	double		sb_sample[CHANMAX3][3][SCALE_BLOCK][SBLIMIT],	
	unsigned int	scalar[][3][SBLIMIT], 				
	unsigned int	scfsi[][SBLIMIT]
	);
	
#ifdef Augmentation_7ch
void take_dyn_cross_aug (
	int		aug_dyn_cr,	
	frame_params	*fr_ps,  		    
	double		sb_sample_sum[5][3][SCALE_BLOCK][SBLIMIT], 	    
    	unsigned int	scalar_sum[][3][SBLIMIT], 			
	unsigned int	scfsi_sum[][SBLIMIT],  				
	unsigned int	scfsi_dyn[][SBLIMIT],  				
	double		sb_sample[CHANMAX3][3][SCALE_BLOCK][SBLIMIT],	
	unsigned int	scalar[][3][SBLIMIT], 				
	unsigned int	scfsi[][SBLIMIT]
	);
#endif
	
void trans_chan (frame_params *fr_ps);
#ifdef Augmentation_7ch
void trans_chan_aug (frame_params *fr_ps);
#endif
	 				
void combine (frame_params *fr_ps, double sb_sample[CHANMAX3][3][SCALE_BLOCK][SBLIMIT],
	      double sb_sample_sum[5][3][SCALE_BLOCK][SBLIMIT]);
#ifdef Augmentation_7ch
void combine_aug (frame_params *fr_ps, double sb_sample[CHANMAX3][3][SCALE_BLOCK][SBLIMIT],
		  double sb_sample_sum[5][3][SCALE_BLOCK][SBLIMIT]);
#endif
	
void dyn_bal (
	unsigned int	scfsi[CHANMAX3][SBLIMIT],  
	int		sbgr,			    
	frame_params	*fr_ps,
	int		min_ch,			    
	int		min_sb,			   
	int		*seli,			    
	int		*scale
	);
	
#ifdef Augmentation_7ch
void dyn_bal_aug (
	unsigned int	scfsi[CHANMAX3][SBLIMIT],  
	int		sbgr,			    
	frame_params	*fr_ps,
	int		min_ch,			    
	int		min_sb,			   
	int		*seli,			    
	int		*scale
	);
#endif

void choose_dyn (
	frame_params	*fr_ps, 
	int		min_ch,				
	int		min_sb,				
	int		sbgr, 				
	unsigned int	bit_alloc[CHANMAX3][SBLIMIT]
	);	
				    
#ifdef Augmentation_7ch
void choose_dyn_aug (
	frame_params	*fr_ps, 
	int		min_ch,				
	int		min_sb,				
	int		sbgr, 				
	unsigned int	bit_alloc[CHANMAX3][SBLIMIT]
	);	
#endif
			    
void scfsi_calc_dyn (
	unsigned int	scalar_dyn[][3][SBLIMIT],   
	int		ch, 			    
	int		sblimit,		    
	unsigned int	scfsi_dyn[][SBLIMIT]
	);
	 	    
void scfsi_calc (
	unsigned int	scalar[][3][SBLIMIT], 	
	int		ch, 			
	int		sblimit,		
	unsigned int	scfsi[][SBLIMIT]
	);
	 	
void scf_calc (
	double		sample[][3][SCALE_BLOCK][SBLIMIT], 
	int		sblimit,		
	int		ch,			
	unsigned int	scf[][3][SBLIMIT]
	);	


/* The following functions are in the file "lfe.c" */

extern void        lfe_filter(double**,double[12]);
extern void        lf_scalefactor(double[12],unsigned int*);
extern void        II_encode_lfe_scale(unsigned int, Bit_stream_struc*);
extern void        lfe_allocation(unsigned int*,int*);
extern void        II_lfe_quantisation(unsigned int,double[12],
                                       unsigned int, unsigned int[12],
                                       frame_params*);
extern void        II_encode_lfe_alloc(unsigned int, frame_params*, Bit_stream_struc*);
extern void        II_lfe_sample_encoding(unsigned int[12],
                                       unsigned int, frame_params*,
                                       Bit_stream_struc*);
