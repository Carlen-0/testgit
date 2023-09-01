/*************************************************************
Copyright (C), 2021--2022, Wuhan Guide Sensmart Tech Co., Ltd
File name   : PortingAPI.h
Author      : Guide Sensmart RD. wangyan
Version     : 1.0
Date        : 2021/11/9
Description : Porting API definition.
*************************************************************/
#ifndef PORTING_API_H
#define PORTING_API_H

#include "../Base/Config.h"

#ifdef __cplusplus
extern "C" {
#endif

	/* generic type redefinitions */
	typedef int                 INT32_T;
	typedef unsigned int        UINT32_T;
	typedef short               INT16_T;
	typedef unsigned short      UINT16_T;
	typedef char                CHAR_T;
	typedef unsigned char       BYTE_T;

	typedef long                LONG_T;
	typedef unsigned long       ULONG_T;
	typedef unsigned long long  UINT64_T;
	typedef long long           INT64_T;
	typedef float               FLOAT_T;
	typedef double              DOUBLE_T;
#ifdef WINDOWS_PLATFORM
	typedef void* HANDLE_T;	//兼容32bit和64bit操作系统
#else
	typedef UINT64_T			HANDLE_T;
#endif // WINDOWS_PLATFORM
#define CONST				const
#define VOID				void

	typedef enum
	{
		FALSE_T = 0,
		TRUE_T = 1,
	} BOOL_T;

	typedef struct
	{
		UINT16_T year;
		UINT16_T month;
		UINT16_T day;
		UINT16_T hour;
		UINT16_T minute;
		UINT16_T second;
		UINT16_T milliseconds;
	} TIME_T;

	typedef enum ITAModuleID
	{
		ITA_IMAGE_MODULE,
		ITA_MT_MODULE,
		ITA_PARSER_MODULE,
		ITA_WRAPPER_MODULE,
		ITA_OTHER_MODULE,
		ITA_NONE_MODULE
	}ITA_MODULE_ID;

	typedef struct
	{
		ITA_MODULE_ID id;
		VOID* pMem;
		int size;
	} MEM_NODE;

#define MAX_MEM_NODE	128

	/**
	* @brief	申请分配内存
	*/
	VOID* porting_calloc_mem(INT32_T num, INT32_T size, ITA_MODULE_ID id);

	/**
	* @brief	申请释放内存
	*/
	VOID porting_free_mem(VOID* p);

	/**
	* @brief	打开文件
	* @param
	* filename	文件路径+名称
	* mode		使用文件方式，类型和操作要求，参考fopen。
	* @return	文件句柄，NULL表示打开文件失败。
	*/
	HANDLE_T porting_file_open(CONST CHAR_T* filename, CONST CHAR_T* mode);

	/**
	* @brief	关闭文件
	* @param
	* fd		文件句柄
	* @return
	*/
	INT32_T porting_file_close(HANDLE_T fd);

	/**
	* @brief	读文件
	* @param
	* fd		文件句柄
	* buffer	读数据缓存
	* nbytes	读取大小
	* @return	实际读取大小
	*/
	INT32_T porting_file_read(HANDLE_T fd, BYTE_T* buffer, INT32_T nbytes);

	/**
	* @brief	写文件
	* @param
	* fd		文件句柄
	* buffer	写数据缓存
	* nbytes	写入大小
	* @return	实际写入大小
	*/
	INT32_T porting_file_write(HANDLE_T fd, CONST BYTE_T* buffer, INT32_T nbytes);

	/**
	* @brief	将缓冲区内容写入文件
	* @param
	* fd		文件句柄
	*/
	INT32_T porting_file_flush(HANDLE_T fd);

	/**
	* @brief	得到文件当前读写位置
	* @param
	* fd		文件句柄
	* @return	当前读写位置偏离文件头部的字节数
	*/
	INT32_T porting_file_tell(HANDLE_T fd);

	/**
	* @brief	把文件读写指针移到指定位置
	* @param
	* fd		文件句柄
	* offset	偏移位置
	* seek		起始点，取值有0 1 2，代表文件不同的位置。
	* SEEK_SET 0   文件开头
	* SEEK_CUR 1   文件当前位置
	* SEEK_END 2   文件末尾
	* @return
	* 0			成功
	* -1		失败
	*/
	INT32_T porting_file_seek(HANDLE_T fd, INT32_T offset, INT32_T seek);

	/**
	* @brief	把文件读写指针重新指向文件开头
	* @param
	* fd		文件句柄
	* @return
	* 0			成功
	* -1		失败
	*/
	INT32_T porting_file_rewind(HANDLE_T fd);

	/**
	* @brief	删除文件
	* @param
	* name		文件路径+名称
	* @return
	* 0			成功
	* -1		失败
	*/
	INT32_T porting_file_delete(CONST CHAR_T* name);

	/**
	* @brief		获取当前日期时间，精确到毫秒。
	* @param		time，输出当前日期时间。
	* @return
	* 0				成功
	* -1			失败
	*/
	INT32_T porting_get_time(TIME_T* time);

	/**
	* @brief		获取当前时间和程序启动时间之间经过的毫秒数。
	* @param		无
	* @return		返回当前时间和程序启动时间之间经过的毫秒数。
	* @note			使用该接口可以计算某个过程的耗时。过程开始时调用一次函数，结束时调用一次函数，两者相减即过程消耗的毫秒数。
	*/
	UINT32_T porting_get_ms();

	/**
	* @brief		获取当前时间和程序启动时间之间经过的微秒数。
	* @param		无
	* @return		返回当前时间和程序启动时间之间经过的微秒数。
	* @note			使用该接口可以计算某个过程的耗时。过程开始时调用一次函数，结束时调用一次函数，两者相减即过程消耗的微秒数。
	* @note
	*/
	UINT32_T porting_get_us();

	/**
	* @brief		获取当前时间，单位秒。
	* @param		无
	* @return		返回当前时间，单位秒。
	* @note
	*/
	INT64_T porting_get_time_second();

	/**
	* @brief		获取当前时间，单位毫秒。
	* @param		无
	* @return		返回当前时间，单位毫秒。
	* @note
	*/
	INT64_T porting_get_time_ms();

	/**
	* @brief		挂起一段时间，让出CPU
	* @param
	* ms			休眠时间，单位毫秒
	* @return		无
	*/
	VOID porting_thread_sleep(INT32_T ms);

#ifdef MULTI_THREAD_FUNCTION

	typedef  VOID(*GUIDEIR_THREAD_PROC)(VOID* param);

	/**
	* @brief		创建线程
	* @param
	* func:			函数入口地址:typedef VOID(*IPANEL_THREAD_PROC)(VOID *);
	* param:		参数列表
	* stack_size:	栈大小，可以不设置
	* priority:		优先级别，可以不设置
	* @return		线程句柄
	*/
	HANDLE_T porting_thread_create(GUIDEIR_THREAD_PROC func, VOID* param, INT32_T priority, UINT32_T stack_size);

	/**
	* @brief		销毁线程。等待目标线程结束后回收目标线程对象资源。另一种方式将目标线程分离，由其自己结束，不用等待。
	*				采用第一种方式实现销毁线程。无论哪种方式都要求线程函数保证能自动退出。
	* @param
	* handle:		线程句柄
	* @return
	* 0				成功
	* -1			失败
	*/
	INT32_T porting_thread_destroy(HANDLE_T handle);

	/**
	* @brief		创建互斥量，类型是ERRORCHECK_MUTEX，同一线程不能重复加锁，加上的锁只能由本线程解锁。
	* @param		无
	* @return		mutex句柄
	*/
	HANDLE_T porting_mutex_create();

	/**
	* @brief		销毁互斥量
	* @param		mutex句柄
	* @return
	* 0				成功
	* -1			失败
	*/
	INT32_T porting_mutex_destroy(HANDLE_T handle);

	/**
	* @brief		互斥量加锁
	* @param		mutex句柄
	* @return
	* 0				成功
	* -1			失败
	*/
	INT32_T porting_mutex_lock(HANDLE_T handle);

	/**
	* @brief		互斥量解锁
	* @param		mutex句柄
	* @return
	* 0				成功
	* -1			失败
	*/
	INT32_T porting_mutex_unlock(HANDLE_T handle);

#endif // MULTI_THREAD_FUNCTION

#ifdef __cplusplus
}
#endif

#endif // !PORTING_API_H
