#include "CustomPalette.h"
#include "../Module/PortingAPI.h"
#include <string.h>
#include <stdio.h>


CustomPalette::CustomPalette()
{
	customPaletteMaxNum = 10;
	customCurPaleNum = 0;
	//int paleLen = customPaletteMaxNum * 256 * 4;
	paletteData = (unsigned char*)porting_calloc_mem(customPaletteMaxNum, 256 * 4, ITA_IMAGE_MODULE);
	head = tail = paletteData;
}

CustomPalette::~CustomPalette()
{
	if (paletteData) {
		porting_free_mem(paletteData);
		paletteData = nullptr;
	}
}

ITA_RESULT CustomPalette::addCustomPalette(ITAPaletteInfo* param,int PaleId)
{
	ITAPaletteInfo* paletteInfo = param;
	unsigned char* data = paletteInfo->data;
	int len = paletteInfo->dataLen;
	int siglePaletteLen = 256 * 4;

	int customPaleId = PaleId;
	int newAddPaleNum = len / siglePaletteLen;
	if (customCurPaleNum + newAddPaleNum > customPaletteMaxNum) {
		return ITA_ARG_OUT_OF_RANGE;
	}

	if (customPaleId == customCurPaleNum)
	{
		memcpy(tail, data, len);
		tail += len;
	}
	else {
		int startLen = customPaleId * siglePaletteLen;
		unsigned char* src = head + startLen;
		unsigned char* dst = src + len;
		int moveLen = tail - src;
		memmove(dst, src, moveLen);
		memcpy(src, data, len);
		tail += len;
	}
	customCurPaleNum += newAddPaleNum;
	return ITA_OK;
}
ITA_RESULT CustomPalette::deleteCustomPalette(ITAPaletteInfo* param,int PaleId)
{
	ITAPaletteInfo* paletteInfo = param;
	int len = paletteInfo->dataLen;
	int siglePaletteLen = 256 * 4;
	int customPaleId = PaleId;
	int newDelePaleNum = len / siglePaletteLen;
	if (customCurPaleNum - newDelePaleNum < 0) {
		return ITA_ARG_OUT_OF_RANGE;
	}
	else if (customCurPaleNum - newDelePaleNum == 0) {
		tail -= len;
	}
	else {
		int startLen = customPaleId * siglePaletteLen;
		unsigned char* src = head + startLen;
		unsigned char* dst = src + len;
		int moveLen = tail - dst;
		memmove(src, dst, moveLen);
		tail -= len;
	}
	customCurPaleNum -= newDelePaleNum;
	return ITA_OK;
}
//更新单条伪彩
ITA_RESULT CustomPalette::updateCustomPalette(ITAPaletteInfo* param, int PaleId)
{
	ITAPaletteInfo* paletteInfo = param;
	unsigned char* data = paletteInfo->data;
	int len = paletteInfo->dataLen;
	int siglePaletteLen = 256 * 4;
	int customPaleId = PaleId;
	int startLen = customPaleId * siglePaletteLen;
	unsigned char* src = head + startLen;
	memcpy(src, data, len);
	return ITA_OK;
}

void CustomPalette::printCustomPalette()
{
	for (int i = 0; i < customCurPaleNum; i++) {
		printf("current paleNum:%d\n", i);
		printf("{");
		for (int j = 0; j < 256; j++) {
			printf("{");
			for (int k = 0; k < 4; k++) {
				printf("%d,", head[i * 256 * 4 + j * 4 + k]);
			}
			printf("}");
		}
		printf("}\n");
	}
}