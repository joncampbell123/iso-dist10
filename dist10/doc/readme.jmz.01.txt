/*******************************************************************
                Thomson Consumer Electronics (TCE),
           Corporate Research, DTB/HAP-Hannover (c) 1995

    Project: MPEG-2
    File___: multilingual.doc
    Job____: Adding multilingual features to the program
    Date___: 30/03/1995

    Author: Jean-Marc ZINS (JMZ)
*******************************************************************/

I The encoder.
/************/

New features
	
	Multilingual feature has been implemented to allow up to seven multilingual
	channels to be encoded.
	
	Channel configuration (described here as "Multimode") has been  improved
	to allow all configurations with aiff data as input stream. There are some
	work to be done in case of a second stereo program. Most of these 
	improvements are made in the "tc_alloc()" function (encoder.c).

Limitations
	
	As input, the encoder will read 2 files : one file containing the standard 
	audio channels and one file containing up to seven multilingual channels. 
	Both files must use the aiff data format.
	
	The multilingual features are only tested on a UNIX platform!

	The multilingual features need the use of aiff files for the multilingual
	channels.
	
	The multilingual features are not accessible when specifying parameters 
	in the command line ( parse_arg() has not been modified for multilingual 
	features).

Global variables added for multilingual purposes :

	FILE			*musicin_ml;	
	char 			multilingual_file_name[MAX_NAME_SIZE];	

	unsigned long 	num_samples_ml;	

Functions added for multilingual purposes :

	void 	II_encode_bit_alloc_ml()
	void 	II_encode_scale_ml()
	void 	II_subband_quantization_ml()
	void 	II_sample_encoding_ml()
	
Many existing functions have been modified to take into account the multilingual
channels. The modifications are noticed with a commentary containing the key word "JMZ".
	
The arrays have been extended to be able to contain up to 14 channels (channels
7 to 14 reserved for multilingual channels)

	Examples :
		double 				sb_sample[14][3][12][SBLIMIT];	
		long 				*win_buf[14];
		static long			buffer[14][1152];			
		double 				spiki[14][SBLIMIT];	
		static unsigned int	bit_alloc[14][SBLIMIT];
		static unsigned int	scfsi[14][SBLIMIT];
		static unsigned int	scfsi_dyn[14][SBLIMIT];
		static unsigned int	scalar[14][3][SBLIMIT];
		static double 		ltmin[14][SBLIMIT];
		static double		lgmin[14][SBLIMIT];
		static double		max_sc[14][SBLIMIT];
		static double		smr[14][SBLIMIT];
		short 				sam[14][1056];
		double 				buffer_matr[14][1152];

	
Main function in case of multilingual channels : 

main(argc, argv)
	{
	obtain_parameters();
    hdr_to_frps();
	while (get_audio())
		{
		if (info.multiling_ch != 0)
			{
			get_audio_ml();
			for(...)	
				{
				window_subband();
				filter_subband();
				}
			}
		switch (info.lay) 
			{
			case 1 :	break;
			case 2 :	
				if( aiff != 1)
				{
				for (...) 
					{
					window_subband();
					filter_subband();
					}
				II_scale_factor_calc();
				if (info.multiling_ch>0)
					II_scale_factor_calc(/*multilingual*/);
				pick_scale();	
				II_Psycho_One();
				II_transmission_pattern();
				II_main_bit_allocation();
				if (error_protection)
					II_CRC_calc();
					encode_info(&fr_ps, &bs);
				if (error_protection) encode_CRC();
				II_encode_bit_alloc();
				II_encode_scale();
				II_subband_quantization();
				II_sample_encoding();
				if(info.multiling_ch >0)
					{
					II_encode_bit_alloc_ml();
					II_encode_scale_ml();
					II_subband_quantization_ml();
					II_sample_encoding_ml();
					}
				}
			else
				{
				for (...) 
					{
					window_subband();
					filter_subband();
					}
  				matricing();
				II_scale_factor_calc();
				if (info.multiling_ch>0)
					II_scale_factor_calc(/*multilingual*/);
				pick_scale();
				II_Psycho_One();
				II_transmission_pattern();
				II_main_bit_allocation();
				if(fr_ps.actual_mode != MPG_MD_JOINT_STEREO)
					{
				    if(fr_ps.actual_mode != MPG_MD_JOINT_STEREO)
						{
						if(info.matrix == 0)
							{
			    			predistortion();
							}
						}
					}
				if (error_protection)
					 II_CRC_calc();
				II_CRC_calcmc();
				encode_info();
				if (error_protection) encode_CRC();
				II_encode_bit_alloc();
				II_encode_scale();
				II_subband_quantization();
				II_sample_encoding();
					
				encode_infomc1();
				encode_CRC();
				encode_infomc2();    
				II_encode_bit_allocmc();
				II_encode_scale();
				II_subband_quantizationmc();
				II_sample_encodingmc();
	
				if(info.multiling_ch >0)
					{
					II_encode_bit_alloc_ml();
					II_encode_scale_ml();
					II_subband_quantization_ml();
					II_sample_encoding_ml();
					}
					
				}
			case 3 : break;
			}
		}/* end of while(get_audio) - loop */
	}
	
BUG CORRECTION
  Corrected bugs are noticed with a commentary containing the keywords "BUG" and "JMZ"
	
  In file musicin.c
	*Replaced "char *blubb" by "char blubb[128]" to prevent segmentation fault
	
	*Changed "< frameNum -2" to "<=  frameNum -1" : there was a floating 
			exception if there was only one frame to encode.
			
  In file encode.c
	*if((rq_db = II_bits_for_nonoise(perm_smr, scfsi, fr_ps,a, b, aiff) 
	Replaced "&a" and "&b" by "a" and "b".

  In file tonal.c
	*bitrate[info->lay-1][info->bitrate_index]/(stereo+stereomc)); 
	Replaced "stereo" by "stereo+stereomc" : if MPG_MD_NONE stereo==0 !!

II The decoder.
/************/

Limitations

	As output, the decoder will produce 2 files : one file containing the 
	standard audio datas and one file containing the multilingual channels.
	Both files contain raw datas (aiff output has not yet benn implemented 
	for multilingual channels). the second files has a ".ml" extension.
	
	The multilingual channels cannot be written onto an aiff format file.
	
Global variables added for multilingual purposes :

	PCM  			*pcm_sample_ml;
	SAM  			*sample_ml;	
	FRA  			*fraction_ml;
	FRA_HELP 		*fraction_help_ml;
	FILE			*musicout_ml;
	unsigned long	sample_frames_ml;
	unsigned int 	bit_alloc_ml[5][SBLIMIT];
	unsigned int 	scfsi_ml[5][SBLIMIT];
	unsigned int 	scale_index_ml[5][3][SBLIMIT];
	char			decoded_file_name_ml[MAX_NAME_SIZE];
	
Functions added for multilingual purposes :

	II_decode_bitalloc_ml()
	II_decode_scale_ml()
	II_buffer_sample_ml()
	II_dequantize_sample_ml()
	II_denormalize_sample_ml()
	dematricing_ml()
	SubBandSynthesis_ml()
	out_fifo_ml()
	
The variable and functions have been "duplicated". Variables and functions 
suffixed with "_ml" are intended for multilingual channels.	
	
Main function in case of multilingual channels : 

main(argc, argv)
	{
	while (!end_bs(&bs))
		{
		sync = seek_sync();
		if (!sync)
			{
			if (info.lay == 2) 
				{
				out_fifo();
				out_fifo_ml();
				}
			else 
				out_fifo();
			}
		decode_info();
		hdr_to_frps();
		switch (info.lay)
			{
			case 1:  break;
			case 2:
				{
				II_decode_bitalloc();
				II_decode_scale();
				if (error_protection) 
					{
					II_CRC_calc(&fr_ps, bit_alloc, scfsi, &new_crc);
					}
				 for (i=0;i<12;i++)
			 		{
					II_buffer_sample();
					II_dequantize_sample();
					II_denormalize_sample();
					} 

				mc_header();
				hdr_to_frps();
				buffer_CRC();   /* read CRC - check from header */
				mc_composite_status_info();
				
				II_decode_bitalloc();
				II_decode_scale();
				
				if (crc_check)
		 			{
					mc_error_check();
		  			}
				
				for (i=0;i<12;i++)
			  		{
					II_buffer_samplemc();
					II_dequantize_samplemc();
					II_denormalize_samplemc();
			   		}

				if (info.no_of_multi_lingual_ch>0) 
					{
					II_decode_bitalloc_ml();
		  			II_decode_scale_ml();
					
					for (i=0;i<12;i++)
			  			{
						II_buffer_sample_ml();
						II_dequantize_sample_ml();
						II_denormalize_sample_ml);
			   			}
					}

			 	if(crc_error_count == 0)
			   		dematricing(*fraction, &fr_ps, *pred_buf);

			   	for(...)
					(*fraction_help)[jj][k][j][ii]=(*fraction)[k][ii][j][jj];

				for (i=0;i<12;i++)
					{			  
			  		for (j=0;j<3;j++) 
					for (k=0; k<5; k++)
						clip += SubBandSynthesis();
					out_fifo();
					}
			
				if(info.no_of_multi_lingual_ch>0) 
					{			
			   		dematricing_ml();
			   
			   		for(...)
						(*fraction_help_ml)[jj][k][j][ii]
						=(*fraction_ml)[k][ii][j][jj];

					for (i=0;i<12;i++)
						{				  
			  			for (j=0;j<3;j++) 
						for (k=0; k<info.no_of_multi_lingual_ch; k++)
							clip += SubBandSynthesis_ml();
						out_fifo_ml();
						}
					}
				break;
				} /*end of layer 2*/
			}/*end of switch layer - loop*/
		}/*end of while(!endof(bs)) - loop */
	}
