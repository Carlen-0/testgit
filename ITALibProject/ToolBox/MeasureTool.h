#ifndef MEASURETOOL_H
#define MEASURETOOL_H

#include "../Module/GuideLog.h"
#include "../Base/Config.h"
#include "../ITAMeasureTool.h"

#ifdef MEASURETOOL_EDITION

class Parser;
class MeasureTool {
public:
	MeasureTool();
	~MeasureTool();
	ITA_RESULT ConfigFileExport(ITAConfig* config, char* encryStr, int encryStrLen, int* stringLen);
	ITA_RESULT ConfigFileImport(char* inputStr, int length, ITAConfig* config);
	void RecordConfigInfo(std::string key, std::string value);
	void RecordConfigInfo(std::string key, std::string range1, std::string range2);
	ITA_RESULT ExportFunctionInfo(ITAConfig* config);
	ITA_RESULT ExportMtConfig(ITAConfig* config);
	ITA_RESULT ExportIspConfig(ITAConfig* config);
private:
	Parser* m_parse;
	GuideLog* m_logger;
	std::string m_configFileContent;
};

#endif

#endif