#ifndef ITA_MEASURETOOL_H
#define ITA_MEASURETOOL_H

#ifdef __cplusplus
#    define ITA_MEASURETOOL_START  extern "C" {
#    define ITA_MEASURETOOL_END    };
#else
#    define ITA_TOOLBOX_START
#    define ITA_TOOLBOX_END
#endif

ITA_MEASURETOOL_START

#include "ERROR.h"
#include "ITADTD.h"

/**
* @brief:	���¹�����ģ���ʼ����
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
* @note:
**/
ITA_API HANDLE_T ITA_MeasureToolInit();

/**
* @brief:	�ͷ�һ�����¹��ߣ�ITA����ʵ����Դ��
* @param:	HANDLE_T instance	ITAʵ����
* @return:	�ޡ�
**/
ITA_API void ITA_MeasureToolUnInit(HANDLE_T instance);

/**
* @brief:	�����ļ����ݵ���,�û��Զ��屣���ļ���
* @param:	HANDLE_T instance									ITA������ʵ����
* @param:	ITAConfig* config									�����ļ�����Ĳ����ṹ�塣
* @param:	char* outputEncryStr								������������ɼ��ܵ������ļ��ַ������ڴ����û����롣
* @param:	int encryStrLen										�û���������ɼ��ܵ������ļ��ַ����ĳ��ȡ�
* @param:	int* length											������������ɼ��ܵ������ļ��ַ����ȡ�
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_ConfigFileExport(HANDLE_T instance, ITAConfig* config,char* outputEncryStr,int encryStrLen,int* length);


/**
* @brief:	�����ļ����ݵ���
* @param:	HANDLE_T instance									ITA������ʵ����
* @param:	char* inputStr										��������������ļ����ַ������ݡ�
* @param:	int length											��������������ļ����ַ������ȡ�
* @param:	ITAConfig* config								    ���������ITAConfig�ṹ�塣
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_ConfigFileImport(HANDLE_T instance, char* inputStr, int length, ITAConfig* config);
ITA_MEASURETOOL_END

#endif // !ITA_MEASURETOOL_H
