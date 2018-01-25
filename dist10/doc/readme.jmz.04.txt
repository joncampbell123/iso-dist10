
/***********************************************************************/
Modifications made in file decode.c
JMZ 29/03/1995
/***********************************************************************/


void decode_info()					-> No change

void mc_header()					-> No change

void mc_composite_status_info()		-> No change

int mc_ext_header()					-> No change

void II_decode_bitalloc()			-> No change

void II_decode_bitalloc_ml()		-> NEW for Multilingual

void II_decode_scale()				-> No change

void II_decode_scale_ml()			-> NEW for Multilingual

void II_buffer_sample()				-> No change

void II_buffer_samplemc()			-> No change

void II_buffer_sample_ml()			-> NEW for Multilingual

void II_dequantize_sample()			-> No change

void II_dequantize_samplemc()		-> No change

void II_dequantize_sample_ml()		-> NEW for Multilingual

void II_denormalize_sample()		-> No change

void II_denormalize_samplemc()		-> No change

void II_denormalize_sample_ml()		-> NEW for Multilingual

void create_syn_filter()			-> No change

void read_syn_window()				-> No change

int SubBandSynthesis()				-> No change

int SubBandSynthesis_ml()			-> NEW for Multilingual

void out_fifo()						-> No change

void out_fifo_ml()					-> No change

void  buffer_CRC()					-> No change

void  recover_CRC_error()			-> No change

void dematricing() 					-> Adaptations for Multilingual

void dematricing_ml() 				-> NEW for Multilingual


/***********************************************************************/
Modifications made in file common.c
JMZ 29/03/1995
/***********************************************************************/

void hdr_to_frps()					-> Adaptations for Multilingual


/***********************************************************************/
Modifications made in file musicout.c
JMZ 29/03/1995
/***********************************************************************/

main()								-> Adaptations for Multilingual

