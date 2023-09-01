/*************************************************************
Copyright (C), 2021--2022, Wuhan Guide Sensmart Tech Co., Ltd
File name   : GuideLog.h
Author      : Guide Sensmart RD. wangyan
Version     : 1.0
Date        : 2021/11/9
Description : Log module.
*************************************************************/
#ifndef ITA_LOG_H
#define ITA_LOG_H

#include "../ERROR.h"
#include "PortingAPI.h"
#include <string>

#include "ita_log.h"

typedef enum
{
	DBG_OUTPUT_FILE = 1,  //��־������ļ���Ĭ�������ʽ
	DBG_OUTPUT_CONSOLE = 2,  //���������̨
	DBG_OUTPUT_FILE_EX = 3,  //�û��Զ�����־����������£�ͬʱ������ļ���
	DBG_OUTPUT_ETHERNET = 4,  //ʹ�����������Ŀǰ��֧��
}DBG_MODE_T;

typedef enum
{
	LOG_DEBUG = 1,	// ��� Debug ������Ϣ 
	LOG_INFO = 2,	// ��� Info ������Ϣ 
	LOG_WARN = 3,	// ��� Warning ������Ϣ 
	LOG_ERROR = 4,	// ��� Error ������Ϣ
	LOG_FATAL = 5,	// ��� Fatal Error ������Ϣ 
}LOG_LEVEL_T;

typedef void(*OutputFunc)(int cameraID, const char* logInfo, void* userParam);

class GuideLog
{
public:
	GuideLog();
	~GuideLog();

	/**
	* @brief ������־�ļ�����·������������ã���־�����ڳ���ͬ��Ŀ¼�¡�
	* @param
	*  path	�ļ�·��
	* @return
	* GUIDEIR_OK : ���óɹ�
	* GUIDEIR_ERR : ����ʧ��
	*/
	INT32_T setPath(CONST CHAR_T* path);

	// void updatePath(CONST CHAR_T* path);

	// void setFileSize(int size);

	// void setFileNumber(int number);

	/**
	* @brief ������־���ģʽ����������ã�Ĭ�������־�ļ���
	* @param
	*  mode	��־ģʽ
	* @return
	* GUIDEIR_OK : ���óɹ�
	* GUIDEIR_ERR : ����ʧ��
	*/
	INT32_T setMode(DBG_MODE_T mode);

	/**
	* @brief ע��һ����־�����������־��Ϣ��������������
	* @param
	*  path	�ļ�·��
	* @return ��
	*/
	VOID registerOutputFunc(OutputFunc func, int cameraID, void* userParam);

	/**
	* @brief �����Եȼ������־���������Ҫ��֧�ֵ��̵߳��ã���֧�ֶ��̡߳�
	* @param level ��־�ȼ�
	* @param fmt ��ʽ���ַ���������˵�����ձ�׼��printf������
	* @return
	* > 0, ʵ��������ַ�����
	* = 0, GUIDEIR_OK��������ȼ������������
	* < 0��GUIDEIR_ERR�������Ƿ�������־����ļ�д��ʧ�ܡ�
	* @note ÿ�δ�ӡһ����־������8KB��
	*/
	INT32_T output(LOG_LEVEL_T level, CONST CHAR_T* fmt, ...);

	//������־ռ�õĴ洢�ռ�͵�����־�ļ���С��
private:
	// CHAR_T *log_path;
	DBG_MODE_T m_mode;						//��־�����ʽ��FILE ���� CONSOLE��
	CHAR_T* m_msg;							//һ�������ӡMSG_SIZE�ֽ���־
	// HANDLE_T  fp_print;						//��־�ļ����
	OutputFunc m_func;
	int m_cameraID;
	void* m_userParam;
	// unsigned long m_logSize;	//��־�ļ���С���ޣ�Ĭ��5*1024*1024B
	// unsigned long m_sizeCount;//��־��С��������
	// int m_fileNum;//��־�ļ�����������
	// std::string prefix;//�ļ�ǰ׺��
	// std::string suffix;//�ļ���׺��
	// std::string dir;//�ļ�Ŀ¼
	// std::string fileNameArray[10];
	// int indexArray;
	// unsigned long m_singleSize;
	// int m_totalNum;//֧�ֵ���־�ļ�������Ĭ����5�����֧��10����־�ļ�,�û��ɽ������á�
#ifdef MULTI_THREAD_FUNCTION
	HANDLE_T m_mutex;
#endif // MULTI_THREAD_FUNCTION

	ita_logger_t* m_logger;
};


#define LOGD(logger, fmt, ...) if (NULL == (logger)) {} else (logger)->output(LOG_DEBUG, fmt, ##__VA_ARGS__)
#define LOGI(logger, fmt, ...) if (NULL == (logger)) {} else (logger)->output(LOG_INFO,  fmt, ##__VA_ARGS__)
#define LOGW(logger, fmt, ...) if (NULL == (logger)) {} else (logger)->output(LOG_WARN,  fmt, ##__VA_ARGS__)
#define LOGE(logger, fmt, ...) if (NULL == (logger)) {} else (logger)->output(LOG_ERROR, fmt, ##__VA_ARGS__)
#define LOGF(logger, fmt, ...) if (NULL == (logger)) {} else (logger)->output(LOG_FATAL, fmt, ##__VA_ARGS__)


#define LOGD_IF(logger, bool_expression, fmt, ...) \
    if (NULL == (logger) || !(bool_expression)) {} else (logger)->output(LOG_DEBUG, fmt, ##__VA_ARGS__)

#define LOGI_IF(logger, bool_expression, fmt, ...) \
    if (NULL == (logger) || !(bool_expression)) {} else (logger)->output(LOG_INFO,  fmt, ##__VA_ARGS__)

#define LOGW_IF(logger, bool_expression, fmt, ...) \
    if (NULL == (logger) || !(bool_expression)) {} else (logger)->output(LOG_WARN,  fmt, ##__VA_ARGS__)

#define LOGE_IF(logger, bool_expression, fmt, ...) \
    if (NULL == (logger) || !(bool_expression)) {} else (logger)->output(LOG_ERROR, fmt, ##__VA_ARGS__)

#define LOGF_IF(logger, bool_expression, fmt, ...) \
    if (NULL == (logger) || !(bool_expression)) {} else (logger)->output(LOG_FATAL, fmt, ##__VA_ARGS__)



#endif // !ITA_LOG_H

