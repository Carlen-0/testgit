/*************************************************************
Copyright (C), 2021--2022, Wuhan Guide Sensmart Tech Co., Ltd
File name   : ImageProcessor.h
Author      : Guide Sensmart RD. wangyan
Version     : 1.0
Date        : 2021/11/9
Description : Imaging processing module.
*************************************************************/
#ifndef ITA_IMAGE_PROCESSOR_H
#define ITA_IMAGE_PROCESSOR_H

#include "CInfraredCore.h"
#include "../ITA.h"
#include "../Module/GuideLog.h"
#include "../Base/Parser.h"
#include "../Module/Debugger.h"
#include "../AppAlg/SrBicubic.h"
#include "../AppAlg/ImageFusion.h"

typedef struct ISP_PARAM
{
	int contrast;
	int brightness;
	bool isTimeFilter;	//时域滤波开关，可以关闭或者打开。
	bool isSpatialFilter;	//空域滤波开关，降噪，可以关闭或者打开。
	bool isRemoveVertical;//去竖纹，可以关闭或者打开。
	bool isRemoveHorizontal;//去横纹，可以关闭或者打开。
	bool isHSM;			//初始化气体检测, 可以关闭或者打开
	bool isSharpening;	//锐化，可以关闭或者打开。
	bool isDetailEnhance;	//细节增强，可以关闭或者打开。
	bool isBlockHistogram;//分块直方图，可以关闭或者打开。
	bool isGammaCorrection;//Gamma校正，可以关闭或者打开。
	bool isY8AdjustBC;	//Y8纠偏，可以关闭或者打开。
	bool isDrtY8;	//调光，默认打开。如果关闭，那么不处理Y16转Y8，不会生成imgDst图像数据。
	unsigned char isAutoRpBdPs;//自动去坏点，可以关闭或者打开。
	int tffStd; //时域滤波标准差
	int restrainRange; //调光抑制范围
	int discardUpratio;  //调光上抛点比例
	int discardDownratio;  //调光下抛点比例
	int mixThrHigh;		//调光高阈值
	int mixThrLow;		//调光低阈值
	int heqPlatThresh;  //平台直方图调光参数
	int heqRangeMax;  //平台直方图调光参数
	int heqMidvalue;  //平台直方图调光参数
	int iieEnhanceCoef; //细节增益系数
	int iieGaussStd;  //增强算法参数
	float gmcGamma;  //Gamma算法参数
	int gmcType;  //Gamma算法参数
	int hsmWeight;//气体检测算法参数
	int hsmStayThr;//气体检测静态阈值
	int hsmStayWeight;//气体检测静态帧间差值权重
	int hsmInterval;//气体检测缓冲区间隔
	bool hsmMidfilter; //气体检测中值滤波开关
	int hsmMidfilterWinsize;//气体检测中值滤波窗口
	int hsmDeltaUpthr;//帧间差值的阈值上限
	int hsmDeltaDownthr;//帧间差值的阈值下限
	int hsmPointThr;	//单点噪声阈值
	int zoomType; //图像放大类型参数，0:Near,1:Bilinear,2:BilinearFast
	float laplaceWeight;//图像锐化参数。
	int linearResrainRange;//均匀面抑制.
}ISPParam;

class ImageProcessor
{
public:
	ImageProcessor(ITA_MODE mode, int w, int h, float scale, ITA_PIXEL_FORMAT pixelFormat);
	~ImageProcessor();

	ITA_RESULT setCurrentK(unsigned short* currentK, int len);
	//NUC状态：0，初始空状态；1，开始做NUC；2，正在做NUC；3，NUC已完成。
	ITA_RESULT updateB(short* b, int len, int nucStatus);

	short getAvgB();

	ITA_RESULT getImageInfo(ITA_ROTATE_TYPE rotate, ITA_PIXEL_FORMAT pixelFormat, ITAImgInfo * pInfo);

	ITA_RESULT process(short * srcData, int srcSize, ITAISPResult * pResult);

	ITA_RESULT copyY8(short * srcData, int srcSize, ITAISPResult * pResult, unsigned char *y8Data, int y8Size);

	void setLogger(GuideLog *logger);

	int getPaletteNum();

	ITA_RESULT setPaletteIndex(ITA_PALETTE_TYPE paletteIndex);

	ITA_RESULT setContrast(int contrast);

	ITA_RESULT setBrightness(int brightness);

	ITA_RESULT setDimmingType(ITA_DRT_TYPE type, ITADRTParam *param);
	ITA_DRT_TYPE getDimmingType();

	ITA_RESULT setEqualLineType(ITA_EQUAL_LINE_TYPE type, ITAEqualLineParam param);
	ITA_EQUAL_LINE_TYPE getEqualLineType();

	ITA_RESULT setFlip(ITA_FLIP_TYPE type);

	ITA_RESULT setRotate(ITA_ROTATE_TYPE type);

	ITA_RESULT setScale(float scale);

	ITA_RESULT setHsmWeight(int hsmWeight);

	ISPParam getISPParam();

	ITA_RESULT setISPParam(ISPParam param);

	void getParamInfo(ITAParamInfo *info);

	int loadConfig(ITAConfig * config, ITA_RANGE range, ITA_FIELD_ANGLE lensType);

	void setDebugger(Debugger *debugger);

	ITA_RESULT control(ITA_DEBUG_TYPE type, void *param);

	ITA_RESULT setPixelFormat(ITA_PIXEL_FORMAT pixelFormat);

	void setRepeatCorrection(int repeatCorrection);
	void setThreshold(void* threshold);

	ITA_RESULT zoomY16(short* srcY16, int srcW, int srcH, short* dstY16, int dstW, int dstH, float times);

	ITA_RESULT processBiCubic(void* dst, void* src, int w, int h, int w_zoom, int h_zoom, ITA_DATA_TYPE dataType);

	ITA_RESULT convertY8(const unsigned char* src, int srcW, int srcH, unsigned char* dst, int dstW, int dstH, int dateLen, ITA_Y8_DATA_MODE datamode = ITA_Y8_NORMAL);

	ITA_RESULT getColorImage(unsigned char *pY8, int w, int h, unsigned char* pPalette, int paletteLen, unsigned char* pImage, int imgLen);

	ITA_RESULT customPalette(ITA_PALETTE_OP_TYPE type, void* param);

	ITA_RESULT potCoverRestrain(ITA_POTCOVER_OP_TYPE type, void* para);
	ITA_RESULT putGuoGaiPara(stDGGTPara* guoGaiPara);
	ITA_RESULT getGuoGaiPara(stDGGTPara* guoGaiPara);

	bool getPotCoverSwitch();

	bool getGainMat(unsigned short* pus_high_base, unsigned short* pus_low_base, unsigned short* pn_gain_mat,int n_width, int n_height);
private:
	CInfraredCore *m_IrImg;
	//CInfraredCore* m_IrImgInstance;
	ITA_MODE m_mode;
	int m_width, m_height;
	float m_scale;
	GuideLog *m_logger;
	ITAConfig m_defaultConf;
	short m_avgB;
	Debugger *m_debugger;
	int m_snapShot;	//0，初始状态；1，抓当前帧；2，正在录流；3，停止录流。
	bool m_isDrtY8; //调光，默认打开。如果关闭，那么不处理Y16转Y8，不会生成imgDst图像数据。
	ITA_EQUAL_LINE_TYPE m_equalLineType;
	ITA_DRT_TYPE m_drtType;
	int m_nucRepeatCorrection; //重复NUC纠错机制。默认0关闭。1，用户打开nuc重复纠错机制。
	int m_threshold; //阈值，120模组不大于400，256模组待定。
	SR_BICUBIC* m_biCubic;//超分放大图像实例。
	bool isGuoGai;//去锅盖算法开关
};

#endif // !ITA_IMAGE_PROCESSOR_H

