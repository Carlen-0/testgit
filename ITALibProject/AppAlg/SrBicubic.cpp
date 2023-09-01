
#include"SrBicubic.h"
#include <cmath>
#include <stdlib.h>

void SR_BICUBIC::bicubic(unsigned char* dst, unsigned char* src, int n_width, int n_height, int n_width_zoom, int n_height_zoom)
{
	int i, j, s, t;
	float a = -0.5;//BiCubic基函数
	float x, y, w_x[4], w_y[4];
	int temp;
	int nzoom = int(n_height_zoom / n_height);
	int n_len_zoom = n_width_zoom * n_height_zoom;
	if (!y8matrix) {
		y8matrix = (unsigned char*)porting_calloc_mem(n_len_zoom, sizeof(unsigned char), ITA_IMAGE_MODULE);
	}
	memset(y8matrix, 0, n_len_zoom * sizeof(unsigned char));
	int nStartIndex = nzoom;
	int nEndIndex = 2 * nzoom;

	for (i = nStartIndex; i < n_height_zoom - nEndIndex; i++)
	{
		for (j = nStartIndex; j < n_width_zoom - nEndIndex; j++)
		{
			x = i * 1.0 / nzoom;//放大后的图像的像素位置相对于源图像的位置
			y = j * 1.0 / nzoom;

			getW_x(w_x, x, a);
			getW_y(w_y, y, a);

			temp = 0;
			for (s = 0; s <= 3; s++)
			{
				for (t = 0; t <= 3; t++)
				{
					temp = (int)(temp + src[(int(x) + s - 1) * n_width + int(y) + t - 1] * w_x[s] * w_y[t]);
				}
			}
			y8matrix[i * int(n_width_zoom) + j] = temp;
		}
	}
	//对边缘进行镜像
	for (i = nStartIndex; i < n_height_zoom - nEndIndex; i++)
	{
		for (j = 0; j < nStartIndex; j++)
		{
			y8matrix[i * n_width_zoom + j] = y8matrix[i * n_width_zoom + nStartIndex];
		}
		for (j = n_width_zoom - nEndIndex; j < n_width_zoom; j++)
		{
			y8matrix[i * n_width_zoom + j] = y8matrix[i * n_width_zoom + n_width_zoom - nEndIndex - 1];
		}
	}
	for (i = 0; i < nStartIndex; i++)
	{
		memcpy(y8matrix + i * n_width_zoom, y8matrix + nStartIndex * n_width_zoom, n_width_zoom * sizeof(unsigned char));
	}
	for (i = n_height_zoom - nEndIndex; i < n_height_zoom; i++)
	{
		memcpy(y8matrix + i * n_width_zoom, y8matrix + (n_height_zoom - nEndIndex - 1) * n_width_zoom, n_width_zoom * sizeof(unsigned char));
	}

	//float weight = 2;
	//LaplaceSharpen_y16(pus_dst, tempmatrix, n_width_zoom, n_height_zoom, weight);
	memcpy(dst, y8matrix, n_width_zoom * n_height_zoom * sizeof(unsigned char));
}

void SR_BICUBIC::bicubic(short* dst, short* src, int n_width, int n_height, int n_width_zoom, int n_height_zoom)
{
	int i, j, s, t;
	float a = -0.5;//BiCubic基函数
	float x, y, w_x[4], w_y[4];
	int temp;
	int nzoom = int(n_height_zoom / n_height);
	int n_len_zoom = n_width_zoom * n_height_zoom;
	if (!y16matrix) {
		y16matrix = (short*)porting_calloc_mem(n_len_zoom, sizeof(short), ITA_IMAGE_MODULE);
	}
	memset(y16matrix, 0, n_len_zoom * sizeof(short));
	int nStartIndex = nzoom;
	int nEndIndex = 2 * nzoom;

	for (i = nStartIndex; i < n_height_zoom - nEndIndex; i++)
	{
		for (j = nStartIndex; j < n_width_zoom - nEndIndex; j++)
		{
			x = i * 1.0 / nzoom;//放大后的图像的像素位置相对于源图像的位置
			y = j * 1.0 / nzoom;

			getW_x(w_x, x, a);
			getW_y(w_y, y, a);

			temp = 0;
			for (s = 0; s <= 3; s++)
			{
				for (t = 0; t <= 3; t++)
				{
					temp = (int)(temp + src[(int(x) + s - 1) * n_width + int(y) + t - 1] * w_x[s] * w_y[t]);
				}
			}
			y16matrix[i * int(n_width_zoom) + j] = temp;
		}
	}
	//对边缘进行镜像
	for (i = nStartIndex; i < n_height_zoom - nEndIndex; i++)
	{
		for (j = 0; j < nStartIndex; j++)
		{
			y16matrix[i * n_width_zoom + j] = y16matrix[i * n_width_zoom + nStartIndex];
		}
		for (j = n_width_zoom - nEndIndex; j < n_width_zoom; j++)
		{
			y16matrix[i * n_width_zoom + j] = y16matrix[i * n_width_zoom + n_width_zoom - nEndIndex - 1];
		}
	}
	for (i = 0; i < nStartIndex; i++)
	{
		memcpy(y16matrix + i * n_width_zoom, y16matrix + nStartIndex * n_width_zoom, n_width_zoom * sizeof(short));
	}
	for (i = n_height_zoom - nEndIndex; i < n_height_zoom; i++)
	{
		memcpy(y16matrix + i * n_width_zoom, y16matrix + (n_height_zoom - nEndIndex - 1) * n_width_zoom, n_width_zoom * sizeof(short));
	}

	//float weight = 2;
	//LaplaceSharpen_y16(pus_dst, tempmatrix, n_width_zoom, n_height_zoom, weight);
	memcpy(dst, y16matrix, n_width_zoom * n_height_zoom * sizeof(short));
}

/*计算系数*/
void SR_BICUBIC::getW_x(float w_x[4], float x, float a)
{
	int X = (int)x;//取整数部分
	float stemp_x[4];
	stemp_x[0] = 1 + (x - X);
	stemp_x[1] = x - X;
	stemp_x[2] = 1 - (x - X);
	stemp_x[3] = 2 - (x - X);

	w_x[0] = a*abs(stemp_x[0] * stemp_x[0] * stemp_x[0]) - 5 * a*stemp_x[0] * stemp_x[0] + 8 * a*abs(stemp_x[0]) - 4 * a;
	w_x[1] = (a + 2)*abs(stemp_x[1] * stemp_x[1] * stemp_x[1]) - (a + 3)*stemp_x[1] * stemp_x[1] + 1;
	w_x[2] = (a + 2)*abs(stemp_x[2] * stemp_x[2] * stemp_x[2]) - (a + 3)*stemp_x[2] * stemp_x[2] + 1;
	w_x[3] = a*abs(stemp_x[3] * stemp_x[3] * stemp_x[3]) - 5 * a*stemp_x[3] * stemp_x[3] + 8 * a*abs(stemp_x[3]) - 4 * a;
}

void SR_BICUBIC::getW_y(float w_y[4], float y, float a)
{
	int Y = (int)y;
	float stemp_y[4];
	stemp_y[0] = 1.0 + (y - Y);
	stemp_y[1] = y - Y;
	stemp_y[2] = 1 - (y - Y);
	stemp_y[3] = 2 - (y - Y);

	w_y[0] = a*abs(stemp_y[0] * stemp_y[0] * stemp_y[0]) - 5 * a*stemp_y[0] * stemp_y[0] + 8 * a*abs(stemp_y[0]) - 4 * a;
	w_y[1] = (a + 2)*abs(stemp_y[1] * stemp_y[1] * stemp_y[1]) - (a + 3)*stemp_y[1] * stemp_y[1] + 1;
	w_y[2] = (a + 2)*abs(stemp_y[2] * stemp_y[2] * stemp_y[2]) - (a + 3)*stemp_y[2] * stemp_y[2] + 1;
	w_y[3] = a*abs(stemp_y[3] * stemp_y[3] * stemp_y[3]) - 5 * a*stemp_y[3] * stemp_y[3] + 8 * a*abs(stemp_y[3]) - 4 * a;
}

