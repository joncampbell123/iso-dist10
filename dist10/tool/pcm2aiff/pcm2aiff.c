/**********************************************************************
 * ISO MPEG Audio Subgroup Software Simulation Group (1996)
 * Audio Interchange File Format (AIFF) / PCM file conversion.
 *
 * $Id: pcm2aiff.c,v 1.2 1996/04/18 06:15:42 rowlands Exp $
 *
 * $Log: pcm2aiff.c,v $
 * Revision 1.2  1996/04/18 06:15:42  rowlands
 * Added multilingual file name format facility.
 *
 * Revision 1.1  1996/04/18 03:39:59  rowlands
 * Initial revision
 *
 * Revision 1.0  1995/06/13  fukuchi
 * UNIX version.
 *
 * Revision 1.1  1995/10/03  09:35:00 fukuchi 
 * Ported new code for MSDOS .
 *
 **********************************************************************/

/**********************************************************************
 * VERSION 1.1                                                        *
 *   changes made since last update:                                  *
 *   date   programmers                comment                        *
 * 6/13/95  Hiroyuki Fukuchi    start of version 1.0 records          *
 * 10/03/95  Hiroyuki Fukuchi   Ported new code for MSDOS.            *
 **********************************************************************/

/***********************************************************************
*
*  Global Include Files
*
***********************************************************************/
#include        "common.h"
#include        "decoder.h"

/***************************************************************************/

#define	MaxNumberOfChannels	7

char *	multichannelPCMFilenameSuffix[] = {
	"l", "r", "c", "ls", "rs", NULL
};
char *	multilingualPCMFilenameSuffix[] = {
	"m1", "m2", "m3", "m4", "m5", "m6", "m7", NULL
};
char **	pcmFilenameSuffix	= multichannelPCMFilenameSuffix;

#ifdef __MSDOS__
char *	multichannelAIFFFilenameSuffix = ".aif";
char *	multilingualAIFFFilenameSuffix = ".aif";
char *	aiffFilenameSuffix	= ".aif";
#else
char *	multichannelAIFFFilenameSuffix = ".aiff";
char *	multilingualAIFFFilenameSuffix = ".aiff.ml";
char *	aiffFilenameSuffix	= ".aiff";
#endif

/***************************************************************************/

main(int argc, char *argv[])
{
char    file_name[50];
int     i,j,c,disp,arg_count,config,aiff_mode;
IFF_AIFF        aiff,*aiff_ptr;
int     pcm2aiff(),aiff2pcm();        

	aiff_ptr=&aiff;
	if(argc==1){
	    printf("usage : pcm2aiff <input file> [-a],[-c[nx]],[-r[nx]],[-m]\n");
	    printf("            pcm <-> aiff file conversion\n");
	    printf("         [option]\n");
	    printf("        a : aiff->pcm (default: pcm->aiff)\n");
	    printf("        c[nx]: channel configuration for pcm->aiff\n");
	    printf("            nx (default: 1)\n");
	    printf("            0: L \n");
	    printf("            1: L R\n"); 
	    printf("            2: L R C\n");
	    printf("            3: L R C LS\n");
	    printf("            4: L R C LS RS\n");
	    printf("        m : multilingual filename format (default multichannel)\n");
	    printf("        r[nx]: sampling frequency\n");
	    printf("            nx (default: 0)\n");
	    printf("            0: 44.1 kHz \n");
	    printf("            1: 48 kHz\n"); 
	    printf("            2: 32 KHz\n");
	    printf("        L  ch file is <[input file]_l.pcm>\n");
	    printf("        R  ch file is <[input file]_r.pcm>\n");
	    printf("        LS ch file is <[input file]_ls.pcm>\n");
	    printf("        RS ch file is <[input file]_rs.pcm>\n");
	    printf("        C  ch file is <[input file]_c.pcm>\n");
	    printf("        aiff  file is <[input file].aiff> or <[input file].aiff.ml>\n");
	    exit(1);
	}
	for(i=0;i<50;i++)
	    file_name[i]=0;
	i=0;
	file_name[i]=(*++argv)[0];
	i++;
	while(file_name[i]=*++argv[0])
	    i++;

	config=1;
	aiff_mode=0;
	arg_count=argc;
	aiff_ptr->sampleRate=44100.00;
	while(arg_count>=3){
	    arg_count--;
	    if((*++argv)[0]=='-'){
		while(c=*++argv[0]){
		    switch(c){
		    case        'r':
			c=*++argv[0];
			switch(c){
			case    '0':
			    aiff_ptr->sampleRate=44100.00;
			    break;
			case    '1':
			    aiff_ptr->sampleRate=48000.00;
			    break;
			case    '2':
			    aiff_ptr->sampleRate=32000.00;
			    break;
			default :
			    aiff_ptr->sampleRate=44100.00;
			    break;
			};
			break;
		    case	'm':
			pcmFilenameSuffix = multilingualPCMFilenameSuffix;
			aiffFilenameSuffix = multilingualAIFFFilenameSuffix;
			break;
		    case        'c':
			config=*++argv[0]-'0';
			break;
		    case        'a':
			aiff_mode=1;
			break;
		    default:
			printf("	????\n");
			exit(1);
		    }
		}
	    }
	}
	if(aiff_mode){
	    aiff2pcm(file_name,aiff_ptr);
	}else{
	    pcm2aiff(file_name,aiff_ptr,config);
	}
};

/**********************************************************************
*
*  PCM to Audio Interchange File Format(AIFF) conversion.
*
**********************************************************************/

int pcm2aiff(char *fname,IFF_AIFF *aiff_ptr,int config)
{
register char   i;
register long   seek_offset;
int             index,lflag,rflag,cflag,lsflag,rsflag;
int             size;
int             BUF_SIZE=384;
unsigned long   frame;
char            temp_sampleRate[10];
static char     tmp_file_name[200];
char            holder;
FILE            *file_ptr_pcm[MaxNumberOfChannels];
FILE            *file_ptr_aiff;
static char     buf[MaxNumberOfChannels][1152*2];
char            buf0[2]={0,0};
Chunk           FormChunk;
CommonChunk     CommChunk;
SoundDataChunk  SndDChunk;

#ifdef __MSDOS__
long            lholder;
void            swab();
#endif

	printf("        pcm to aiff conversion\n");
	aiff_ptr->numChannels = config + 1;

	for (i = 0; i < aiff_ptr->numChannels; i++) {
	    sprintf(tmp_file_name, "%s_%s.pcm", fname, pcmFilenameSuffix[i]);
	    if(NULL==(file_ptr_pcm[i]=fopen(tmp_file_name,"rb"))){
		printf("        read file open error\n");
		exit(1);
	    }
	}

	strcpy( tmp_file_name,fname); 
	strcat( tmp_file_name,aiffFilenameSuffix); 
	if(NULL==(file_ptr_aiff=fopen(tmp_file_name,"wb"))){
		printf("        write file open error\n");
		exit(1);
	}

	aiff_ptr->sampleSize=16;
	aiff_ptr->blkAlgn.offset=0;
	aiff_ptr->blkAlgn.blockSize=0;
	strcpy(aiff_ptr->sampleType,IFF_ID_SSND);
	
	double_to_extended(&aiff_ptr->sampleRate, temp_sampleRate);
	
	strcpy( FormChunk.ckID, IFF_ID_FORM);
	strcpy( FormChunk.formType, IFF_ID_AIFF);
	strcpy( CommChunk.ckID, IFF_ID_COMM); 

	for (i = 0; i < sizeof(char[10]); i++)
	    CommChunk.sampleRate[i] = temp_sampleRate[i];

	CommChunk.numChannels             = aiff_ptr->numChannels;
	CommChunk.sampleSize              = aiff_ptr->sampleSize;
	SndDChunk.offset                  = aiff_ptr->blkAlgn.offset;
	SndDChunk.blockSize               = aiff_ptr->blkAlgn.blockSize;
	strncpy( SndDChunk.ckID, aiff_ptr->sampleType,4);
	CommChunk.ckSize = sizeof(CommChunk.numChannels) +
	sizeof(CommChunk.numSampleFrames)+sizeof(CommChunk.sampleSize)+
	sizeof(CommChunk.sampleRate);

	SndDChunk.ckSize = sizeof(SoundDataChunk)-sizeof(ChunkHeader)+
	(CommChunk.sampleSize + BITS_IN_A_BYTE - 1) / BITS_IN_A_BYTE *
	CommChunk.numChannels * CommChunk.numSampleFrames;

	FormChunk.ckSize=sizeof(Chunk)+SndDChunk.ckSize+sizeof(ChunkHeader)+
	    CommChunk.ckSize;


	if(fwrite(&FormChunk, sizeof(Chunk), 1, file_ptr_aiff) != 1)
	    return(-1);

	if(fwrite(CommChunk.ckID, sizeof(ID),1,file_ptr_aiff) != 1)
	    return(-1);

	if(fwrite(&CommChunk.ckSize,sizeof(long),1,file_ptr_aiff)!=1)
	    return(-1);

	if(fwrite(&CommChunk.numChannels,sizeof(short),1,file_ptr_aiff)!=1)
	    return(-1);

	if(fwrite(&CommChunk.numSampleFrames,sizeof(unsigned long),1,
	      file_ptr_aiff) != 1)
	    return(-1);

	if(fwrite(&CommChunk.sampleSize,sizeof(short),1,file_ptr_aiff)!=1)
	    return(-1);
	if(fwrite(CommChunk.sampleRate,sizeof(char[10]),1,file_ptr_aiff)!=1)
	    return(-1);

	if(fwrite(&SndDChunk,sizeof(SoundDataChunk),1,file_ptr_aiff)!=1)
	    return(-1);

	frame=0;
	while(1){
	    for (i = 0; i < aiff_ptr->numChannels; i++) {
		    size=fread(&buf[i],2,BUF_SIZE,file_ptr_pcm[i]);
		    if(size==0) break;
	    }

	    for(index=0;index<size;index++){
		for (i = 0; i < aiff_ptr->numChannels; i++) {
			fwrite(&buf[i][2*index],2,1,file_ptr_aiff);
		}
	    }
	    frame++;
	    if(size<BUF_SIZE){
		for(index=size;index<BUF_SIZE;index++){
		    for (i = 0; i < aiff_ptr->numChannels; i++) {
			fwrite(&buf0[i],2,1,file_ptr_aiff);
		    }
		}
		break;
	    }
	}
	printf("        number of samples in frame / channel = %ld\n",BUF_SIZE*frame-size);

	aiff_ptr->numSampleFrames=BUF_SIZE*frame;
	CommChunk.numSampleFrames         = aiff_ptr->numSampleFrames;
	CommChunk.ckSize = sizeof(CommChunk.numChannels)+
	sizeof(CommChunk.numSampleFrames)+sizeof(CommChunk.sampleSize)+
	sizeof(CommChunk.sampleRate);
	SndDChunk.ckSize =sizeof(SoundDataChunk)-sizeof(ChunkHeader)+
	(CommChunk.sampleSize + BITS_IN_A_BYTE - 1) / BITS_IN_A_BYTE *
	CommChunk.numChannels * CommChunk.numSampleFrames;
	FormChunk.ckSize = sizeof(Chunk)+SndDChunk.ckSize+sizeof(ChunkHeader)+
	    CommChunk.ckSize;


	if (fseek(file_ptr_aiff, 0, SEEK_SET) != 0)
	    return(-1);

#ifdef __MSDOS__
	lholder=FormChunk.ckSize;
	swab(&lholder,&FormChunk.ckSize,4);
	lholder=FormChunk.ckSize&0xffff;
	FormChunk.ckSize=(((FormChunk.ckSize>>16)&0xffff)+(lholder<<16))&0xffffffff;
#endif

	if (fwrite(&FormChunk, sizeof(Chunk), 1, file_ptr_aiff) != 1)
	    return(-1);

#ifdef __MSDOS__
	lholder=CommChunk.ckSize;
	swab(&lholder,&CommChunk.ckSize,4);
	lholder=CommChunk.ckSize&0xffff;
	CommChunk.ckSize=(((CommChunk.ckSize>>16)&0xffff)+(lholder<<16))&0xffffffff;
	
	lholder=CommChunk.sampleSize;
	swab(&lholder,&CommChunk.sampleSize,2);
	lholder=CommChunk.numChannels;
	swab(&lholder,&CommChunk.numChannels,2);
	
	lholder=CommChunk.numSampleFrames;
	swab(&lholder,&CommChunk.numSampleFrames,4);
	lholder=CommChunk.numSampleFrames&0xffff;
	CommChunk.numSampleFrames=(((CommChunk.numSampleFrames>>16)&0xffff)+(lholder<<16))&0xffffffff;
#endif

	if (fwrite(CommChunk.ckID,sizeof(ID),1,file_ptr_aiff)!=1)
	    return(-1);
	if (fwrite(&CommChunk.ckSize,sizeof(long),1,file_ptr_aiff)!=1)
	    return(-1);
	if (fwrite(&CommChunk.numChannels,sizeof(short),1,file_ptr_aiff)!=1)
	    return(-1);
	if (fwrite(&CommChunk.numSampleFrames,sizeof(unsigned long),1,
	      file_ptr_aiff)!=1)
	    return(-1);
	if (fwrite(&CommChunk.sampleSize, sizeof(short),1,file_ptr_aiff)!=1)
	    return(-1);
	if (fwrite(CommChunk.sampleRate,sizeof(char[10]),1,file_ptr_aiff)!=1)
	    return(-1);

#ifdef __MSDOS__
	lholder=SndDChunk.ckSize;
	swab(&lholder,&SndDChunk.ckSize,4);
	lholder=SndDChunk.ckSize&0xffff;
	SndDChunk.ckSize=(((SndDChunk.ckSize>>16)&0xffff)+(lholder<<16))&0xffffffff;

	lholder=SndDChunk.blockSize;
	swab(&lholder,&SndDChunk.blockSize,4);
	lholder=SndDChunk.blockSize&0xffff;
	SndDChunk.blockSize=(((SndDChunk.blockSize>>16)&0xffff)+(lholder<<16))&0xffffffff;

	lholder=SndDChunk.offset;
	swab(&lholder,&SndDChunk.offset,4);
	lholder=SndDChunk.offset&0xffff;
	SndDChunk.offset=(((SndDChunk.offset>>16)&0xffff)+(lholder<<16))&0xffffffff;
#endif

	if(fwrite(&SndDChunk,sizeof(SoundDataChunk),1,file_ptr_aiff)!=1)
	    return(-1);

	for (i = 0; i < aiff_ptr->numChannels; i++) {
		fclose(file_ptr_pcm[i]);
	}
	fclose(file_ptr_aiff);
	
	return(0);
}

/**********************************************************************
*
*  Audio Interchange File Format(AIFF) to PCM conversion.
*
**********************************************************************/
int aiff2pcm(char *fname,IFF_AIFF *aiff_ptr)
{
int             *byte_per_sample;
FILE            *file_ptr,*write_file_ptr[MaxNumberOfChannels];
int             ch,numChan,diff_numChan,sample_size;
register char   i;
long            index;
register long   seek_offset;
char            temp_sampleRate[10];
char            tmpBuf[10];
int             *dummy,*dummy1;
char            holder;
static char     tmp_file_name[200];

Chunk           FormChunk;
CommonChunk     CommChunk;
SoundDataChunk  SndDChunk;
identifier      ident;

IEEE_DBL        *p_dbl,dbl;

#ifdef  __MSDOS__
unsigned long   lholder;
void            swab();
#endif
	p_dbl=&dbl;
	printf("        aiff to pcm conversion\n");
	strcpy( tmp_file_name,fname); 
	strcat( tmp_file_name,aiffFilenameSuffix); 
	if(NULL==(file_ptr=fopen(tmp_file_name,"rb"))){
	    printf("    read file open error\n");
	    exit(1);
	}
	if (fseek(file_ptr, 0, SEEK_SET) != 0)
	    return(-1);
	if (fread(&FormChunk, sizeof(Chunk), 1, file_ptr) != 1)
	    return(-1);
	if (strncmp(FormChunk.ckID,IFF_ID_FORM,4) != 0 ||
	      strncmp(FormChunk.formType,IFF_ID_AIFF,4) != 0)
	    return(-1);
	if (fread(&ident,sizeof(identifier), 1, file_ptr) != 1)
	    return(-1);

#ifdef __MSDOS__
	lholder=ident.ck_length;
	swab(&lholder,&ident.ck_length,4);
	lholder=ident.ck_length&0xffff;
	ident.ck_length=(((ident.ck_length>>16)&0xffff)+(lholder<<16))&0xffffffff;
#endif
	
	while(strncmp(ident.name,IFF_ID_COMM,4) != 0)
	{
	    if(NULL==(dummy = (int *)calloc( ident.ck_length, sizeof(char)))){
		printf("        NULL pointer 1 \n");
	    }
	    if(fread( dummy, ident.ck_length, 1, file_ptr) != 1)
		return(-1);
	    free(dummy);
	    if(fread( &ident, sizeof(identifier),1, file_ptr) != 1)
		return(-1);

#ifdef __MSDOS__
	    lholder=ident.ck_length;
	    swab(&lholder,&ident.ck_length,4);
	    lholder=ident.ck_length&0xffff;
	    ident.ck_length=(((ident.ck_length>>16)&0xffff)+(lholder<<16))&0xffffffff;
#endif
	}
	
	for( i = 0; i < 4; ++i){
		CommChunk.ckID[i] = ident.name[i];
	}
	CommChunk.ckSize = ident.ck_length;

	if (fread(&CommChunk.numChannels, sizeof(short), 1, file_ptr) != 1)
	    return(-1);
	if (fread(&CommChunk.numSampleFrames, sizeof(unsigned long), 1,
	      file_ptr) != 1)
	    return(-1);
	if (fread(&CommChunk.sampleSize, sizeof(short), 1, file_ptr) != 1)
	    return(-1);
	if (fread(CommChunk.sampleRate, sizeof(char[10]), 1, file_ptr) != 1)
	    return(-1);

#ifdef __MSDOS__
	lholder=CommChunk.numChannels;
	swab(&lholder,&CommChunk.numChannels,2);
	
	lholder=CommChunk.numSampleFrames;
	swab(&lholder,&CommChunk.numSampleFrames,4);
	lholder=CommChunk.numSampleFrames&0xffff;
	CommChunk.numSampleFrames=((CommChunk.numSampleFrames>>16)+(lholder<<16))&0xffffffff;
	
	lholder=CommChunk.sampleSize;
	swab(&lholder,&CommChunk.sampleSize,2);
#endif
	for (i = 0; i < sizeof(char[10]); i++)
	    temp_sampleRate[i] = CommChunk.sampleRate[i];

	extended_to_double(temp_sampleRate, &aiff_ptr->sampleRate);

	fseek (file_ptr, sizeof(Chunk), SEEK_SET);

	if (fread(&ident, sizeof(identifier), 1, file_ptr) != 1)
	    return(-1);

#ifdef __MSDOS__
	lholder=ident.ck_length;
	swab(&lholder,&ident.ck_length,4);
	lholder=ident.ck_length&0xffff;
	ident.ck_length=(((ident.ck_length>>16)&0xffff)+(lholder<<16))&0xffffffff;
#endif

	while(strncmp(ident.name,IFF_ID_SSND,4) != 0)
	{
	    if(NULL==(dummy1 = (int *)calloc( ident.ck_length, sizeof(char)))){
		printf("        NULL pointer2 \n");
	    }
	    if(fread( dummy1, ident.ck_length, 1, file_ptr) != 1)
		return(-1);
	    free(dummy1);
	    if(fread( &ident, sizeof(identifier),1, file_ptr) != 1)
		return (-1);

#ifdef __MSDOS__
	    lholder=ident.ck_length;
	    swab(&lholder,&ident.ck_length,4);
	    lholder=ident.ck_length&0xffff;
	    ident.ck_length=(((ident.ck_length>>16)&0xffff)+(lholder<<16))&0xffffffff;
#endif
	}

	for(i = 0; i < 4; ++i)
	    SndDChunk.ckID[i] = ident.name[i];

	if (fread(&SndDChunk.offset, sizeof(unsigned long), 1, file_ptr)!=1)
	    return(-1);

#ifdef __MSDOS__
	lholder=SndDChunk.offset;
	swab(&lholder,&SndDChunk.offset,4);
	lholder=SndDChunk.offset&0xffff;
	SndDChunk.offset=(((SndDChunk.offset>>16)&0xffff)+(lholder<<16))&0xffffffff;
#endif

	if (fread(&SndDChunk.blockSize, sizeof(unsigned long), 1,
	     file_ptr) != 1)
	    return(-1);

#ifdef __MSDOS__
	lholder=SndDChunk.blockSize;
	swab(&lholder,&SndDChunk.blockSize,4);
	lholder=SndDChunk.blockSize&0xffff;
	SndDChunk.blockSize=(((SndDChunk.blockSize>>16)&0xffff)+(lholder<<16))&0xffffffff;
#endif

	numChan = CommChunk.numChannels;
	ch=0;
	for (i = 0; i < numChan; i++) {
	    sprintf(tmp_file_name, "%s_%s.pcm", fname, pcmFilenameSuffix[i]);
	    if(NULL==(write_file_ptr[ch]=fopen(tmp_file_name,"wb"))){
		printf("        write file open error\n");
		exit(1);
	    }
	    ch++;
	}
	diff_numChan=numChan-CommChunk.numChannels;
	if(diff_numChan>0){
	}

	printf("        number of samples in frame / channel = %ld\n",CommChunk.numSampleFrames);
	sample_size=(CommChunk.sampleSize+BITS_IN_A_BYTE-1)/BITS_IN_A_BYTE;
	for(index = 0; index < CommChunk.numSampleFrames; index++){
	    for(ch = 0; ch < CommChunk.numChannels; ch++){
		if (fread(&tmpBuf, sample_size, 1,file_ptr) != 1)
		    return(-1);
		if (fwrite(&tmpBuf, sample_size, 1,write_file_ptr[ch]) != 1)
		    return(-1);
	    }
	}

	aiff_ptr->numChannels       = CommChunk.numChannels;
	aiff_ptr->numSampleFrames   = CommChunk.numSampleFrames;
	aiff_ptr->sampleSize        = CommChunk.sampleSize;
	aiff_ptr->blkAlgn.offset    = SndDChunk.offset;
	aiff_ptr->blkAlgn.blockSize = SndDChunk.blockSize;
	strncpy(aiff_ptr->sampleType, SndDChunk.ckID, 4);
	
	for (i = 0; i < numChan; i++) {
		fclose(write_file_ptr[i]);
	}
	fclose(file_ptr);
	
	return(0);
}


/**********************************************************************
*
*  Seek past some Audio Interchange File Format (AIFF) headers to sound data.
*
**********************************************************************/
int aiff_seek_to_sound_data(FILE *file_ptr)
{
	if (fseek(file_ptr,sizeof(Chunk)+sizeof(SoundDataChunk),SEEK_SET)!=0)
	    return(-1);
	else
	    return(0);
}

/**********************************************************************
*
*  double_to_extended()
*
*  Purpose:     Convert from IEEE double precision format to SANE extended
*               format.
*
*  Passed:      Pointer to the double precision number and a pointer to what
*               will hold the Apple SANE extended format value.
*
*  Outputs:     The SANE extended format pointer will be filled with the
*               converted value.
*
*  Returned:    Nothing.
*
*********************************************************************/

void    double_to_extended(double *pd, char ps[10])
{
#ifdef  MACINTOSH
	x96tox80(pd, (extended *) ps);
#else
register unsigned long  top2bits;
register unsigned short *ps2;
register IEEE_DBL       *p_dbl;
register SANE_EXT       *p_ext;
SANE_EXT ext_align;
char *c_align;
int i;

#ifdef  __MSDOS__
unsigned long  holder;
void    swab();
#endif

	p_dbl = (IEEE_DBL *) pd;

#ifdef  __MSDOS__
	holder = p_dbl->lo;
	p_dbl->lo = p_dbl->hi;
	p_dbl->hi = holder;
#endif

	p_ext=&ext_align;
	
	top2bits = p_dbl->hi & 0xc0000000L;
	p_ext->l1 = ((p_dbl->hi >> 4) & 0x3ff0000L) | top2bits;
	p_ext->l1 |= ((p_dbl->hi >> 5) & 0x7fffL) | 0x8000L;
	p_ext->l2 = (p_dbl->hi << 27) & 0xf8000000L;
	p_ext->l2 |= ((p_dbl->lo >> 5) & 0x07ffffffL);

#ifdef  __MSDOS__
	holder = p_ext->l1 ;
	swab(&holder,&p_ext->l1,4);
	holder=p_ext->l1&0xffff;
	p_ext->l1=((p_ext->l1>>16)+(holder<<16))&0xffffffff;
	holder = p_ext->l2 ;
	swab(&holder,&p_ext->l2,4);
	holder=p_ext->l2&0xffff;
	p_ext->l2=((p_ext->l2>>16)+(holder<<16))&0xffffffff;
#endif
	
	ps2 = (unsigned short *) & (p_dbl->lo);
	ps2++;
	ext_align.s1 = (*ps2 << 11) & 0xf800;
	
	c_align = (char *) p_ext;
	for (i=0;i<10;i++)
	    ps[i] = c_align[i];

#endif
}

/**********************************************************************
*
*  extended_to_double()
*
*  Purpose:     Convert from SANE extended format to IEEE double precision
*               format.
*
*  Passed:      Pointer to the Apple SANE extended format value and a pointer
*               to what will hold the the IEEE double precision number.
*
*  Outputs:     The IEEE double precision format pointer will be filled with
*               the converted value.
*
*  Returned:    Nothing.
*
**********************************************************************/
void    extended_to_double(char ps[10], double *pd)
{
#ifdef  MACINTOSH
   x80tox96((extended *) ps, pd);
#else
register unsigned long  top2bits;
register IEEE_DBL       *p_dbl;
register SANE_EXT       *p_ext;
SANE_EXT ext_align;
char *c_align;
int i;

#ifdef  __MSDOS__
unsigned long  holder;
void    swab();
#endif

	p_dbl = (IEEE_DBL *) pd;
	p_ext = &ext_align;

	c_align = (char *) p_ext;
	for (i=0;i<10;i++)
	   c_align[i] = ps[i];

#ifdef  __MSDOS__
	holder = p_ext->l1;
	swab(&holder,&p_ext->l1,4);
	holder = p_ext->l1&0xffff;
	p_ext->l1 = (((p_ext->l1>>16)&0xffff)+(holder<<16))&0xffffffff;
	holder = p_ext->l2;
	swab(&holder,&p_ext->l2,4);
	holder = p_ext->l2&0xffff;
	p_ext->l2 = (((p_ext->l2>>16)&0xffff)+(holder<<16))&0xffffffff;
	holder = p_ext->s1;
	swab(&holder,&p_ext->s1,2);
#endif

	top2bits = p_ext->l1 & 0xc0000000L;
	p_dbl->hi = ((p_ext->l1 << 4) & 0x3ff00000L) | top2bits;
	p_dbl->hi |= (p_ext->l1 << 5) & 0xffff0L;
	p_dbl->hi |= (p_ext->l2 >> 27) & 0x1f;
	p_dbl->lo = (p_ext->l2 << 5) & 0xffffffe0L;
	p_dbl->lo |= (unsigned long) ((p_ext->s1 >> 11) & 0x1f);

#ifdef  __MSDOS__
	holder = p_dbl->lo;
	p_dbl->lo = p_dbl->hi;
	p_dbl->hi = holder;
#endif

#endif
};
