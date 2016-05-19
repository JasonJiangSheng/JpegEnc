/*
 * RGBYUV.c
 *
 *  Created on: Sep 1, 2014
 *      Author: jason
 */

#include "JpegRtpApi.h"

unsigned int  clip(unsigned int data)
{
	if(data<=16) return 16;
	if(data>=235) return 235;
	return data;
}

void RGB24toYUV444(unsigned char *RGB, struct JpegEncBuffer *buf)
{
	int x,y;
	int index=0;
	for(y=0;y<buf->height;y++)
	{
		for(x=0;x<buf->width;x++)
		{
			double R,G,B;
			R=(double)RGB[index*3+2];
			G=(double)RGB[index*3+1];
			B=(double)RGB[index*3+0];

			buf->Ybuf[index]=(unsigned char)clip((unsigned int )(0.257*R+0.504*G+0.098*B+16.0));
			buf->Cbbuf[index]=(unsigned char)clip((unsigned int )(-0.148*R-0.291*G+0.439*B+128.0));
			buf->Crbuf[index]=(unsigned char)clip((unsigned int )(0.439*R-0.368*G-0.071*B+128.0));
			index++;
		}
	}
}

void RGB24toYUV422(unsigned char *RGB, struct JpegEncBuffer *buf)
{
	int x,y;
	int index=0;
	unsigned int U2 ,V2;

	index=0;

	U2=0;V2=0;
	for(y=0;y<buf->height;y++)
	{
		for(x=0;x<buf->width;x++)
		{
			double R,G,B;

			R=(double)RGB[index*3+2];
			G=(double)RGB[index*3+1];
			B=(double)RGB[index*3+0];

			buf->Ybuf[index]=(unsigned char)clip((unsigned int )(0.257*R+0.504*G+0.098*B+16.0));
			U2=clip((unsigned int )(-0.148*R-0.291*G+0.439*B+128.0))+U2;
			V2=clip((unsigned int )(0.439*R-0.368*G-0.071*B+128.0))+V2;
			if(index%2)
			{
				buf->Cbbuf[index>>1]=(unsigned char)(clip(U2/2));
				buf->Crbuf[index>>1]=(unsigned char)(clip(V2/2));
				U2=0;V2=0;
			}
			index++;
		}
	}
}

void RGB24toYUV420(unsigned char *RGB, struct JpegEncBuffer *buf)
{
	int x,y;
	int width,height;
	unsigned int U2 ,V2;

	U2=0;V2=0;
	height=buf->height/2;
	width=buf->width/2;
	for(y=0;y<height;y++)
	{
		for(x=0;x<width;x++)
		{
			double R,G,B;

			R=(double)RGB[((y*2)*(width*2)+x*2)*3+2];
			G=(double)RGB[((y*2)*(width*2)+x*2)*3+1];
			B=(double)RGB[((y*2)*(width*2)+x*2)*3+0];
			buf->Ybuf[(y*2)*(width*2)+x*2]=(unsigned char)clip((unsigned int )(0.257*R+0.504*G+0.098*B+16.0));
			U2=clip((unsigned int )(-0.148*R-0.291*G+0.439*B+128.0))+U2;
			V2=clip((unsigned int )(0.439*R-0.368*G-0.071*B+128.0))+V2;

			R=(double)RGB[((y*2)*(width*2)+x*2+1)*3+2];
			G=(double)RGB[((y*2)*(width*2)+x*2+1)*3+1];
			B=(double)RGB[((y*2)*(width*2)+x*2+1)*3+0];

			buf->Ybuf[(y*2)*(width*2)+x*2+1]=(unsigned char)clip((unsigned int )(0.257*R+0.504*G+0.098*B+16.0));
			U2=clip((unsigned int )(-0.148*R-0.291*G+0.439*B+128.0))+U2;
			V2=clip((unsigned int )(0.439*R-0.368*G-0.071*B+128.0))+V2;

			R=(double)RGB[((y*2+1)*(width*2)+x*2)*3+2];
			G=(double)RGB[((y*2+1)*(width*2)+x*2)*3+1];
			B=(double)RGB[((y*2+1)*(width*2)+x*2)*3+0];

			buf->Ybuf[(y*2+1)*(width*2)+x*2]=(unsigned char)clip((unsigned int )(0.257*R+0.504*G+0.098*B+16.0));
			U2=clip((unsigned int )(-0.148*R-0.291*G+0.439*B+128.0))+U2;
			V2=clip((unsigned int )(0.439*R-0.368*G-0.071*B+128.0))+V2;

			R=(double)RGB[((y*2+1)*(width*2)+x*2+1)*3+2];
			G=(double)RGB[((y*2+1)*(width*2)+x*2+1)*3+1];
			B=(double)RGB[((y*2+1)*(width*2)+x*2+1)*3+0];

			buf->Ybuf[(y*2+1)*(width*2)+x*2+1]=(unsigned char)clip((unsigned int )(0.257*R+0.504*G+0.098*B+16.0));
			U2=clip((unsigned int )(-0.148*R-0.291*G+0.439*B+128.0))+U2;
			V2=clip((unsigned int )(0.439*R-0.368*G-0.071*B+128.0))+V2;

			buf->Cbbuf[y*width+x]=U2/4;
			buf->Crbuf[y*width+x]=V2/4;

			U2=0;V2=0;

		}
	}
}

