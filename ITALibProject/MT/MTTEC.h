/*************************************************************
Copyright (C), 2022--2023, Wuhan Guide Sensmart Tech Co., Ltd
File name   : MTTEC.h
Author      : Guide Sensmart RD. wangyan
Version     : 1.0
Date        : 2022/1/15
Description : MTTEC.
*************************************************************/
#ifndef MTTEC_H
#define MTTEC_H

#include "MeasureTempCoreTEC.h"
#include "MTBase.h"

class PolygonAnalysiser;
class MTTEC :public MTBase
{
public:
	MTTEC(ITA_RANGE range, int w, int h, ITA_PRODUCT_TYPE type);
	~MTTEC();
	int loadData(PackageHeader *ph, short* fpaTempArray, short* allCurvesArray);

	/*测温距离范围也决定着一组距离补偿系数。用到时再加。*/
	ITA_RESULT loadConfig(ITAConfig * config, ITA_RANGE range, ITA_FIELD_ANGLE lensType);

	/*只用在快门状态更新时通知，不用每帧刷新。*/
	ITA_RESULT refreshShutterStatus(short y16CenterValue, ITA_MODE m_mode);

	ITA_RESULT calcFPAGear(int *gear);

	/*测温参数m_stMtParams在ITAWrapper中实时更新，切换范围时通知。*/
	ITA_RESULT changeRange(ITA_RANGE range, ITA_FIELD_ANGLE lensType);

	ITA_RESULT calcTempByY16(short y16, float *temp);

	ITA_RESULT calcBodyTemp(float surfaceTemp, float envirTemp, float *bodyTemp);

	ITA_RESULT calcY16ByTemp(float temp, short *y16);

	ITA_RESULT calcY16MatrixByTemp(float *tempMatrix, int w, int h, short *y16Matrix, int matrixSize);

	ITA_RESULT calcTempMatrix(short* y16Array, int y16W, int y16H, float distance, int x, int y, int w, int h, ITA_MATRIX_TYPE type, float *tempMatrix);

	ITA_RESULT parametersControl(ITA_MC_TYPE type, void *param);

	IMAGE_MEASURE_STRUCT *getTECParams();
	MtParams *getMtParams();
	void setMtParams(void *param, int paramLen);
	void getParamInfo(ITAParamInfo *info);
	float smoothFocusTemp(float fpaTemp);

	/*计算中心温用到。*/
	ITA_RESULT refreshFrame(short* y16Array, int w, int h);

	// 自动校温接口，内部校温使用
	bool AutoCorrectTempNear(float *fNearKf, float *fNearB, AutoCorrectTempPara * autocorTpara);
	bool AutoCorrectTempFar(float *fFarKf, float *fFarB, AutoCorrectTempPara * autocorTpara);


	//二次校温接口
	bool AutoCorrectTempFarByUser(float *fFarKf, float *fFarB, AutoCorrectTempPara *autocorTpara);
	bool AutoCorrectTempNearByUser(float *fNearKf, float *fNearB, AutoCorrectTempPara *autocorTpara);
	void setLogger(GuideLog *logger);
	/*设置从配置文件中读取的产品型号*/
	void setProductType(ITA_PRODUCT_TYPE type);
	ITA_RESULT isChangeRange(short *pSrc, int width, int height, int range,
		float areaTh1, float areaTh2, int low2high, int high2low, int *isChange);
	ITA_RESULT isMultiChangeRange(short* pSrc, int width, int height, int range,
		float areaTh1, float areaTh2, int low2mid, int mid2low, int mid2high, int high2mid, int* isChange);
	int setKFB(ITA_CALIBRATE_TYPE type, float p);

	ITA_RESULT refreshCoreParaInfo();
	
	/* 自动校温接口，ITA外部接口调用 */
	ITA_RESULT AutoCorrectTemp(float *pBlackBodyTemp, short *pY16, int arrLength);

	// 分析对象测温接口
	ITA_RESULT AnalysisAdd(ITAShape* shape, int size);
	ITA_RESULT AnalysisClear();
	ITA_RESULT AnalysisTemp(short* y16Data, int width, int height, ITAAnalysisTemp* analysisTemp, int size, short avgB);
	ITA_RESULT AnalysisY16(short* y16Array, int width, int height, ITAAnalysisY16Info* analysisY16Info, int size);
	ITA_RESULT GetRangeMask(ITARangeMask* rangeMask, int size);
private:
	//上一次打快门时候的镜筒温度
	//float m_lensTempWhenShutter;
	int m_w;
	int m_h;
	int m_y16Len;
	MeasureTempCoreTEC *m_coreTEC;
	IMAGE_MEASURE_STRUCT m_paramsTEC;
	//MtParams m_params;
	//int m_shutterClosed;	//记录上一帧快门状态，闭合1，打开0。
	GuideLog *m_logger;
	//ITAConfig m_defaultConf;
	short m_centralArray[8]; //空域滤波中心点Y16数组
	unsigned int m_centralCount; //帧计数
	ITA_PRODUCT_TYPE m_product;

	PolygonAnalysiser* m_analysiser;
};

#endif // !MTTEC_H


