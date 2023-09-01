/*************************************************************
Copyright (C), 2021--2022, Wuhan Guide Sensmart Tech Co., Ltd
File name   : Debugger.cpp
Author      : Guide Sensmart RD. wangyan
Version     : 1.0
Date        : 2021/11/9
Description : Debug module.
*************************************************************/
#include "Debugger.h"
#include "PortingAPI.h"
#include <string>

Debugger::Debugger()
{
	m_xFile = 0;
	m_yFile = 0;
}

Debugger::~Debugger()
{
	if (m_xFile)
	{
		porting_file_close(m_xFile);
		m_xFile = NULL;
	}
	if (m_yFile)
	{
		porting_file_close(m_yFile);
		m_yFile = NULL;
	}
}

ITA_RESULT Debugger::control(ITA_DEBUG_TYPE type, void * param)
{
	switch (type)
	{
	case ITA_DEBUGGING_PATH:
	{
		char *path = (char *)param;
		std::string pathStr;
		pathStr.append(path);
		int index = (int)pathStr.find_last_of("/");
		std::string temp = pathStr.substr(0, index+1);	//带上/
		m_path.clear();
		m_path.append(temp.data());
		break;
	}
	//case ITA_SNAPSHOT:				//拍照保存当前帧图像，存储在程序的同级目录下。
	//	break;
	//case ITA_START_RECORD:			//开始录X16/Y16数据。
	//	break;
	//case ITA_STOP_RECORD:			//停止录X16/Y16数据。
	//	break;
	default:
		break;
	}
	return ITA_OK;
}

ITA_RESULT Debugger::snapShot(SNAP_SHOT_TYPE type, unsigned char * data, int len)
{
	ITA_RESULT ret = ITA_OK;
	CHAR_T name[64] = { '\0' };
	std::string fileName;
	if (type < SNAP_START_X || type > SNAP_STOP_Y)
	{   //指针非空判断
		if (!data || len <= 0)
		{
			return ITA_NULL_PTR_ERR;
		}
	}
	if (m_path.length() > 0)
		fileName.append(m_path);
	switch (type)
	{
	case SNAP_SHOT_X:
		fileName.append("X_");
		timeString(name, 64);
		fileName.append(name);
		fileName.append(".raw");
		ret = saveFile(fileName, data, len);
		break;
	case SNAP_SHOT_Y:
		fileName.append("Y_");
		timeString(name, 64);
		fileName.append(name);
		fileName.append(".raw");
		ret = saveFile(fileName, data, len);
		break;
	case SNAP_SHOT_B:
		fileName.append("B_");
		timeString(name, 64);
		fileName.append(name);
		fileName.append(".raw");
		ret = saveFile(fileName, data, len);
		break;
	case SNAP_SHOT_K:
		fileName.append("K_");
		timeString(name, 64);
		fileName.append(name);
		fileName.append(".raw");
		ret = saveFile(fileName, data, len);
		break;
	case SNAP_SHOT_RGB:
		fileName.append("IMG_");
		timeString(name, 64);
		fileName.append(name);
		fileName.append(".raw");
		ret = saveFile(fileName, data, len);
		break;
	case SNAP_START_X:
	{
		fileName.append("XVideo_");
		timeString(name, 64);
		fileName.append(name);
		fileName.append(".raw");
		m_xFile = porting_file_open(fileName.data(), "wb+");
		if (!m_xFile)
		{
			ret = ITA_CREATE_FILE_FAILURE;
		}
		break;
	}
	case SNAP_STOP_X:
		if (m_xFile)
		{
			porting_file_close(m_xFile);
			m_xFile = NULL;
		}
		break;
	case SNAP_START_Y:
		fileName.append("YVideo_");
		timeString(name, 64);
		fileName.append(name);
		fileName.append(".raw");
		m_yFile = porting_file_open(fileName.data(), "wb+");
		if (!m_yFile)
		{
			ret = ITA_CREATE_FILE_FAILURE;
		}
		break;
	case SNAP_STOP_Y:
		if (m_yFile)
		{
			porting_file_close(m_yFile);
			m_yFile = NULL;
		}
		break;
	case SNAP_RECORD_X:
		if (m_xFile)
		{
			porting_file_write(m_xFile, data, len);
		}
		break;
	case SNAP_RECORD_Y:
		if (m_yFile)
		{
			porting_file_write(m_yFile, data, len);
		}
		break;
	default:
		break;
	}
	return ret;
}

ITA_RESULT Debugger::checkPath(const char * path)
{
	//路径非法字符检查
	std::string myPath = (const char *)path;
	int ret;
	if (!path)
	{
		return ITA_NULL_PTR_ERR;
	}
	if (myPath.length() > 128)
		return ITA_BUF_OVERFLOW;
	if ((ret = (int)myPath.find('*')) >= 0)
	{
		return ITA_ILLEGAL_PATH;
	}
	else if ((ret = (int)myPath.find('?')) >= 0)
	{
		return ITA_ILLEGAL_PATH;
	}
	else if ((ret = (int)myPath.find('"')) >= 0)
	{
		return ITA_ILLEGAL_PATH;
	}
	else if ((ret = (int)myPath.find('<')) >= 0)
	{
		return ITA_ILLEGAL_PATH;
	}
	else if ((ret = (int)myPath.find('>')) >= 0)
	{
		return ITA_ILLEGAL_PATH;
	}
	else if ((ret = (int)myPath.find('|')) >= 0)
	{
		return ITA_ILLEGAL_PATH;
	}
	//文件名非法字符检查
	int index = (int)myPath.find_last_of("/");
	if (index < 0)
		return ITA_ILLEGAL_PATH; //不是合法的完整路径
	if ((unsigned int)index + 1 == myPath.length())	//以/结尾
		return ITA_ILLEGAL_PATH;
	std::string temp = myPath.substr(index + 1, myPath.length());
	if ((ret = (int)temp.find(':')) >= 0)
	{
		return ITA_ILLEGAL_PATH;
	}
	else if ((ret = (int)temp.find('\\')) >= 0)
	{
		return ITA_ILLEGAL_PATH;
	}
	else if ((ret = (int)temp.find('/')) >= 0)
	{
		return ITA_ILLEGAL_PATH;
	}
	return ITA_OK;
}

std::string Debugger::getPath()
{
	return m_path;
}

ITA_RESULT Debugger::saveFile(std::string fileName, unsigned char * data, int len)
{
	ITA_RESULT ret = ITA_OK;
	HANDLE_T hd = porting_file_open(fileName.data(), "wb+");
	if (hd)
	{
		porting_file_write(hd, data, len);
		porting_file_close(hd);
	}
	else
	{
		ret = ITA_CREATE_FILE_FAILURE;
	}
	return ret;
}

ITA_RESULT Debugger::timeString(char * buf, int size)
{
	TIME_T time;
	CHAR_T *ptr;
	int len;
	if (size < 24)
		return ITA_ILLEGAL_PAPAM_ERR;
	//以时间命名文件
	porting_get_time(&time);
	ptr = buf;
	//日期作为目录
	porting_get_time(&time);
	len = sprintf(ptr, "%d", time.year);
	ptr += len;
	len = sprintf(ptr, "%d", time.month);
	ptr += len;
	len = sprintf(ptr, "%d", time.day);
	ptr += len;
	len = sprintf(ptr, "%d", time.hour);
	ptr += len;
	len = sprintf(ptr, "%d", time.minute);
	ptr += len;
	len = sprintf(ptr, "%d", time.second);
	ptr += len;
	len = sprintf(ptr, "%d", time.milliseconds);
	ptr += len;
	return ITA_OK;
}
