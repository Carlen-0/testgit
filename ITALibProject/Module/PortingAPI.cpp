/*************************************************************
Copyright (C), 2021--2022, Wuhan Guide Sensmart Tech Co., Ltd
File name   : PortingAPI.cpp
Author      : Guide Sensmart RD. wangyan
Version     : 1.0
Date        : 2021/11/9
Description : Porting API implementation.
*************************************************************/
#if defined(_WIN32) || defined(WIN32) || defined(_WIN32_)
#define ITA_OS_WIN 1
#else
#define ITA_OS_UNIX 1
#endif // 

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <thread>
#include <mutex>
#include <errno.h>
#ifdef ITA_OS_WIN
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE
#endif
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif
#include <Windows.h>
#include <direct.h>
#include <io.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h> 
#include <sys/time.h>
//#include <dirent.h>
#endif 


#include "PortingAPI.h"
#include "GuideLog.h"

int g_memArray[ITA_NONE_MODULE];
int g_nodeCount = 0;
int g_nodeSize = 0;	//当前g_pNode容量
MEM_NODE* g_pNode = NULL;

VOID* porting_calloc_mem(INT32_T num, INT32_T size, ITA_MODULE_ID id)
{
	int count = 0, sum = g_memArray[id];
	void* pO = calloc(num, size);
	if (!pO)
	{
		return pO;
	}
	g_memArray[id] = sum + num * size;
	if (!g_pNode)
	{
		g_nodeSize = MAX_MEM_NODE;
		g_pNode = (MEM_NODE*)calloc(g_nodeSize, sizeof(MEM_NODE));
		sum = g_memArray[ITA_OTHER_MODULE];
		g_memArray[ITA_OTHER_MODULE] = sum + g_nodeSize * sizeof(MEM_NODE);
	}
	//如果节点个数超过上限，重新申请内存。
	if (g_nodeCount >= g_nodeSize)
	{
		//先减去当前内存量
		sum = g_memArray[ITA_OTHER_MODULE];
		g_memArray[ITA_OTHER_MODULE] = sum - g_nodeSize * sizeof(MEM_NODE);
		//再申请新内存
		g_nodeSize += MAX_MEM_NODE;
		MEM_NODE* pT = (MEM_NODE*)calloc(g_nodeSize, sizeof(MEM_NODE));
		memcpy(pT, g_pNode, (g_nodeSize - MAX_MEM_NODE) * sizeof(MEM_NODE));
		free(g_pNode);
		g_pNode = pT;
		sum = g_memArray[ITA_OTHER_MODULE];
		g_memArray[ITA_OTHER_MODULE] = sum + g_nodeSize * sizeof(MEM_NODE);
	}
	//查找空闲节点
	while (count < g_nodeSize)
	{
		if (!(g_pNode + count)->pMem)
		{
			(g_pNode + count)->id = id;
			(g_pNode + count)->pMem = pO;
			(g_pNode + count)->size = num * size;
			g_nodeCount++;
			break;
		}
		count++;
	}
	return pO;
}

VOID porting_free_mem(VOID* p)
{
	//查找节点
	int count = 0, sum;
	while (count < g_nodeSize)
	{
		if ((g_pNode + count)->pMem == p)
		{
			sum = g_memArray[(g_pNode + count)->id];
			g_memArray[(g_pNode + count)->id] = sum - (g_pNode + count)->size;
			(g_pNode + count)->pMem = NULL;
			(g_pNode + count)->id = ITA_NONE_MODULE;
			(g_pNode + count)->size = 0;
			g_nodeCount--;
			break;
		}
		count++;
	}
	if (count >= g_nodeSize)
	{
		//ERROR
	}
	free(p);
}

HANDLE_T porting_file_open(CONST CHAR_T* filename, CONST CHAR_T* mode)
{
#ifdef ITA_OS_WIN
	FILE* fp = 0;
	/*在windows操作系统中加入第三个参数，指定打开文件的共享方式，也就是打开这个文件时，其他进程是否可以读写该文件。
	** 共享方式参数可以是下面的值：
	** 0x10                   //_SH_DENYRW   Denies   read   and   write   access   to   the   file
	** 0x20                   //_SH_DENYWR   Denies   write   access   to   the   file
	** 0x30                   //_SH_DENYRD   Denies   read   access   to   the   file.
	** 0x40                   //_SH_DENYNO   Permits   read   and   write   access
	*/
	fp = _fsopen(filename, mode, 0x20);
	return (HANDLE_T)fp;
#else
	FILE* fp = 0;
	/*S_IROTH 00004 权限，代表其他用户具有可读的权限*/
	fp = fopen(filename, mode);
	return (HANDLE_T)fp;
#endif
}

INT32_T porting_file_close(HANDLE_T fd)
{
	return fclose((FILE*)fd);
}

INT32_T porting_file_read(HANDLE_T fd, BYTE_T* buffer, INT32_T nbytes)
{
	return fread(buffer, 1, nbytes, (FILE*)fd);
}

INT32_T porting_file_write(HANDLE_T fd, CONST BYTE_T* buffer, INT32_T nbytes)
{
	return fwrite(buffer, 1, nbytes, (FILE*)fd);
}

INT32_T porting_file_flush(HANDLE_T fd)
{
	return fflush((FILE*)fd);
}

INT32_T porting_file_tell(HANDLE_T fd)
{
	return ftell((FILE*)fd);
}

INT32_T porting_file_seek(HANDLE_T fd, INT32_T offset, INT32_T seek)
{
	return fseek((FILE*)fd, offset, seek);
}

INT32_T porting_file_rewind(HANDLE_T fd)
{
	rewind((FILE*)fd);
	return ITA_OK;
}

INT32_T porting_file_delete(CONST CHAR_T* name)
{
	return remove(name);
}

INT32_T porting_get_time(TIME_T* pTime)
{
#ifdef ITA_OS_WIN
	SYSTEMTIME sys;

	GetLocalTime(&sys);
	pTime->year = sys.wYear;
	pTime->month = sys.wMonth;
	pTime->day = sys.wDay;
	pTime->hour = sys.wHour;
	pTime->minute = sys.wMinute;
	pTime->second = sys.wSecond;
	pTime->milliseconds = sys.wMilliseconds;
	return ITA_OK;
#else
	time_t timep;
	struct tm* p;
	struct timeval tm;

	time(&timep);
	p = localtime(&timep); /*取得当地时间*/
	gettimeofday(&tm, NULL); /*获取当前时间毫秒数*/
	pTime->year = p->tm_year + 1900;
	pTime->month = p->tm_mon + 1;
	pTime->day = p->tm_mday;
	pTime->hour = p->tm_hour;
	pTime->minute = p->tm_min;
	pTime->second = p->tm_sec;
	pTime->milliseconds = tm.tv_usec / 1000;
	return ITA_OK;
#endif
}

UINT32_T porting_get_ms()
{
#ifdef ITA_OS_WIN
	time_t clock;
	struct tm tm;
	SYSTEMTIME wtm;
	UINT16_T ms;
	UINT32_T time_ms;
	static time_t clock_start = 0;
	static UINT16_T ms_start = 0;

	if (!clock_start)
	{
		GetLocalTime(&wtm);
		tm.tm_year = wtm.wYear - 1900;
		tm.tm_mon = wtm.wMonth - 1;
		tm.tm_mday = wtm.wDay;
		tm.tm_hour = wtm.wHour;
		tm.tm_min = wtm.wMinute;
		tm.tm_sec = wtm.wSecond;
		tm.tm_isdst = -1;
		clock_start = mktime(&tm);
		if (clock_start == -1)
		{
			//GuideLog::output(LOG_ERROR, "porting_get_ms clock_start=%d", clock_start);
			return 0;
		}
		ms_start = wtm.wMilliseconds;
	}
	GetLocalTime(&wtm);
	tm.tm_year = wtm.wYear - 1900;
	tm.tm_mon = wtm.wMonth - 1;
	tm.tm_mday = wtm.wDay;
	tm.tm_hour = wtm.wHour;
	tm.tm_min = wtm.wMinute;
	tm.tm_sec = wtm.wSecond;
	tm.tm_isdst = -1;
	clock = mktime(&tm);
	if (clock == -1)
	{
		//GuideLog::output(LOG_ERROR, "porting_get_ms clock=%d", clock);
		return 0;
	}
	ms = wtm.wMilliseconds;
	time_ms = (UINT32_T)((clock - clock_start) * 1000 + (ms - ms_start));
	return time_ms;
#else
	static struct timeval tmstart = { 0, 0 };
	struct timeval tm;
	UINT32_T tmptm;

	/*获取启动时间*/
	if ((0 == tmstart.tv_sec) && (0 == tmstart.tv_usec))
	{
		gettimeofday(&tmstart, NULL);
		return 0;
	}
	/*获取当前时间，计算出当前时刻距启动时刻的毫秒数*/
	gettimeofday(&tm, NULL);
	tmptm = (tm.tv_sec - tmstart.tv_sec) * 1000 + (tm.tv_usec - tmstart.tv_usec) / 1000;
	if (tm.tv_sec < tmstart.tv_sec)
	{
		//表明系统时间被改小，需要更新起始相对时间。并重新计算相对时间。
		gettimeofday(&tmstart, NULL);
		gettimeofday(&tm, NULL);
		tmptm = (tm.tv_sec - tmstart.tv_sec) * 1000 + (tm.tv_usec - tmstart.tv_usec) / 1000;
	}
	return tmptm;
#endif
}

UINT32_T porting_get_us()
{
#ifdef ITA_OS_WIN
	return 0;
#else
	static struct timeval tmstart = { 0, 0 };
	struct timeval tm;
	UINT32_T tmptm;

	/*获取启动时间*/
	if ((0 == tmstart.tv_sec) && (0 == tmstart.tv_usec))
	{
		gettimeofday(&tmstart, NULL);
		return 0;
	}
	/*获取当前时间，计算出当前时刻距启动时刻的毫秒数*/
	gettimeofday(&tm, NULL);
	tmptm = (tm.tv_sec - tmstart.tv_sec) * 1000000 + (tm.tv_usec - tmstart.tv_usec);
	if (tm.tv_sec < tmstart.tv_sec)
	{
		//表明系统时间被改小，需要更新起始相对时间。并重新计算相对时间。
		gettimeofday(&tmstart, NULL);
		gettimeofday(&tm, NULL);
		tmptm = (tm.tv_sec - tmstart.tv_sec) * 1000000 + (tm.tv_usec - tmstart.tv_usec);
	}
	return tmptm;
#endif
}

INT64_T porting_get_time_second()
{
#ifdef ITA_OS_WIN
	INT64_T seconds;
	time_t clock;
	struct tm tm;
	SYSTEMTIME wtm;
	GetLocalTime(&wtm);
	tm.tm_year = wtm.wYear - 1900;
	tm.tm_mon = wtm.wMonth - 1;
	tm.tm_mday = wtm.wDay;
	tm.tm_hour = wtm.wHour;
	tm.tm_min = wtm.wMinute;
	tm.tm_sec = wtm.wSecond;
	tm.tm_isdst = -1;
	clock = mktime(&tm);
	seconds = clock + wtm.wMilliseconds / 1000;
	//#include <sys/time.h>
	/*struct timeval tv;
	struct timezone tz;
	gettimeofday(&tv, &tz);
	seconds = tv.tv_sec + tv.tv_usec / 1000000;*/
	return seconds;
#else
	INT64_T seconds;
	struct timeval tv;
	struct timezone tz;

	gettimeofday(&tv, &tz);
	seconds = tv.tv_sec + tv.tv_usec / 1000000;
	return seconds;
#endif
}

INT64_T porting_get_time_ms()
{
#ifdef ITA_OS_WIN
	INT64_T ms;
	time_t clock;
	struct tm tm;
	SYSTEMTIME wtm;
	GetLocalTime(&wtm);
	tm.tm_year = wtm.wYear - 1900;
	tm.tm_mon = wtm.wMonth - 1;
	tm.tm_mday = wtm.wDay;
	tm.tm_hour = wtm.wHour;
	tm.tm_min = wtm.wMinute;
	tm.tm_sec = wtm.wSecond;
	tm.tm_isdst = -1;
	clock = mktime(&tm);
	ms = clock * 1000 + wtm.wMilliseconds;
	//#include <sys/time.h>
	/*struct timeval tv;
	struct timezone tz;
	gettimeofday(&tv, &tz);
	seconds = tv.tv_sec * 1000 + tv.tv_usec / 1000;*/
	return ms;
#else
	INT64_T ms;
	struct timeval tv;
	struct timezone tz;

	gettimeofday(&tv, &tz);
	ms = tv.tv_sec * 1000 + tv.tv_usec / 1000;
	return ms;
#endif
}

VOID porting_thread_sleep(INT32_T ms)
{
#ifdef ITA_OS_WIN
	Sleep(ms);
#else
	usleep(ms * 1000);
#endif
}

#ifdef MULTI_THREAD_FUNCTION

HANDLE_T porting_thread_create(GUIDEIR_THREAD_PROC func, VOID* param, INT32_T priority, UINT32_T stack_size)
{
	std::thread* t = new std::thread(func, param);
	//GuideLog::output(LOG_INFO, "thread_create %#x", t);
	return (HANDLE_T)t;
}

INT32_T porting_thread_destroy(HANDLE_T handle)
{
	std::thread* t = (std::thread*)handle;
	//GuideLog::output(LOG_INFO, "thread_destroy wait %#x", t);
	t->join();
	delete t;
	//GuideLog::output(LOG_INFO, "thread_destroy success %#x", t);
	return ITA_OK;
}

HANDLE_T porting_mutex_create()
{
	std::mutex* m = new std::mutex();
	return (HANDLE_T)m;
}

INT32_T porting_mutex_destroy(HANDLE_T handle)
{
	std::mutex* m = (std::mutex*)handle;
	delete m;
	return ITA_OK;
}

INT32_T porting_mutex_lock(HANDLE_T handle)
{
	std::mutex* m = (std::mutex*)handle;
	m->lock();
	return ITA_OK;
}

INT32_T porting_mutex_unlock(HANDLE_T handle)
{
	std::mutex* m = (std::mutex*)handle;
	m->unlock();
	return ITA_OK;
}

#endif // MULTI_THREAD_FUNCTION
