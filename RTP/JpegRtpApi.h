/*
 * JpegRtpApi.h
 *
 *  Created on: Sep 1, 2014
 *      Author: jason
 */

#ifndef JPEGRTPAPI_H_
#define JPEGRTPAPI_H_

#include<fcntl.h>
#include<stdlib.h>
#include<unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>

//编码是熵编码信息临时内存，图像较小可以减小
#define  MAXSCANSIZE   (64*1024*1024)
//高速模式，全部使用查表，多占用0x1FFFF内存
//#define  SPEEDFIRST

//内部使用
struct DQTable		
{
	unsigned char count;
	unsigned char table[2][8][8];
};

struct RTPJpegHead
{

	unsigned char TypeSpecific;
	unsigned char FragmentOffset0;
	unsigned char FragmentOffset1;
	unsigned char FragmentOffset2;
	unsigned char type;
	unsigned char Q;
	unsigned char Width;
	unsigned char Height;
};

struct RTPRestartMarkerHead
{
	unsigned short RestartInterval;
	unsigned short RestartCount:14;
	unsigned short L:1;
	unsigned short F:1;
};

struct RTPQuantizationTableHead		
{
	unsigned char MBZ;
	unsigned char Precision;
	unsigned short Length;
};

//外部使用

//状态
enum JPEGRTPSTATUS
{
	JPEGRTPSTATUSOK=0,
	JPEGRTPSTATUSERROR=-1,
	MALLOCERROR=-2,
	ARGERROR=-3,
	FILEERROR=-4,
	IMAGESIZEERROR=-5,
	SCANSIZEERROR=-6,
	PIXFMTERROR=-7
};





//色彩格式
enum PixFmt
{
	YUV444=0,
	YUV422=1,
	YUV420=2,
};

struct JPEG
{
//DQTable
	unsigned char Q;		//量化标准，1~99，越大质量越好，用于编码
	struct DQTable dqtable;	//量化表
//SOF0
	unsigned short width,height;
	enum PixFmt pixfmt;//YUV444 0 YUV422 1 YUV420 2
//DRI
	unsigned short RSTCount;	
//SOS
	unsigned int ScanSize;	//熵编码长度
	unsigned char *ScanData;	//熵编码存放位置，用完释放
};

//获取jpeg信息
//参数jpeg为信息存放位置，如果成功，将为编码数据申请空间，不使用时须调用JpegInfoDestroy释放
//getdata为回调函数，用于从数据源读取数据，须自行实现
//返回JPEGRTPSTATUSOK成功
enum JPEGRTPSTATUS JpegInfoGet(struct JPEG *jpeg, int (*getdata)(unsigned char *buf, int offset, unsigned int size));

//显示jpeg信息
//返回JPEGRTPSTATUSOK成功
enum JPEGRTPSTATUS JpegInfoShow(struct JPEG *jpeg);

//将jpeg信息存储
//参数jpeg为信息存放位置
//putdata为回调函数，用于保存信息，须自行实现
//返回JPEGRTPSTATUSOK成功
enum JPEGRTPSTATUS JpegInfoPut(struct JPEG *jpeg, int (*getdata)(unsigned char *buf, int offset, unsigned int size));

//释放jpeg中的熵编码内存
//返回JPEGRTPSTATUSOK成功
enum JPEGRTPSTATUS JpegInfoDestroy(struct JPEG *jpeg);

//从文件读取jpeg信息
//jpeg信息
//filename为文件名
//返回JPEGRTPSTATUSOK成功
enum JPEGRTPSTATUS JpegInfoGetFromFile(struct JPEG *jpeg, const char *filename);

//向文件写入jpeg信息
//jpeg信息
//filename为文件名
//返回JPEGRTPSTATUSOK成功
enum JPEGRTPSTATUS JpegInfoPutToFile(struct JPEG *jpeg, const char *filename);

//jpeginfo相关函数列子，读取src的文件信息显示出来然后保存到dst中
int JpegInfoTest(const char *src, const char *dst);

//jpeg编码配置
struct JpegEncBuffer
{
	unsigned short width, height;		//宽高，必须为16的倍数
	enum PixFmt pixfmt;							//颜色格式
	unsigned char *Ybuf;						//Y分量内存
	unsigned char *Cbbuf;						//Cb分量内存
	unsigned char *Crbuf;						//Cr分量内存
	unsigned char Q;//(1-99)				//量化系数，越大效果越好，占用空间越大
};
//初始化input
//input为需要初始化的
//Q量化系数 1~99
//width ,height 宽高，必须为16的倍数
//pixfmt 颜色格式
//返回JPEGRTPSTATUSOK成功
enum JPEGRTPSTATUS JpegEncBufferInit(struct JpegEncBuffer *input, unsigned char Q,unsigned short int width,unsigned short int height, enum PixFmt pixfmt);

//同JpegEncBufferInit，但将为YCbCr各分量申请内存，用完使用JpegEncBufferDestroy释放
//input为需要初始化的
//Q量化系数 1~99
//width ,height 宽高，必须为16的倍数
//pixfmt 颜色格式
//返回JPEGRTPSTATUSOK成功
enum JPEGRTPSTATUS JpegEncBufferCreate(struct JpegEncBuffer *input, unsigned char Q,unsigned short int width,unsigned short int height, enum PixFmt pixfmt);

//释放JpegEncBufferCreate申请的内存
//input为需要释放的内存所在结构体
//返回JPEGRTPSTATUSOK成功
enum JPEGRTPSTATUS JpegEncBufferDestroy(struct JpegEncBuffer *input);

//初始化jpec编码器，使用编码器前必须调用
void JpegEncInit(void);

//开始编码
//jpeg为编码结果存放位置，不需要配置
//input为编码所需数据，所有结构都需要配置好
//返回JPEGRTPSTATUSOK成功
enum JPEGRTPSTATUS JpegEncCode(struct JPEG *jpeg, struct JpegEncBuffer *input);

//测试jpeg编码器
//file保存的文件名
int JpegEncTest(const char *file);


//色彩转换
//RGB为24bitRGB数据，从低到高一次为BGR， buf为YCbCr数据存放处，Ybuf、Cbbuf和Crbuf必须有效
void RGB24toYUV444(unsigned char *RGB, struct JpegEncBuffer *buf);
void RGB24toYUV422(unsigned char *RGB, struct JpegEncBuffer *buf);
void RGB24toYUV420(unsigned char *RGB, struct JpegEncBuffer *buf);


struct BMP
{
	unsigned int width;
	unsigned int height;
	unsigned char *RGB;
};
enum JPEGRTPSTATUS BMPInfoGet(struct BMP *bmp,int (*getdata)(unsigned char *buf, int offset, unsigned int size));
enum JPEGRTPSTATUS BMPInfoDestroy(struct BMP *bmp);
enum JPEGRTPSTATUS BMPInfoGetFromFile(struct BMP *bmp, const char *filename);
enum JPEGRTPSTATUS BMPToJpeg(const char*src, const char *dst, enum PixFmt pixfmt, unsigned char Q);


struct RTPHead
{
		unsigned char CC:4;
		unsigned char X:1;
		unsigned char P:1;
		unsigned char V:2;

		unsigned char PT:7;
		unsigned char M:1;

		unsigned short SequenceNumber;
		unsigned int TimeStamp;
		unsigned int ssrc;
		unsigned int csrc[16];
};

struct RTPJpegFrame
{
	struct RTPHead rtphead;
	struct RTPJpegHead rtpjh;
	struct RTPRestartMarkerHead rtprmh;
	struct RTPQuantizationTableHead rtpqth;
	unsigned char qtable[128];
	unsigned char *scan;
	unsigned int scansize;
};


//使用jpeg结构体初始化rtpjf,在发送完之后才能释放jpeg
//jpeg
//rtpjf
//返回JPEGRTPSTATUSOK成功
enum JPEGRTPSTATUS RTPJpegFrameInitFromJpeg(struct JPEG *jpeg,struct RTPJpegFrame *rtpjf);

//显示RTP的jpeg包头信息
//rtpjf包头信息
void RTPJpegFrameShow(struct RTPJpegFrame *rtpjf);

//发送一帧jpeg数据
//rtpjf存放jpeg数据的结构体
//timestampinc时间戳增加值，为90000/帧率
//返回JPEGRTPSTATUSOK成功
enum JPEGRTPSTATUS RTPJpegFrameSend(struct RTPJpegFrame *rtpjf, unsigned int timestampinc);

//初始化RTP头
//rtphead要初始化的头结构
//seq随机
//tim随机
//ssrc随机
void RTPHeadInit(struct RTPHead *rtphead, unsigned short seq, unsigned int tim, unsigned int ssrc);

//初始化RTP连接
//ipaddr ip地址
//port端口
//返回值为sock描述符
int RTPConnetInit(const char *ipaddr, unsigned short port);

//测试RTP
//file为要发送的jpg文件
//times为发送的帧数
//ipaddr为ip地址
//port为端口
int RTPJpegTest(const char *file, int times, const char *ipaddr, unsigned short port);


#endif /* JPEGRTPAPI_H_ */
