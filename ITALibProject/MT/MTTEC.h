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

	/*���¾��뷶ΧҲ������һ����벹��ϵ�����õ�ʱ�ټӡ�*/
	ITA_RESULT loadConfig(ITAConfig * config, ITA_RANGE range, ITA_FIELD_ANGLE lensType);

	/*ֻ���ڿ���״̬����ʱ֪ͨ������ÿ֡ˢ�¡�*/
	ITA_RESULT refreshShutterStatus(short y16CenterValue, ITA_MODE m_mode);

	ITA_RESULT calcFPAGear(int *gear);

	/*���²���m_stMtParams��ITAWrapper��ʵʱ���£��л���Χʱ֪ͨ��*/
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

	/*�����������õ���*/
	ITA_RESULT refreshFrame(short* y16Array, int w, int h);

	// �Զ�У�½ӿڣ��ڲ�У��ʹ��
	bool AutoCorrectTempNear(float *fNearKf, float *fNearB, AutoCorrectTempPara * autocorTpara);
	bool AutoCorrectTempFar(float *fFarKf, float *fFarB, AutoCorrectTempPara * autocorTpara);


	//����У�½ӿ�
	bool AutoCorrectTempFarByUser(float *fFarKf, float *fFarB, AutoCorrectTempPara *autocorTpara);
	bool AutoCorrectTempNearByUser(float *fNearKf, float *fNearB, AutoCorrectTempPara *autocorTpara);
	void setLogger(GuideLog *logger);
	/*���ô������ļ��ж�ȡ�Ĳ�Ʒ�ͺ�*/
	void setProductType(ITA_PRODUCT_TYPE type);
	ITA_RESULT isChangeRange(short *pSrc, int width, int height, int range,
		float areaTh1, float areaTh2, int low2high, int high2low, int *isChange);
	ITA_RESULT isMultiChangeRange(short* pSrc, int width, int height, int range,
		float areaTh1, float areaTh2, int low2mid, int mid2low, int mid2high, int high2mid, int* isChange);
	int setKFB(ITA_CALIBRATE_TYPE type, float p);

	ITA_RESULT refreshCoreParaInfo();
	
	/* �Զ�У�½ӿڣ�ITA�ⲿ�ӿڵ��� */
	ITA_RESULT AutoCorrectTemp(float *pBlackBodyTemp, short *pY16, int arrLength);

	// ����������½ӿ�
	ITA_RESULT AnalysisAdd(ITAShape* shape, int size);
	ITA_RESULT AnalysisClear();
	ITA_RESULT AnalysisTemp(short* y16Data, int width, int height, ITAAnalysisTemp* analysisTemp, int size, short avgB);
	ITA_RESULT AnalysisY16(short* y16Array, int width, int height, ITAAnalysisY16Info* analysisY16Info, int size);
	ITA_RESULT GetRangeMask(ITARangeMask* rangeMask, int size);
private:
	//��һ�δ����ʱ��ľ�Ͳ�¶�
	//float m_lensTempWhenShutter;
	int m_w;
	int m_h;
	int m_y16Len;
	MeasureTempCoreTEC *m_coreTEC;
	IMAGE_MEASURE_STRUCT m_paramsTEC;
	//MtParams m_params;
	//int m_shutterClosed;	//��¼��һ֡����״̬���պ�1����0��
	GuideLog *m_logger;
	//ITAConfig m_defaultConf;
	short m_centralArray[8]; //�����˲����ĵ�Y16����
	unsigned int m_centralCount; //֡����
	ITA_PRODUCT_TYPE m_product;

	PolygonAnalysiser* m_analysiser;
};

#endif // !MTTEC_H


