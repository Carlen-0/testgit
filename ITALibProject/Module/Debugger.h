/*************************************************************
Copyright (C), 2021--2022, Wuhan Guide Sensmart Tech Co., Ltd
File name   : Debugger.h
Author      : Guide Sensmart RD. wangyan
Version     : 1.0
Date        : 2021/11/9
Description : Debug module.
*************************************************************/
#ifndef ITA_DEBUGGER_H
#define ITA_DEBUGGER_H

#include <string>
#include "../ITA.h"

typedef enum SnapShotType
{
	SNAP_SHOT_X,
	SNAP_SHOT_Y,
	SNAP_SHOT_B,
	SNAP_SHOT_K,
	SNAP_SHOT_RGB,
	SNAP_START_X,
	SNAP_STOP_X,
	SNAP_START_Y,
	SNAP_STOP_Y,
	SNAP_RECORD_X,
	SNAP_RECORD_Y
}SNAP_SHOT_TYPE;

class Debugger
{
public:
	Debugger();
	~Debugger();

	ITA_RESULT control(ITA_DEBUG_TYPE type, void *param);

	ITA_RESULT snapShot(SNAP_SHOT_TYPE type, unsigned char *data, int len);

	ITA_RESULT checkPath(const char * path);

	std::string getPath();

private:
	ITA_RESULT saveFile(std::string fileName, unsigned char *data, int len);

	ITA_RESULT timeString(char *buf, int size);

private:
	std::string m_path;
	HANDLE_T m_xFile;
	HANDLE_T m_yFile;
};

#endif // !ITA_DEBUGGER_H
