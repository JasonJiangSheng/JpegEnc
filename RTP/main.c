/*
 * main.c
 *
 *  Created on: Aug 26, 2014
 *      Author: jason
 */

#include <stdio.h>
#include "JpegRtpApi.h"

#define PIXFMT (YUV420)
#define Q     (94)
#define FRAMES (100)
#define FPS (5)

int main(int argc, char **argv)
{
	BMPToJpeg(argv[1], argv[2], atoi(argv[3]), atoi(argv[4]));
}




int main1(int argc, char **argv)
{

	int i;

	enum JPEGRTPSTATUS  ret;
	struct JPEG jpeg;
	struct JpegEncBuffer input;
	struct RTPJpegFrame jpegframe;
	struct BMP bmp;

	int sock;

	char file[1024];

	sock=RTPConnetInit("192.168.100.100",6666);
	if(sock<0)
	{
		printf("RTPConnetInit error!\n");
		return -1;
	}
	memset(&jpegframe, 0, sizeof(struct RTPJpegFrame));
	RTPHeadInit(&jpegframe.rtphead, 0, 0, 0);

	JpegEncInit();
	for(i=1;i<FRAMES;i++)
	{
		sprintf(file,"/root/capture/1 (%d).bmp",i);
		printf("%s\n",file);

		ret=BMPInfoGetFromFile(&bmp, file);
		if(ret!=JPEGRTPSTATUSOK)
		{
			printf("BMPInfoGetFromFile Error: %d\n",ret);
			goto out;
		}

		ret=JpegEncBufferCreate(&input,Q,bmp.width,bmp.height,PIXFMT);
		if(ret!=JPEGRTPSTATUSOK)
		{
			printf("JpegEncBufferCreate Error: %d\n",ret);
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
		ret=RTPJpegFrameInitFromJpeg(&jpeg, &jpegframe);
		if(ret!=JPEGRTPSTATUSOK)
		{
			printf("RTPJpegFrameInit error %d\n",ret);
			return -1;
		}

		RTPJpegFrameSend(&jpegframe, 90000/FPS);
		JpegInfoDestroy(&jpeg);
		JpegEncBufferDestroy(&input);
		BMPInfoDestroy(&bmp);
	}

out:
	JpegInfoDestroy(&jpeg);
	JpegEncBufferDestroy(&input);
	return 0;
}






