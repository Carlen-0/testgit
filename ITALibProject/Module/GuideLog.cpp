/*************************************************************
Copyright (C), 2021--2022, Wuhan Guide Sensmart Tech Co., Ltd
File name   : GuideLog.cpp
Author      : Guide Sensmart RD. wangyan
Version     : 1.0
Date        : 2021/11/9
Description : Log module.
*************************************************************/
#ifdef _WIN32
#pragma warning (disable: 4244)
#include <Windows.h>
#else
#include <sys/time.h> // for gettimeoffday
#endif

#include "GuideLog.h"
#include "../ERROR.h"
#include "PortingAPI.h"
#include "ita_log.h"
#include <cstddef>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

#define MSG_SIZE		512					//一行日志上限
#define PATH_SIZE		128					//路径长度上限

// GuideLog::GuideLog()
// {
// 	m_func = NULL;
// 	m_mode = DBG_OUTPUT_FILE;
// 	fp_print = NULL;
// 	m_msg = (CHAR_T*)porting_calloc_mem(MSG_SIZE, 1, ITA_OTHER_MODULE);
// 	log_path = (CHAR_T*)porting_calloc_mem(PATH_SIZE, 1, ITA_OTHER_MODULE);
// 	memcpy(log_path, "ita.log", strlen("ita.log"));
// 	m_singleSize = 5;
// 	m_logSize = m_singleSize * 1024 * 1024;//默认5M
// 	m_sizeCount = 0;
// 	m_fileNum = 1;
// 	m_totalNum = 5;
// 	dir = "";
// 	prefix = "ita";
// 	suffix = "log";
// 	indexArray = 0;
// 	fileNameArray[indexArray++] = "ita.log";
// #ifdef MULTI_THREAD_FUNCTION
// 	m_mutex = porting_mutex_create();
// #endif // MULTI_THREAD_FUNCTION
//     m_logger = ita_logger_create();
// }

// GuideLog::~GuideLog()
// {
// 	if (fp_print)
// 	{
// 		porting_file_close(fp_print);
// 		fp_print = NULL;
// 	}
// 	if (m_msg)
// 	{
// 		porting_free_mem(m_msg);
// 		m_msg = NULL;
// 	}
// 	if (log_path)
// 	{
// 		porting_free_mem(log_path);
// 		log_path = NULL;
// 	}
// #ifdef MULTI_THREAD_FUNCTION
// 	porting_mutex_destroy(m_mutex);
// #endif // MULTI_THREAD_FUNCTION
// }

// void GuideLog::setFileSize(int size)
// {
// 	m_singleSize = size;
// }

// void GuideLog::setFileNumber(int number)
// {
// 	if (number >= 10) {
// 		m_totalNum = 10;
// 		return;
// 	}
// 	m_totalNum = number;
// }

// void GuideLog::updatePath(CONST CHAR_T* path)
// {
// 	std::string temp = path;
// 	int index = (int)temp.find_last_of("/");
// 	int indexPoint = (int)temp.find_last_of(".");
// 	dir = temp.substr(0, index + 1);
// 	prefix = temp.substr(index + 1, indexPoint - index - 1);
// 	suffix = temp.substr(indexPoint + 1);
// }

// INT32_T GuideLog::setPath(CONST CHAR_T* path)
// {
// 	memset(log_path, '\0', PATH_SIZE);
// 	memcpy(log_path, path, strlen(path));
// 	updatePath(path);
// #ifdef MULTI_THREAD_FUNCTION
// 	porting_mutex_lock(m_mutex);
// #endif // MULTI_THREAD_FUNCTION
// 	if (fp_print)
// 	{
// 		porting_file_close(fp_print);
// 	}
// 	fp_print = porting_file_open(log_path, "w+");
// #ifdef MULTI_THREAD_FUNCTION
// 	porting_mutex_unlock(m_mutex);
// #endif // MULTI_THREAD_FUNCTION
// 	if (indexArray >= 1)
// 	{
// 		indexArray = 0;
// 		fileNameArray[indexArray++] = log_path;
// 	}
// 	return ITA_OK;
// }

INT32_T GuideLog::setMode(DBG_MODE_T mode)
{
    m_mode = mode;
    return ITA_OK;
}

VOID GuideLog::registerOutputFunc(OutputFunc func, int cameraID, void* userParam)
{
    m_func = func;
    m_cameraID = cameraID;
    m_userParam = userParam;
}

// INT32_T GuideLog::output(LOG_LEVEL_T level, CONST CHAR_T* fmt, ...)
// {
// 	CHAR_T* ptr;
// 	UINT32_T len = 0;
// 	INT32_T ret = ITA_ERROR;
// 	TIME_T time;
// #ifdef MULTI_THREAD_FUNCTION
// 	porting_mutex_lock(m_mutex);
// #endif // MULTI_THREAD_FUNCTION
// 	memset(m_msg, '\0', MSG_SIZE);
// 	ptr = m_msg;
// 	//每行日志开头加上日期时间
// 	porting_get_time(&time);
// 	len = sprintf(ptr, "%d/", time.year);
// 	ptr += len;
// 	len = sprintf(ptr, "%d/", time.month);
// 	ptr += len;
// 	len = sprintf(ptr, "%d ", time.day);
// 	ptr += len;
// 	len = sprintf(ptr, "%d:", time.hour);
// 	ptr += len;
// 	len = sprintf(ptr, "%d:", time.minute);
// 	ptr += len;
// 	len = sprintf(ptr, "%d:", time.second);
// 	ptr += len;
// 	len = sprintf(ptr, "%d ", time.milliseconds);
// 	ptr += len;

// 	switch (level)
// 	{
// 	case LOG_DEBUG:
// 		//_CRT_SECURE_NO_WARNINGS 忽略微软安全警告，未用sprintf_s
// 		len = sprintf(ptr, "DBG ");
// 		break;
// 	case LOG_INFO:
// 		len = sprintf(ptr, "INFO ");
// 		break;
// 	case LOG_WARN:
// 		len = sprintf(ptr, "WARN ");
// 		break;
// 	case LOG_ERROR:
// 		len = sprintf(ptr, "ERROR ");
// 		break;
// 	case LOG_FATAL:
// 		len = sprintf(ptr, "FATAL ");
// 		break;
// 	default:
// 		len = 0;
// 		break;
// 	}
// 	ptr += len;

// 	va_list args;
// 	va_start(args, fmt);
// 	//_CRT_SECURE_NO_WARNINGS 忽略微软安全警告，未用sprintf_s
// 	len = vsprintf(ptr, fmt, args);
// 	va_end(args);
// 	if (ptr - m_msg + len >= MSG_SIZE)
// 	{
// #ifdef MULTI_THREAD_FUNCTION
// 		porting_mutex_unlock(m_mutex);
// #endif // MULTI_THREAD_FUNCTION
// 		//printf("Too many logs in one line. %d fatal error. Upper limit is %d.\n", ptr - m_msg + len, MSG_SIZE);
// 		return ITA_LOG_BUF_OVERFLOW;
// 	}
// 	ptr += len;
// 	*ptr = '\n';
// 	ptr++;
// 	m_sizeCount += strlen(m_msg);
// 	if (m_sizeCount >= m_logSize)
// 	{
// 		/*拆分日志*/
// 		std::string fileName;
// 		fileName = dir + prefix + std::to_string(m_fileNum) + "." + suffix;
// 		memset(log_path, '\0', PATH_SIZE);
// 		memcpy(log_path, fileName.c_str(), fileName.size());
// 		if (m_fileNum >= m_totalNum && indexArray == m_totalNum) {
// 			indexArray = 0;
// 			remove(fileNameArray[indexArray].c_str());
// 			fileNameArray[indexArray++] = log_path;
// 		}
// 		else if (m_fileNum > m_totalNum && indexArray < m_totalNum) {
// 			remove(fileNameArray[indexArray].c_str());
// 			fileNameArray[indexArray++] = log_path;
// 		}
// 		else if (m_fileNum <= m_totalNum && indexArray < m_totalNum) {
// 			fileNameArray[indexArray++] = log_path;
// 		}
// 		m_fileNum++;
// 		m_sizeCount = 0;
// 		porting_file_close(fp_print);
// 		fp_print = porting_file_open(log_path, "w+");
// 	}
// 	//当用户注册日志函数后，SDK会优先使用注册的日志函数。
// 	if (m_func)
// 	{
// 		m_func(m_cameraID, m_msg, m_userParam);
// 		//当用户注册输出日志函数后，ITA内部不再输出日志。除非用户设置ITA_DEBUGGING_MODE为3。
// 		if (m_mode != DBG_OUTPUT_FILE_EX)
// 		{
// #ifdef MULTI_THREAD_FUNCTION
// 			porting_mutex_unlock(m_mutex);
// #endif // MULTI_THREAD_FUNCTION
// 			return ITA_OK;
// 		}
// 	}
// 	if (m_mode == DBG_OUTPUT_CONSOLE)
// 	{
// 		ret = printf("%s", m_msg);
// 		porting_file_flush((HANDLE_T)stdout);
// 	}
// 	else if (m_mode == DBG_OUTPUT_FILE || m_mode == DBG_OUTPUT_FILE_EX)
// 	{
// 		if (!fp_print)
// 		{
// 			//打开可读/写文件，若文件存在则文件长度清为零，即该文件内容会消失；若文件不存在则创建该文件。
// 			fp_print = porting_file_open(log_path, "w+");
// 			if (!fp_print)
// 			{
// 				memset(log_path, '\0', PATH_SIZE);
// 				memcpy(log_path, "ita.log", strlen("ita.log"));
// 				fp_print = porting_file_open(log_path, "w+");
// 				if (!fp_print)
// 				{
// #ifdef MULTI_THREAD_FUNCTION
// 					porting_mutex_unlock(m_mutex);
// #endif // MULTI_THREAD_FUNCTION
// 					return ITA_LOG_FILE_OPEN_FAILED;
// 				}
// 			}
// 		}
// 		ret = porting_file_write(fp_print, (const BYTE_T*)m_msg, ptr - m_msg);
// 		porting_file_flush(fp_print);
// 	}
// #ifdef MULTI_THREAD_FUNCTION
// 	porting_mutex_unlock(m_mutex);
// #endif // MULTI_THREAD_FUNCTION
// 	return ret;
// }

GuideLog::GuideLog()
    :m_func(nullptr),
    m_mode(DBG_OUTPUT_FILE),
    m_msg(nullptr),
    m_cameraID(0),
    m_userParam(nullptr),
    m_logger(nullptr)
{
    // IMPROVE: fix alloc fail
    m_msg = (CHAR_T*)porting_calloc_mem(MSG_SIZE, 1, ITA_OTHER_MODULE);
    // IMPROVE: fix alloc fail
    m_logger = ita_logger_create();

#ifdef MULTI_THREAD_FUNCTION
    m_mutex = porting_mutex_create();
#endif

    // set file size
    ita_logger_set_max_filesize(m_logger, 5 * 1024 * 1024); // 5 MB
    ita_logger_set_max_filecount(m_logger, 4);  // 5 = 4 + 1
    ita_logger_enable_flush(m_logger, 1); // fflush
    ita_logger_set_level(m_logger, LOG_DEBUG); // debug level
}

GuideLog::~GuideLog()
{
    if (m_msg)
    {
        porting_free_mem(m_msg);
        m_msg = nullptr;
    }

    if (m_logger)
    {
        ita_logger_destroy(m_logger);
        m_logger = nullptr;
    }

#ifdef MULTI_THREAD_FUNCTION
    porting_mutex_destroy(m_mutex);
#endif
}

INT32_T GuideLog::setPath(CONST CHAR_T* path)
{
    ita_logger_set_file(m_logger, path);
    return ITA_OK;
}

INT32_T GuideLog::output(LOG_LEVEL_T level, CONST CHAR_T* fmt, ...)
{
    ITA_RESULT err = ITA_OK;
#ifdef MULTI_THREAD_FUNCTION
    porting_mutex_lock(m_mutex);
#endif

    memset(m_msg, 0, MSG_SIZE);
    CHAR_T* ptr = m_msg;

    if (m_func)
    {
        int len = 0;
        int year, month, day, hour, min, sec, us;
#ifdef _WIN32
        SYSTEMTIME tm;
        GetLocalTime(&tm);
        year = tm.wYear;
        month = tm.wMonth;
        day = tm.wDay;
        hour = tm.wHour;
        min = tm.wMinute;
        sec = tm.wSecond;
        us = tm.wMilliseconds * 1000;
#else
        struct timeval tv;
        struct tm* tm = NULL;
        gettimeofday(&tv, NULL);
        time_t tt = tv.tv_sec;
        tm = localtime(&tt);
        year = tm->tm_year + 1900;
        month = tm->tm_mon + 1;
        day = tm->tm_mday;
        hour = tm->tm_hour;
        min = tm->tm_min;
        sec = tm->tm_sec;
        us = tv.tv_usec;
#endif
        const char* levelstr = ita_logger_levelstr(level);

        len += snprintf(ptr + len, MSG_SIZE - len,
            "%04d-%02d-%02d %02d:%02d:%02d.%03d %s ",
            year, month, day, hour, min, sec, us / 1000,
            levelstr);

        int prefixLen = len; // for ita_logger_printf

        va_list ap;
        va_start(ap, fmt);
        len += vsnprintf(ptr + len, MSG_SIZE - len, fmt, ap);
        va_end(ap);

        *(ptr + len) = '\n';
        ++len;

        // log print
        m_func(m_cameraID, ptr, m_userParam);

        if (m_mode == DBG_OUTPUT_FILE_EX)
        {
            ita_logger_print(m_logger, level, "%s", ptr + prefixLen);
        }

        err = ITA_OK;
    }
    else if (DBG_OUTPUT_CONSOLE == m_mode)
    {
        int len = 0;
        int year, month, day, hour, min, sec, us;
#ifdef _WIN32
        SYSTEMTIME tm;
        GetLocalTime(&tm);
        year = tm.wYear;
        month = tm.wMonth;
        day = tm.wDay;
        hour = tm.wHour;
        min = tm.wMinute;
        sec = tm.wSecond;
        us = tm.wMilliseconds * 1000;
#else
        struct timeval tv;
        struct tm* tm = NULL;
        gettimeofday(&tv, NULL);
        time_t tt = tv.tv_sec;
        tm = localtime(&tt);
        year = tm->tm_year + 1900;
        month = tm->tm_mon + 1;
        day = tm->tm_mday;
        hour = tm->tm_hour;
        min = tm->tm_min;
        sec = tm->tm_sec;
        us = tv.tv_usec;
#endif
        const char* levelstr = ita_logger_levelstr(level);

        len += snprintf(ptr + len, MSG_SIZE - len,
            "%04d-%02d-%02d %02d:%02d:%02d.%03d %s ",
            year, month, day, hour, min, sec, us / 1000,
            levelstr);

        va_list ap;
        va_start(ap, fmt);
        len += vsnprintf(ptr + len, MSG_SIZE - len, fmt, ap);
        va_end(ap);

        *(ptr + len) = '\n';
        ++len;

        fprintf(stdout, "%.*s", len, ptr);

        err = ITA_OK;
    }
    else if (DBG_OUTPUT_FILE == m_mode)
    {
        int len = 0;

        va_list ap;
        va_start(ap, fmt);
        len += vsnprintf(ptr + len, MSG_SIZE - len, fmt, ap);
        va_end(ap);
        *(ptr + len) = '\n';
        ++len;

        ita_logger_print(m_logger, level, "%s", ptr);

        err = ITA_OK;
    }


#ifdef MULTI_THREAD_FUNCTION
    porting_mutex_unlock(m_mutex);
#endif
    return err;
}
