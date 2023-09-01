/*************************************************************
Copyright (C), 2021--2022, Wuhan Guide Sensmart Tech Co., Ltd
File name   : ITA.h
Author      : Guide Sensmart RD. wangyan
Version     : 1.0
Date        : 2021/11/9
Description : ITA SDK interface definition.
*************************************************************/
#ifndef ITA_LIB_H
#define ITA_LIB_H

#ifdef __cplusplus
#    define ITA_LIB_START  extern "C" {
#    define ITA_LIB_END    };
#else
#    define ITA_LIB_START
#    define ITA_LIB_END
#endif

ITA_LIB_START

#include "ITADTD.h"

/**
* @brief:	��ʼ��һ��������ITA����ʵ����ITA��image and temperature measurement algorithm����
* @param:	ITA_MODE mode	ģʽ��X16��̽���������ԭʼ���ݣ���Y16��ǰ��ͼ���㷨����������ݣ���
* @param:	ITA_RANGE range	��Χ����������¡���ҵ���³��¶κ͹�ҵ���¸��¶Ρ���ʼ��ʱ���û�ָ�����·�Χ��Ҳ����ʵʱ�л����·�Χ��
* @param:	int width		�ֱ��� ��
* @param:	int height		�ֱ��� ��
* @param:	int cameraID	camera id���û����룬���������豸��֧��ͬʱ�򿪶�̨�豸��
* @return:	�ɹ�������ʵ�������ʧ�ܣ�����NULL��
* @see ITA_SetMeasureRange��ʵʱ�л����·�Χ��ITA_ShutterControl��cameraID������ʶ�豸�����ݸ�HAL�ӿڡ�
**/
ITA_API HANDLE_T ITA_Init(ITA_MODE mode, ITA_RANGE range, int width, int height, int cameraID);

/**
* @brief:	�ͷ�һ��������ITA����ʵ����Դ��
* @param:	HANDLE_T instance	ITAʵ����
* @return:	�ޡ�
**/
ITA_API void ITA_Uninit(HANDLE_T instance);

/**
* @brief:	ע����ơ���ITA_Init֮����á��������㲻ͬ��Ʒ�Ķ���������ǿ����չ�ԡ�
* @param:	HANDLE_T instance		ITAʵ����
* @param:	ITARegistry *registry	�û�ע�ắ����
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_Register(HANDLE_T instance, ITARegistry *registry);

/**
* @brief:  �л����·�Χ��
* @param:  HANDLE_T instance	ITAʵ����
* @param:  ITA_RANGE range		��Χ����������¡���ҵ���³��¶κ͹�ҵ���¸��¶Ρ�
* @return: ����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_SetMeasureRange(HANDLE_T instance, ITA_RANGE range);

/**
* @brief:	�ڳ�ʼ������̽����֮ǰ���û�������������̽������������֧�ֶ�̬���ã������ITA_ConfigureDetector֮ǰ���á�
* @param:	HANDLE_T instance			ITAʵ����
* @param:	ITA_DC_TYPE type			�������͡�
* @param:	void *param					����ֵ������ַ��
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_DetectorControl(HANDLE_T instance, ITA_DC_TYPE type, void *param);

/**
* @brief:	����̽����ʱ�Ӳ���������̽��������ITA_Register֮����á�
* @param:	HANDLE_T instance		ITAʵ����
* @param:	int clock				̽����ʱ�Ӳ�������λHz������12000000Hz��256ģ��֧��12M��6M����ʱ�ӡ�120ģ��ʱ��ͨ����5M��������8M��
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_ConfigureDetector(HANDLE_T instance, int clock);

/**
* @brief:	��ȡͼ����Ϣ��
* @param:	HANDLE_T instance		ITAʵ����
* @param:	ITAImgInfo *pInfo		ͼ����Ϣ�����������
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
* @note:	
**/
ITA_API ITA_RESULT ITA_Prepare(HANDLE_T instance, ITAImgInfo *pInfo);

/**
* @brief:	ͼ���źŴ�������X��Y�����ͼ����������ݡ����ӿڷ��سɹ�ʱ����֡ͼ�������ʾ��
* @param:	HANDLE_T instance		ITAʵ����
* @param:	unsigned char *srcData	�����Դ���ݣ�X16����Y16���ڴ����û�������ͷš�
* @param:	int srcSize				Դ���ݳ��ȣ���λByte��
* @param:	ITAISPResult *pResult	���������imgDst��y16Dataʹ��SDK������ڴ棬����Ҫ�û����롣�û���Ҫ�޸�pResult��ֵ��
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
            ʧ�ܿ��������¼��������ITA_SHUTTER_CLOSING��ITA_FIRST_NUC_NOT_FINISH��ITA_SKIP_FRAME�ȡ�
* @note:	�������srcData���ڴ����û�������ͷš��������pResult���ڴ���ITA������û���Ҫ�޸�pResult��ֵ��
**/
ITA_API ITA_RESULT ITA_ISP(HANDLE_T instance, unsigned char *srcData, int srcSize, ITAISPResult *pResult);

/**
* @brief:	����Y16ֵ�����¶ȡ�
* @param:	HANDLE_T instance		ITAʵ����
* @param:	short y16				Y16ֵ��
* @param:	float *surfaceTemp		��������¶ȡ�
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_MeasureTempPoint(HANDLE_T instance, short y16, float *surfaceTemp);

/**
* @brief:	�����¶Ⱦ���
* @param:	HANDLE_T instance		ITAʵ����
* @param:	short* y16Array			Y16�����������ת�ͷ�ת����ô������ת�ͷ�ת���Y16���ݡ�
* @param:	y16W,y16H				Y16����ֱ��ʡ��������ת�ͷ�ת����ô������ת�ͷ�ת��ķֱ��ʡ�
* @param:	x,y,w,h					Ŀ������������������ת����ôע�ⲻҪԽ�磬����ᱨ��
* @param:	float distance			Ŀ����룬��λ���ס���ǰ���������Ч����ɺ󽫻ָ�����ǰ��ֵ��
* @param:	ITA_MATRIX_TYPE type	��������
* @param:	float *tempMatrix		���Ŀ��������¶Ⱦ���
* @param:	int matrixSize			���󻺴��С������С��Ŀ������w*h����λfloat��
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_MeasureTempMatrix(HANDLE_T instance, short* y16Array, int y16W, int y16H,
				int x, int y, int w, int h, float distance, ITA_MATRIX_TYPE type, float *tempMatrix, int matrixSize);

/**
* @brief:	�������ơ�
* @param:	HANDLE_T instance			ITAʵ����
* @param:	ITA_BC_TYPE type			�������͡�
* @param:	void *param					����ֵ������ַ��
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_BaseControl(HANDLE_T instance, ITA_BC_TYPE type, void *param);

/**
* @brief:	���ò��²�����
* @param:	HANDLE_T instance			ITAʵ����
* @param:	ITA_MC_TYPE type			�������͡�
* @param:	void *param					����ֵ������ַ��
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_MeasureControl(HANDLE_T instance, ITA_MC_TYPE type, void *param);

/**
* @brief:	���ó��������
* @param:	HANDLE_T instance			ITAʵ����
* @param:	ITA_IC_TYPE type		    �������͡�
* @param:	void *param					����ֵ������ַ��
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_ImageControl(HANDLE_T instance, ITA_IC_TYPE type, void *param);

/**
* @brief:	��ȡ����Ͳ��²�����Ϣ��
* @param:	HANDLE_T instance		ITAʵ����
* @param:	ITAParamInfo *info		���������Ϣ��
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_GetParamInfo(HANDLE_T instance, ITAParamInfo *info);

/**
* @brief:	�����������õ��Բ�����
* @param:	HANDLE_T instance			ITAʵ����
* @param:	ITA_DEBUG_TYPE type		    �������͡�
* @param:	void *param					����ֵ������ַ��
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_Debugger(HANDLE_T instance, ITA_DEBUG_TYPE type, void *param);

/**
* @brief:	�û��Զ���α������
* @param:	HANDLE_T instance			ITAʵ����
* @param:	ITA_PALETTE_OP_TYPE type	�û��Զ���α�ʲ������͡�
* @param:	void *param					����ֵ������ַ�����Ͳο�ITA_PALETTE_OP_TYPEע�͡�
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_CustomPalette(HANDLE_T instance, ITA_PALETTE_OP_TYPE type, void *param);

/**
* @brief:	���������Y8��α�����ɺ���ͼ�������ͼ���ʽΪRGB888��
* @param:	HANDLE_T instance				ITAʵ����
* @param:	unsigned char *pY8				�����y8
* @param:	int w							y8��
* @param:	int h							y8��
* @param:	unsigned char *pPalette			α�����ݣ�RGBA��ʽ��ÿ��α�����ݳ���256*4B��
* @param:	int paletteLen					α�ʳ���
* @param:   unsigned char *pImage			�����ͼ������
* @param:   int imgLen						ͼ�񻺴��С
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_GetColorImage(HANDLE_T instance, unsigned char *pY8, int w, int h, unsigned char *pPalette, int paletteLen, unsigned char *pImage, int imgLen);

/**
* @brief:	�����¶ȷ���y16��
* @param:	HANDLE_T instance		ITAʵ����
* @param:	float surfaceTemp		�����¶ȡ�
* @param:	short *y16				���Y16ֵ��
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_CalcY16ByTemp(HANDLE_T instance, float surfaceTemp, short *y16);

/**
* @brief:	�����¶Ⱦ��󷴲�y16����
* @param:	HANDLE_T instance		ITAʵ����
* @param:	float *tempMatrix		�����¶Ⱦ���
* @param:	int w					��
* @param:	int h					��
* @param:	short *y16Matrix		���Y16����
* @param:	int matrixSize			���󻺴��С
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_CalcY16MatrixByTemp(HANDLE_T instance, float *tempMatrix, int w, int h, short *y16Matrix, int matrixSize);

/**
* @brief:  ��ѯ�汾�źͰ汾������Ϣ����ͬ�İ汾�����Ĺ��ܿ��ܴ��ڲ��졣
* @param:  HANDLE_T instance		ITAʵ����
* @param:  ITA_VERSION_INFO *pVersion
* @return: ����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_Version(HANDLE_T instance, ITAVersionInfo *pVersion);

/**
* @brief: �û����²��������ݣ�ITA��������״ֵ̬���´�ֵ�͵�λ����Ϣ����ITA_ISP֮ǰ���á�
* @param: HANDLE_T instance					ITAʵ����
* @param: unsigned char *paramLine			���������ݡ�
* @param: int len							���ݳ��ȡ�
* @return: ����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
* @note:   Y16ģʽ�£��Զ����š�NUC���Ժ��Զ��е�һ������λ������ʵ�ֲ�ͨ�������д��ݣ�FPGA���⣩��
* �û�Ӧ�õ���ITA_UpdateParamLine���ݲ����и�ITA�����������һ���û�����ͨ������Э������ֶ����ź�NUC�����ص���ITA�ӿڡ�
**/
ITA_API ITA_RESULT ITA_UpdateParamLine(HANDLE_T instance, unsigned char *paramLine, int len);

/**
* @brief:	����������£���������¶ȼ��������¶ȡ�
* @param:	HANDLE_T instance		ITAʵ����
* @param:	float surfaceTemp		��������¶ȡ�
* @param:	float envirTemp			�����¶ȡ�
* @param:	float *bodyTemp			��������¶ȡ�
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
* @note ͨ��ITA_GET_ENVIRON��ȡ�ڲ�����Ļ��¡�ע��������ɢ�ȷ����Ӱ�죬�ڲ�����Ļ�����ʵ��ֵ��ƫ�
* һ����Ҫ��ȥƫ�����õ�ʵ�ʻ��¡������ƫ��ֵ���û����Զ�̨������õ���
**/
ITA_API ITA_RESULT ITA_MeasureBody(HANDLE_T instance, float surfaceTemp, float envirTemp, float *bodyTemp);

/**
* @brief:	���õ��ⷽʽ��
* @param:	HANDLE_T instance			ITAʵ����
* @param:	ITA_DRT_TYPE type			���ⷽʽ��
* @param:	ITADRTParam *param		    �ֶ�����������������ⷽʽ��NULL��
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_DimmingType(HANDLE_T instance, ITA_DRT_TYPE type, ITADRTParam *param);

/**
* @brief:  �����ߡ�
* @param:  HANDLE_T instance			ITAʵ����
* @param:  ITA_EQUAL_LINE_TYPE type     ����������
* @param:  ITAEqualLineParam param		�����߲���				
* @return: ����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_EqualLine(HANDLE_T instance, ITA_EQUAL_LINE_TYPE type, ITAEqualLineParam param);

/*������У����ӿ�*/

/**
* @brief: У���㡣
* @param: HANDLE_T instance			ITAʵ����
* @param: ITA_CORRECT_TYPE type		�������͡�
* @param: void *param				����ֵ������ַ��
* @return: ����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_CorrectBadPoints(HANDLE_T instance, ITA_CORRECT_TYPE type, void *param);

/*�������Զ�У�½ӿ�*/
/**
* @brief:  ��ʼ�ɼ����ݣ�����Լ1�������ң���ɺ�ͨ���ص�����֪ͨ�����ӿ�ʹ�÷�������ʽ���������ء�
* @param:  HANDLE_T instance            ITAʵ����
* @param:  ITACollectParam collectParam �ɼ�����
* @param:  ITACollectFinish cb          �ɼ���ɺ�֪ͨ
* @param:  void *userParam              �û���������ITACollectFinishʱ�����û���
* @return: ����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_StartCollect(HANDLE_T instance, ITACollectParam collectParam, ITACollectFinish cb, void *userParam);

/**
* @brief: ��ʼ�Զ�У��(��TEC)��ע��У׼���ǵ�ǰ���·�Χ�Ĳ�����
* @param: HANDLE_T instance					ITAʵ����
* @param: float *kf							���У�²�����
* @param: float *b							���У�²�����
* @param: ITACalibrateResult *result		���У�½����
* @return: ����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_StartCalibrate(HANDLE_T instance, float *kf, float *b, ITACalibrateResult *result);


/**
* @brief:  �ֶ�У�¡�
* @param:  HANDLE_T instance			ITAʵ����
* @param:  ITA_RANGE range              ���·�Χ��
* @param:  ITA_CALIBRATE_TYPE type		У�²������͡�
* @param:  float p						����ֵ
* @return: ����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_CalibrateByUser(HANDLE_T instance, ITA_RANGE range, ITA_CALIBRATE_TYPE type, float p);

/**
* @brief:  ģ�鿪ʼ�ɼ�K����ɺ�ͨ���ص�����֪ͨ�����ӿ�ʹ�÷�������ʽ���������ء�
* @param:  HANDLE_T instance		ITAʵ����
* @param:  int gear     			���µ�λ����0��ʼ�����µ�λ����ͨ��ITAVersionInfo��ѯ��
* @param:  float blackTemp			�����¶�
* @param:  ITACollectBaseFinish cb      �ɼ���ɺ�֪ͨ
* @param:  void *userParam          �û���������ITACollectFinishʱ�����û���
* @return: ����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_CollectK(HANDLE_T instance, int gear, float blackTemp, ITACollectBaseFinish cb, void *userParam);

/**
* @brief:  ģ�鱣��K��ITA_CollectK�ɼ���ɺ��ٱ��档
* @param:  HANDLE_T instance		ITAʵ����
* @param:  int gear     			���µ�λ����0��ʼ�����µ�λ����ͨ��ITAVersionInfo��ѯ��
* @param:  ITACollectBaseFinish cb	�ɼ���ɺ�֪ͨ
* @param:  void* param				�û���������ITACollectFinishʱ�����û���
* @return: ����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_SaveK(HANDLE_T instance, int gear, ITACollectBaseFinish cb,void* param);

/**
* @brief:	ͼ���źŴ�������X��Y�����ͼ��������y8���ݡ����ӿڷ��سɹ�ʱ����֡ͼ�������ʾ��
* @param:	HANDLE_T instance		ITAʵ����
* @param:	unsigned char *srcData	�����Դ���ݣ�X16����Y16���ڴ����û�������ͷš�
* @param:	int srcSize				Դ���ݳ��ȣ���λByte��
* @param:	ITAISPResult *pResult	���������imgDst��y16Dataʹ��SDK������ڴ棬����Ҫ�û����롣�û���Ҫ�޸�pResult��ֵ��
* @param:	unsigned char *y8Data	y8���ݻ����ַ���ڴ����û�������ͷš�
* @param:	int y8Size				y8���ݻ����С����λByte��
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
ʧ�ܿ��������¼��������ITA_SHUTTER_CLOSING��ITA_FIRST_NUC_NOT_FINISH��ITA_SKIP_FRAME�ȡ�
* @note:	�������srcData���ڴ����û�������ͷš�y8Data���ڴ�Ҳ���û������������pResult���ڴ���ITA������û���Ҫ�޸�pResult��ֵ��
**/
ITA_API ITA_RESULT ITA_ISP_Y8(HANDLE_T instance, unsigned char *srcData, int srcSize, ITAISPResult *pResult, unsigned char *y8Data, int y8Size);

/**
* @brief:	���Ӻ�����������Ҫ����Androidƽ̨�߳���ͨ��postHandle֪ͨӦ�ò��ͷ���Դ��
* @param:	HANDLE_T instance	 	ʵ�������
* @param:	PostHandle postHandle	������
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
* @note:
**/
ITA_API ITA_RESULT ITA_AddInterceptor(HANDLE_T instance, PostHandle postHandle);

/**
* @brief:	�����ֳֺͻ�о��Ʒ���²�����Ϣ��
* @param:	HANDLE_T instance		ITAʵ����
* @param:	ITATECInfo info		    ���������Ϣ��
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_SetTECInfo(HANDLE_T instance, ITATECInfo info);

/**
* @brief:	��ȡ�ֳֺͻ�о��Ʒ���²�����Ϣ��
* @param:	HANDLE_T instance		ITAʵ����
* @param:	ITATECInfo *pInfo		���������Ϣ��
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_GetTECInfo(HANDLE_T instance, ITATECInfo *pInfo);

/**
* @brief:	���÷�TEC��Ʒ���²�����Ϣ��
* @param:	HANDLE_T instance		ITAʵ����
* @param:	ITAMeasureInfo info		���������Ϣ��
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_SetMeasureInfo(HANDLE_T instance, ITAMeasureInfo info);

/**
* @brief:	��ȡ��TEC��Ʒ���²�����Ϣ��
* @param:	HANDLE_T instance		ITAʵ����
* @param:	ITAMeasureInfo *Info	���������Ϣ��
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_GetMeasureInfo(HANDLE_T instance, ITAMeasureInfo* info);

/**
* @brief:	�������߲��£�����ͼƬ���������ܡ��û���ȡ���²������������ݺ󣬱������ݡ�
* @param:	HANDLE_T instance		ITAʵ��
* @param:	ITASceneParam *param	������²�������������
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_GetScene(HANDLE_T instance, ITASceneParam *param);

/**
* @brief:	�������߲��£�����ͼƬ���������ܡ��û�����һ����ITAʵ�������ò��²������������ݺ󣬿�ʼ���¡�
* @param:	HANDLE_T instance		ITAʵ��
* @param:	ITASceneParam param		������²�������������
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_SetScene(HANDLE_T instance, ITASceneParam param);

/**
* @brief:	Y16�Ŵ��ܡ����㳬�ֱ����¶Ⱦ���ʱʹ�á����֧�ַŴ�20����
* @param:	HANDLE_T instance		ITAʵ����
* @param:	short* srcY16			ԴY16�����ڴ����û�����
* @param:	srcW, srcH				ԴY16����ֱ��ʡ�
* @param:	short* dstY16			Ŀ��Y16�����ڴ����û�����
* @param:	float times 			�Ŵ�����
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_ZoomY16(HANDLE_T instance, short* srcY16, int srcW, int srcH,
				short* dstY16, float times);

/**
* @brief:	�ж��Ƿ��л����·�Χ�������û����ټ��1s���ü��һ�Ρ�
* @param:	HANDLE_T instance		ITAʵ����
* @param:	short *pSrc				ԴY16ͼ���ڴ����û�����
* @param:	width, height			ԴY16ͼ��ֱ��ʡ�
* @param:	ITA_RANGE range			��ǰ���·�Χ��
* @param:	areaTh1, areaTh2		��ֵ��������Χ��ֱ�ο�73/10000��9985/10000��
* @param:	low2high, high2low 		��ֵ��������Χ��ֱ�ο�140��130��
* @param:	int *isChange 		    ���������1���л����·�Χ��0�����ֲ��䡣
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_IsChangeRange(HANDLE_T instance, short *pSrc, int width, int height, ITA_RANGE range,
	            float areaTh1, float areaTh2, int low2high, int high2low, int *isChange);

/**
* @brief:	�ж϶൲λ�Ƿ��л����·�Χ�������û����ټ��1s���ü��һ�Ρ�
* @param:	HANDLE_T instance		ITAʵ����
* @param:	short *pSrc				ԴY16ͼ���ڴ����û�����
* @param:	width, height			ԴY16ͼ��ֱ��ʡ�
* @param:	ITA_RANGE range			��ǰ���·�Χ��
* @param:	areaTh1, areaTh2		��ֵ��������Χ��ֱ�ο�73/10000��9985/10000��
* @param:	low2mid					��ֵ���������µ��и��µ��¶���ֵ140��
* @param:	mid2low 				��ֵ���������µ��е��µ��¶���ֵ130��
* @param:	mid2high 				��ֵ���������µ��и��µ��¶���ֵ600��
* @param:	high2mid 				��ֵ���������µ��е��µ��¶���ֵ580��
* @param:	areaTh1, areaTh2		��ֵ��������Χ�ֱ�ο�0.0020��0.9995��
* @param:	int *isChange 		    ���������-1��ʾ�����е���0��ʾ���е���1��ʾ�����е���
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_IsMultiChangeRange(HANDLE_T instance, short* pSrc, int width, int height, ITA_RANGE range,
	float areaTh1, float areaTh2, int low2mid, int mid2low, int mid2high, int high2mid, int* isChange);

/**
* @brief:	���ַŴ�,����Y8/Y16��	ʵ��ͼ�����ݵĳ��ַŴ��ܡ����֧�ַŴ�8����
* @param:	HANDLE_T instance	 	ʵ�������
* @param:	void* dst				�����Ŀ�����ݡ���������Ϊshort����unsigned char���ڴ����û�������ͷš�
* @param:	void* src				�����Դ����Y8/Y16����������Ϊshort����unsigned char���ڴ����û�������ͷš�
* @param:	int w					�����Դͼ�����ݵĿ�ȡ�
* @param:	int h					�����Դͼ�����ݵĸ߶ȡ�
* @param:	int zoom				���ַŴ�����
* @param:	ITA_DATA_TYPE dataType	ָ��������������͡�
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
* @note:
**/
ITA_API ITA_RESULT ITA_SrBicubic(HANDLE_T instance, void* dst, void* src, int w, int h, int zoom, ITA_DATA_TYPE dataType);

/**
* @brief:	���������Y8��α�ʺ�����ָ����ʽ�ĺ���ͼ����ת�Ƕ�,ͼ���ʽ,�Ŵ���,�Ŵ�����,��α�ʺ���ITA_ImageControl�ӿ�ָ��
* @param:	HANDLE_T instance	 			ʵ�������
* @param:	unsigned char* src				�����Y8ͼ�����ݡ��ڴ����û�������ͷš�
* @param:	int srcW						�����Y8ͼ�����ݵĿ�
* @param:	int srcH						�����Y8ͼ�����ݵĸߡ�
* @param:	unsigned char* dst				����Ĵ�α��ͼ������,�ڴ����û�������ͷš�
* @param:	int dstW					    �����Y8ͼ�����ݵĿ�ȡ�
* @param:	int dstH					    �����Y8ͼ�����ݵĸ߶ȡ�
* @param:	int dataLen					    �����Y8ͼ�����ݵ����ݳ��ȡ�
* @param:	ITA_Y8_DATA_MODE datamode		����Y8������Դ��ITA_Y8_NORMAL y8ȡֵ��Χ0-255��ITA_Y8_XINSHENG:16-235��
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
* @note:
**/
ITA_API ITA_RESULT ITA_ConvertY8(HANDLE_T instance, const unsigned char* src, int srcW, int srcH, unsigned char* dst, int dstW, int dstH,int dateLen, ITA_Y8_DATA_MODE datamode = ITA_Y8_NORMAL);

/**
* @brief:	ȥ���ǲ�����
* @param:	HANDLE_T instance			ITAʵ����
* @param:	ITA_GUOGAI_OP_TYPE type		�û�ȥ���ǵĲ������͡�
* @param:	void *param					����ֵ������ַ�����Ͳο�ITA_POTCOVER_OP_TYPE��ע�͡�
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_PotCoverRestrain(HANDLE_T instance, ITA_POTCOVER_OP_TYPE type, void* para);

/**
* @brief:  ��ʼ�����¶ȣ���ɺ�ͨ���ص�����֪ͨ�����ӿ�ʹ�÷�������ʽ���������ء�
* @param:  HANDLE_T instance		ITAʵ����
* @param:  float temper				�����¶�
* @param:  ITAReviewFinish cb		�û�ע���֪ͨ����
* @param:  void* userParam			�û�����
* @return: ����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_StartReview(HANDLE_T instance, float temper, ITAReviewFinish cb, void* userParam);

/**
* @brief:	�Զ�У��,�Զ�������µ�KF��B1����
* @param:	HANDLE_T instance									ITAʵ����
* @param:	float *pBlackBodyTemp								�������ĵ��¶�
* @param:	short *pY16											�����Ӧ��Y16
* @param:	int arrLength										�����¶����顢Y16���鳤��
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_AutoCorrectTemp(HANDLE_T instance, float *pBlackBodyTemp, short *pY16, int arrLength);

/**
* @brief:    ����һ����������
* @param:    HANDLE_T instance           ITAʵ����
* @param:    ITAShape* shape             һ�����飬����һ�����������ζ���
* @param:    int size                    ����ĳ��ȡ�
* @return:   ����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����
**/
ITA_API ITA_RESULT ITA_AnalysisAdd(HANDLE_T instance, ITAShape* shape, int size);

/**
* @brief:    ��ȡ�������������ڵ����롣
* @param:    HANDLE_T instance           ITAʵ����
* @param:    ITARangeMask* rangeMask	 ����ֵ�������������������롣������ֵΪ255,������ֵΪ0.�ڴ����û����롣
* @param:    int size					 ���������������鳤�ȡ�
* @return:   ����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����
**/
ITA_API ITA_RESULT ITA_AnalysisGetRangeMask(HANDLE_T instance, ITARangeMask* rangeMask,int size);

/**
* @brief:    ��շ�������
* @param:    OA_HANDLE_T instance            ITAʵ����
* @return:   ����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����
**/
ITA_API ITA_RESULT ITA_AnalysisClear(HANDLE_T instance);

/**
* @brief:    �Է��������ڲ�������¶������ദ��,��ȡ����������¶���Ϣ(���ֵ,��Сֵ,ƽ��ֵ)��
* @param:    HANDLE_T instance                 ITAʵ����
* @param:    short* y16Data                    Y16����
* @param:	 int width  int height			   Y16ͼ��ֱ��ʡ�
* @param:	 ITAAnalysisTemp* analysisTemp	   �������:����������¶����飬�ڴ����û�������ͷ�
* @param:	 int size						   ����������¶�����(analysisTemp)�Ĵ�С(size������������ӵķ������������ͬ)
* @return:   ����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����
**/
ITA_API ITA_RESULT ITA_AnalysisTemp(HANDLE_T instance, short* y16Data, int width, int height, ITAAnalysisTemp* analysisTemp, int size);

/**
* @brief:    �Է��������ڲ������Y16�����ദ��,��ȡ���������Y16��Ϣ(���ֵ,��Сֵ,ƽ��ֵ)��
* @param:    HANDLE_T instance							ITAʵ����
* @param:    short* y16Data								Y16���ݡ�
* @param:	 int width  int height						Y16ͼ��ֱ��ʡ�
* @param:    ITAAnalysisY16Info* analysisY16Info        �������:�����ķ�������Y16����,����ÿ���������������Y16,��СY16�Լ�ƽ��Y16,�ڴ����û������ͷ�
* @param:	 int size									���������Y16��Ϣ����(analysisY16Info)�Ĵ�С(size������������ӵķ������������ͬ)
* @return:   ����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����
**/
ITA_API ITA_RESULT ITA_AnalysisY16Info(HANDLE_T instance, short* y16Data, int width, int height, ITAAnalysisY16Info* analysisY16Info, int size);

/**
* @brief:    �ж�һ�����Ƿ���һ�����������ڲ���
* @param:    ITAShape* shape             һ����������(�ڲ�����һ������εĶ������)
* @param:    ITAPoint point              һ�������ꡣ
* @param:    int *flag              	 0����ʾ�㲻�ڷ��������ڡ�  1����ʾ���ڷ��������ڡ�
* @return:   ����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����
**/
ITA_API ITA_RESULT ITA_PointInPolygon(ITAShape* shape, ITAPoint point, int* flag);

ITA_LIB_END

#endif // !ITA_LIB_H

