




/***********************************************************************/
Modifications made in file encode.c
JMZ 29/03/1995
/***********************************************************************/


unsigned long read_samples()	-> No change

unsigned long get_audio()		-> Adaptation for Multimode

void matricing_old()
void matricing()				-> Adaptation for Multilingual

void matricing_fft()			-> No change

void read_ana_window()			-> No change

void window_subband()			-> Adaptation for Multilingual

void create_ana_filter()		-> No change

void filter_subband_old()
void filter_subband()			-> Adaptation to improve filter function

void encode_info()				-> No change

void encode_infomc1()			-> No change

void encode_infomc2()			-> No change

void encode_infomc_ext1()		-> No change

void encode_infomc_ext2()		-> No change

double mod()					-> No change

void I_combine_LR()				-> No change

void II_combine_LR()			-> No change


void I_scale_factor_calc()		-> No change

void II_scale_factor_calc()		-> Adaptation for Multilingual

void II_scale_factor_calc1()	-> No change

void pick_scale()				-> Adaptation for Multilingual

void tc_alloc_old()
void tc_alloc()					-> Adaptation for Multimode


void put_scale()				-> No change

void II_transmission_pattern()	-> Adaptation for Multilingual

void I_encode_scale()			-> No change

void II_encode_scale()			-> Adaptation for Multilingual

void II_encode_scale_ml()		-> NEW for Multilingual

int I_bits_for_nonoise()		-> No change

int II_bits_for_nonoise()		-> Adaptation for Multilingual

int II_bits_for_indi()			-> Adaptation for Multilingual

void I_main_bit_allocation()	-> No change

void II_main_bit_allocation()	-> Adaptation for Multilingual

int I_a_bit_allocation()		-> No change

int II_a_bit_allocation()		-> Adaptation for Multilingual

int	sort_init()

void II_subband_quantization_ml()-> NEW for Multilingual

void I_subband_quantization()	-> No change

void II_subband_quantization()	-> Adaptation for Multilingual

void II_subband_quantizationmc()-> Adaptation for Multilingual

void I_encode_bit_alloc()		-> No change

void II_encode_bit_alloc()		-> Adaptation for Multilingual
								-> Adaptation for Multimode

void II_encode_bit_allocmc()	-> Adaptation for Multilingual

void I_sample_encoding()		-> No change

void II_sample_encoding()		-> Adaptation for Multilingual

void II_sample_encodingmc()		-> Adaptation for Multilingual

void encode_CRC()				-> No change

/***********************************************************************/
Modifications made in file common.c
JMZ 29/03/1995
/***********************************************************************/

void hdr_to_frps() 				-> Adaptation for Multimode

/***********************************************************************/
Modifications made in file common.c
JMZ 29/03/1995
/***********************************************************************/

int bit_all()					-> Adaptation for Multilingual
								-> Adaptation for Multimode

/***********************************************************************/
Modifications made in file tonal.c
JMZ 29/03/1995
/***********************************************************************/


void II_Psycho_One()			-> Adaptation for Multilingual

/***********************************************************************/
Modifications made in file musicin.c
JMZ 29/03/1995
/***********************************************************************/

void obtain_parameters()		-> Adaptation for Multilingual
								-> Adaptation for Multimode

main()							-> Adaptation for Multilingual
								-> Adaptation for Multimode

/***********************************************************************/
File lingual.c added for multilingualfunctions
JMZ 29/03/1995
/***********************************************************************/
unsigned long read_samples_ml()

unsigned long get_audio_ml()

void II_sample_encoding_ml()

void II_encode_bit_alloc_ml()




