
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
	/*����˵�������ַŴ�,����Y8ͼ�����ݡ�
	�������ƣ�bicubic
	���������unsigned char *pus_src					ԭʼ����Y8
			  int n_width, int n_height				ԭʼ���
			  int n_width_zoom,int n_height_zoom	���ֺ���
	���������unsigned char *pus_dst					���ֺ������Y8
	����ֵ����
	*/
	void bicubic(unsigned char* dst, unsigned char* src, int n_width, int n_height, int n_width_zoom, int n_height_zoom);

	/*����˵�������ַŴ�,����Y16ͼ�����ݡ�
	�������ƣ�bicubic
	���������short *pus_src							ԭʼ����Y16
			  int n_width, int n_height				ԭʼ���
			  int n_width_zoom,int n_height_zoom	���ֺ���
	���������short *pus_dst							���ֺ������Y16
	����ֵ����
	*/
	void bicubic(short* dst, short* src, int n_width, int n_height, int n_width_zoom, int n_height_zoom);

private:
	/*����ϵ��*/
	void getW_x(float w_x[4], float x, float a);

	void getW_y(float w_y[4], float y, float a);

	unsigned char* y8matrix;
	short* y16matrix;
};

#endif // !SR_BICUBIC_H

