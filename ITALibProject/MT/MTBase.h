/*************************************************************
Copyright (C), 2022--2023, Wuhan Guide Sensmart Tech Co., Ltd
File name   : MTBase.h
Author      : Guide Sensmart RD. wangyan
Version     : 1.0
Date        : 2022/1/15
Description : MTBase.
*************************************************************/
#ifndef MTBASE_H
#define MTBASE_H

#include "../ITA.h"
#include "../Base/Parser.h"
#include "../MT/MeasureTempCoreTEC.h"

class MTBase
{
public:
	MTBase() { };
	virtual ~MTBase() { };

	virtual int loadData(PackageHeader *ph, short* fpaTempArray, short* allCurvesArray) = 0;

	/*测温距离范围也决定着一组距离补偿系数。用到时再加。*/
	virtual ITA_RESULT loadConfig(ITAConfig * config, ITA_RANGE range, ITA_FIELD_ANGLE lensType) = 0;

	/*只用在快门状态更新时通知，不用每帧刷新。*/
	virtual ITA_RESULT refreshShutterStatus(short y16CenterValue, ITA_MODE m_mode) = 0;

	virtual ITA_RESULT calcFPAGear(int *gear) = 0;

	/*测温参数m_stMtParams在ITAWrapper中实时更新，切换范围时通知。*/
	virtual ITA_RESULT changeRange(ITA_RANGE range, ITA_FIELD_ANGLE lensType) = 0;

	virtual ITA_RESULT calcTempByY16(short y16, float *temp) = 0;

	virtual ITA_RESULT calcBodyTemp(float surfaceTemp, float envirTemp, float *bodyTemp) = 0;

	virtual ITA_RESULT calcY16ByTemp(float temp, short *y16) = 0;

	virtual ITA_RESULT calcY16MatrixByTemp(float *tempMatrix, int w, int h, short *y16Matrix, int matrixSize) = 0;

	virtual ITA_RESULT calcTempMatrix(short* y16Array, int y16W, int y16H, float distance, int x, int y, int w, int h, ITA_MATRIX_TYPE type, float *tempMatrix) = 0;

	virtual ITA_RESULT parametersControl(ITA_MC_TYPE type, void *param) = 0;
	virtual IMAGE_MEASURE_STRUCT *getTECParams() = 0;
	virtual MtParams *getMtParams() = 0;
	virtual void setMtParams(void *param, int paramLen) = 0;
	virtual void getParamInfo(ITAParamInfo *info) = 0;
	virtual float smoothFocusTemp(float fpaTemp) = 0;

	/*计算中心温用到。*/
	virtual ITA_RESULT refreshFrame(short* y16Array, int w, int h) = 0;

	// 自动校温接口，内部校温使用
	virtual bool AutoCorrectTempNear(float *fNearKf, float *fNearB, AutoCorrectTempPara * autocorTpara) = 0;
	virtual bool AutoCorrectTempFar(float *fFarKf, float *fFarB, AutoCorrectTempPara * autocorTpara) = 0;

	//二次校温接口
	virtual bool AutoCorrectTempFarByUser(float *fFarKf, float *fFarB, AutoCorrectTempPara *autocorTpara) = 0;
	virtual bool AutoCorrectTempNearByUser(float *fNearKf, float *fNearB, AutoCorrectTempPara *autocorTpara) = 0;
	virtual void setLogger(GuideLog *logger) = 0;
	/*设置从配置文件中读取的产品型号*/
	virtual void setProductType(ITA_PRODUCT_TYPE type) = 0;
	virtual ITA_RESULT isChangeRange(short *pSrc, int width, int height, int range,
		float areaTh1, float areaTh2, int low2high, int high2low, int *isChange) = 0;
	virtual ITA_RESULT isMultiChangeRange(short* pSrc, int width, int height, int range,
		float areaTh1, float areaTh2, int low2mid, int mid2low, int mid2high, int high2mid, int* isChange) = 0;
	virtual int setKFB(ITA_CALIBRATE_TYPE type, float p) = 0;

	virtual ITA_RESULT refreshCoreParaInfo() = 0;
	
	/* 自动校温接口，ITA外部接口调用 */
	virtual ITA_RESULT AutoCorrectTemp(float *pBlackBodyTemp, short *pY16, int arrLength) = 0;

	// 分析对象测温接口
	virtual ITA_RESULT AnalysisAdd(ITAShape* shape, int size) = 0;
	virtual ITA_RESULT AnalysisClear() = 0;
	virtual ITA_RESULT AnalysisTemp(short* y16Data, int width, int height, ITAAnalysisTemp* analysisTemp, int size, short avgB) = 0;
	virtual ITA_RESULT AnalysisY16(short* y16Array, int width, int height, ITAAnalysisY16Info* analysisY16Info, int size) = 0;
	virtual ITA_RESULT GetRangeMask(ITARangeMask* rangeMask, int size) = 0;
};
#endif // !MTBASE_H

