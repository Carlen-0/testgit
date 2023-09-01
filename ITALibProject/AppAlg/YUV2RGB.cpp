#include <stdlib.h>

#define CLIPVALUE(x, minValue, maxValue) ((x) < (minValue) ? (minValue) : ((x) > (maxValue) ? (maxValue) : (x)))

#define YUVToR(Y, U, V) ( (Y) + 1.4075 * ((V) - 128) )
#define YUVToG(Y, U, V) ( (Y) - 0.3455 * ((U) - 128) - 0.7169 * ((V) - 128) )
#define YUVToB(Y, U, V) ( (Y) + 1.779 * ((U) - 128) )

/*
rgbImageData:rgb数据区，yuvImageData:yuv数据区,width:图像宽度,height:图像高
*/
int YUYVToRGB(unsigned char *rgbImageData, unsigned char *yuvImageData, short width, short height)
{
	if (yuvImageData == NULL)
	{
		return -1;
	}

	if (rgbImageData == NULL)
	{
		return -1;
	}

	int y, u, v;

	for (int j = 0; j < height; j++)
	{
		unsigned char* pRGB = rgbImageData + j * width * 3;
		unsigned char* pYUV = yuvImageData + j * width * 2;
		for (int i = 0; i < width; i++)
		{
			y = *(pYUV + 2 * i + 1);
			u = *(pYUV + 4 * (i >> 1));
			v = *(pYUV + 4 * (i >> 1) + 2);

			*(pRGB) = (unsigned char)CLIPVALUE(YUVToR(y, u, v), 0, 255);
			*(pRGB + 1) = (unsigned char)CLIPVALUE(YUVToG(y, u, v), 0, 255);
			*(pRGB + 2) = (unsigned char)CLIPVALUE(YUVToB(y, u, v), 0, 255);
			pRGB += 3;
		}
	}

	return 0;
}

/*
rgbImageData:rgb数据区，yuvImageData:yuv数据区,width:图像宽度,height:图像高
*/
int YUYVToRGB565(unsigned short *rgbImageData, unsigned char *yuvImageData, short width, short height)
{
	if (yuvImageData == NULL)
	{
		return -1;
	}

	if (rgbImageData == NULL)
	{
		return -1;
	}

	int y, u, v;
	int r, g, b;

	for (int j = 0; j < height; j++)
	{
		unsigned short* pRGB = rgbImageData + j * width;
		unsigned char* pYUV = yuvImageData + j * width * 2;
		for (int i = 0; i < width; i++)
		{
			y = *(pYUV + 2 * i + 1);
			u = *(pYUV + 4 * (i >> 1));
			v = *(pYUV + 4 * (i >> 1) + 2);

			r = (int)CLIPVALUE(YUVToR(y, u, v), 0, 255);
			g = (int)CLIPVALUE(YUVToG(y, u, v), 0, 255);
			b = (int)CLIPVALUE(YUVToB(y, u, v), 0, 255);

			*(pRGB) = (((b >> 3) & 0x1F))
				| (((g >> 2) & 0x3F) << 5)
				| (((r >> 3) & 0x1F) << 11);
			pRGB++;
		}
	}

	return 0;
}