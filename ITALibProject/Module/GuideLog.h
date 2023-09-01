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
	DBG_OUTPUT_FILE = 1,  //日志输出到文件，默认输出方式
	DBG_OUTPUT_CONSOLE = 2,  //输出到控制台
	DBG_OUTPUT_FILE_EX = 3,  //用户自定义日志函数的情况下，同时输出到文件。
	DBG_OUTPUT_ETHERNET = 4,  //使用网络输出，目前不支持
}DBG_MODE_T;

typedef enum
{
	LOG_DEBUG = 1,	// 输出 Debug 级别信息 
	LOG_INFO = 2,	// 输出 Info 级别信息 
	LOG_WARN = 3,	// 输出 Warning 级别信息 
	LOG_ERROR = 4,	// 输出 Error 级别信息
	LOG_FATAL = 5,	// 输出 Fatal Error 级别信息 
}LOG_LEVEL_T;

typedef void(*OutputFunc)(int cameraID, const char* logInfo, void* userParam);

class GuideLog
{
public:
	GuideLog();
	~GuideLog();

	/**
	* @brief 设置日志文件保存路径。如果不设置，日志保存在程序同级目录下。
	* @param
	*  path	文件路径
	* @return
	* GUIDEIR_OK : 设置成功
	* GUIDEIR_ERR : 设置失败
	*/
	INT32_T setPath(CONST CHAR_T* path);

	// void updatePath(CONST CHAR_T* path);

	// void setFileSize(int size);

	// void setFileNumber(int number);

	/**
	* @brief 设置日志输出模式。如果不设置，默认输出日志文件。
	* @param
	*  mode	日志模式
	* @return
	* GUIDEIR_OK : 设置成功
	* GUIDEIR_ERR : 设置失败
	*/
	INT32_T setMode(DBG_MODE_T mode);

	/**
	* @brief 注册一个日志输出函数。日志信息用这个函数输出。
	* @param
	*  path	文件路径
	* @return 无
	*/
	VOID registerOutputFunc(OutputFunc func, int cameraID, void* userParam);

	/**
	* @brief 按调试等级输出日志。按照设计要求支持单线程调用，不支持多线程。
	* @param level 日志等级
	* @param fmt 格式化字符串。参数说明参照标准的printf函数。
	* @return
	* > 0, 实际输出的字符数。
	* = 0, GUIDEIR_OK，不满足等级条件不输出。
	* < 0，GUIDEIR_ERR，参数非法或者日志输出文件写入失败。
	* @note 每次打印一行日志上限是8KB。
	*/
	INT32_T output(LOG_LEVEL_T level, CONST CHAR_T* fmt, ...);

	//限制日志占用的存储空间和单个日志文件大小。
private:
	// CHAR_T *log_path;
	DBG_MODE_T m_mode;						//日志输出方式，FILE 或者 CONSOLE。
	CHAR_T* m_msg;							//一次至多打印MSG_SIZE字节日志
	// HANDLE_T  fp_print;						//日志文件句柄
	OutputFunc m_func;
	int m_cameraID;
	void* m_userParam;
	// unsigned long m_logSize;	//日志文件大小上限：默认5*1024*1024B
	// unsigned long m_sizeCount;//日志大小计数器。
	// int m_fileNum;//日志文件数量计数器
	// std::string prefix;//文件前缀名
	// std::string suffix;//文件后缀名
	// std::string dir;//文件目录
	// std::string fileNameArray[10];
	// int indexArray;
	// unsigned long m_singleSize;
	// int m_totalNum;//支持的日志文件数量，默认是5，最大支持10个日志文件,用户可进行设置。
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

