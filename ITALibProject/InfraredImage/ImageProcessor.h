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
	bool isTimeFilter;	//ʱ���˲����أ����Թرջ��ߴ򿪡�
	bool isSpatialFilter;	//�����˲����أ����룬���Թرջ��ߴ򿪡�
	bool isRemoveVertical;//ȥ���ƣ����Թرջ��ߴ򿪡�
	bool isRemoveHorizontal;//ȥ���ƣ����Թرջ��ߴ򿪡�
	bool isHSM;			//��ʼ��������, ���Թرջ��ߴ�
	bool isSharpening;	//�񻯣����Թرջ��ߴ򿪡�
	bool isDetailEnhance;	//ϸ����ǿ�����Թرջ��ߴ򿪡�
	bool isBlockHistogram;//�ֿ�ֱ��ͼ�����Թرջ��ߴ򿪡�
	bool isGammaCorrection;//GammaУ�������Թرջ��ߴ򿪡�
	bool isY8AdjustBC;	//Y8��ƫ�����Թرջ��ߴ򿪡�
	bool isDrtY8;	//���⣬Ĭ�ϴ򿪡�����رգ���ô������Y16תY8����������imgDstͼ�����ݡ�
	unsigned char isAutoRpBdPs;//�Զ�ȥ���㣬���Թرջ��ߴ򿪡�
	int tffStd; //ʱ���˲���׼��
	int restrainRange; //�������Ʒ�Χ
	int discardUpratio;  //�������׵����
	int discardDownratio;  //�������׵����
	int mixThrHigh;		//�������ֵ
	int mixThrLow;		//�������ֵ
	int heqPlatThresh;  //ƽֱ̨��ͼ�������
	int heqRangeMax;  //ƽֱ̨��ͼ�������
	int heqMidvalue;  //ƽֱ̨��ͼ�������
	int iieEnhanceCoef; //ϸ������ϵ��
	int iieGaussStd;  //��ǿ�㷨����
	float gmcGamma;  //Gamma�㷨����
	int gmcType;  //Gamma�㷨����
	int hsmWeight;//�������㷨����
	int hsmStayThr;//�����⾲̬��ֵ
	int hsmStayWeight;//�����⾲̬֡���ֵȨ��
	int hsmInterval;//�����⻺�������
	bool hsmMidfilter; //��������ֵ�˲�����
	int hsmMidfilterWinsize;//��������ֵ�˲�����
	int hsmDeltaUpthr;//֡���ֵ����ֵ����
	int hsmDeltaDownthr;//֡���ֵ����ֵ����
	int hsmPointThr;	//����������ֵ
	int zoomType; //ͼ��Ŵ����Ͳ�����0:Near,1:Bilinear,2:BilinearFast
	float laplaceWeight;//ͼ���񻯲�����
	int linearResrainRange;//����������.
}ISPParam;

class ImageProcessor
{
public:
	ImageProcessor(ITA_MODE mode, int w, int h, float scale, ITA_PIXEL_FORMAT pixelFormat);
	~ImageProcessor();

	ITA_RESULT setCurrentK(unsigned short* currentK, int len);
	//NUC״̬��0����ʼ��״̬��1����ʼ��NUC��2��������NUC��3��NUC����ɡ�
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
	int m_snapShot;	//0����ʼ״̬��1��ץ��ǰ֡��2������¼����3��ֹͣ¼����
	bool m_isDrtY8; //���⣬Ĭ�ϴ򿪡�����رգ���ô������Y16תY8����������imgDstͼ�����ݡ�
	ITA_EQUAL_LINE_TYPE m_equalLineType;
	ITA_DRT_TYPE m_drtType;
	int m_nucRepeatCorrection; //�ظ�NUC������ơ�Ĭ��0�رա�1���û���nuc�ظ�������ơ�
	int m_threshold; //��ֵ��120ģ�鲻����400��256ģ�������
	SR_BICUBIC* m_biCubic;//���ַŴ�ͼ��ʵ����
	bool isGuoGai;//ȥ�����㷨����
};

#endif // !ITA_IMAGE_PROCESSOR_H

