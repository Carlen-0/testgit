#ifndef CUSTOMPALETTE_H
#define CUSTOMPALETTE_H
#include "../ITADTD.h"

class CustomPalette {
public:
	CustomPalette();

	~CustomPalette();

	ITA_RESULT addCustomPalette(ITAPaletteInfo* param,int PaleId);

	ITA_RESULT deleteCustomPalette(ITAPaletteInfo* param,int PaleId);

	ITA_RESULT updateCustomPalette(ITAPaletteInfo* param, int PaleId);

	int getCustomCurPaleNum()
	{
		return customCurPaleNum;
	}

	unsigned char* getCustomPaleHead()
	{
		return head;
	}

	void printCustomPalette();//����ʹ��

private:
	int customCurPaleNum;
	int customPaletteMaxNum;
	unsigned char* paletteData;
	//��¼α�����ݵ�ͷ��βָ��
	unsigned char* head;
	unsigned char* tail;
};

#endif //!CUSTOMPALETTE_H
