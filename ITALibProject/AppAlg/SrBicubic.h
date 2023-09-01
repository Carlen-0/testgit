
#ifndef SR_BICUBIC_H
#define SR_BICUBIC_H

#include<string.h>
#include "../Module/PortingAPI.h"
#include "../ITADTD.h"

class SR_BICUBIC {
public:
	SR_BICUBIC(){
		y8matrix = nullptr;
		y16matrix = nullptr;
	}
	~SR_BICUBIC()
	{
		if (y8matrix) {
			porting_free_mem(y8matrix);
			y8matrix = nullptr;
		}
		if (y16matrix) {
			porting_free_mem(y16matrix);
			y16matrix = nullptr;
		}
	}
	/*函数说明：超分放大,处理Y8图像数据。
	函数名称：bicubic
	输入参数：unsigned char *pus_src					原始数据Y8
			  int n_width, int n_height				原始宽高
			  int n_width_zoom,int n_height_zoom	超分后宽高
	输出参数：unsigned char *pus_dst					超分后的数据Y8
	返回值：无
	*/
	void bicubic(unsigned char* dst, unsigned char* src, int n_width, int n_height, int n_width_zoom, int n_height_zoom);

	/*函数说明：超分放大,处理Y16图像数据。
	函数名称：bicubic
	输入参数：short *pus_src							原始数据Y16
			  int n_width, int n_height				原始宽高
			  int n_width_zoom,int n_height_zoom	超分后宽高
	输出参数：short *pus_dst							超分后的数据Y16
	返回值：无
	*/
	void bicubic(short* dst, short* src, int n_width, int n_height, int n_width_zoom, int n_height_zoom);

private:
	/*计算系数*/
	void getW_x(float w_x[4], float x, float a);

	void getW_y(float w_y[4], float y, float a);

	unsigned char* y8matrix;
	short* y16matrix;
};

#endif // !SR_BICUBIC_H

