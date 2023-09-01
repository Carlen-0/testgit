/*************************************************************
Copyright (C), 2022--2023, Wuhan Guide Sensmart Tech Co., Ltd
File name   : CIrFireDrc.h
Author      : Guide Sensmart RD. wangyan
Version     : 1.0
Date        : 2022/10/29
Description : CIrFireDrc.
*************************************************************/
#ifndef FIRE_DRC_H
#define FIRE_DRC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../ITAFireDRC.h"
#include "../Base/Config.h"
//#include <PortingAPI.h>

#ifdef FIRE_DRC_EDITION

#ifndef max_ir
#define max_ir(a,b)            (((a) > (b)) ? (a) : (b))
#endif
#ifndef min_ir
#define min_ir(a,b)            (((a) < (b)) ? (a) : (b))
#endif
#define GRAYLEVEL_16BIT         (65536) 
#define FRAME_SMOOTH_NUM        (8)
#define ClipU8(x) (x < 0) ? 0 : ((x > 255) ? 255 : x)

//输出图像类型
#define COLORIMGTYPENUM (12)

#define PSEUDONUM (10)
//extern unsigned char PseudoTableRGB[PSEUDONUM][256][4];
//extern unsigned char PseudoTableYUV[PSEUDONUM][256][4];
#define WORKMODE (6)
#define MARKER_NUM1 (6)

class CIrFireDrc
{
public:
	//默认构造函数
	CIrFireDrc();
	//构造函数
	CIrFireDrc(int nWidth, int nHeight, ITA_PIXEL_FORMAT imgFormat);
	//析构函数
	~CIrFireDrc();

	//消防调光模式（分段调光）
	//输入参数：
	//short *pus_src, 输入Y16数据
	//unsigned char* puc_dst, 输出8bit图像
	//int n_width, int n_height, 输入Y16数据宽和高 
	//DrcPara* sDrcPara  成像参数结构体
	//int nUpY16, int nDownY16  两个温度反差Y16
	void DRCFire(short* pus_src, unsigned char* puc_dst, int n_width, int n_height, int nUpY16, int nDownY16);

	//正常调光模式（低测温范围使用）
	//输入参数：
	//short *pus_src, 输入Y16数据
	//unsigned char* puc_dst, 输出8bit图像
	//int n_width, int n_height, 输入Y16数据宽和高 
	//DrcPara* sDrcPara  成像参数结构体
	void DRCFireLow(short* pus_src, unsigned char* puc_dst, int n_width, int n_height);

	//DrcPara参数设置
	long PutDrcPara(ITAFireDRCParam sPara);
	long GetDrcPara(ITAFireDRCParam* sPara);

	ITA_RESULT CalcROITemperBarHeightSimplify(int centerY16, ITAY16TempMap map, ITAFireWorkMode workMode, float* barHeight);

	// ITA_RESULT  DrcFireAdaptive2DifferentMode(short* pus_src, unsigned char* puc_dst, int n_width, int n_height, int MaxTemperY16, int ThreshY16, ITAFireWorkMode work_mode, ITAFirePaletteType pesudo,ITA_PIXEL_FORMAT imgtype);
	ITA_RESULT  DrcFireAdaptive2DifferentMode(short* pus_src, unsigned char* puc_dst, int n_width, int n_height, int MaxTemperY16, int ThreshY16, ITAFireWorkMode work_mode, ITAFirePaletteType pesudo);

	//void RegistryMemoryOp(ITADmaMemoryOp* memoryOp);
private:
	unsigned char* gDstDrc = NULL;
	short* gSrcL = NULL;
	short* gSrcPad = NULL;
	int gWidth, gHeight, gLen;  //红外图像宽高
	int gSrcPadW, gSrcPadH, gWinW;
	int gOffset = 32768;
	int gCurFrameNum = 0;
	int gMaxContrast = 4;
	int gRangeSmooth[FRAME_SMOOTH_NUM] = { 0 };
	int gIDEPosWeightTable[9] = { 0 };
	int gIDePos1DweightTable[3] = { 0 };

	// 伪彩设置
	float gTemperMarkerLocBase[MARKER_NUM1] = { 0.0f,0.25f,0.45f,0.65f,0.95f,1.0f };
	float gTemperMarkerLocFire[MARKER_NUM1] = { 0.0f,0.375f,0.575f,0.7f,0.95f,1.0f };
	float gTemperMarkerLocSense[MARKER_NUM1] = { 0.0f,0.625f,0.78f,0.95f,1.0f,1.0f };
	float gSceneMeanY16;
	float gLinearBrightScale;
	float gExpectBrightness;
	float gBrightChangeSpeed;

	ITAFireDRCParam sDrcPara;

	unsigned short* Enimage;//---------fuhang

	void PadMatrix(short* pus_pad_mat, short* pus_mat, int n_width, int n_height, int n_win_size_width, int n_win_size_height);


	void FastGaussianFilter16bit(short* pus_dst, short* pus_src, int n_width, int n_height, int n_win_size, int* weight);

	void GaussianFilter16bit(short* pus_dst, short* pus_src, int n_width, int n_height, int n_win_wid, int* pos_weight_table);

	void GetGuassPosWeightTable(int* a_pos_weight_table, int n_win_wid, int n_win_hei, int std);
	void GetGuass1DweightTable(int* weight, int n_win_size, int std) {
		int i, j;
		int n_half_size = (n_win_size - 1) / 2;
		int n_dist;
		float f_arg = 0;
		float f_sum = 0;
		float f_tmp;

		for (j = 0; j < n_win_size; j++)
		{
			n_dist = j - n_half_size;
			f_arg = -(n_dist * n_dist) / (2.f * std * std / 100 + 0.01f);
			f_tmp = (float)exp(f_arg);
			f_sum += f_tmp;
			weight[j] = (int)(f_tmp * 4096 + 0.5f);//精度为1/4096
		}
		//归一化位置高斯空间滤波器
		for (i = 0; i < n_win_size; i++)
		{
			weight[i] = (int)(weight[i] / f_sum);
		}
	}


	void GetHist(unsigned short* pHist, int* pnMaxOri, int* pnMinOri, int* pnMaxDisc, int* pnMinDisc, int* pGrayMean, short* pusSrcImg, int n_width,
		int n_height, int nHistSize, int nUpDiscardRatio, int nDownDiscardRatio, int nDownY16);

	void GetHistLow(unsigned short* pHist, int* pnMaxOri, int* pnMinOri, int* pnMaxDisc, int* pnMinDisc, int* pGrayMean, short* pusSrcImg, int n_width,
		int n_height, int nHistSize, int nUpDiscardRatio, int nDownDiscardRatio);

	void DRCMix(unsigned char* puc_dst, short* pus_src, int n_width, int n_height, int nMixContrastExp, int nMixBrightExp, int nRestrainRangeThre,
		int n_mixrange, int nMidValue, int nPlatThresh, int nUpDiscardRatio, int nDownDiscardRatio, int nDownY16);

	void DRCMixForZC17A(unsigned char* puc_dst, short* pus_src, int& Y16mean, int& DiscMax, int& DiscMin, int n_width, int n_height, int nMixContrastExp, int nMixBrightExp, int nRestrainRangeThre,
		int n_mixrange, int nMidValue, int nPlatThresh, int nUpDiscardRatio, int nDownDiscardRatio, int nDownY16);

	void DRCMixLow(unsigned char* puc_dst, short* pus_src, int n_width, int n_height, int nMixContrastExp, int nMixBrightExp, int nRestrainRangeThre,
		int n_mixrange, int nMidValue, int nPlatThresh, int nUpDiscardRatio, int nDownDiscardRatio);

	int inline AdaptiveLinearBrightExp(int Y16Mean, int Y16Up, int Y16down) {
		float r = 0.5 * float(Y16Mean - Y16down) / float(Y16Up - Y16down);
		float expBrightness = r * gLinearBrightScale + sDrcPara.mixBrightExp;
		int actualBrightness = int(sDrcPara.mixBrightExp * gBrightChangeSpeed + expBrightness * (1 - gBrightChangeSpeed));
		return actualBrightness;
	}
	ITA_FIRE_PALETTE_TYPE paletteType;
	ITA_PIXEL_FORMAT format;
	short* temp_short;
	//ITADmaMemoryOp m_memoryOp;
};

#endif // !FIRE_DRC_EDITION
#endif // !FIRE_DRC_H
