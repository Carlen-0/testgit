/*************************************************************
Copyright (C), 2022--2023, Wuhan Guide Sensmart Tech Co., Ltd
File name   : ITAToolBox.h
Author      : Guide Sensmart RD. wangyan
Version     : 1.0
Date        : 2023/2/13
Description : ITA ToolBox.
*************************************************************/
#ifndef ITA_TOOLBOX_H
#define ITA_TOOLBOX_H

#ifdef __cplusplus
#    define ITA_TOOLBOX_START  extern "C" {
#    define ITA_TOOLBOX_END    };
#else
#    define ITA_TOOLBOX_START
#    define ITA_TOOLBOX_END
#endif

ITA_TOOLBOX_START

#include "ERROR.h"
#include "ITADTD.h"

/* ��һ�������� */
typedef enum ITA_IFCONSISTENCY_TYPE
{
	ITA_IFCONSISTENCY_NQ,								//���ϸ�
	ITA_IFCONSISTENCY_QUALIFIED,						//�ϸ�
}ITAIfConsistencyType;

/* ��������ϵ������ģʽ */
typedef enum ITA_DISTANCECOFF_MODE
{
	ITA_DISTANCECOFF_6GROUP,							//6�����ϵ��
	ITA_DISTANCECOFF_9GROUP,							//9�����ϵ��
}ITADistanceCoffMode;

/* ��ӻ���ģʽ */
typedef enum ITA_ADDBADLINE_MODE
{
	ITA_ADDBADLINE_ROW,									//��ӻ���
	ITA_ADDBADLINE_COLUMN,								//��ӻ���
}ITAAddBadLineMode;

/* ��Ư��Ϣ�ṹ�� */
typedef struct ITA_DRIFT_INFO
{
	ITA_SHUTTER_STATUS   shutterStatus;				  //����״̬
	short y16;										  //Y16
	float realTimeShutterTemp;						  //ʵʱ������
	float realTimeLenTemp;							  //ʵʱ��Ͳ��
}ITADriftInfo;

/**
* @brief:	�Զ�������Ӧ�ʡ�
* @param:	HANDLE_T instance									ITA������ʵ����
* @param:	short lowY16										���º������ĵ�Y16
* @param:	short highY16										���º������ĵ�Y16
* @param:	int currentINT										��ǰ̽����INTֵ
* @param:	int *pOutINT										�������֮���INTֵ
* @param:	short responseRate									��ǰ��λ�Ͳ��·�Χ����Ҫ�����Ӧ��ֵ
* @param:	short precision										��ǰ��λ�Ͳ��·�Χ�����ܽ��ܵ���Ӧ�����
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_AutoResponseRate(HANDLE_T instance, short lowY16, short highY16, int currentINT, int *pOutINT, short responseRate, short precision);

/**
* @brief:	��K��
* @param:	HANDLE_T instance									ITA������ʵ����
* @param:	short *pLowBase										���±��ף����Ե��º���ʱ�ɼ���һ֡X16����
* @param:	short *pHighBase									���±��ף����Ը��º���ʱ�ɼ���һ֡X16����
* @param:	int width											ͼ���
* @param:	int height											ͼ���
* @param:	unsigned short *pKMatrix							�����K����
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_AutoGetKMatirx(HANDLE_T instance, short *pLowBase, short *pHighBase, int width, int height, unsigned short *pKMatrix);

/**
* @brief:	�Զ���ǻ��㡣
* @param:	HANDLE_T instance									ITA������ʵ����
* @param:	unsigned short *pKMatrix							����K���������Ǻ��K����
* @param:	int width											ͼ���
* @param:	int height											ͼ���
* @param:	unsigned short thresh								������ֵ��һ����Ϊ50
* @param:	int *pBadPointInfo									������㡢���С����С�������Ϣ�����鳤��Ϊ51
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_AutoCorrectBadPoint(HANDLE_T instance, unsigned short *pKMatrix, int width, int height, unsigned short thresh, int *pBadPointInfo);

/**
* @brief:	��ӻ��㡣
* @param:	HANDLE_T instance									ITA������ʵ����
* @param:	unsigned short *pKMatrix							����K���������ӻ�����K����
* @param:	int x												x����
* @param:	int y												y����
* @param:	int width											ͼ���
* @param:	int height											ͼ���
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_AddBadPoint(HANDLE_T instance, unsigned short *pKMatrix, int x, int y, int width, int height);

/**
* @brief:	��ӻ��ߡ�
* @param:	HANDLE_T instance									ITA������ʵ����
* @param:	unsigned short *pKMatrix							����K���������ӻ��ߺ��K����
* @param:	int lineIndex										ָ�����л���
* @param:	int width											ͼ���
* @param:	int height											ͼ���
* @param:	ITAAddBadLineMode mode								��ӻ��ߵ�ģʽ
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_AddBadLine(HANDLE_T instance, unsigned short *pKMatrix, int lineIndex, int width, int height, ITAAddBadLineMode mode);

/**
* @brief:	�ж���һ���ԡ�
* @param:	HANDLE_T instance									ITA������ʵ����
* @param:	float centerPointTemp								�������ĵ�ʵ���¶�
* @param:	float *pOtherPointTemp								�����������¶ȵ���¶�
* @param:	int otherPointNum									�����¶ȵ�ĸ���
* @param:	ITAIfConsistencyType *pResult						�����һ�����Ƿ�ϸ�
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_IfConsistency(HANDLE_T instance, float centerPointTemp, float *pOtherPointTemp, int otherPointNum, ITAIfConsistencyType *pResult);

/**
* @brief:	���ɲ������ߡ�
* @param:	HANDLE_T instance									ITA������ʵ����
* @param:	double *pTempMatrix									�������ĵ��¶Ⱦ���
* @param:	short *pY16											�¶Ⱦ����ӦY16
* @param:	int arrLength										�¶Ⱦ���Y16�����鳤��
* @param:	int maxTemp											����¶�
* @param:	short *pCurve										����Ĳ�������
* @param:	int curveLength										�������鳤�ȣ�һ�㳤��Ϊ16384
* @param:	int *pValidCurveLength								���ʵ�����ɵ����߳���
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_GenerateCurve(HANDLE_T instance, double *pTempMatrix, short *pY16, int arrLength, int maxTemp, short *pCurve, int curveLength,
	int *pValidCurveLength);

/**
* @brief:	�Զ�������Ư����������
* @param:	HANDLE_T instance									ITA������ʵ����
* @param:	ITADriftInfo *pDriftInfo							¼�Ƶ���Ư��Ϣ
* @param:	int driftInfoLength									��Ư��Ϣ���鳤��
* @param:	int *pShutterDrift									����Ŀ�����Ư
* @param:	int *pLenDrift										����ľ�Ͳ��Ư
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_AutoGetDrift(HANDLE_T instance, ITADriftInfo *pDriftInfo, int driftInfoLength, int *pShutterDrift, int *pLenDrift);

/**
* @brief:	�Զ������������������
* @param:	HANDLE_T instance									ITA������ʵ����
* @param:	float *pTempMatrix									��ͬ�����µ��¶Ⱦ������鳤��=�������x�������
* @param:	double *pDistance									�������飬���鳤��ΪnumOfDistance
* @param:	int numOfBlackBody									�������
* @param:	int numOfDistance									�������
* @param:	int standDistance									��׼���룬ȡֵ��Χ��>0; һ����1��3��5��7��9�׵ľ��룬ȡ��Ӧ��ֵ��ʾ�Ըþ���Ϊ��׼����
* @param:	int *pDistanceCoff									����ľ�������ϵ�����飬���鳤��Ϊ9
* @param:	ITADistanceCoffMode mode							��������ϵ������ģʽ��ITA_DISTANCECOFF_6GROUP��ʾ����ľ�������ϵ����Ч����Ϊ6��
*																ITA_DISTANCECOFF_9GROUP��ʾ����ľ�������ϵ����Ч����Ϊ9
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_AutoMeasureDistanceCoff(HANDLE_T instance, float *pTempMatrix, double *pDistance, int numOfBlackBody, int numOfDistance, 
	int standDistance, int *pDistanceCoff, ITADistanceCoffMode mode);

/**
* @brief:	�Զ����㻷������������
* @param:	HANDLE_T instance									ITA������ʵ����
* @param:	double *pStandardAmbientTemp						��׼�����µ��¶ȸ�������
* @param:	float standardAmbientTemp							��׼�����¶�
* @param:	double *pCurrentAmbientTemp							��ǰ�����µ��¶�ʵ������
* @param:	float currentAmbientTemp							��ǰ�����¶�
* @param:	int tempMatrixSize									�¶ȸ������ݡ��¶�ʵ�����ݵ����鳤��
* @param:	int *pK3											�����������������ϵ��(���㻯10000����Ĭ��0)
* @param:	int *pB2											�����������������ƫ����(���㻯10000����Ĭ��0)
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_AutoMeasureAmbientCoff(HANDLE_T instance, double *pStandardAmbientTemp, float standardAmbientTemp, 
	double *pCurrentAmbientTemp, float currentAmbientTemp, int tempMatrixSize, int *pK3, int *pB2);

/**
* @brief:	�Զ������¶�
*			Ŀ���¶ȴ���0��ʱ�����ա�2,��2%ȡ��ֵ�ı�׼��Ŀ���¶�С��0��ʱ�����ա�3�ı�׼
* @param:	HANDLE_T instance									ITA������ʵ����
* @param:	float *pStandTemp									Ŀ���¶�
* @param:	float *pMeasureTemp									ʵ���¶�
* @param:	int arrLength										Ŀ���¶����顢ʵ���¶����鳤��
* @param:	int *pNonConformTempIndex							���������ʵ���¶��в����ϸ��˱�׼������ֵ������ΪarrLength��
* @param:	int *pNonConformTempNum								���������ʵ���¶��в����ϸ��˱�׼������
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_AutoCheckTemperature(HANDLE_T instance, float *pStandTemp, float *pMeasureTemp, int arrLength, 
	int *pNonConformTempIndex, int *pNonConformTempNum);

ITA_TOOLBOX_END

#endif // !ITA_TOOLBOX_H