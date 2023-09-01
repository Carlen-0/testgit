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
	typedef void* HANDLE_T;	//����32bit��64bit����ϵͳ
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
	* @brief	��������ڴ�
	*/
	VOID* porting_calloc_mem(INT32_T num, INT32_T size, ITA_MODULE_ID id);

	/**
	* @brief	�����ͷ��ڴ�
	*/
	VOID porting_free_mem(VOID* p);

	/**
	* @brief	���ļ�
	* @param
	* filename	�ļ�·��+����
	* mode		ʹ���ļ���ʽ�����ͺͲ���Ҫ�󣬲ο�fopen��
	* @return	�ļ������NULL��ʾ���ļ�ʧ�ܡ�
	*/
	HANDLE_T porting_file_open(CONST CHAR_T* filename, CONST CHAR_T* mode);

	/**
	* @brief	�ر��ļ�
	* @param
	* fd		�ļ����
	* @return
	*/
	INT32_T porting_file_close(HANDLE_T fd);

	/**
	* @brief	���ļ�
	* @param
	* fd		�ļ����
	* buffer	�����ݻ���
	* nbytes	��ȡ��С
	* @return	ʵ�ʶ�ȡ��С
	*/
	INT32_T porting_file_read(HANDLE_T fd, BYTE_T* buffer, INT32_T nbytes);

	/**
	* @brief	д�ļ�
	* @param
	* fd		�ļ����
	* buffer	д���ݻ���
	* nbytes	д���С
	* @return	ʵ��д���С
	*/
	INT32_T porting_file_write(HANDLE_T fd, CONST BYTE_T* buffer, INT32_T nbytes);

	/**
	* @brief	������������д���ļ�
	* @param
	* fd		�ļ����
	*/
	INT32_T porting_file_flush(HANDLE_T fd);

	/**
	* @brief	�õ��ļ���ǰ��дλ��
	* @param
	* fd		�ļ����
	* @return	��ǰ��дλ��ƫ���ļ�ͷ�����ֽ���
	*/
	INT32_T porting_file_tell(HANDLE_T fd);

	/**
	* @brief	���ļ���дָ���Ƶ�ָ��λ��
	* @param
	* fd		�ļ����
	* offset	ƫ��λ��
	* seek		��ʼ�㣬ȡֵ��0 1 2�������ļ���ͬ��λ�á�
	* SEEK_SET 0   �ļ���ͷ
	* SEEK_CUR 1   �ļ���ǰλ��
	* SEEK_END 2   �ļ�ĩβ
	* @return
	* 0			�ɹ�
	* -1		ʧ��
	*/
	INT32_T porting_file_seek(HANDLE_T fd, INT32_T offset, INT32_T seek);

	/**
	* @brief	���ļ���дָ������ָ���ļ���ͷ
	* @param
	* fd		�ļ����
	* @return
	* 0			�ɹ�
	* -1		ʧ��
	*/
	INT32_T porting_file_rewind(HANDLE_T fd);

	/**
	* @brief	ɾ���ļ�
	* @param
	* name		�ļ�·��+����
	* @return
	* 0			�ɹ�
	* -1		ʧ��
	*/
	INT32_T porting_file_delete(CONST CHAR_T* name);

	/**
	* @brief		��ȡ��ǰ����ʱ�䣬��ȷ�����롣
	* @param		time�������ǰ����ʱ�䡣
	* @return
	* 0				�ɹ�
	* -1			ʧ��
	*/
	INT32_T porting_get_time(TIME_T* time);

	/**
	* @brief		��ȡ��ǰʱ��ͳ�������ʱ��֮�侭���ĺ�������
	* @param		��
	* @return		���ص�ǰʱ��ͳ�������ʱ��֮�侭���ĺ�������
	* @note			ʹ�øýӿڿ��Լ���ĳ�����̵ĺ�ʱ�����̿�ʼʱ����һ�κ���������ʱ����һ�κ���������������������ĵĺ�������
	*/
	UINT32_T porting_get_ms();

	/**
	* @brief		��ȡ��ǰʱ��ͳ�������ʱ��֮�侭����΢������
	* @param		��
	* @return		���ص�ǰʱ��ͳ�������ʱ��֮�侭����΢������
	* @note			ʹ�øýӿڿ��Լ���ĳ�����̵ĺ�ʱ�����̿�ʼʱ����һ�κ���������ʱ����һ�κ���������������������ĵ�΢������
	* @note
	*/
	UINT32_T porting_get_us();

	/**
	* @brief		��ȡ��ǰʱ�䣬��λ�롣
	* @param		��
	* @return		���ص�ǰʱ�䣬��λ�롣
	* @note
	*/
	INT64_T porting_get_time_second();

	/**
	* @brief		��ȡ��ǰʱ�䣬��λ���롣
	* @param		��
	* @return		���ص�ǰʱ�䣬��λ���롣
	* @note
	*/
	INT64_T porting_get_time_ms();

	/**
	* @brief		����һ��ʱ�䣬�ó�CPU
	* @param
	* ms			����ʱ�䣬��λ����
	* @return		��
	*/
	VOID porting_thread_sleep(INT32_T ms);

#ifdef MULTI_THREAD_FUNCTION

	typedef  VOID(*GUIDEIR_THREAD_PROC)(VOID* param);

	/**
	* @brief		�����߳�
	* @param
	* func:			������ڵ�ַ:typedef VOID(*IPANEL_THREAD_PROC)(VOID *);
	* param:		�����б�
	* stack_size:	ջ��С�����Բ�����
	* priority:		���ȼ��𣬿��Բ�����
	* @return		�߳̾��
	*/
	HANDLE_T porting_thread_create(GUIDEIR_THREAD_PROC func, VOID* param, INT32_T priority, UINT32_T stack_size);

	/**
	* @brief		�����̡߳��ȴ�Ŀ���߳̽��������Ŀ���̶߳�����Դ����һ�ַ�ʽ��Ŀ���̷߳��룬�����Լ����������õȴ���
	*				���õ�һ�ַ�ʽʵ�������̡߳��������ַ�ʽ��Ҫ���̺߳�����֤���Զ��˳���
	* @param
	* handle:		�߳̾��
	* @return
	* 0				�ɹ�
	* -1			ʧ��
	*/
	INT32_T porting_thread_destroy(HANDLE_T handle);

	/**
	* @brief		������������������ERRORCHECK_MUTEX��ͬһ�̲߳����ظ����������ϵ���ֻ���ɱ��߳̽�����
	* @param		��
	* @return		mutex���
	*/
	HANDLE_T porting_mutex_create();

	/**
	* @brief		���ٻ�����
	* @param		mutex���
	* @return
	* 0				�ɹ�
	* -1			ʧ��
	*/
	INT32_T porting_mutex_destroy(HANDLE_T handle);

	/**
	* @brief		����������
	* @param		mutex���
	* @return
	* 0				�ɹ�
	* -1			ʧ��
	*/
	INT32_T porting_mutex_lock(HANDLE_T handle);

	/**
	* @brief		����������
	* @param		mutex���
	* @return
	* 0				�ɹ�
	* -1			ʧ��
	*/
	INT32_T porting_mutex_unlock(HANDLE_T handle);

#endif // MULTI_THREAD_FUNCTION

#ifdef __cplusplus
}
#endif

#endif // !PORTING_API_H
