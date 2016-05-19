#include "JpegRtpApi.h"


//Jpeg�ļ��б�־�Ķ���
#define  MARKER	0xFF
//Start Of Frame markers, non-differential,Huffman coding
#define  SOF0 	0xC0
#define  SOF1 	0xC1
#define  SOF2 	0xC2
#define  SOF3 	0xC3

//Start Of Frame markers,differential,Huffman Coding
#define  SOF5 	0xC5
#define  SOF6 	0xC6
#define  SOF7 	0xC7

//Start Of Frame markers,no-differential.arithmetic coding
#define  JPG 	0xC8
#define  SOF9 	0xC9
#define  SOF10 	0xCA
#define  SOF11 	0xCB

//Start Of Frame markers,differential.arithmetic coding
#define  SOF13 	0xCD
#define  SOF14 	0xCE
#define  SOF15 	0xCF

//HUffman table specification
#define  DHT  	0xC4

//Arithmetic coding conditioning specification
#define  DAC  	0xCC

//Restart interval termination
#define  RST0	0xD0
#define  RST1	0xD1
#define  RST2	0xD2
#define  RST3	0xD3
#define  RST4	0xD4
#define  RST5	0xD5
#define  RST6	0xD6
#define  RST7	0xD7

//Other markers
#define  SOI  	0xD8
#define  EOI  	0xD9
#define  SOS  	0xDA
#define  DQT  	0xDB
#define  DNL  	0xDC
#define  DRI  	0xDD
#define  DHP  	0xDE
#define  EXP  	0xDF

#define  APP0 	0xE0
#define  APP1 	0xE1
#define  APP2 	0xE2
#define  APP3 	0xE3
#define  APP4 	0xE4
#define  APP5 	0xE5
#define  APP6 	0xE6
#define  APP7 	0xE7
#define  APP8 	0xE8
#define  APP9 	0xE9
#define  APP10 	0xEA
#define  APP11 	0xEB
#define  APP12 	0xEC
#define  APP13 	0xED
#define  APP14 	0xEE
#define  APP15 	0xEF

#define  JPG0	0xF0
#define  JPG1	0xF1
#define  JPG2	0xF2
#define  JPG3	0xF3
#define  JPG4	0xF4
#define  JPG5	0xF5
#define  JPG6	0xF6
#define  JPG7	0xF7
#define  JPG8	0xF8
#define  JPG9	0xF9
#define  JPG10	0xFA
#define  JPG11	0xFB
#define  JPG12	0xFC
#define  JPG13	0xFD
#define  COM  	0xFE

//Reserved markers
#define  TEM  	0x01
//0xFF02 0xFFBF RES




static int SolveSOI(struct JPEG *jpeg);
static int SolveAPPn(struct JPEG *jpeg, unsigned char *buf, unsigned int length);
static int SolveDQT(struct JPEG *jpeg, unsigned char *buf, unsigned int length);
static int SolveSOF0(struct JPEG *jpeg, unsigned char *buf, unsigned int length);
static int SolveDHT(struct JPEG *jpeg, unsigned char *buf, unsigned int length);
static int SolveDRI(struct JPEG *jpeg, unsigned char *buf, unsigned int length);
static int SolveSOS(struct JPEG *jpeg, unsigned char *buf, unsigned int length);
static int SolveEOI(struct JPEG *jpeg);

static int WriteSOI(struct JPEG *jpeg, int (*putdata)(unsigned char *buf, int offset, unsigned int size));
static int WriteDQT(struct JPEG *jpeg, int (*putdata)(unsigned char *buf, int offset, unsigned int size));
static int WriteSOF0(struct JPEG *jpeg, int (*putdata)(unsigned char *buf, int offset, unsigned int size));
static int WriteDHT(struct JPEG *jpeg, int (*putdata)(unsigned char *buf, int offset, unsigned int size));
static int WriteDRI(struct JPEG *jpeg, int (*putdata)(unsigned char *buf, int offset, unsigned int size));
static int WriteSOS(struct JPEG *jpeg, int (*putdata)(unsigned char *buf, int offset, unsigned int size));
static int WriteEOI(struct JPEG *jpeg, int (*putdata)(unsigned char *buf, int offset, unsigned int size));



unsigned char lum_dc_codelens[] = {
        0, 1, 5, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
};

unsigned char lum_dc_symbols[] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
};

unsigned char lum_ac_codelens[] = {
        0, 2, 1, 3, 3, 2, 4, 3, 5, 5, 4, 4, 0, 0, 1, 0x7d,
};

unsigned char lum_ac_symbols[] = {
        0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12,
        0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07,
        0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xa1, 0x08,
        0x23, 0x42, 0xb1, 0xc1, 0x15, 0x52, 0xd1, 0xf0,
        0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0a, 0x16,
        0x17, 0x18, 0x19, 0x1a, 0x25, 0x26, 0x27, 0x28,
        0x29, 0x2a, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
        0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
        0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
        0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
        0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
        0x7a, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
        0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98,
        0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
        0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6,
        0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3, 0xc4, 0xc5,
        0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4,
        0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe1, 0xe2,
        0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea,
        0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
        0xf9, 0xfa,
};

unsigned char chm_dc_codelens[] = {
        0, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
};

unsigned char chm_dc_symbols[] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
};

unsigned char chm_ac_codelens[] = {
        0, 2, 1, 2, 4, 4, 3, 4, 7, 5, 4, 4, 0, 1, 2, 0x77,
};

unsigned char chm_ac_symbols[] = {
        0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21,
        0x31, 0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71,
        0x13, 0x22, 0x32, 0x81, 0x08, 0x14, 0x42, 0x91,
        0xa1, 0xb1, 0xc1, 0x09, 0x23, 0x33, 0x52, 0xf0,
        0x15, 0x62, 0x72, 0xd1, 0x0a, 0x16, 0x24, 0x34,
        0xe1, 0x25, 0xf1, 0x17, 0x18, 0x19, 0x1a, 0x26,
        0x27, 0x28, 0x29, 0x2a, 0x35, 0x36, 0x37, 0x38,
        0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
        0x49, 0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
        0x59, 0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
        0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
        0x79, 0x7a, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
        0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95, 0x96,
        0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5,
        0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4,
        0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3,
        0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2,
        0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda,
        0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9,
        0xea, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
        0xf9, 0xfa,
};


static int SolveSOI(struct JPEG *jpeg)
{
	//printf("SolveSOI\n");
	return 0;
}

static int SolveAPPn(struct JPEG *jpeg, unsigned char *buf, unsigned int length)
{
	//printf("SolveAPPn\n");
	return 0;
}

static int SolveDQT(struct JPEG *jpeg, unsigned char *buf, unsigned int length)
{
	unsigned int x,y;
	unsigned int offset;
	//printf("SolveDQT\n");
	offset=0;
	while(length>64)				//����������ִ�ŷ�ʽ���������һ���ŵĻ���Ҫ�ٴα��
	{
		if(((buf[offset]>>4)&0xF)==1)  return -1; //ֻ֧��8bit��������
		for(y=0;y<8;y++)
		{
			unsigned int temp;
			temp=y<<3;
			for(x=0;x<8;x++)
			{
				jpeg->dqtable.table[buf[offset]&0xF][y][x]=buf[temp+x+1+offset];//����������
			}
		}
		length=length-65;
		offset=offset+65;
		jpeg->dqtable.count++;
		if(jpeg->dqtable.count>2) return -1; //���ֻ֧������������
	}
	return 0;
}

static int SolveSOF0(struct JPEG *jpeg, unsigned char *buf, unsigned int length)
{
	//printf("SolveSOF0\n");
	if(buf[0]!=8) return -1;   //ֻ֧��8bit������
	jpeg->height=(buf[1]<<8)|buf[2];		
	jpeg->width=(buf[3]<<8)|buf[4];
	if(buf[5]!=3) return -1;		//ֻ֧��������ɫ������
	if(((buf[7]&0xF)==1)&&((((buf[7]>>4)&0xF)==1)))
	{
		jpeg->pixfmt=YUV444;
	}
	else if(((buf[7]&0xF)==1)&&(((buf[7]>>4)&0xF)==2))
	{

		jpeg->pixfmt=YUV422;		
	}
	else
	{
			jpeg->pixfmt=YUV420;		
	}
	return 0;
}

static int SolveDHT(struct JPEG *jpeg, unsigned char *buf, unsigned int length)
{
	//printf("SolveDHT\n");
	return 0;
}

static int SolveDRI(struct JPEG *jpeg, unsigned char *buf, unsigned int length)
{
	printf("SolveDRI\n");
	jpeg->RSTCount=(buf[0]<<8)|buf[1];
	return 0;
}

static int SolveSOS(struct JPEG *jpeg, unsigned char *buf, unsigned int length)
{
	//printf("SolveSOS\n");
	return 0;
}

static int SolveEOI(struct JPEG *jpeg)
{
	//printf("SolveEOI\n");
	return 0;
}


//��ȡjpeg��Ϣ
//����jpegΪ��Ϣ���λ�ã�����ɹ�����Ϊ������������ռ䣬��ʹ��ʱ�����JpegInfoDestroy�ͷ�
//getdataΪ�ص����������ڴ�����Դ��ȡ���ݣ�������ʵ��
//����JPEGRTPSTATUSOK�ɹ�
enum JPEGRTPSTATUS JpegInfoGet(struct JPEG *jpeg, int (*getdata)(unsigned char *buf, int offset, unsigned int size))
{

	unsigned char ch;
	unsigned char buf[0xFFFF];
	unsigned int length;
	unsigned int offset;
	unsigned int scanstart,scanend;
	int ret=0;

	if(getdata(buf, -1, 2)!=2)return FILEERROR;
	if(((buf[0]<<8)|(buf[1]))!=0xFFD8)				//jpeg�ļ��ĸտ�ʼ�����ֽڿ϶�Ϊ0xFFD8
		return FILEERROR;
	else
		ret=SolveSOI(jpeg);
	if(ret) return JPEGRTPSTATUSERROR;
	offset=2;
	while(getdata(&ch, -1, 1)==1)
	{
		offset++;
		if(ch==MARKER)
		{
			if(getdata(&ch, -1, 1)!=1)	//��ȡ��־
				return FILEERROR;
			offset++;
			switch(ch)
			{
			case 0:break;
			case MARKER:break;
			case SOI:	ret=SolveSOI(jpeg);if(ret) return JPEGRTPSTATUSERROR;break;
			case APP0:
			case APP1:
			case APP2:
			case APP3:
			case APP4:
			case APP5:
			case APP6:
			case APP7:
			case APP8:
			case APP9:
			case APP10:
			case APP11:
			case APP12:
			case APP13:
			case APP14:
			case APP15:	if(getdata(buf, -1, 2)!=2)return FILEERROR;
						length=((buf[0]<<8)|buf[1])-2;
						if(getdata(buf, -1, length)!=length)return FILEERROR;
						offset+=2+length;
						ret=SolveAPPn(jpeg, buf, length);if(ret) return JPEGRTPSTATUSERROR;break;
			case DQT:	if(getdata(buf, -1, 2)!=2)return FILEERROR;
						length=((buf[0]<<8)|buf[1])-2;
						if(getdata(buf, -1, length)!=length)return FILEERROR;
						offset+=2+length;
						ret=SolveDQT(jpeg, buf, length);if(ret) return JPEGRTPSTATUSERROR;break;
			case SOF0:	if(getdata(buf, -1, 2)!=2)return FILEERROR;
						length=((buf[0]<<8)|buf[1])-2;
						if(getdata(buf, -1, length)!=length)return FILEERROR;
						offset+=2+length;
						ret=SolveSOF0(jpeg, buf, length);if(ret) return JPEGRTPSTATUSERROR;break;
			case DHT:	if(getdata(buf, -1, 2)!=2)return FILEERROR;
						length=((buf[0]<<8)|buf[1])-2;
						if(getdata(buf, -1, length)!=length)return FILEERROR;
						offset+=2+length;
						ret=SolveDHT(jpeg, buf, length);if(ret) return JPEGRTPSTATUSERROR;break;
			case DRI:	if(getdata(buf, -1, 2)!=2)return FILEERROR;
						length=((buf[0]<<8)|buf[1])-2;
						if(getdata(buf, -1, length)!=length)return FILEERROR;
						offset+=2+length;
						ret=SolveDRI(jpeg, buf, length);if(ret) return JPEGRTPSTATUSERROR;break;
			case SOS:	if(getdata(buf, -1, 2)!=2)return FILEERROR;
						length=((buf[0]<<8)|buf[1])-2;
						if(getdata(buf, -1, length)!=length)return FILEERROR;
						offset+=2+length;
						scanstart=offset;
						ret=SolveSOS(jpeg, buf, length);if(ret) return JPEGRTPSTATUSERROR;break;
			case EOI:	scanend=offset-3;
						jpeg->ScanSize=scanend-scanstart+1;
						jpeg->ScanData=(unsigned char*)malloc(jpeg->ScanSize);			//Ϊʵ�ʵ�jpeg���ر�������ռ�
						if(jpeg==NULL)
							return MALLOCERROR;
						ret=getdata(jpeg->ScanData,scanstart,jpeg->ScanSize);
						if(ret!=jpeg->ScanSize) return FILEERROR;
						ret=SolveEOI(jpeg);if(ret) return JPEGRTPSTATUSERROR;
						return JPEGRTPSTATUSOK;
						break;

			default:printf("unexpected marker: oxFF%X\n",ch);break;
			}
		}
	}
	return JPEGRTPSTATUSOK;
}

//��ʾjpeg��Ϣ
//����JPEGRTPSTATUSOK�ɹ�
enum JPEGRTPSTATUS JpegInfoShow(struct JPEG *jpeg)
{
	unsigned int i;
	if(jpeg==NULL) return ARGERROR;
	printf("\nShow Jpeg Info\n");
	printf("DQTable==============================================\n");
	printf("DQTable count: %u\n",jpeg->dqtable.count);
	for(i=0;i<jpeg->dqtable.count;i++)
	{
		printf("DQTable: %u\n",i);
		unsigned char x,y;
		for(y=0;y<8;y++)
		{
			for(x=0;x<8;x++)
			{
				printf("%u\t",jpeg->dqtable.table[i][y][x]);
			}
			printf("\n");
		}
		printf("\n");
	}
	printf("\n");
	printf("SOF0=================================================\n");
	printf("width: %u\theight: %u\n",jpeg->width,jpeg->height);
	printf("pixfmt: %u\n",jpeg->pixfmt);
	printf("\n");
	printf("DRI==================================================\n");
	printf("RST count: %u\n",jpeg->RSTCount);
	printf("\n");
	printf("SOS==================================================\n");
	printf("ScanSize: %u\n",jpeg->ScanSize);
	printf("ScanData: %p\n",jpeg->ScanData);
	return JPEGRTPSTATUSOK;
}

static int WriteSOI(struct JPEG *jpeg, int (*putdata)(unsigned char *buf, int offset, unsigned int size))
{
	unsigned char ch;
	ch=0xFF;
	if(putdata(&ch,-1,1)!=1) return -1;
	ch=SOI;
	if(putdata(&ch,-1,1)!=1) return -1;
	return 0;
}

static int WriteDQT(struct JPEG *jpeg, int (*putdata)(unsigned char *buf, int offset, unsigned int size))
{
	unsigned char ch;
	unsigned short length;
	unsigned int i;
	unsigned char x,y;

	length=67;
	for(i=0;i<jpeg->dqtable.count;i++)
	{
		ch=0xFF;
		if(putdata(&ch,-1,1)!=1) return -1;
		ch=DQT;
		if(putdata(&ch,-1,1)!=1) return -1;
		ch=length>>8;
		if(putdata(&ch,-1,1)!=1) return -1;
		ch=length&0xFF;
		if(putdata(&ch,-1,1)!=1) return -1;
		ch=i;
		if(putdata(&ch,-1,1)!=1) return -1;

		for(y=0;y<8;y++)
		{
			for(x=0;x<8;x++)
			{
				ch=jpeg->dqtable.table[i][y][x];
				if(putdata(&ch,-1,1)!=1) return -1;
			}
		}
	}
	return 0;
}

static int WriteSOF0(struct JPEG *jpeg, int (*putdata)(unsigned char *buf, int offset, unsigned int size))
{
	unsigned char ch;
	unsigned short length;
	unsigned char  YQtableID, CbCrQtableID;
	unsigned char YWH,CbCrWH;

	ch=0xFF;
	if(putdata(&ch,-1,1)!=1) return -1;
	ch=SOF0;
	if(putdata(&ch,-1,1)!=1) return -1;
	length=17;
	ch=length>>8;
	if(putdata(&ch,-1,1)!=1) return -1;
	ch=length&0xFF;
	if(putdata(&ch,-1,1)!=1) return -1;

	ch=8;
	if(putdata(&ch,-1,1)!=1) return -1;
	ch=jpeg->height>>8;
	if(putdata(&ch,-1,1)!=1) return -1;
	ch=jpeg->height&0xFF;
	if(putdata(&ch,-1,1)!=1) return -1;
	ch=jpeg->width>>8;
	if(putdata(&ch,-1,1)!=1) return -1;
	ch=jpeg->width&0xFF;
	if(putdata(&ch,-1,1)!=1) return -1;
	ch=3;
	if(putdata(&ch,-1,1)!=1) return -1;

	YQtableID=0;
	if(jpeg->dqtable.count==1)
		CbCrQtableID=0;
	else
		CbCrQtableID=1;

	if(jpeg->pixfmt==YUV444)
	{
		YWH=(1<<4)|1;
		CbCrWH=(1<<4)|1;
	}
	else if(jpeg->pixfmt==YUV422)
	{
		YWH=(2<<4)|1;
		CbCrWH=(1<<4)|1;
	}
	else
	{
		YWH=(2<<4)|2;
		CbCrWH=(1<<4)|1;
	}
	ch=1;
	if(putdata(&ch,-1,1)!=1) return -1;
	ch=YWH;
	if(putdata(&ch,-1,1)!=1) return -1;
	ch=YQtableID;
	if(putdata(&ch,-1,1)!=1) return -1;

	ch=2;
	if(putdata(&ch,-1,1)!=1) return -1;
	ch=CbCrWH;
	if(putdata(&ch,-1,1)!=1) return -1;
	ch=CbCrQtableID;
	if(putdata(&ch,-1,1)!=1) return -1;

	ch=3;
	if(putdata(&ch,-1,1)!=1) return -1;
	ch=CbCrWH;
	if(putdata(&ch,-1,1)!=1) return -1;
	ch=CbCrQtableID;
	if(putdata(&ch,-1,1)!=1) return -1;
	return 0;
}

static int WriteDHT(struct JPEG *jpeg, int (*putdata)(unsigned char *buf, int offset, unsigned int size))
{
	unsigned char ch;
	unsigned short length;

//DC0
	ch=0xFF;
	if(putdata(&ch,-1,1)!=1) return -1;
	ch=DHT;
	if(putdata(&ch,-1,1)!=1) return -1;
	length=3+sizeof(lum_dc_codelens)+sizeof(lum_dc_symbols);
	ch=length>>8;
	if(putdata(&ch,-1,1)!=1) return -1;
	ch=length&0xFF;
	if(putdata(&ch,-1,1)!=1) return -1;
	ch=0;
	if(putdata(&ch,-1,1)!=1) return -1;
	if(putdata(lum_dc_codelens,-1,16)!=16) return -1;
	if(putdata(lum_dc_symbols,-1,12)!=12) return -1;

//DC1
	ch=0xFF;
	if(putdata(&ch,-1,1)!=1) return -1;
	ch=DHT;
	if(putdata(&ch,-1,1)!=1) return -1;
	length=3+sizeof(chm_dc_codelens)+sizeof(chm_dc_symbols);
	ch=length>>8;
	if(putdata(&ch,-1,1)!=1) return -1;
	ch=length&0xFF;
	if(putdata(&ch,-1,1)!=1) return -1;
	ch=1;
	if(putdata(&ch,-1,1)!=1) return -1;
	if(putdata(chm_dc_codelens,-1,16)!=16) return -1;
	if(putdata(chm_dc_symbols,-1,12)!=12) return -1;
//AC0
	ch=0xFF;
	if(putdata(&ch,-1,1)!=1) return -1;
	ch=DHT;
	if(putdata(&ch,-1,1)!=1) return -1;
	length=3+sizeof(lum_ac_codelens)+sizeof(lum_ac_symbols);
	ch=length>>8;
	if(putdata(&ch,-1,1)!=1) return -1;
	ch=length&0xFF;
	if(putdata(&ch,-1,1)!=1) return -1;
	ch=(1<<4);
	if(putdata(&ch,-1,1)!=1) return -1;
	if(putdata(lum_ac_codelens,-1,16)!=16) return -1;
	if(putdata(lum_ac_symbols,-1,162)!=162) return -1;

//AC1
	ch=0xFF;
	if(putdata(&ch,-1,1)!=1) return -1;
	ch=DHT;
	if(putdata(&ch,-1,1)!=1) return -1;
	length=3+sizeof(chm_ac_codelens)+sizeof(chm_ac_symbols);
	ch=length>>8;
	if(putdata(&ch,-1,1)!=1) return -1;
	ch=length&0xFF;
	if(putdata(&ch,-1,1)!=1) return -1;
	ch=(1<<4)|1;
	if(putdata(&ch,-1,1)!=1) return -1;
	if(putdata(chm_ac_codelens,-1,16)!=16) return -1;
	if(putdata(chm_ac_symbols,-1,162)!=162) return -1;

	return 0;
}

static int WriteDRI(struct JPEG *jpeg, int (*putdata)(unsigned char *buf, int offset, unsigned int size))
{
	unsigned char ch;
	ch=0xFF;
	if(putdata(&ch,-1,1)!=1) return -1;
	ch=DRI;
	if(putdata(&ch,-1,1)!=1) return -1;
	//length
	ch=0x00;
	if(putdata(&ch,-1,1)!=1) return -1;
	ch=0x04;
	if(putdata(&ch,-1,1)!=1) return -1;
	//RSTCount
	ch=jpeg->RSTCount>>8;
	if(putdata(&ch,-1,1)!=1) return -1;
	ch=jpeg->RSTCount&0xFF;
	if(putdata(&ch,-1,1)!=1) return -1;
	return 0;
}

static int WriteSOS(struct JPEG *jpeg, int (*putdata)(unsigned char *buf, int offset, unsigned int size))
{
	unsigned char ch;
	unsigned short length;

	ch=0xFF;
	if(putdata(&ch,-1,1)!=1) return -1;
	ch=SOS;
	if(putdata(&ch,-1,1)!=1) return -1;
	length=12;
	ch=length>>8;
	if(putdata(&ch,-1,1)!=1) return -1;
	ch=length&0xFF;
	if(putdata(&ch,-1,1)!=1) return -1;
	ch=3;
	if(putdata(&ch,-1,1)!=1) return -1;

	ch=1;
	if(putdata(&ch,-1,1)!=1) return -1;
	ch=0;
	if(putdata(&ch,-1,1)!=1) return -1;

	ch=2;
	if(putdata(&ch,-1,1)!=1) return -1;
	ch=(1<<4)|1;
	if(putdata(&ch,-1,1)!=1) return -1;
	ch=3;
	if(putdata(&ch,-1,1)!=1) return -1;
	ch=(1<<4)|1;
	if(putdata(&ch,-1,1)!=1) return -1;

	ch=0;
	if(putdata(&ch,-1,1)!=1) return -1;
	ch=63;
	if(putdata(&ch,-1,1)!=1) return -1;
	ch=0;
	if(putdata(&ch,-1,1)!=1) return -1;
	if(jpeg->ScanSize>0)
	if(putdata(jpeg->ScanData,-1, jpeg->ScanSize)!=jpeg->ScanSize) return -1;
	return 0;
}

static int WriteEOI(struct JPEG *jpeg, int (*putdata)(unsigned char *buf, int offset, unsigned int size))
{
	unsigned char ch;
	ch=0xFF;
	if(putdata(&ch,-1,1)!=1) return -1;
	ch=EOI;
	if(putdata(&ch,-1,1)!=1) return -1;
	return 0;
}

//��jpeg��Ϣ�洢
//����jpegΪ��Ϣ���λ��
//putdataΪ�ص����������ڱ�����Ϣ��������ʵ��
//����JPEGRTPSTATUSOK�ɹ�
enum JPEGRTPSTATUS JpegInfoPut(struct JPEG *jpeg, int (*putdata)(unsigned char *buf, int offset, unsigned int size))
{

	int ret;

	ret=WriteSOI(jpeg, putdata);
	if(ret) return JPEGRTPSTATUSERROR;

	ret=WriteDQT(jpeg, putdata);
	if(ret) return JPEGRTPSTATUSERROR;

	ret=WriteSOF0(jpeg, putdata);
	if(ret) return JPEGRTPSTATUSERROR;

	ret=WriteDHT(jpeg, putdata);
	if(ret) return JPEGRTPSTATUSERROR;


	if(jpeg->RSTCount)
	{
		ret=WriteDRI(jpeg, putdata);
		if(ret) return JPEGRTPSTATUSERROR;
	}

	ret=WriteSOS(jpeg, putdata);
	if(ret) return JPEGRTPSTATUSERROR;

	ret=WriteEOI(jpeg, putdata);
	if(ret) return JPEGRTPSTATUSERROR;
	return JPEGRTPSTATUSOK;
}

//�ͷ�jpeg�е��ر����ڴ�
//����JPEGRTPSTATUSOK�ɹ�
enum JPEGRTPSTATUS JpegInfoDestroy(struct JPEG *jpeg)
{
	if(jpeg->ScanData!=NULL)
	{
		free(jpeg->ScanData);
		jpeg->ScanData=NULL;
	}
	return JPEGRTPSTATUSOK;
}

int fdr;
int GetData(unsigned char *buf,int offset, unsigned int size)
{
	if(offset>=0)
	{
		if(lseek(fdr,offset,SEEK_SET)==-1) return -1;
		return read(fdr,buf,size);
	}
	else
	{
	return read(fdr,buf,size);
	}
}

//���ļ���ȡjpeg��Ϣ
//jpeg��Ϣ
//filenameΪ�ļ���
//����JPEGRTPSTATUSOK�ɹ�
enum JPEGRTPSTATUS JpegInfoGetFromFile(struct JPEG *jpeg, const char *filename)
{
	enum JPEGRTPSTATUS ret;

	fdr=open(filename ,O_RDONLY);
	if(fdr<0)
	{
		ret=FILEERROR;
		goto out;
	}
	memset(jpeg, 0, sizeof(struct JPEG));
	ret=JpegInfoGet(jpeg,GetData);
	if(ret!=JPEGRTPSTATUSOK)
	{
		goto out;
	}
out:
	if(fdr>0)
		close(fdr);
	return ret;
}

int fdw;
int PutData(unsigned char *buf,int offset, unsigned int size)
{
	if(offset>=0)
	{
		if(lseek(fdw,offset,SEEK_SET)==-1) return -1;
		return write(fdw,buf,size);
	}
	else
	{
	return write(fdw,buf,size);
	}
}

//���ļ�д��jpeg��Ϣ
//jpeg��Ϣ
//filenameΪ�ļ���
//����JPEGRTPSTATUSOK�ɹ�
enum JPEGRTPSTATUS JpegInfoPutToFile(struct JPEG *jpeg, const char *filename)
{
	enum JPEGRTPSTATUS ret;

	ret=JPEGRTPSTATUSOK;

	fdw=open(filename , O_RDWR|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
	if(fdw<0)
	{
		ret=-1;
		goto out;
	}
	ret=JpegInfoPut(jpeg,PutData);
	if(ret!=JPEGRTPSTATUSOK)
	{
		goto out;
	}
out:
	if(fdw>0)
		close(fdw);
	return ret;
}

//jpeginfo��غ������ӣ���ȡsrc���ļ���Ϣ��ʾ����Ȼ�󱣴浽dst��
int JpegInfoTest(const char *src, const char *dst)
{
	struct JPEG jpeg;
	enum JPEGRTPSTATUS ret;

	ret=JPEGRTPSTATUSOK;
	ret=JpegInfoGetFromFile(&jpeg, src);
	if(ret!=JPEGRTPSTATUSOK)
	{
		printf("JpegInfoGetFromFile error %d\n",ret);
		goto out;
	}
	ret=JpegInfoShow(&jpeg);
	if(ret!=JPEGRTPSTATUSOK)
	{
		printf("JpegInfoShow error %d\n",ret);
	}
	ret=JpegInfoPutToFile(&jpeg, dst);
	if(ret!=JPEGRTPSTATUSOK)
	{
		printf("JpegInfoPutToFile error %d\n",ret);
	}
out:
	JpegInfoDestroy(&jpeg);
	return 0;
}


