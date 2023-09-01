/*************************************************************
Copyright (C), 2022--2023, Wuhan Guide Sensmart Tech Co., Ltd
File name   : ITARefriger.h
Author      : Guide Sensmart RD. wangyan
Version     : 1.0
Date        : 2022/10/29
Description : Temperature measurement model of refrigeration detector.
*************************************************************/
#ifndef ITA_REFRIGER_H
#define ITA_REFRIGER_H

#ifdef __cplusplus
#    define ITA_REFRIGER_START  extern "C" {
#    define ITA_REFRIGER_END    };
#else
#    define ITA_REFRIGER_START
#    define ITA_REFRIGER_END
#endif

ITA_REFRIGER_START

#include "ERROR.h"
#include "ITADTD.h"

//������ز����ṹ��
typedef struct ITA_REFRIGER_MEASURE_PARAM
{
	short sY16Offset;										//Y16ƫ����(Ĭ��0)
	int nKF;												//������ʱY16��������(���㻯100��,Ĭ��ֵΪ100)
	int nB1;												//������ʱY16��ƫ����(���㻯100����Ĭ��0)
	int nDistance_a0;										//����У��ϵ��(���㻯10000000000����Ĭ��0)
	int nDistance_a1;										//����У��ϵ��(���㻯1000000000����Ĭ��0)
	int nDistance_a2;										//����У��ϵ��(���㻯10000000����Ĭ��0)
	int nDistance_a3;										//����У��ϵ��(���㻯100000����Ĭ��0)
	int nDistance_a4;										//����У��ϵ��(���㻯100000����Ĭ��0)
	int nDistance_a5;										//����У��ϵ��(���㻯10000����Ĭ��0)
	int nDistance_a6;										//����У��ϵ��(���㻯1000����Ĭ��0)
	int nDistance_a7;										//����У��ϵ��(���㻯100����Ĭ��0)
	int nDistance_a8;										//����У��ϵ��(���㻯100����Ĭ��0)
	int nK1;												//��������ϵ��(���㻯100����Ĭ��0��
	int nK2;												//��Ͳ��Ưϵ��(���㻯100����Ĭ��0)
	int nK3;												//��������ϵ��(���㻯10000����Ĭ��0)
	int nB2;												//��������ƫ����(���㻯10000����Ĭ��0)
	int nKFOffset;											//�Զ�У��KFƫ�����������ָ�����У�����ã�Ĭ��0
	int nB1Offset;											//�Զ�У��B1ƫ�����������ָ�����У�����ã�Ĭ��0
	int nGear;												//���µ�λ, 0:���µ�, 1:���µ�
	int nHumidity;											//ʪ��(���㻯100����Ĭ��60)
	int nAtmosphereTransmittance;							//����͸����(��Χ0~100��Ĭ��100)
	float nCurrentB;										//��ǰ��Χ��Ĭ�ϱ����¶ȣ�Ĭ��20.0
	int nBCurveIndex;										//���׷������߶�Ӧ��Y16ֵ
	float fEmiss;											//������(0.01-1.0,Ĭ��1.0)
	float fDistance;										//���¾��룬Ĭ��3��
	float fReflectT;										//�����¶ȣ����µ�Ĭ��3�����µ�Ĭ��23��
	float fAmbient;											//�����¶ȣ�ȡ���������£�
	float fAtmosphereTemperature;							//�����¶�
	float fWindowTransmittance;								//����͸����(��Χ0~1��Ĭ��1)
	float fWindowTemperature;								//�����¶ȣ����µ�Ĭ��3�����µ�Ĭ��23��
	float fRealTimeLensTemp;								//ʵʱ��Ͳ��
	float fCurrentLensTemp;									//���һ�δ����ʱ�ľ�Ͳ��
	float fOriginLensTemp;									//������Ͳ��
	float fRealShutterTemp;									//ʵʱ�����¶�
	float fOriShutterTemp;									//���������¶�
	bool bHumidityCorrection;								//ʪ���������أ�Ĭ�ϴ�
	bool bLensCorrection;									//��Ͳ�������أ�Ĭ�ϴ�
	bool bShutterCorrection;								//�����������أ�Ĭ�ϴ�
	bool bEmissCorrection;									//�������������أ�Ĭ�ϴ�
	bool bDistanceCorrection;								//�����������أ�Ĭ�ϴ�
	bool bAmbientCorrection;								//�����������أ�Ĭ�ϴ�
	bool bB1Correction;										//B1�������أ�Ĭ�ϴ�
	bool bAtmosphereCorrection;								//����͸�����������أ�Ĭ�ϴ�
	bool bWindowTransmittanceCorrection;					//����͸���ʿ��أ�Ĭ�ϴ�
}ITARefrigerMeasureParam;
/**
* @brief:	��ʼ��һ��ITA����̽��������ģ��ʵ����
* @param:	int w  ��
* @param:	int h  �ߡ�
* @param:	short *pusCurve  �������ݵ�ַ��
* @param:	int curveLength  �������ݳ��ȡ�
* @return:	�ɹ�������ʵ�������ʧ�ܣ�����NULL��
**/
ITA_API HANDLE_T ITA_RefrigerInit(int w, int h, short *pusCurve, int curveLength);

/**
* @brief:	�ͷ�һ��ITA����̽��������ģ��ʵ����Դ��
* @param:	HANDLE_T instance	ITA�������ģ��ʵ����
* @return:	�ޡ�
**/
ITA_API void ITA_RefrigerUninit(HANDLE_T instance);

/**
* @brief:	���ò��²�����
* @param:	HANDLE_T instance           ITA����̽��������ģ��ʵ����
* @param:	ITARefrigerMeasureParam params    ���²�����
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_RefrigerSetParams(HANDLE_T instance, ITARefrigerMeasureParam params);

/**
* @brief:	��ѯ���²�����Ϣ��
* @param:	HANDLE_T instance           ITA����̽��������ģ��ʵ����
* @param:	ITARefrigerMeasureParam *params   ������²�����Ϣ��
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_RefrigerGetParams(HANDLE_T instance, ITARefrigerMeasureParam *params);

/**
* @brief:	�����������ݡ�
* @param:	HANDLE_T instance			ITA����̽��������ģ��ʵ����
* @param:	short *pusCurve 	        �������ݵ�ַ��
* @param:	int curveLength				�������ݳ��ȡ�
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_RefrigerSetCurve(HANDLE_T instance, short *pusCurve, int curveLength);

/**
* @brief:	������¡�
* @param:	HANDLE_T instance           ITA����̽��������ģ��ʵ����
* @param:	short y16value				����Y16��
* @param:	float *temp					����¶ȡ�
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_RefrigerCalcTemp(HANDLE_T instance, short y16value, float *temp);

/**
* @brief:	�����¶Ⱦ���
* @param:	HANDLE_T instance		ITA����̽��������ģ��ʵ����
* @param:	short* y16Array			Y16����
* @param:	y16W,y16H				Y16����ֱ��ʡ�
* @param:	x,y,w,h					Ŀ���������ע�ⲻҪԽ�磬����ᱨ��
* @param:	float distance			Ŀ����룬��λ���ס���ǰ���������Ч����ɺ󽫻ָ�����ǰ��ֵ��
* @param:	float emiss				�����ʡ���ǰ���������Ч����ɺ󽫻ָ�����ǰ��ֵ��
* @param:	ITA_MATRIX_TYPE type	��������
* @param:	float *tempMatrix		���Ŀ��������¶Ⱦ���
* @param:	int matrixSize			���󻺴��С������С��Ŀ������w*h����λfloat��
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_RefrigerCalcTempMatrix(HANDLE_T instance, short* y16Array, int y16W, int y16H,
	int x, int y, int w, int h, float distance, float emiss, ITA_MATRIX_TYPE type, float *tempMatrix, int matrixSize);

/**
* @brief:	�����¶ȷ���Y16��
* @param:	HANDLE_T instance			ITA����̽��������ģ��ʵ����
* @param:	float temp				    �����¶ȡ�
* @param:	short *y16value	            ���y16ֵ��
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_RefrigerCalcY16(HANDLE_T instance, float temp, short *y16value);

/**
* @brief:	�����¶Ⱦ��󷴲�y16����
* @param:	HANDLE_T instance		ITA����̽��������ģ��ʵ����
* @param:	float *tempMatrix		�����¶Ⱦ���
* @param:	int w					��
* @param:	int h					��
* @param:	short *y16Matrix		���Y16����
* @param:	int matrixSize			���󻺴��С
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_RefrigerCalcY16Matrix(HANDLE_T instance, float *tempMatrix, int w, int h, short *y16Matrix, int matrixSize);

/**
* @brief:	�ж��Ƿ��л����·�Χ��
* @param:	HANDLE_T instance		ITA����̽��������ģ��ʵ����
* @param:	short *pSrc				ԴY16ͼ���ڴ����û�����
* @param:	width, height			ԴY16ͼ��ֱ��ʡ�
* @param:	ITA_RANGE range			��ǰ���·�Χ��
* @param:	areaTh1, areaTh2		��ֵ��������Χ��ֱ�ο�73/10000��9985/10000��
* @param:	low2high, high2low 		��ֵ��������Χ��ֱ�ο�140��130��
* @param:	int *isChange 		    ���������1���л����·�Χ��0�����ֲ��䡣
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_RefrigerIsChangeRange(HANDLE_T instance, short *pSrc, int width, int height, ITA_RANGE range,
	float areaTh1, float areaTh2, int low2high, int high2low, int *isChange);

/**
* @brief:	�����������õ��Բ�����
* @param:	HANDLE_T instance			ITAʵ����
* @param:	ITA_DEBUG_TYPE type		    �������͡�
* @param:	void *param					����ֵ������ַ��
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_RefrigerDebugger(HANDLE_T instance, ITA_DEBUG_TYPE type, void* param);

/**
* @brief:	����K���󣬹̶����׾���
* @param:	HANDLE_T instance			ITAʵ����
* @param:	short* stationaryB			�̶����׾����ڴ����û����롣
* @param:	short* kMatrix				K�����ڴ����û����롣
* @param:	int length					���󳤶�,��λ��short��
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_RefrigerSetMatrixData(HANDLE_T instance, short* stationaryB, short* kMatrix, int length);

/*
* @brief:	����Y16����ת����
* @param:	HANDLE_T instance			ITAʵ����
* @param:	short* srcY16			    ԭʼY16����
* @param:	short* dstY16				ת�����Y16����
* @param:	short averageB				���׾�ֵ��
* @param:	short* realB				ʵʱ���׾���
* @param:	short* stationaryB			�̶����׾����ڴ����û����롣
* @param:	short* kMatrix				K�����ڴ����û����롣
* @param:	int width					ͼ���
* @param:	int height					ͼ��ߡ�
* @param:	bool mode					true����ʵʱ����ת�̶����ף�false: �����෴��
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
*/
ITA_API ITA_RESULT ITA_RefrigerY16MatrixConvert(HANDLE_T instance, short* srcY16, short* dstY16, short averageB, short* realB, short* stationaryB, short* kMatrix, int width,int height,bool mode);

/*
* @brief:	����Y16����ת����
* @param:	HANDLE_T instance			ITAʵ����
* @param:	short srcY16			    ԭʼY16��
* @param:	short* dstY16				ת�����Y16��
* @param:	short stationaryB			�̶�����ֵ��
* @param:	short k						Kֵ��
* @param:	short averageB				ʵʱ���׾�ֵ��
* @param:	short realB					ʵʱ����ֵ��
* @param:	bool mode					true����ʵʱ����ת�̶����ף�false: �̶�����תʵʱ���ס�
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
*/
ITA_API ITA_RESULT ITA_RefrigerY16SingleConvert(HANDLE_T instance, short srcY16, short* dstY16, short stationaryB,short averageB, short realB,short k, bool mode);

ITA_REFRIGER_END

#endif // !ITA_REFRIGER_H
