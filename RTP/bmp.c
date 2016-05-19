/*
 * bmp.c
 *
 *  Created on: Sep 4, 2014
 *      Author: jason
 */
#include "JpegRtpApi.h"

enum JPEGRTPSTATUS BMPInfoGet(struct BMP *bmp,int (*getdata)(unsigned char *buf, int offset, unsigned int size))
{
	unsigned char buf[0xFFFF];
	unsigned int size;
	unsigned int start;
	unsigned int temp;
	unsigned int offset;
//BM
	if(getdata(buf, -1, 2)!=2)return FILEERROR;
	offset=2;
	if(((buf[0]<<8)|(buf[1]))!=0x424D)
		return FILEERROR;
//size
	if(getdata(buf, -1, 4)!=4)return FILEERROR;
	offset=offset+4;
	size=(buf[3]<<24)|(buf[2]<<16)|(buf[1]<<8)|(buf[0]);
//res
	if(getdata(buf, -1, 4)!=4)return FILEERROR;
	offset=offset+4;
//start
	if(getdata(buf, -1, 4)!=4)return FILEERROR;
	offset=offset+4;
	start=(buf[3]<<24)|(buf[2]<<16)|(buf[1]<<8)|(buf[0]);
//part2
	if(getdata(buf, -1, 4)!=4)return FILEERROR;
	offset=offset+4;
	temp=(buf[3]<<24)|(buf[2]<<16)|(buf[1]<<8)|(buf[0]);
	if(temp!=0x28) return FILEERROR;
//width
	if(getdata(buf, -1, 4)!=4)return FILEERROR;
	offset=offset+4;
	bmp->width=(buf[3]<<24)|(buf[2]<<16)|(buf[1]<<8)|(buf[0]);
//height
	if(getdata(buf, -1, 4)!=4)return FILEERROR;
	offset=offset+4;
	bmp->height=(buf[3]<<24)|(buf[2]<<16)|(buf[1]<<8)|(buf[0]);
//must be 1
	if(getdata(buf, -1, 2)!=2)return FILEERROR;
	offset=offset+2;
	temp=(buf[1]<<8)|(buf[0]);
	if(temp!=1) return FILEERROR;
//colour count  must be 24
	if(getdata(buf, -1, 2)!=2)return FILEERROR;
	offset=offset+2;
	temp=(buf[1]<<8)|(buf[0]);
	if(temp!=24) return FILEERROR;
//RLE
	if(getdata(buf, -1, 4)!=4)return FILEERROR;
	offset=offset+4;
	temp=(buf[3]<<24)|(buf[2]<<16)|(buf[1]<<8)|(buf[0]);
	if(temp!=0) return FILEERROR;
//useless
	if(getdata(buf, -1, 20)!=20)return FILEERROR;
	offset=offset+20;

//useless
	if(getdata(buf, -1, start-offset)!=start-offset)return FILEERROR;
	offset=start;

	size=size-start;

	bmp->RGB=(unsigned char *)malloc(size);
	if(bmp->RGB==NULL) return MALLOCERROR;
	if(getdata(bmp->RGB, -1, size)!=size)return FILEERROR;

	return JPEGRTPSTATUSOK;
}

enum JPEGRTPSTATUS BMPInfoDestroy(struct BMP *bmp)
{
	if(bmp->RGB!=NULL)
	{
		free(bmp->RGB);
		bmp->RGB=NULL;
	}
	return JPEGRTPSTATUSOK;
}

int bmpfdr;
int BMPGetData(unsigned char *buf,int offset, unsigned int size)
{
	if(offset>=0)
	{
		if(lseek(bmpfdr,offset,SEEK_SET)==-1) return -1;
		return read(bmpfdr,buf,size);
	}
	else
	{
	return read(bmpfdr,buf,size);
	}
}

enum JPEGRTPSTATUS BMPInfoGetFromFile(struct BMP *bmp, const char *filename)
{
	enum JPEGRTPSTATUS ret;

	bmpfdr=open(filename ,O_RDONLY);
	if(bmpfdr<0)
	{
		ret=FILEERROR;
		goto out;
	}
	memset(bmp, 0, sizeof(struct BMP));
	ret=BMPInfoGet(bmp,BMPGetData);
	if(ret!=JPEGRTPSTATUSOK)
	{
		goto out;
	}
out:
	if(bmpfdr>0)
		close(bmpfdr);
	return ret;
}

enum JPEGRTPSTATUS BMPToJpeg(const char*src, const char *dst, enum PixFmt pixfmt, unsigned char Q)
{
	struct BMP bmp;
	struct JPEG jpeg;
	struct JpegEncBuffer input;
	enum JPEGRTPSTATUS ret;
	ret=BMPInfoGetFromFile(&bmp,src);
	if(ret!=JPEGRTPSTATUSOK)
	{
	printf("BMPInfoGetFromFile error: %d\n",ret);
	goto out;
	}

	ret=JpegEncBufferCreate(&input,Q,bmp.width,bmp.height,pixfmt);
	if(ret!=JPEGRTPSTATUSOK)
	{
		printf("JpegEncBufferCreate error: %d\n",ret);
		goto out;
	}

	switch(input.pixfmt)
	{
	case YUV444:
		RGB24toYUV444(bmp.RGB, &input);break;
	case YUV422:
		RGB24toYUV422(bmp.RGB, &input);break;
	case YUV420:
		RGB24toYUV420(bmp.RGB, &input);break;
	default:goto out;break;
	}
	ret=JpegEncCode(&jpeg,&input);
	if(ret!=JPEGRTPSTATUSOK)
	{
		printf("JpegEncCodeError: %d\n",ret);
		goto out;
	}

	ret=JpegInfoPutToFile(&jpeg,dst);
	if(ret!=JPEGRTPSTATUSOK)
	{
		printf("JJpegInfoPutToFile: %d\n",ret);
		goto out;
	}

out:
	BMPInfoDestroy(&bmp);
	JpegInfoDestroy(&jpeg);
	JpegEncBufferDestroy(&input);
	return ret;

}
