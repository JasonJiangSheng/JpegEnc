/*
 * RTP.c
 *
 *  Created on: Aug 26, 2014
 *      Author: jason
 */
#include "JpegRtpApi.h"



#define QVALUE 254
#define PACKSIZE 1500

static unsigned char Packet[PACKSIZE];
static int rtpsock;
static struct sockaddr_in rtpaddr;

static void RTPHeadMSet(struct RTPHead *rtphead);
static void RTPHeadMClear(struct RTPHead *rtphead);
static void RTPHeadMClear(struct RTPHead *rtphead);
static void RTPSequenceNumberInc(struct RTPHead *rtphead);
static void RTPTimeStampInc(struct RTPHead *rtphead, unsigned int inc);
static unsigned int FragmentOffsetGet(struct RTPJpegHead *rtpjh);
static void FragmentOffsetSet(struct RTPJpegHead *rtpjh,unsigned int data);
static void FragmentOffsetAdd(struct RTPJpegHead *rtpjh,unsigned int inc);

static void RTPHeadMSet(struct RTPHead *rtphead)
{
	rtphead->M=1;
}
static void RTPHeadMClear(struct RTPHead *rtphead)
{
	rtphead->M=0;
}

static void RTPSequenceNumberInc(struct RTPHead *rtphead)
{
    unsigned short sequence;
    sequence = ntohs(rtphead->SequenceNumber);
    sequence++;
    rtphead->SequenceNumber = htons(sequence);
}

static void RTPTimeStampInc(struct RTPHead *rtphead, unsigned int inc)
{
    unsigned int time;
    time = ntohl(rtphead->TimeStamp);
    time=time+inc;
    rtphead->TimeStamp = htonl(time);
}

static unsigned int FragmentOffsetGet(struct RTPJpegHead *rtpjh)
{
	return (rtpjh->FragmentOffset0<<16)|(rtpjh->FragmentOffset1<<8)|(rtpjh->FragmentOffset2);
}

static void FragmentOffsetSet(struct RTPJpegHead *rtpjh,unsigned int data)
{
	rtpjh->FragmentOffset0=(data>>16)&0xFF;
	rtpjh->FragmentOffset1=(data>>8)&0xFF;
	rtpjh->FragmentOffset2=(data)&0xFF;

}

static void FragmentOffsetAdd(struct RTPJpegHead *rtpjh,unsigned int inc)
{
	unsigned int data;
	data=FragmentOffsetGet(rtpjh);
	data=data+inc;
	FragmentOffsetSet(rtpjh,data);
}

//使用jpeg结构体初始化rtpjf,在发送完之后才能释放jpeg
//jpeg
//rtpjf
//返回JPEGRTPSTATUSOK成功
enum JPEGRTPSTATUS RTPJpegFrameInitFromJpeg(struct JPEG *jpeg,struct RTPJpegFrame *rtpjf)
{
	unsigned int i,x,y,z;
	if(jpeg->ScanSize>0xFFFFFF)
		return SCANSIZEERROR;
	rtpjf->scan=jpeg->ScanData;
	rtpjf->scansize=jpeg->ScanSize;

	if(jpeg->RSTCount)
	{
		rtpjf->rtprmh.RestartInterval=htons(jpeg->RSTCount);
		rtpjf->rtprmh.F=1;
		rtpjf->rtprmh.L=1;
		rtpjf->rtprmh.RestartCount=0x3FFF;
		if(jpeg->pixfmt==YUV422)
			rtpjf->rtpjh.type=64;
		else if(jpeg->pixfmt==YUV420)  //U
			rtpjf->rtpjh.type=65;
		else
			return PIXFMTERROR;
	}
	else
	{
		if(jpeg->pixfmt==YUV422)
			rtpjf->rtpjh.type=0;
		else if(jpeg->pixfmt==YUV420)  //U
			rtpjf->rtpjh.type=1;
		else
			return PIXFMTERROR;
	}
	rtpjf->rtpjh.TypeSpecific=0;
	FragmentOffsetSet(&rtpjf->rtpjh,0);
	if(jpeg->Q==0)
		rtpjf->rtpjh.Q=255;
	else
		rtpjf->rtpjh.Q=jpeg->Q;


	if((jpeg->width>2040)||(jpeg->height>2040)) return IMAGESIZEERROR;
	rtpjf->rtpjh.Width=jpeg->width/8;
	rtpjf->rtpjh.Height=jpeg->height/8;
	if(jpeg->dqtable.count>2)		return JPEGRTPSTATUSERROR;
	if(rtpjf->rtpjh.Q>127)
	{
		rtpjf->rtpqth.MBZ=0;
		rtpjf->rtpqth.Precision=0;
		rtpjf->rtpqth.Length=htons(128);
		i=0;
		for(z=0;z<jpeg->dqtable.count;z++)
		{
			for(y=0;y<8;y++)
			{
				for(x=0;x<8;x++)
				{
				rtpjf->qtable[i]=(unsigned char)jpeg->dqtable.table[z][y][x];
				if(jpeg->dqtable.count==1)
				{
					rtpjf->qtable[i+64]=rtpjf->qtable[i];
				}
				i++;
				}
			}
		}
	}
	return JPEGRTPSTATUSOK;
}

//显示RTP的jpeg包头信息
//rtpjf包头信息
void RTPJpegFrameShow(struct RTPJpegFrame *rtpjf)
{
	unsigned int i,count;

	printf("\nRTPJpegFrameShow\n");
	printf("\nRTPHead=========================================\n");

	printf("V: %u\n",rtpjf->rtphead.V);
	printf("P: %u\n",rtpjf->rtphead.P);
	printf("X: %u\n",rtpjf->rtphead.X);
	printf("CC: %u\n",rtpjf->rtphead.CC);
	printf("PT: %u\n",rtpjf->rtphead.PT);
	printf("M: %u\n",rtpjf->rtphead.M);
	printf("SequenceNumber: %u\n",ntohs(rtpjf->rtphead.SequenceNumber));
	printf("TimeStamp: %u\n",ntohl(rtpjf->rtphead.TimeStamp));
	printf("ssrc: %u\n",ntohl(rtpjf->rtphead.ssrc));

	printf("\nJpegHead========================================\n");
	printf("TypeSpecific: %u\n",rtpjf->rtpjh.TypeSpecific);
	printf("FragmentOffset: %u\n",	FragmentOffsetGet(&rtpjf->rtpjh));
	printf("type: %u\n",rtpjf->rtpjh.type);
	printf("Q: %u\n",rtpjf->rtpjh.Q);
	printf("Width: %u\n",rtpjf->rtpjh.Width*8);
	printf("Height: %u\n",rtpjf->rtpjh.Height*8);

	printf("\nRTPRestartMarkerHead============================\n");
	printf("RestartInterval: %u\n",ntohs(rtpjf->rtprmh.RestartInterval));
	printf("F: %u\n",rtpjf->rtprmh.F);
	printf("L: %u\n",rtpjf->rtprmh.L);
	printf("RestartCount: %u\n",rtpjf->rtprmh.RestartCount);

	printf("\nRTPQuantizationTableHead========================\n");
	printf("MBZ: %u\n",rtpjf->rtpqth.MBZ);
	printf("Precision: %u\n",rtpjf->rtpqth.Precision);
	printf("Length: %u\n",ntohs(rtpjf->rtpqth.Length));

	printf("\nDQTable\n");

	count=ntohs(rtpjf->rtpqth.Length);
	for(i=0;i<count;i++)
	{
		if(i%8==0) printf("\n");
		printf("%u\t",rtpjf->qtable[i]);

	}
	printf("\n");
	printf("ScanSize: %u\n",rtpjf->scansize);
	printf("ScanData: %p\n",rtpjf->scan);
	printf("\n");

}

//发送一帧jpeg数据
//rtpjf存放jpeg数据的结构体
//timestampinc时间戳增加值，为90000/帧率
//返回JPEGRTPSTATUSOK成功
enum JPEGRTPSTATUS RTPJpegFrameSend(struct RTPJpegFrame *rtpjf, unsigned int timestampinc)
{
	unsigned char *packp;
	unsigned int packrestsize;

	unsigned char *scanp;
	unsigned int scanrestsize;

	unsigned short qtlength;
	unsigned int packcount;

	packcount=0;
	qtlength=ntohs(rtpjf->rtpqth.Length);
	scanp=rtpjf->scan;
	scanrestsize=rtpjf->scansize;
	FragmentOffsetSet(&rtpjf->rtpjh,0);

	while(1)
	{
		packp=Packet;
		packrestsize=PACKSIZE;

		memcpy(packp,&rtpjf->rtphead,sizeof(struct RTPHead)-64+(rtpjf->rtphead.CC<<2));
		packp=packp+sizeof(struct RTPHead)-64+(rtpjf->rtphead.CC<<2);
		packrestsize=packrestsize-(sizeof(struct RTPHead)-64+(rtpjf->rtphead.CC<<2));

		memcpy(packp,&rtpjf->rtpjh,sizeof(struct RTPJpegHead));
		packp=packp+sizeof(struct RTPJpegHead);
		packrestsize=packrestsize-sizeof(struct RTPJpegHead);

		if((rtpjf->rtpjh.type>=64)&&(rtpjf->rtpjh.type<=127))
		{
			memcpy(packp,&rtpjf->rtprmh,sizeof(struct RTPRestartMarkerHead));
			packp=packp+sizeof(struct RTPRestartMarkerHead);
			packrestsize=packrestsize-sizeof(struct RTPRestartMarkerHead);
		}

		if((qtlength>0)&&(rtpjf->rtpjh.Q>=128)&&(packcount==0))
		{
			memcpy(packp,&rtpjf->rtpqth,sizeof(struct RTPQuantizationTableHead));
			packp=packp+sizeof(struct RTPQuantizationTableHead);
			packrestsize=packrestsize-sizeof(struct RTPQuantizationTableHead);

			memcpy(packp,rtpjf->qtable,qtlength);
			packp=packp+qtlength;
			packrestsize=packrestsize-qtlength;
		}

		if(scanrestsize<=packrestsize)
		{
			RTPHeadMSet(&rtpjf->rtphead);
			memcpy(Packet,&rtpjf->rtphead,sizeof(struct RTPHead)-64+(rtpjf->rtphead.CC<<2));
			RTPHeadMClear(&rtpjf->rtphead);

			memcpy(packp,scanp,scanrestsize);
			sendto(rtpsock, Packet, scanrestsize+(PACKSIZE-packrestsize), 0, (const struct sockaddr*)&rtpaddr, sizeof(struct sockaddr_in));
			RTPSequenceNumberInc(&rtpjf->rtphead);
			break;
		}
		else
		{
			memcpy(packp,scanp,packrestsize);
			sendto(rtpsock, Packet, PACKSIZE, 0, (const struct sockaddr*)&rtpaddr, sizeof(struct sockaddr_in));
			scanp=scanp+packrestsize;
			scanrestsize=scanrestsize-packrestsize;

			RTPSequenceNumberInc(&rtpjf->rtphead);
			FragmentOffsetAdd(&rtpjf->rtpjh,packrestsize);
			packcount++;
		}
	}
	RTPTimeStampInc(&rtpjf->rtphead, timestampinc);
	return JPEGRTPSTATUSOK;
}

//初始化RTP连接
//ipaddr ip地址
//port端口
//返回值为sock描述符
int RTPConnetInit(const char *ipaddr, unsigned short port)
{
	int sock;
	sock=socket(AF_INET, SOCK_DGRAM, 0);
	rtpaddr.sin_family=AF_INET;
	rtpaddr.sin_port = htons(port);
	rtpaddr.sin_addr.s_addr =inet_addr(ipaddr);
	bzero(&(rtpaddr.sin_zero),8);
	rtpsock=sock;
	return sock;
}

//初始化RTP头
//rtphead要初始化的头结构
//seq随机
//tim随机
//ssrc随机
void RTPHeadInit(struct RTPHead *rtphead, unsigned short seq, unsigned int tim, unsigned int ssrc)
{
	rtphead->V = 2;
	rtphead->P = 0;
	rtphead->X = 0;
	rtphead->CC = 0;
	rtphead->M = 0;
	rtphead->PT = 26;
	rtphead->SequenceNumber = htons(seq);
	rtphead->TimeStamp = htonl(tim);
	rtphead->ssrc = htonl(ssrc);
}

//测试RTP
//file为要发送的jpg文件
//times为发送的帧数
//ipaddr为ip地址
//port为端口
int RTPJpegTest(const char *file, int times , const char *ipaddr, unsigned short port)
{
	enum JPEGRTPSTATUS  ret;
	struct JPEG jpeg;
	struct RTPJpegFrame jpegframe;
	int sock;
	int i;

	sock=RTPConnetInit(ipaddr,port);
	if(sock<0)
	{
		printf("RTPConnetInit error!\n");
		return -1;
	}

	memset(&jpegframe, 0, sizeof(struct RTPJpegFrame));
	RTPHeadInit(&jpegframe.rtphead, 0, 0, 0);

	for(i=0;i<times;i++)
	{
		ret=JpegInfoGetFromFile(&jpeg, file);
		if(ret!=JPEGRTPSTATUSOK)
		{
			printf("GetJpegInfoFromFile error %d\n",ret);
			return -1;
		}
		JpegInfoShow(&jpeg);
		ret=RTPJpegFrameInitFromJpeg(&jpeg, &jpegframe);
		if(ret!=JPEGRTPSTATUSOK)
		{
			printf("RTPJpegFrameInit error %d\n",ret);
			return -1;
		}

		RTPJpegFrameSend(&jpegframe, 90000/30);
		JpegInfoDestroy(&jpeg);
	}
	return 0;
}

