#include "MeasureTool.h"
#include "../Base/Parser.h"

#ifdef MEASURETOOL_EDITION
MeasureTool::MeasureTool()
{
	int w = 120;
	int h = 90;
	ITA_MODE mode = ITA_X16;
	ITA_PRODUCT_TYPE type = ITA_256_TIMO;
	m_parse = new Parser(mode,w,h,type);
	m_logger = new GuideLog();
}
MeasureTool::~MeasureTool()
{
	if (m_parse) {
		delete m_parse;
		m_parse = nullptr;
	}
	if (m_logger) {
		delete m_logger;
		m_logger = nullptr;
	}
}

void MeasureTool::RecordConfigInfo(std::string key, std::string value)
{
	std::string temp = key + " = " + value + ";\n";
	m_configFileContent += temp;
}

void MeasureTool::RecordConfigInfo(std::string key, std::string range1, std::string range2)
{
	std::string temp = key + " = " + range1 + "-" + range2 + ";\n";
	m_configFileContent += temp;
}

ITA_RESULT MeasureTool::ExportFunctionInfo(ITAConfig* config)
{
	m_configFileContent += "//FUNCTION Configuration item\n";
	ITAFunctionConfig* function = config->functionConfig;

	if (function->versionNumberS)
	{
		RecordConfigInfo("versionNumber", config->functionConfig->versionNumber);
	}
	if (function->productTypeS)
	{
		RecordConfigInfo("productType", std::to_string(config->functionConfig->productType));
	}
	if (function->shutterTempUpperS)
	{
		RecordConfigInfo("shutterTempUpper", std::to_string(config->functionConfig->shutterTempUpper));
	}
	if (function->nucTempUpperS)
	{
		RecordConfigInfo("nucTempUpper", std::to_string(config->functionConfig->nucTempUpper));
	}
	if (function->nucIntervalBottomS)
	{
		RecordConfigInfo("nucIntervalBottom", std::to_string(config->functionConfig->nucIntervalBottom));
	}
	if (function->shutterIntervalBottomS)
	{
		RecordConfigInfo("shutterIntervalBottom", std::to_string(config->functionConfig->shutterIntervalBottom));
	}
	if (function->isSubAvgBS)
	{
		RecordConfigInfo("isSubAvgB", std::to_string(config->functionConfig->isSubAvgB));
	}
	if (function->shutterCollectIntervalS)
	{
		RecordConfigInfo("shutterCollectInterval", std::to_string(config->functionConfig->shutterCollectInterval));
	}
	if (function->collectIntervalS)
	{
		RecordConfigInfo("collectInterval", std::to_string(config->functionConfig->collectInterval));
	}
	if (function->collectNumberOfTimesS)
	{
		RecordConfigInfo("collectNumberOfTimes", std::to_string(config->functionConfig->collectNumberOfTimes));
	}
	if (function->bootTimeToNow1S)
	{
		RecordConfigInfo("bootTimeToNow1", std::to_string(config->functionConfig->bootTimeToNow1));
	}
	if (function->shutterPeriod1S)
	{
		RecordConfigInfo("shutterPeriod1", std::to_string(config->functionConfig->shutterPeriod1));
	}
	if (function->bootTimeToNow2S)
	{
		RecordConfigInfo("bootTimeToNow2", std::to_string(config->functionConfig->bootTimeToNow2));
	}
	if (function->shutterPeriod2S)
	{
		RecordConfigInfo("shutterPeriod2", std::to_string(config->functionConfig->shutterPeriod2));
	}
	if (function->bootTimeToNow3S)
	{
		RecordConfigInfo("bootTimeToNow3", std::to_string(config->functionConfig->bootTimeToNow3));
	}
	if (function->shutterPeriod3S)
	{
		RecordConfigInfo("shutterPeriod3", std::to_string(config->functionConfig->shutterPeriod3));
	}
	if (function->maxFramerateS)
	{
		RecordConfigInfo("maxFramerate", std::to_string(config->functionConfig->maxFramerate));
	}
	if (function->internalFlashS)
	{
		RecordConfigInfo("internalFlash", std::to_string(config->functionConfig->internalFlash));
	}
	return ITA_OK;
}

ITA_RESULT MeasureTool::ExportMtConfig(ITAConfig* config)
{
	if (!config->mtConfig) {
		m_logger->output(LOG_INFO, "MT CONFIG NULL");
		return ITA_NULL_PTR_ERR;
	}
	m_configFileContent += "//MTOPTION Configuration items\n";
	for (int i = 0; i < config->mtConfigCount; i++)
	{
		ITAMTConfig* temp = config->mtConfig + i;
		RecordConfigInfo("lensType", std::to_string(temp->lensType));
		RecordConfigInfo("mtDistanceRange", std::to_string(temp->mtDistanceRangeN),std::to_string(temp->mtDistanceRangeF));
		RecordConfigInfo("mtType", std::to_string(temp->mtType));
		if (temp->correctDistanceS)
		{
			RecordConfigInfo("correctDistance", std::to_string(temp->correctDistance));
		}
		if (temp->distanceS)
		{
			RecordConfigInfo("distance", std::to_string(temp->distance));
		}
		if (temp->lowLensCorrKS)
		{
			RecordConfigInfo("lowLensCorrK", std::to_string(temp->lowLensCorrK));
		}
		if (temp->highLensCorrKS)
		{
			RecordConfigInfo("highLensCorrK", std::to_string(temp->highLensCorrK));
		}
		if (temp->lowShutterCorrCoffS)
		{
			RecordConfigInfo("lowShutterCorrCoff", std::to_string(temp->lowShutterCorrCoff));
		}
		if (temp->highShutterCorrCoffS)
		{
			RecordConfigInfo("highShutterCorrCoff", std::to_string(temp->highShutterCorrCoff));
		}
		if (temp->mtDisTypeS)
		{
			RecordConfigInfo("mtDisType", std::to_string(temp->mtDisType));
		}
		if (temp->coefA1S)
		{
			RecordConfigInfo("coefA1", std::to_string(temp->coefA1));
		}
		if (temp->coefA2S)
		{
			RecordConfigInfo("coefA2", std::to_string(temp->coefA2));
		}
		if (temp->coefA3S)
		{
			RecordConfigInfo("coefA3", std::to_string(temp->coefA3));
		}
		if (temp->coefB1S)
		{
			RecordConfigInfo("coefB1", std::to_string(temp->coefB1));
		}
		if (temp->coefB2S)
		{
			RecordConfigInfo("coefB2", std::to_string(temp->coefB2));
		}
		if (temp->coefB3S)
		{
			RecordConfigInfo("coefB3", std::to_string(temp->coefB3));
		}
		if (temp->coefC1S)
		{
			RecordConfigInfo("coefC1", std::to_string(temp->coefC1));
		}
		if (temp->coefC2S)
		{
			RecordConfigInfo("coefC2", std::to_string(temp->coefC2));
		}
		if (temp->coefC3S)
		{
			RecordConfigInfo("coefC3", std::to_string(temp->coefC3));
		}
	}
	return ITA_OK;
}

ITA_RESULT MeasureTool::ExportIspConfig(ITAConfig* config)
{
	if (!config->ispConfig) {
		m_logger->output(LOG_INFO, "ISP CONFIG NULL");
		return ITA_NULL_PTR_ERR;
	}
	m_configFileContent += "//ISP Configuration items\n";
	for (int i = 0; i < config->ispConfigCount; i++)
	{
		ITAISPConfig* temp = config->ispConfig + i;
		RecordConfigInfo("lensType", std::to_string(temp->lensType));
		RecordConfigInfo("mtType", std::to_string(temp->mtType));
		if (temp->tffStdS)
		{
			RecordConfigInfo("tffStd", std::to_string(temp->tffStd));
		}
		if (temp->vStripeWinWidthS)
		{
			RecordConfigInfo("vStripeWinWidth", std::to_string(temp->vStripeWinWidth));
		}
		if (temp->hStripeStdS)
		{
			RecordConfigInfo("hStripeStd", std::to_string(temp->hStripeStd));
		}
		if (temp->vStripeWeightThreshS)
		{
			RecordConfigInfo("vStripeWeightThresh", std::to_string(temp->vStripeWeightThresh));
		}
		if (temp->vStripeDetailThreshS)
		{
			RecordConfigInfo("vStripeDetailThresh", std::to_string(temp->vStripeDetailThresh));
		}
		if (temp->hStripeWinWidthS)
		{
			RecordConfigInfo("hStripeWinWidth", std::to_string(temp->hStripeWinWidth));
		}
		if (temp->hStripeStdS)
		{
			RecordConfigInfo("hStripeStd", std::to_string(temp->hStripeStd));
		}
		if (temp->hStripeWeightThreshS)
		{
			RecordConfigInfo("hStripeWeightThresh", std::to_string(temp->hStripeWeightThresh));
		}
		if (temp->hStripeDetailThreshS)
		{
			RecordConfigInfo("hStripeDetailThresh", std::to_string(temp->hStripeDetailThresh));
		}
		if (temp->rnArithTypeS)
		{
			RecordConfigInfo("rnArithType", std::to_string(temp->rnArithType));
		}
		if (temp->distStdS)
		{
			RecordConfigInfo("distStd", std::to_string(temp->distStd));
		}
		if (temp->grayStdS)
		{
			RecordConfigInfo("grayStd", std::to_string(temp->grayStd));
		}
		if (temp->discardUpratioS)
		{
			RecordConfigInfo("discardUpratio", std::to_string(temp->discardUpratio));
		}
		if (temp->discardDownratioS)
		{
			RecordConfigInfo("discardDownratio", std::to_string(temp->discardDownratio));
		}
		if (temp->linearBrightnessS)
		{
			RecordConfigInfo("linearBrightness", std::to_string(temp->linearBrightness));
		}
		if (temp->linearContrastS)
		{
			RecordConfigInfo("linearContrast", std::to_string(temp->linearContrast));
		}
		if (temp->linearRestrainRangethreS)
		{
			RecordConfigInfo("linearRestrainRangethre", std::to_string(temp->linearRestrainRangethre));
		}
		if (temp->heqPlatThreshS)
		{
			RecordConfigInfo("heqPlatThresh", std::to_string(temp->heqPlatThresh));
		}
		if (temp->heqRangeMaxS)
		{
			RecordConfigInfo("heqRangeMax", std::to_string(temp->heqRangeMax));
		}
		if (temp->heqMidvalueS)
		{
			RecordConfigInfo("heqMidvalue", std::to_string(temp->heqMidvalue));
		}
		if (temp->iieEnhanceCoefS)
		{
			RecordConfigInfo("iieEnhanceCoef", std::to_string(temp->iieEnhanceCoef));
		}
		if (temp->iieGrayStdS)
		{
			RecordConfigInfo("iieGrayStd", std::to_string(temp->iieGrayStd));
		}
		if (temp->iieGaussStdS)
		{
			RecordConfigInfo("iieGaussStd", std::to_string(temp->iieGaussStd));
		}
		if (temp->iieDetailThrS)
		{
			RecordConfigInfo("iieDetailThr", std::to_string(temp->iieDetailThr));
		}
		if (temp->claheCliplimitS)
		{
			RecordConfigInfo("claheCliplimit", std::to_string(temp->claheCliplimit));
		}
		if (temp->claheHistMaxS)
		{
			RecordConfigInfo("claheHistMax", std::to_string(temp->claheHistMax));
		}
		if (temp->claheBlockWidthS)
		{
			RecordConfigInfo("claheBlockWidth", std::to_string(temp->claheBlockWidth));
		}
		if (temp->claheBlockHeightS)
		{
			RecordConfigInfo("claheBlockHeight", std::to_string(temp->claheBlockHeight));
		}
		if (temp->spLaplaceWeightS)
		{
			RecordConfigInfo("spLaplaceWeight", std::to_string(temp->spLaplaceWeight));
		}
		if (temp->gmcTypeS)
		{
			RecordConfigInfo("gmcType", std::to_string(temp->gmcType));
		}
		if (temp->gmcGammaS)
		{
			RecordConfigInfo("gmcGamma", std::to_string(temp->gmcGamma));
		}
		if (temp->adjustbcBrightS)
		{
			RecordConfigInfo("adjustbcBright", std::to_string(temp->adjustbcBright));
		}
		if (temp->adjustbcContrastS)
		{
			RecordConfigInfo("adjustbcContrast", std::to_string(temp->adjustbcContrast));
		}
		if (temp->zoomTypeS)
		{
			RecordConfigInfo("zoomType", std::to_string(temp->zoomType));
		}
		if (temp->mixThrLowS)
		{
			RecordConfigInfo("mixThrLow", std::to_string(temp->mixThrLow));
		}
		if (temp->mixThrHighS)
		{
			RecordConfigInfo("mixThrHigh", std::to_string(temp->mixThrHigh));
		}
	}
	return ITA_OK;
}
extern std::string EncryptionAES(const std::string& strSrc);
ITA_RESULT MeasureTool::ConfigFileExport(ITAConfig* config,char* encryStr,int encryStrLen,int* length)
{
	if (!config || !config->functionConfig)
	{
		return ITA_NULL_PTR_ERR;
	}
	m_configFileContent.clear();
	ExportFunctionInfo(config);
	if (config->mtConfigCount <= 0) {
		m_logger->output(LOG_INFO, "ITA_NO_MT_CONF");
	}
	else {
		ExportMtConfig(config);
	}

	if (config->ispConfig <= 0) {
		m_logger->output(LOG_INFO, "ITA_NO_ISP_CONF");
	}
	else {
		ExportIspConfig(config);
	}
	std::string outPut = EncryptionAES(m_configFileContent);
	if (outPut.size() > encryStrLen)
	{
		return ITA_CONF_FILE_TOO_LARGE;
	}
	memcpy(encryStr, outPut.c_str(), outPut.length());
	*length = outPut.length();
	return ITA_OK;
}

ITA_RESULT MeasureTool::ConfigFileImport(char* inputStr, int length, ITAConfig* config)
{
	return m_parse->parseConfig((unsigned char*)inputStr, length, config);
}
#endif