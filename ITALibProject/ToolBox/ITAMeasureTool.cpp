#include "../ITAMeasureTool.h"
#include "MeasureTool.h"

ITA_API HANDLE_T ITA_MeasureToolInit()
{
#ifdef MEASURETOOL_EDITION
	MeasureTool* measureTool = new MeasureTool();
	return (HANDLE_T)measureTool;
#else
	return 0;
#endif
}

ITA_API void ITA_MeasureToolUnInit(HANDLE_T instance)
{
#ifdef MEASURETOOL_EDITION
	MeasureTool* measureTool = (MeasureTool*)instance;
	delete measureTool;
#else
	return ;
#endif
}

ITA_API ITA_RESULT ITA_ConfigFileExport(HANDLE_T instance, ITAConfig* config, char* outputEncryStr , int encryStrLen, int* length)
{
#ifdef MEASURETOOL_EDITION
	CHECK_NULL_POINTER(instance);
	MeasureTool* measureTool = (MeasureTool*)instance;
	return measureTool->ConfigFileExport(config, outputEncryStr, encryStrLen, length);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif // MEASURETOOL

}

ITA_API ITA_RESULT ITA_ConfigFileImport(HANDLE_T instance, char* inputStr, int length, ITAConfig* config)
{
#ifdef MEASURETOOL_EDITION
	CHECK_NULL_POINTER(instance);
	MeasureTool* measureTool = (MeasureTool*)instance;
	return measureTool->ConfigFileImport(inputStr,length,config);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}