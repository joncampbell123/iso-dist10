/*******************************************************************
                Institut fuer Rundfunktechnik (IRT),
                             (c) 1995

    Project: MPEG-2
    File___: dyn_cross.c
    Job____: Adding dyn_cross features to the program
    Date___: 06/02/1995

    Author: Susanne RITSCHER (JMZ)
*******************************************************************/

I The encoder.
/************/

New features
	
	Dynamic Crosstalk feature has been implemented .
	

Limitations
	
	As input you must tell the encoder which mode you want.
        It is static, that means, that in each frame there will 
        be the same dyn_cross mode in the same subband.
        If different modes within one frame are wanted, it must
        be changed in take_dyn_cross(). 
	Predistortion is not working together with dyn_cross.

Global variables added for multilingual purposes :

	common.h:
		CHANMAX1        5
 		CHANMAX2        7
 		SBGRS          12
	    	L0	    0							    
	    	R0	    1							    
	    	C	    2							    
	    	LS	    3							    
	    	RS	    4							    
	    	L	    5							    
	    	R	    6	
	    	T23	    0
	    	T24	    1
	    	T34	    2
	    	T234        3

	common.c
		int	T2[12], T3[12], T4[12];	
		int	T2outof[12];		
						    

Functions added for multilingual purposes :
	
	dyn_cross.c:

		void take_dyn_cross();
		void trans_chan();
		void combine();
		void dyn_bal();
		void choose_dyn();	
		void scfsi_calc_dyn();
		void scfsi_calc();
		void scf_calc();	




	
Many existing functions have been modified to take into account the multilingual
channels. The modifications are noticed with a commentary containing the key word "JMZ".

BUG CORRECTION

  In file musicin.c
	Reactivated parse_arg()
	Now the commandline is workin again, at least with 5 channels, not with
	multilingual.


