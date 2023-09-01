/*************************************************************
Copyright (C), 2021--2022, Wuhan Guide Sensmart Tech Co., Ltd
File name   : Parser.cpp
Author      : Guide Sensmart RD. wangyan
Version     : 1.0
Date        : 2021/11/9
Description : Parser module.
*************************************************************/
#include <iostream>
#include "Parser.h"
#include "../Module/GuideLog.h"
#include "../Secure/CRC32.h"

Parser::Parser(ITA_MODE mode, int width, int height, ITA_PRODUCT_TYPE type)
{
	m_mode = mode;
	m_type = type;
	jwTableNumber = 0;
	jwTableArray = NULL;
	m_4CurvesData = NULL;
	headerArray = NULL;
	m_confBuf = NULL;
	m_frameSize = width*height;
	m_width = width;
	m_height = height;
	if (ITA_X16 == mode || ITA_MCU_X16 == mode)
	{
		m_kArray = (unsigned short *)porting_calloc_mem(m_frameSize, sizeof(unsigned short), ITA_PARSER_MODULE);
		//初K
		for (int i = 0; i < m_frameSize; i++)
		{
			m_kArray[i] = 8192;
		}
	}
	else
	{
		m_kArray = NULL;
	}
	memset(&m_ph, 0, sizeof(PackageHeader));
	m_avgB = 0;
	m_isConfEncrypt = true;
	m_isCold = true;
	m_builtInFlash = 0;
	m_flash120 = NULL;
	m_lowPackageLen = 0;
	m_lowPackage = NULL;
	m_highPackageLen = 0;
	m_highPackage = NULL;
	m_humanPackageLen = 0;
	m_humanPackage = NULL;
	m_versionV = 0;
	m_lowStartSector = 0;
	m_highStartSector = 0;
	m_humanStartSector = 0;
	memset(m_correctionFactors, 0, sizeof(float));
	memset(&m_dp, 0, sizeof(DetectorParameter));
	m_reservedData = NULL;
	m_last_lltt = 0;
	m_lltt = 0; //免校温镜头。1，是；0，否。
	m_lastHighSCoff = 0.0f;
	m_lastHighLCork = 0.0f;
	m_lastLowSCoff = 0.0f;
	m_lastLowLCork = 0.0f;
	m_isCoffChange = false;

	memset(&m_burnInfo, 0, sizeof(ITA_BURN_INFO));

	curCurveLen = 0;
	curJwNumber = 0;
	curGJwNumber = 0;
}

Parser::~Parser()
{
	if (m_kArray)
	{
		porting_free_mem(m_kArray);
		m_kArray = NULL;
	}
	if (jwTableArray)
	{
		porting_free_mem(jwTableArray);
		jwTableArray = NULL;
	}
	if (m_4CurvesData)
	{
		porting_free_mem(m_4CurvesData);
		m_4CurvesData = NULL;
	}
	if (headerArray)
	{
		porting_free_mem(headerArray);
		headerArray = NULL;
	}
	if (m_confBuf)
	{
		porting_free_mem(m_confBuf);
		m_confBuf = NULL;
	}
	if (m_flash120)
	{
		delete m_flash120;
		m_flash120 = NULL;
	}
	if (m_lowPackage)
	{
		porting_free_mem(m_lowPackage);
		m_lowPackage = NULL;
	}
	if (m_highPackage)
	{
		porting_free_mem(m_highPackage);
		m_highPackage = NULL;
	}
	if (m_humanPackage)
	{
		porting_free_mem(m_humanPackage);
		m_humanPackage = NULL;
	}
	if (m_reservedData)
	{
		porting_free_mem(m_reservedData);
		m_reservedData = NULL;
	}
}
ITA_RESULT Parser::parseCurveData(int cameraID, ITARegistry * registry, ITA_FIELD_ANGLE lensType, ITA_RANGE range, int gear)
{
	ITA_RESULT result = ITA_ERROR;
	int length;
	if (!registry || (!registry->ReadCurveData && !registry->ReadCurveDataEx))
		return ITA_HAL_UNREGISTER;
	
	m_logger->output(LOG_INFO, "ReadCurveData lensType=%d range=%d gear=%d lltt=%d %#x %#x", lensType, range, gear, m_lltt, registry->ReadCurveData, registry->ReadCurveDataEx);
	/*镜头类型与视场角、焦距、F数、衰减片（高温镜头）以及镜片数目等相关。
	所有产品穷举完全过于复杂，因此增加产品自定义镜头。针对没有列举出来的视场角镜头还是可以扩展的。
	衰减片（高温镜头）以及镜片数目这类差异的镜头可以用产品自定义镜头类型。*/
	//手持测温和机芯测温产品人体和工业曲线长度16384 short
	if (!m_4CurvesData)
		m_4CurvesData = (short *)porting_calloc_mem(MAX_TEC_CURVE_SIZE, sizeof(short), ITA_PARSER_MODULE);
	if(registry->ReadCurveData)
		result = registry->ReadCurveData(cameraID, (unsigned char *)m_4CurvesData, MAX_TEC_CURVE_SIZE*2, &length, lensType, range, gear, registry->userParam);
	else if (registry->ReadCurveDataEx)
	{
		//免校温镜头
		result = registry->ReadCurveDataEx(cameraID, (unsigned char *)m_4CurvesData, MAX_TEC_CURVE_SIZE * 2, &length, 
			lensType, range, gear, &m_lltt, sizeof(short), registry->userParam);
	}
	if (ITA_OK != result)
	{
		m_logger->output(LOG_ERROR, "parseCurveData ret=%d", result);
	}
	return result;
}
/*人体一档参数包500K左右，工业两档参数包1600K左右。*/
ITA_RESULT Parser::parsePackageData(int cameraID, ITARegistry *registry, ITA_RANGE range, ITA_FIELD_ANGLE lensType)
{
	ITA_RESULT result = ITA_ERROR;
	int offsetB = 1400;
	if (!registry || (!registry->ReadPackageData && !registry->ReadPackageDataEx))
		return ITA_HAL_UNREGISTER;
	m_logger->output(LOG_INFO, "parsePackageData range=%d", range);
	m_range = range;
	//256和120模组数据包格式不同，探测器参数区分别是1400和30字节。
	if (ITA_120_TIMO == m_type)
		offsetB = 30;
	if (!headerArray)
		headerArray = (unsigned char *)porting_calloc_mem(MAX_PACKAGE_HEADER, 1, ITA_PARSER_MODULE);
	//读取数据包头
	if(registry->ReadPackageData)
		result = registry->ReadPackageData(cameraID, headerArray, 0, MAX_PACKAGE_HEADER, range, registry->userParam);
	else if(registry->ReadPackageDataEx)
		result = registry->ReadPackageDataEx(cameraID, headerArray, 0, MAX_PACKAGE_HEADER, range, lensType, &m_lltt, sizeof(short), registry->userParam);
	if (ITA_OK != result)
	{
		m_logger->output(LOG_ERROR, "ReadPackageData ret=%d", result);
		return result;
	}
	//在更新包头之前先释放之前的内存。
	if (jwTableArray)
	{
		porting_free_mem(jwTableArray);
		jwTableArray = NULL;
	}
	//人体和工业曲线长度不同，导入不同的数据包要在更新包头之前先释放之前的内存。
	/*if (m_4CurvesData)
	{
		porting_free_mem(m_4CurvesData);
		m_4CurvesData = NULL;
	}*/
	if (!m_4CurvesData)
	{
		m_4CurvesData = (short*)porting_calloc_mem(MAX_CURVE_LENGTH * 4,sizeof(short), ITA_PARSER_MODULE);
	}
	//解析数据包头
	memcpy((void *)&m_ph, headerArray, sizeof(PackageHeader));
	if (m_ph.usHeadLength <= 0 || m_ph.usHeadLength > MAX_PACKAGE_HEADER
		|| m_ph.ucFocusNumber <= 0 || m_ph.ucFocusNumber > MAX_FOCUS_NUMBER
		|| m_ph.usWidth != m_width || m_ph.usHeight != m_height)
	{
		//未导入数据包的模组jwTableNumber=255
		m_logger->output(LOG_ERROR, "ReadPackageData: Wrong header data. HeaderLen=%d,  FocusNumber=%d, Width=%d, Height=%d", m_ph.usHeadLength, m_ph.ucFocusNumber, m_ph.usWidth, m_ph.usHeight);
		return ITA_WRONG_PACKAGE_HEADER;
	}
	//读取120探测器参数，256在DetectorWrapper::DetectorInit中读取。
	if (ITA_120_TIMO == m_type)
		registry->ReadPackageData(cameraID, (unsigned char *)&m_dp, m_ph.usHeadLength, sizeof(DetectorParameter), range, registry->userParam);
	//解析焦温个数
	//为了满足高温环境下的测温精度，采集模组数据的时候有8个焦温。
	//8个焦温对应的温度是15，20，25，30，35，40, 45, 50。
	jwTableNumber = m_ph.ucFocusNumber;
	m_logger->output(LOG_INFO, "read jwTableNumber = %d, ucGearMark = %d %d %d %d %s", jwTableNumber, m_ph.ucGearMark, m_ph.focusType, m_ph.lensType, m_ph.mtType, m_ph.cModuleCode);
	//解析焦温表。如果高低温档焦温数量不同的话，切换范围时可能死机。所以先释放再重新申请。
	if (!jwTableArray)
		jwTableArray = (short *)porting_calloc_mem(m_ph.usFocusArrayLength, 1, ITA_PARSER_MODULE);
	//偏移216 + 1400
	if (registry->ReadPackageData)
		registry->ReadPackageData(cameraID, (unsigned char *)jwTableArray, m_ph.usHeadLength + offsetB, m_ph.usFocusArrayLength, range, registry->userParam);
	else if (registry->ReadPackageDataEx)
		registry->ReadPackageDataEx(cameraID, (unsigned char *)jwTableArray, m_ph.usHeadLength + offsetB, m_ph.usFocusArrayLength, range, lensType, &m_lltt, sizeof(short), registry->userParam);
	for (int i = 0; i < jwTableNumber; i++)
	{
		m_logger->output(LOG_INFO, "jw %d = %d", i, jwTableArray[i]);
	}
	//m_logger->output(LOG_INFO, "read JW table end");
	//解析曲线。如果高低温档曲线长度不同的话，切换范围时可能死机。所以先释放再重新申请。
	//读600*8*2*2B（远近共16条曲线，256模组人体测温项目只采了0.5米的曲线只有8条）。CURVE_LENGTH * mDH.ucFocusNumber * mDH.ucDistanceNumber * sizeof(short)
	//int curveDataLength = jwTableNumber * 2 * ONE_CURVE_DATA_LENGTH / 2;
	//if (!m_4CurvesData)
	//	m_4CurvesData = (short *)porting_calloc_mem(m_ph.usCurveTemperatureNumber * m_ph.ucDistanceNumber * 2, sizeof(short), ITA_PARSER_MODULE);// m_ph.usCurveDataLength);
	if (m_ph.usCurveTemperatureNumber * m_ph.ucDistanceNumber * 2 > MAX_CURVE_LENGTH * 4) {
		porting_free_mem(m_4CurvesData);
		m_4CurvesData = (short*)porting_calloc_mem(m_ph.usCurveTemperatureNumber * m_ph.ucDistanceNumber * 2, sizeof(short), ITA_PARSER_MODULE);
	}
	//偏移216 + 1400 + ph.usFocusArrayLength 初始读取1档位曲线
	if (registry->ReadPackageData)
		registry->ReadPackageData(cameraID, (unsigned char *)m_4CurvesData, m_ph.usHeadLength + offsetB + m_ph.usFocusArrayLength, m_ph.usCurveTemperatureNumber * m_ph.ucDistanceNumber * 2 * sizeof(short), range, registry->userParam);
	else if (registry->ReadPackageDataEx)
		registry->ReadPackageDataEx(cameraID, (unsigned char *)m_4CurvesData, m_ph.usHeadLength + offsetB + m_ph.usFocusArrayLength, m_ph.usCurveTemperatureNumber * m_ph.ucDistanceNumber * 2 * sizeof(short), range, lensType, &m_lltt, sizeof(short), registry->userParam);
	m_logger->output(LOG_INFO, "read CurveData end. 4 curvesLen=%d totalLen=%d", 8 * m_ph.usCurveTemperatureNumber, m_ph.usCurveDataLength);
	//X16模式下需要解析K数据
	if (ITA_X16 == m_mode || ITA_MCU_X16 == m_mode)
	{
		//初始读取0档位的K。高低温档一帧K长度固定。
		//偏移216 + 1400 + ph.usFocusArrayLength + ph.usCurveDataLength
		if (registry->ReadPackageData)
			registry->ReadPackageData(cameraID, (unsigned char *)m_kArray, m_ph.usHeadLength + offsetB + m_ph.usFocusArrayLength + m_ph.usCurveDataLength, m_ph.usKMatLength / m_ph.ucFocusNumber, range, registry->userParam);
		else if (registry->ReadPackageDataEx)
			registry->ReadPackageDataEx(cameraID, (unsigned char *)m_kArray, m_ph.usHeadLength + offsetB + m_ph.usFocusArrayLength + m_ph.usCurveDataLength, m_ph.usKMatLength / m_ph.ucFocusNumber, range, lensType, &m_lltt, sizeof(short), registry->userParam);
		m_logger->output(LOG_INFO, "read K Data end. len=%d", m_ph.usKMatLength);
	}
	return ITA_OK;
}

ITA_RESULT Parser::changeGear(int cameraID, ITARegistry *registry, int currentGear, ITA_FIELD_ANGLE lensType)
{
	if (!m_4CurvesData)
	{
		m_logger->output(LOG_ERROR, "changeGear failed. ITA_NO_PACKAGE ret=%d", ITA_NO_PACKAGE);
		return ITA_NO_PACKAGE;
	}
	//测温库中如果有5个焦温，那么有0 1 2 3 4 5共6个档位。0和5档位的4条曲线是重复的2组。4和5档位共用K。
	//偏移216 + 1400 + ph.usFocusArrayLength 是曲线开始的位置。
	int offset = m_ph.usHeadLength + 1400 + m_ph.usFocusArrayLength;
	if (ITA_120_TIMO == m_type)
		offset = m_ph.usHeadLength + 30 + m_ph.usFocusArrayLength;
	int groupLen = m_ph.usCurveTemperatureNumber * m_ph.ucDistanceNumber;
	// 当档位发生变化时，更新4条测温曲线。最低档和最高档是重复的2组曲线，要分开处理。
	if (!m_builtInFlash)
	{
		if (!registry || (!registry->ReadPackageData && !registry->ReadPackageDataEx))
		{
			return ITA_HAL_UNREGISTER;
		}
		if (currentGear == 0)
		{
			if (registry->ReadPackageData)
			{
				registry->ReadPackageData(cameraID, (unsigned char *)m_4CurvesData, offset, groupLen * sizeof(unsigned short), m_range, registry->userParam);
				registry->ReadPackageData(cameraID, (unsigned char *)(m_4CurvesData + groupLen), offset, groupLen * sizeof(unsigned short), m_range, registry->userParam);
			}
			else if (registry->ReadPackageDataEx)
			{
				registry->ReadPackageDataEx(cameraID, (unsigned char *)m_4CurvesData, offset, groupLen * sizeof(unsigned short), m_range, lensType, &m_lltt, sizeof(short), registry->userParam);
				registry->ReadPackageDataEx(cameraID, (unsigned char *)(m_4CurvesData + groupLen), offset, groupLen * sizeof(unsigned short), m_range, lensType, &m_lltt, sizeof(short), registry->userParam);
			}
		}
		else if (currentGear == m_ph.ucFocusNumber)
		{
			if (registry->ReadPackageData)
			{
				registry->ReadPackageData(cameraID, (unsigned char *)m_4CurvesData, offset + groupLen *(m_ph.ucFocusNumber - 1) * sizeof(unsigned short), groupLen * sizeof(unsigned short), m_range, registry->userParam);
				registry->ReadPackageData(cameraID, (unsigned char *)(m_4CurvesData + groupLen), offset + groupLen *(m_ph.ucFocusNumber - 1) * sizeof(unsigned short), groupLen * sizeof(unsigned short), m_range, registry->userParam);
			}
			else if (registry->ReadPackageDataEx)
			{
				registry->ReadPackageDataEx(cameraID, (unsigned char *)m_4CurvesData, offset + groupLen *(m_ph.ucFocusNumber - 1) * sizeof(unsigned short), groupLen * sizeof(unsigned short), m_range, lensType, &m_lltt, sizeof(short), registry->userParam);
				registry->ReadPackageDataEx(cameraID, (unsigned char *)(m_4CurvesData + groupLen), offset + groupLen *(m_ph.ucFocusNumber - 1) * sizeof(unsigned short), groupLen * sizeof(unsigned short), m_range, lensType, &m_lltt, sizeof(short), registry->userParam);
			}
		}
		else
		{
			if (registry->ReadPackageData)
			{
				registry->ReadPackageData(cameraID, (unsigned char *)m_4CurvesData, offset + groupLen *(currentGear - 1) * sizeof(unsigned short), groupLen * 2 * sizeof(unsigned short), m_range, registry->userParam);
			}
			else if (registry->ReadPackageDataEx)
			{
				registry->ReadPackageDataEx(cameraID, (unsigned char *)m_4CurvesData, offset + groupLen *(currentGear - 1) * sizeof(unsigned short), groupLen * 2 * sizeof(unsigned short), m_range, lensType, &m_lltt, sizeof(short), registry->userParam);
			}
		}
	}
	else
	{
		if (currentGear == 0)
		{
			readFlashPackage((unsigned char *)m_4CurvesData, offset, groupLen * sizeof(unsigned short), m_range);
			readFlashPackage((unsigned char *)(m_4CurvesData + groupLen), offset, groupLen * sizeof(unsigned short), m_range);
		}
		else if (currentGear == m_ph.ucFocusNumber)
		{
			readFlashPackage((unsigned char *)m_4CurvesData, offset + groupLen *(m_ph.ucFocusNumber - 1) * sizeof(unsigned short), groupLen * sizeof(unsigned short), m_range);
			readFlashPackage((unsigned char *)(m_4CurvesData + groupLen), offset + groupLen *(m_ph.ucFocusNumber - 1) * sizeof(unsigned short), groupLen * sizeof(unsigned short), m_range);
		}
		else
		{
			readFlashPackage((unsigned char *)m_4CurvesData, offset + groupLen *(currentGear - 1) * sizeof(unsigned short), groupLen * 2 * sizeof(unsigned short), m_range);
		}
	}
	//X16模式下更新K
	if (ITA_X16 == m_mode || ITA_MCU_X16 == m_mode)
	{
		//最后两档共用K，注意不要溢出。
		int g = currentGear;
		if (currentGear >= jwTableNumber && jwTableNumber > 0)
			g = jwTableNumber - 1;
		offset = m_ph.usHeadLength + 1400 + m_ph.usFocusArrayLength + m_ph.usCurveDataLength;
		if (ITA_120_TIMO == m_type)
			offset = m_ph.usHeadLength + 30 + m_ph.usFocusArrayLength + m_ph.usCurveDataLength;
		if (!m_builtInFlash)
		{
			if (registry->ReadPackageData)
				registry->ReadPackageData(cameraID, (unsigned char *)m_kArray, offset + g * m_ph.usKMatLength / m_ph.ucFocusNumber, m_ph.usKMatLength / m_ph.ucFocusNumber, m_range, registry->userParam);
			else if (registry->ReadPackageDataEx)
				registry->ReadPackageDataEx(cameraID, (unsigned char *)m_kArray, offset + g * m_ph.usKMatLength / m_ph.ucFocusNumber, m_ph.usKMatLength / m_ph.ucFocusNumber, m_range, lensType, &m_lltt, sizeof(short), registry->userParam);
		}
		else
			readFlashPackage((unsigned char *)m_kArray, offset + g * m_ph.usKMatLength / m_ph.ucFocusNumber, m_ph.usKMatLength / m_ph.ucFocusNumber, m_range);
	}
	else if (ITA_FPGA_Y16 == m_mode)
	{
		//通知FPGA切K
		//最后两档共用K，注意不要溢出。
		int g = currentGear;
		if (currentGear >= jwTableNumber && jwTableNumber > 0)
			g = jwTableNumber - 1;
		/*if (GUIDEIR_ERR == UartCmdSend(CMD_SETK_INDEX, g, 0))
		{
			m_logger->output(LOG_ERROR, "UartCmdSend CMD_SETK_INDEX failed!gear=%d", g);
		}*/
	}
	//切换档位后如果探测器参数不同，MCU会配置探测器寄存器值。理论上需要先延时再做一次NUC。
	/*porting_thread_sleep(300);
	nucManual();*/
	m_logger->output(LOG_INFO, "changgeGear to gear=%d", currentGear);
	//测温类型，0:人体测温   1：工业测温常温段    2：工业测温高温段
	//上位机在导两档探测器参数时，先导低温的探测器参数，后面再导入高温的探测器参数。所以安卓、上位机、SDK在切档的时候要注意切高温档的档位时，需要加上低温档的探测器总个数。
	//前面五个低温档的是一样的  后面五个高温档的是一样的
	//同一范围下的档位探测器配置目前一样，当不一样时必须更新。切换测温范围时需要重新配置探测器。
	return ITA_OK;
}

unsigned char * Parser::getConfBuf()
{
	if (!m_confBuf)
		m_confBuf = (unsigned char *)porting_calloc_mem(MAX_CONFIG_FILE_SIZE, 1, ITA_PARSER_MODULE);
	return m_confBuf;
}

void Parser::releaseConfBuf()
{
	if (m_confBuf)
	{
		porting_free_mem(m_confBuf);
		m_confBuf = NULL;
	}
}
extern string EncryptionAES(const string& strSrc);
extern string DecryptionAES(const string& strSrc);
ITA_RESULT Parser::parseConfig(unsigned char * buf, int dataSize, ITAConfig * pConfig)
{
	char*r = NULL;
	if (!buf || dataSize <= 0 || !pConfig)
		return ITA_ILLEGAL_PAPAM_ERR;
	//先解密
	char *plaintext = (char *)porting_calloc_mem(MAX_CONFIG_FILE_SIZE, 1, ITA_PARSER_MODULE);
	int plainSize = dataSize;
	if (m_isConfEncrypt)
	{
		string sSource;
		sSource.append((char *)buf);
		//cout << "解密前:" << endl << sSource << endl;
		string strPlain = DecryptionAES(sSource);
		//cout << "解密后:" << endl << strPlain << endl;
		memcpy(plaintext, strPlain.data(), strPlain.length());
		//printf("%s\r\n",plaintext);
		plainSize = (int)strPlain.length();
	}
	else
	{
		memcpy(plaintext, buf, plainSize);
	}
	//再解析
	int count = 0, lineSize = 0, mtConfCount = 0, ispConfCount = 0;
	char lineBuf[MAX_CONFIG_LINE_SIZE];
	ITA_CONFIG_TYPE configType = ITA_FUNCTION_CONFIG;
	while (count < plainSize)
	{
		memset(lineBuf, '\0', MAX_CONFIG_LINE_SIZE);
		lineSize = readLine((unsigned char *)(plaintext + count), plainSize - count, (unsigned char *)lineBuf, MAX_CONFIG_LINE_SIZE);
		count += lineSize;
		if (lineSize > 0)
		{
			const char * pStart = NULL;
			//解析配置类型
			pStart =  strstr (lineBuf,"FUNCTION");
			if ( pStart != NULL )
			{
				configType = ITA_FUNCTION_CONFIG;
				continue;
			}
			pStart =  strstr (lineBuf,"MTOPTION");
			if ( pStart != NULL )
			{
				configType = ITA_MT_CONFIG;
				continue;
			}
			pStart =  strstr (lineBuf, "ISP" );
			if ( pStart != NULL )
			{
				configType = ITA_ISP_CONFIG;
				continue;
			}
			//跳过注释或者无效行
			if ( strstr (lineBuf,"//") == lineBuf ||  strstr (lineBuf,"=")  == NULL )
				continue;
			if (ITA_FUNCTION_CONFIG == configType)
			{
				if (!pConfig->functionConfig)
				{
					pConfig->functionConfig = (ITAFunctionConfig *)porting_calloc_mem(1, sizeof(ITAFunctionConfig), ITA_PARSER_MODULE);
				}
				if (0 == stringToCharArray(lineBuf, pConfig->functionConfig->versionNumber, 64, (char*)"versionNumber"))
				{
					pConfig->functionConfig->versionNumberS = 1;
					continue;
				}
				if (0 == stringToInt(lineBuf, &pConfig->functionConfig->productType, (char*)"productType"))
				{
					pConfig->functionConfig->productTypeS = 1;
					continue;
				}
				if (0 == stringToFloat(lineBuf, &pConfig->functionConfig->shutterTempUpper, (char*)"shutterTempUpper"))
				{
					pConfig->functionConfig->shutterTempUpperS = 1;
					continue;
				}
				if (0 == stringToFloat(lineBuf, &pConfig->functionConfig->nucTempUpper, (char*)"nucTempUpper"))
				{
					pConfig->functionConfig->nucTempUpperS = 1;
					continue;
				}
				if (0 == stringToInt(lineBuf, &pConfig->functionConfig->nucIntervalBottom, (char*)"nucIntervalBottom"))
				{
					pConfig->functionConfig->nucIntervalBottomS = 1;
					continue;
				}
				if (0 == stringToInt(lineBuf, &pConfig->functionConfig->shutterIntervalBottom, (char*)"shutterIntervalBottom"))
				{
					pConfig->functionConfig->shutterIntervalBottomS = 1;
					continue;
				}
				if (0 == stringToInt(lineBuf, &pConfig->functionConfig->isSubAvgB, (char*)"isSubAvgB"))
				{
					pConfig->functionConfig->isSubAvgBS = 1;
					continue;
				}
				if (0 == stringToInt(lineBuf, &pConfig->functionConfig->shutterCollectInterval, (char*)"shutterCollectInterval"))
				{
					pConfig->functionConfig->shutterCollectIntervalS = 1;
					continue;
				}
				if (0 == stringToInt(lineBuf, &pConfig->functionConfig->collectInterval, (char*)"collectInterval"))
				{
					pConfig->functionConfig->collectIntervalS = 1;
					continue;
				}
				if (0 == stringToInt(lineBuf, &pConfig->functionConfig->collectNumberOfTimes, (char*)"collectNumberOfTimes"))
				{
					pConfig->functionConfig->collectNumberOfTimesS = 1;
					continue;
				}
				if (0 == stringToInt(lineBuf, &pConfig->functionConfig->bootTimeToNow1, (char*)"bootTimeToNow1"))
				{
					pConfig->functionConfig->bootTimeToNow1S = 1;
					continue;
				}
				if (0 == stringToInt(lineBuf, &pConfig->functionConfig->shutterPeriod1, (char*)"shutterPeriod1"))
				{
					pConfig->functionConfig->shutterPeriod1S = 1;
					continue;
				}
				if (0 == stringToInt(lineBuf, &pConfig->functionConfig->bootTimeToNow2, (char*)"bootTimeToNow2"))
				{
					pConfig->functionConfig->bootTimeToNow2S = 1;
					continue;
				}
				if (0 == stringToInt(lineBuf, &pConfig->functionConfig->shutterPeriod2, (char*)"shutterPeriod2"))
				{
					pConfig->functionConfig->shutterPeriod2S = 1;
					continue;
				}
				if (0 == stringToInt(lineBuf, &pConfig->functionConfig->bootTimeToNow3, (char*)"bootTimeToNow3"))
				{
					pConfig->functionConfig->bootTimeToNow3S = 1;
					continue;
				}
				if (0 == stringToInt(lineBuf, &pConfig->functionConfig->shutterPeriod3, (char*)"shutterPeriod3"))
				{
					pConfig->functionConfig->shutterPeriod3S = 1;
					continue;
				}
				if (0 == stringToInt(lineBuf, &pConfig->functionConfig->maxFramerate, (char*)"maxFramerate"))
				{
					pConfig->functionConfig->maxFramerateS = 1;
					continue;
				}
				if (0 == stringToInt(lineBuf, &pConfig->functionConfig->internalFlash, (char*)"internalFlash"))
				{
					pConfig->functionConfig->internalFlashS = 1;
					continue;
				}
			}
			else if (ITA_MT_CONFIG == configType)
			{
				r = strstr(lineBuf,"lensType");
				if (r != NULL )
				{
					if (!pConfig->mtConfig)
					{
						pConfig->mtConfig = (ITAMTConfig *)porting_calloc_mem(1, sizeof(ITAMTConfig), ITA_PARSER_MODULE);
						mtConfCount++;
					}
					else
					{
						//重新动态申请
						ITAMTConfig *tmp = (ITAMTConfig *)porting_calloc_mem(mtConfCount + 1, sizeof(ITAMTConfig), ITA_PARSER_MODULE);
						memcpy(tmp, pConfig->mtConfig, mtConfCount * sizeof(ITAMTConfig));
						porting_free_mem(pConfig->mtConfig);
						pConfig->mtConfig = tmp;
						mtConfCount++;
					}
				}
				if (0 == stringToInt(lineBuf, &(pConfig->mtConfig + (mtConfCount - 1))->lensType, (char*)"lensType"))
					continue;
				if (0 == stringToDistanceRange(lineBuf, &(pConfig->mtConfig + (mtConfCount - 1))->mtDistanceRangeN, &(pConfig->mtConfig + (mtConfCount - 1))->mtDistanceRangeF, (char*)"mtDistanceRange"))
					continue;
				if (0 == stringToInt(lineBuf, &(pConfig->mtConfig + (mtConfCount - 1))->mtType, (char*)"mtType"))
					continue;
				if (0 == stringToFloat(lineBuf, &(pConfig->mtConfig + (mtConfCount - 1))->correctDistance, (char*)"correctDistance"))
				{
					(pConfig->mtConfig + (mtConfCount - 1))->correctDistanceS = 1;
					continue;
				}
				if (0 == stringToFloat(lineBuf, &(pConfig->mtConfig + (mtConfCount - 1))->distance, (char*)"distance"))
				{
					(pConfig->mtConfig + (mtConfCount - 1))->distanceS = 1;
					continue;
				}
				if (0 == stringToFloat(lineBuf, &(pConfig->mtConfig + (mtConfCount - 1))->lowLensCorrK, (char*)"lowLensCorrK"))
				{
					(pConfig->mtConfig + (mtConfCount - 1))->lowLensCorrKS = 1;
					continue;
				}
				if (0 == stringToFloat(lineBuf, &(pConfig->mtConfig + (mtConfCount - 1))->highLensCorrK, (char*)"highLensCorrK"))
				{
					(pConfig->mtConfig + (mtConfCount - 1))->highLensCorrKS = 1;
					continue;
				}
				if (0 == stringToFloat(lineBuf, &(pConfig->mtConfig + (mtConfCount - 1))->lowShutterCorrCoff, (char*)"lowShutterCorrCoff"))
				{
					(pConfig->mtConfig + (mtConfCount - 1))->lowShutterCorrCoffS = 1;
					continue;
				}
				if (0 == stringToFloat(lineBuf, &(pConfig->mtConfig + (mtConfCount - 1))->highShutterCorrCoff, (char*)"highShutterCorrCoff"))
				{
					(pConfig->mtConfig + (mtConfCount - 1))->highShutterCorrCoffS = 1;
					continue;
				}
				if (0 == stringToInt(lineBuf, &(pConfig->mtConfig + (mtConfCount - 1))->mtDisType, (char*)"mtDisType"))
				{
					(pConfig->mtConfig + (mtConfCount - 1))->mtDisTypeS = 1;
					continue;
				}
				if (0 == stringToFloat(lineBuf, &(pConfig->mtConfig + (mtConfCount - 1))->coefA1, (char*)"coefA1"))
				{
					(pConfig->mtConfig + (mtConfCount - 1))->coefA1S = 1;
					continue;
				}
				if (0 == stringToFloat(lineBuf, &(pConfig->mtConfig + (mtConfCount - 1))->coefA2, (char*)"coefA2"))
				{
					(pConfig->mtConfig + (mtConfCount - 1))->coefA2S = 1;
					continue;
				}
				if (0 == stringToFloat(lineBuf, &(pConfig->mtConfig + (mtConfCount - 1))->coefB1, (char*)"coefB1"))
				{
					(pConfig->mtConfig + (mtConfCount - 1))->coefB1S = 1;
					continue;
				}
				if (0 == stringToFloat(lineBuf, &(pConfig->mtConfig + (mtConfCount - 1))->coefB2, (char*)"coefB2"))
				{
					(pConfig->mtConfig + (mtConfCount - 1))->coefB2S = 1;
					continue;
				}
				if (0 == stringToFloat(lineBuf, &(pConfig->mtConfig + (mtConfCount - 1))->coefC1, (char*)"coefC1"))
				{
					(pConfig->mtConfig + (mtConfCount - 1))->coefC1S = 1;
					continue;
				}
				if (0 == stringToFloat(lineBuf, &(pConfig->mtConfig + (mtConfCount - 1))->coefC2, (char*)"coefC2"))
				{
					(pConfig->mtConfig + (mtConfCount - 1))->coefC2S = 1;
					continue;
				}
				if (0 == stringToFloat(lineBuf, &(pConfig->mtConfig + (mtConfCount-1))->coefA3, (char*)"coefA3"))
				{
					(pConfig->mtConfig + (mtConfCount - 1))->coefA3S = 1;
					continue;
				}
				if (0 == stringToFloat(lineBuf, &(pConfig->mtConfig + (mtConfCount - 1))->coefB3, (char*)"coefB3"))
				{
					(pConfig->mtConfig + (mtConfCount - 1))->coefB3S = 1;
					continue;
				}
				if (0 == stringToFloat(lineBuf, &(pConfig->mtConfig + (mtConfCount - 1))->coefC3, (char*)"coefC3"))
				{
					(pConfig->mtConfig + (mtConfCount - 1))->coefC3S = 1;
					continue;
				}

			}
			else if (ITA_ISP_CONFIG == configType)
			{
				r = strstr(lineBuf,"lensType");
				if (r != NULL )
				{
					if (!pConfig->ispConfig)
					{
						pConfig->ispConfig = (ITAISPConfig *)porting_calloc_mem(1, sizeof(ITAISPConfig), ITA_PARSER_MODULE);
						ispConfCount++;
					}
					else
					{
						//重新动态申请
						ITAISPConfig *tmp = (ITAISPConfig *)porting_calloc_mem(ispConfCount + 1, sizeof(ITAISPConfig), ITA_PARSER_MODULE);
						memcpy(tmp, pConfig->ispConfig, ispConfCount * sizeof(ITAISPConfig));
						porting_free_mem(pConfig->ispConfig);
						pConfig->ispConfig = tmp;
						ispConfCount++;
					}
				}
				if (0 == stringToInt(lineBuf, &(pConfig->ispConfig + (ispConfCount - 1))->lensType, (char*)"lensType"))
					continue;
				if (0 == stringToInt(lineBuf, &(pConfig->ispConfig + (ispConfCount - 1))->mtType, (char*)"mtType"))
					continue;
				if (0 == stringToInt(lineBuf, &(pConfig->ispConfig + (ispConfCount - 1))->tffStd, (char*)"tffStd"))
				{
					(pConfig->ispConfig + (ispConfCount - 1))->tffStdS = 1;
					continue;
				}
				if (0 == stringToInt(lineBuf, &(pConfig->ispConfig + (ispConfCount - 1))->vStripeWinWidth, (char*)"vStripeWinWidth"))
				{
					(pConfig->ispConfig + (ispConfCount - 1))->vStripeWinWidthS = 1;
					continue;
				}
				if (0 == stringToInt(lineBuf, &(pConfig->ispConfig + (ispConfCount - 1))->vStripeStd, (char*)"vStripeStd"))
				{
					(pConfig->ispConfig + (ispConfCount - 1))->vStripeStdS = 1;
					continue;
				}
				if (0 == stringToInt(lineBuf, &(pConfig->ispConfig + (ispConfCount - 1))->vStripeWeightThresh, (char*)"vStripeWeightThresh"))
				{
					(pConfig->ispConfig + (ispConfCount - 1))->vStripeWeightThreshS = 1;
					continue;
				}
				if (0 == stringToInt(lineBuf, &(pConfig->ispConfig + (ispConfCount - 1))->vStripeDetailThresh, (char*)"vStripeDetailThresh"))
				{
					(pConfig->ispConfig + (ispConfCount - 1))->vStripeDetailThreshS = 1;
					continue;
				}
				if (0 == stringToInt(lineBuf, &(pConfig->ispConfig + (ispConfCount - 1))->hStripeWinWidth, (char*)"hStripeWinWidth"))
				{
					(pConfig->ispConfig + (ispConfCount - 1))->hStripeWinWidthS = 1;
					continue;
				}
				if (0 == stringToInt(lineBuf, &(pConfig->ispConfig + (ispConfCount - 1))->hStripeStd, (char*)"hStripeStd"))
				{
					(pConfig->ispConfig + (ispConfCount - 1))->hStripeStdS = 1;
					continue;
				}
				if (0 == stringToInt(lineBuf, &(pConfig->ispConfig + (ispConfCount - 1))->hStripeWeightThresh, (char*)"hStripeWeightThresh"))
				{
					(pConfig->ispConfig + (ispConfCount - 1))->hStripeWeightThreshS = 1;
					continue;
				}
				if (0 == stringToInt(lineBuf, &(pConfig->ispConfig + (ispConfCount - 1))->hStripeDetailThresh, (char*)"hStripeDetailThresh"))
				{
					(pConfig->ispConfig + (ispConfCount - 1))->hStripeDetailThreshS = 1;
					continue;
				}
				if (0 == stringToInt(lineBuf, &(pConfig->ispConfig + (ispConfCount - 1))->rnArithType, (char*)"rnArithType"))
				{
					(pConfig->ispConfig + (ispConfCount - 1))->rnArithTypeS = 1;
					continue;
				}
				if (0 == stringToInt(lineBuf, &(pConfig->ispConfig + (ispConfCount - 1))->distStd, (char*)"distStd"))
				{
					(pConfig->ispConfig + (ispConfCount - 1))->distStdS = 1;
					continue;
				}
				if (0 == stringToInt(lineBuf, &(pConfig->ispConfig + (ispConfCount - 1))->grayStd, (char*)"grayStd"))
				{
					(pConfig->ispConfig + (ispConfCount - 1))->grayStdS = 1;
					continue;
				}
				if (0 == stringToInt(lineBuf, &(pConfig->ispConfig + (ispConfCount - 1))->discardUpratio, (char*)"discardUpratio"))
				{
					(pConfig->ispConfig + (ispConfCount - 1))->discardUpratioS = 1;
					continue;
				}
				if (0 == stringToInt(lineBuf, &(pConfig->ispConfig + (ispConfCount - 1))->discardDownratio, (char*)"discardDownratio"))
				{
					(pConfig->ispConfig + (ispConfCount - 1))->discardDownratioS = 1;
					continue;
				}
				if (0 == stringToInt(lineBuf, &(pConfig->ispConfig + (ispConfCount - 1))->linearBrightness, (char*)"linearBrightness"))
				{
					(pConfig->ispConfig + (ispConfCount - 1))->linearBrightnessS = 1;
					continue;
				}
				if (0 == stringToInt(lineBuf, &(pConfig->ispConfig + (ispConfCount - 1))->linearContrast, (char*)"linearContrast"))
				{
					(pConfig->ispConfig + (ispConfCount - 1))->linearContrastS = 1;
					continue;
				}
				if (0 == stringToInt(lineBuf, &(pConfig->ispConfig + (ispConfCount - 1))->linearRestrainRangethre, (char*)"linearRestrainRangethre"))
				{
					(pConfig->ispConfig + (ispConfCount - 1))->linearRestrainRangethreS = 1;
					continue;
				}
				if (0 == stringToInt(lineBuf, &(pConfig->ispConfig + (ispConfCount - 1))->heqPlatThresh, (char*)"heqPlatThresh"))
				{
					(pConfig->ispConfig + (ispConfCount - 1))->heqPlatThreshS = 1;
					continue;
				}
				if (0 == stringToInt(lineBuf, &(pConfig->ispConfig + (ispConfCount - 1))->heqRangeMax, (char*)"heqRangeMax"))
				{
					(pConfig->ispConfig + (ispConfCount - 1))->heqRangeMaxS = 1;
					continue;
				}
				if (0 == stringToInt(lineBuf, &(pConfig->ispConfig + (ispConfCount - 1))->heqMidvalue, (char*)"heqMidvalue"))
				{
					(pConfig->ispConfig + (ispConfCount - 1))->heqMidvalueS = 1;
					continue;
				}
				if (0 == stringToInt(lineBuf, &(pConfig->ispConfig + (ispConfCount - 1))->iieEnhanceCoef, (char*)"iieEnhanceCoef"))
				{
					(pConfig->ispConfig + (ispConfCount - 1))->iieEnhanceCoefS = 1;
					continue;
				}
				if (0 == stringToInt(lineBuf, &(pConfig->ispConfig + (ispConfCount - 1))->iieGrayStd, (char*)"iieGrayStd"))
				{
					(pConfig->ispConfig + (ispConfCount - 1))->iieGrayStdS = 1;
					continue;
				}
				if (0 == stringToInt(lineBuf, &(pConfig->ispConfig + (ispConfCount - 1))->iieGaussStd, (char*)"iieGaussStd"))
				{
					(pConfig->ispConfig + (ispConfCount - 1))->iieGaussStdS = 1;
					continue;
				}
				if (0 == stringToInt(lineBuf, &(pConfig->ispConfig + (ispConfCount - 1))->iieDetailThr, (char*)"iieDetailThr"))
				{
					(pConfig->ispConfig + (ispConfCount - 1))->iieDetailThrS = 1;
					continue;
				}
				if (0 == stringToInt(lineBuf, &(pConfig->ispConfig + (ispConfCount - 1))->claheCliplimit, (char*)"claheCliplimit"))
				{
					(pConfig->ispConfig + (ispConfCount - 1))->claheCliplimitS = 1;
					continue;
				}
				if (0 == stringToInt(lineBuf, &(pConfig->ispConfig + (ispConfCount - 1))->claheHistMax, (char*)"claheHistMax"))
				{
					(pConfig->ispConfig + (ispConfCount - 1))->claheHistMaxS = 1;
					continue;
				}
				if (0 == stringToInt(lineBuf, &(pConfig->ispConfig + (ispConfCount - 1))->claheBlockWidth, (char*)"claheBlockWidth"))
				{
					(pConfig->ispConfig + (ispConfCount - 1))->claheBlockWidthS = 1;
					continue;
				}
				if (0 == stringToInt(lineBuf, &(pConfig->ispConfig + (ispConfCount - 1))->claheBlockHeight, (char*)"claheBlockHeight"))
				{
					(pConfig->ispConfig + (ispConfCount - 1))->claheBlockHeightS = 1;
					continue;
				}
				if (0 == stringToFloat(lineBuf, &(pConfig->ispConfig + (ispConfCount - 1))->spLaplaceWeight, (char*)"spLaplaceWeight"))
				{
					(pConfig->ispConfig + (ispConfCount - 1))->spLaplaceWeightS = 1;
					continue;
				}
				if (0 == stringToInt(lineBuf, &(pConfig->ispConfig + (ispConfCount - 1))->gmcType, (char*)"gmcType"))
				{
					(pConfig->ispConfig + (ispConfCount - 1))->gmcTypeS = 1;
					continue;
				}
				if (0 == stringToFloat(lineBuf, &(pConfig->ispConfig + (ispConfCount - 1))->gmcGamma, (char*)"gmcGamma"))
				{
					(pConfig->ispConfig + (ispConfCount - 1))->gmcGammaS = 1;
					continue;
				}
				if (0 == stringToInt(lineBuf, &(pConfig->ispConfig + (ispConfCount - 1))->adjustbcBright, (char*)"adjustbcBright"))
				{
					(pConfig->ispConfig + (ispConfCount - 1))->adjustbcBrightS = 1;
					continue;
				}
				if (0 == stringToInt(lineBuf, &(pConfig->ispConfig + (ispConfCount - 1))->adjustbcContrast, (char*)"adjustbcContrast"))
				{
					(pConfig->ispConfig + (ispConfCount - 1))->adjustbcContrastS = 1;
					continue;
				}
				if (0 == stringToInt(lineBuf, &(pConfig->ispConfig + (ispConfCount - 1))->zoomType, (char*)"zoomType"))
				{
					(pConfig->ispConfig + (ispConfCount - 1))->zoomTypeS = 1;
					continue;
				}
				if (0 == stringToInt(lineBuf, &(pConfig->ispConfig + (ispConfCount - 1))->mixThrLow, (char*)"mixThrLow"))
				{
					(pConfig->ispConfig + (ispConfCount - 1))->mixThrLowS = 1;
					continue;
				}
				if (0 == stringToInt(lineBuf, &(pConfig->ispConfig + (ispConfCount - 1))->mixThrHigh, (char*)"mixThrHigh"))
				{
					(pConfig->ispConfig + (ispConfCount - 1))->mixThrHighS = 1;
					continue;
				}
			}
			count += lineSize;
		}
		else
			break;
	}
	pConfig->mtConfigCount = mtConfCount;
	pConfig->ispConfigCount = ispConfCount;
	porting_free_mem(plaintext);
	return ITA_OK;
}

void Parser::setProductType(ITA_PRODUCT_TYPE type)
{
	m_type = type;
}

void Parser::setLogger(GuideLog * logger)
{
	m_logger = logger;
}

ITA_RESULT Parser::parseCalibrateParam(int cameraID, ITARegistry * registry, ITA_RANGE range, MtParams * mtParams)
{
	float fNearKf;	//近距离kf系数
	float fNearB;	//近距离偏置系数
	float fFarKf;	//远距离kf系数
	float fFarB;	//远距离偏置系数
	float fNearKf2;	//近距离kf系数
	float fNearB2;	//近距离偏置系数
	float fFarKf2;	//远距离kf系数
	float fFarB2;	//远距离偏置系数
	ITA_RESULT ret;

	if (!registry->ReadCalibrateParam)
	{
		m_logger->output(LOG_WARN, "ReadCalibrateParam Unregister.");
		return ITA_HAL_UNREGISTER;
	}
	if (!mtParams)
	{
		m_logger->output(LOG_WARN, "ReadCalibrateParam mtParams null.");
		return ITA_NULL_PTR_ERR;
	}
	ret = registry->ReadCalibrateParam(cameraID, range, ITA_NEAR_KF_AUTO, &fNearKf, registry->userParam);
	if(ITA_OK == ret)
		mtParams->fNearKf = fNearKf;
	ret = registry->ReadCalibrateParam(cameraID, range, ITA_NEAR_B_AUTO, &fNearB, registry->userParam);
	if (ITA_OK == ret)
		mtParams->fNearB = fNearB;
	ret = registry->ReadCalibrateParam(cameraID, range, ITA_FAR_KF_AUTO, &fFarKf, registry->userParam);
	if (ITA_OK == ret)
		mtParams->fFarKf = fFarKf;
	ret = registry->ReadCalibrateParam(cameraID, range, ITA_FAR_B_AUTO, &fFarB, registry->userParam);
	if (ITA_OK == ret)
		mtParams->fFarB = fFarB;
	ret = registry->ReadCalibrateParam(cameraID, range, ITA_NEAR_KF_MANUAL, &fNearKf2, registry->userParam);
	if (ITA_OK == ret)
		mtParams->fNearKf2 = fNearKf2;
	ret = registry->ReadCalibrateParam(cameraID, range, ITA_NEAR_B_MANUAL, &fNearB2, registry->userParam);
	if (ITA_OK == ret)
		mtParams->fNearB2 = fNearB2;
	ret = registry->ReadCalibrateParam(cameraID, range, ITA_FAR_KF_MANUAL, &fFarKf2, registry->userParam);
	if (ITA_OK == ret)
		mtParams->fFarKf2 = fFarKf2;
	ret = registry->ReadCalibrateParam(cameraID, range, ITA_FAR_B_MANUAL, &fFarB2, registry->userParam);
	if (ITA_OK == ret)
		mtParams->fFarB2 = fFarB2;
	m_logger->output(LOG_INFO, "RCP:%f %f %f %f %f %f %f %f",
		mtParams->fNearKf, mtParams->fNearB, mtParams->fFarKf, mtParams->fFarB,
		mtParams->fNearKf2, mtParams->fNearB2, mtParams->fFarKf2, mtParams->fFarB2);
	return ITA_OK;
}
/*参数bool shutterStatus：是否解析快门状态。MCU普通快门时不要解析，MCU NUC时需要解析。*/
ITA_RESULT Parser::parseParamLine(unsigned char * paramLine, int len, MtParams * mtParams, bool shutterStatus)
{
	ITA_RESULT ret = ITA_OK;
	static int count = 0;
	if (paramLine&&len)
	{
		if (ITA_256_TIMO == m_type)
		{
			if (ITA_MCU_X16 == m_mode)
			{
				//MCU+256模组，解析温传数据，快门状态由ITA库控制。
				short *pFrame = (short *)paramLine;
				/*int length = *(pFrame + 1);
				int w = *(pFrame + 2);
				int h = *(pFrame + 3);*/
				short realtimeShutterTempAD = *(pFrame + 9);
				short lensTempAD = *(pFrame + 10);
				short focalPlaneTempAD = *(pFrame + 11);
				float orinalShutterTemp = getTemp(*(pFrame + 8));
				float realtimeShutterTemp = getTemp(realtimeShutterTempAD);
				float lensTemp = getTemp(lensTempAD);
				float focalPlaneTemp = getFPATemp(focalPlaneTempAD);
				short nucShutterClosed = *(pFrame + 12);
				short shutterClosed = *(pFrame + 13);
				float nearB = 0.0;
				
				float nearKF = 0.0;
				float farB= 0.0;	
				float farKF= 0.0;	
				if (ITA_INDUSTRY_LOW == m_range)
				{
					nearB = *(pFrame + 231);
					nearKF = *(pFrame + 232);
					farB = *(pFrame + 234);
					farKF = *(pFrame + 235);
				}
				else if (ITA_INDUSTRY_HIGH == m_range)
				{
					nearB = *(pFrame + 238);
					nearKF = *(pFrame + 239);
					farB = *(pFrame + 240);
					farKF = *(pFrame + 241);
				}
				if (shutterStatus)
				{
					mtParams->bNucShutterFlag = (unsigned char)nucShutterClosed;
					mtParams->bOrdinaryShutter = (unsigned char)shutterClosed;
				}
				if(m_isCold)
					mtParams->fOrinalShutterTemp = orinalShutterTemp;
				mtParams->fRealTimeShutterTemp = realtimeShutterTemp;
				mtParams->fRealTimeLensTemp = lensTemp;
				mtParams->fRealTimeFpaTemp = focalPlaneTemp;
				mtParams->fNearKf = nearKF;
				mtParams->fNearB = nearB;
				mtParams->fFarKf = farKF;
				mtParams->fFarB = farB;
			}
			else if (ITA_Y16 == m_mode)
			{
				//ASIC+256模组，解析温传数据和快门状态。用户关闭ITA库快门策略。
				short * p = (short *)paramLine;
				unsigned short a = p[0], b = p[2], c = p[3];
				short totalLen = p[1]; //单位short
				unsigned char contType, contLen;
				if (a == 0xAA55)
				{
					if (b == 256 && c == 192)
					{
						if (m_isCold)
							mtParams->fOrinalShutterTemp = (float)(p[8] / 100.0);
						mtParams->fRealTimeShutterTemp = (float)(p[9] / 100.0);
						mtParams->fRealTimeLensTemp = (float)(p[10] / 100.0);
						mtParams->fRealTimeFpaTemp = (float)(p[11] / 100.0);
						mtParams->bOrdinaryShutter = (unsigned char)p[13];
						mtParams->bNucShutterFlag = (unsigned char)p[12];
						m_avgB = p[38];
						//ASIC模式一次和二次校温参数均从参数行中读取。
						//目前校温只用到远距离，近距离是默认值。
						if (ITA_INDUSTRY_HIGH == m_range)
						{
							mtParams->fNearB = p[238];
							mtParams->fNearKf = p[239];
							mtParams->fFarB = p[240];
							mtParams->fFarKf = p[241];
							mtParams->fFarB2 = p[242];
							mtParams->fFarKf2 = p[243];
						}
						else if (ITA_INDUSTRY_LOW == m_range)
						{
							mtParams->fFarB = p[238];
							mtParams->fFarKf = p[239];
						}
						else if (ITA_HUMAN_BODY == m_range)
						{
							mtParams->fNearB = p[234];
							mtParams->fNearKf = p[235];
							mtParams->fFarB = p[230];
							mtParams->fFarKf = p[231];
						}
						//使用sdk内部算法计算档位，不用p[50]档位信息。
					}
					else
					{
						//按照标准协议解析
						if (!count)
						{
							unsigned char p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12;
							unsigned short w, h;
							p1 = b >> 8; //0:未定义 1:安霸;  2:海思;   3:高通;  4:imx6;
							p2 = b >> 4 & 0xF; //0:未定义  1:FPGA;   2:芯晟ASIC;   3:ASIC1.0;  4:ASIC2.0  5:CPLD
							p3 = b & 0xF; //0:网络测温;   1:手持测温;   2:模组测温;
							p4 = c >> 12; //协议版本号
							p5 = c >> 6 & 0x3F;
							p6 = c & 0x3F;
							b = p[4];
							c = p[5];
							p7 = b >> 8; //产品型号
							p8 = b & 0xFF;
							p9 = c >> 8;
							p10 = c & 0xFF;
							w = p[9];
							h = p[10];
							b = p[11];
							p11 = b >> 8;	//传输格式 0:X16+参数行；1:Y16+参数行；2:Y16 + 参数行 + Y8；
							p12 = b & 0xFF;	//帧频
							m_logger->output(LOG_INFO, "ParamLine platform:%d %d %d v:%d %d %d proj:%c %c %d %c, %d %d %d %d", 
								p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, w, h, p11, p12);
						}
						count++;
						p = (short *)paramLine + 15;
						while (totalLen > 0)
						{
							contType = p[0] >> 8;
							contLen = p[0] & 0xFF; //单位short
							if (contType == CONTENT_MT_GENERAL)
							{
								//解析测温信息
								mtParams->m_fieldType = p[1] >> 8 & 0xF; //以前读取数据包。ZC08B项目存在多种镜头，通过参数行能自动识别
								//mtParams->n_fpaGear = p[1] >> 4 & 0xF; //模组测温算法计算，数据包中包含各档位曲线。
								mtParams->mtType = p[1] & 0xF; //用户切换发命令给前端，测温范围与其它参数保持一致，否则结果异常。
								m_last_lltt = m_lltt;
								m_lltt = p[1] >> 15 & 0x1;//记录是否是免校温类型
								mtParams->sCurrentShutterValue = p[2]; //
								m_avgB = p[3];
								if (m_isCold)
									mtParams->fOrinalShutterTemp = (float)(p[9] / 100.0);
								mtParams->fRealTimeShutterTemp = (float)(p[11] / 100.0);
								mtParams->fRealTimeLensTemp = (float)(p[12] / 100.0);
								mtParams->fRealTimeFpaTemp = (float)(p[13] / 100.0);
								mtParams->fCurrentShutterTemp = (float)(p[15] / 100.0); //
								mtParams->fCurrentLensTemp = (float)(p[17] / 100.0); //
								/*if (mtParams->bLogPrint)
									m_logger->output(LOG_DEBUG, "d1 %#x %#x %#x %#x", p[0], p[1], p[2], p[3]);*/
							}
							else if (contType == CONTENT_STATUS)
							{
								/*解析快门状态
								[3]	NUC快门补偿标志位；1:NUC快门补偿中 0:NUC快门补偿结束
								[2]	单次快门标志位；1：闭合；0：弹开。
								[1]	本底快门补偿状态；1:本底快门补偿进行中  0:非补偿状态
								[0]	定格；1:定格  0:无定格*/
								/*asic定格标志的生命周期会覆盖快门标志，因此解析定格标志的数据*/
								mtParams->bOrdinaryShutter = p[1] & 0x1;
								mtParams->bNucShutterFlag = p[1] >> 3 & 0x1;
								if (m_lltt) {
									mtParams->m_cflType = mtParams->m_fieldType;
									mtParams->m_fieldType = p[13] >> 8 & 0xFF;
								}

								if (mtParams->bLogPrint) {
									m_logger->output(LOG_DEBUG,"m_lltt:%d,m_fieldType:%d,%d %d", m_lltt, mtParams->m_fieldType, mtParams->bOrdinaryShutter, mtParams->bNucShutterFlag);
								}
								/*if (mtParams->bLogPrint)
									m_logger->output(LOG_DEBUG, "d2 %#x %#x %#x %#x", p[0], p[1], p[2], p[3]);*/
								break;
							}
							else if (contType == CONTENT_MT_EXPERT)
							{
								if (ITA_INDUSTRY_HIGH == mtParams->mtType)
								{
									m_lastHighSCoff = mtParams->fHighShutterCorrCoff;
									m_lastHighLCork = mtParams->fHighLensCorrK;
									mtParams->fHighShutterCorrCoff = (float)p[2];		//高温挡快门校正系数(工业专用)
									mtParams->fHighLensCorrK = (float)p[4];	//实时高温档镜筒温漂修正系数
								}
								else
								{
									m_lastLowSCoff = mtParams->fLowShutterCorrCoff;
									m_lastLowLCork = mtParams->fLowLensCorrK;
									mtParams->fLowShutterCorrCoff = (float)p[2];		//常温挡快门校正系数(工业专用)
									mtParams->fLowLensCorrK = (float)p[4];	//实时常温档镜筒温漂修正系数
								}
								isCorrChanged(mtParams->fHighShutterCorrCoff, mtParams->fHighLensCorrK, mtParams->fLowShutterCorrCoff, mtParams->fLowLensCorrK);
								mtParams->bAtmosphereCorrection = p[5] >> 7 & 0x1;//大气透过率修正开关
								mtParams->bEnvironmentCorrection = p[5] >> 6 & 0x1;//环温修正开关
								mtParams->bLensCorrection = p[5] >> 5 & 0x1;//镜筒温漂校正开关
								mtParams->bShutterCorrection = p[5] >> 4 & 0x1;//快门温漂校正开关
								mtParams->bDistanceCompensate = p[5] >> 2 & 0x1;//距离补偿开关
								mtParams->bEmissCorrection = p[5] >> 1 & 0x1;//发射率校正开关
								mtParams->bTransCorrection = p[5] & 0x1;//透过率校正开关
								
								mtParams->fEmiss = (p[6] >> 8 & 0xFF) / 100.0;//发射率(工业专用)
								mtParams->fHumidity = (p[6] & 0xFF) / 100.0;//湿度(0.01-1.0,默认0.6)
								mtParams->fEnvironmentTemp = p[7] >> 8 & 0xFF;//环境温度
								mtParams->fDistance = (p[7] & 0xFF) / 10.0;	//目标距离
								//mtParams->fReflectTemp = p[8] >> 8 & 0xFF;	//反射温度(工业专用)
								mtParams->fTransmit = (p[8] & 0xFF) / 100.0;	//窗口透过率(工业专用)
								mtParams->fNearKf = (float)p[9];	//近距离kf系数
								mtParams->fNearB = (float)p[10];	//近距离偏置系数
								mtParams->fFarKf = (float)p[11];	//远距离kf系数
								mtParams->fFarB = (float)p[12];	//远距离偏置系数
								mtParams->fNearKf2 = (float)p[13];	//近距离kf系数
								mtParams->fNearB2 = (float)p[14];	//近距离偏置系数
								mtParams->fFarKf2 = (float)p[15];	//远距离kf系数
								mtParams->fFarB2 = (float)p[16];	//远距离偏置系数
								mtParams->fWindowTemperature = (float)p[17];//窗口温度
								mtParams->fAtmosphereTemperature = (float)p[18];//大气温度
								mtParams->fAtmosphereTransmittance = p[19] / 100.0;//大气透过率
								mtParams->fReflectTemp = (float)p[20]; //反射温度(工业专用)
							}
							/*if (mtParams->bLogPrint)
								m_logger->output(LOG_DEBUG, "d3 %d %d", contType, contLen);*/
							p += contLen + 1;
							totalLen -= contLen;
						}
					}
				}
				else
				{
					m_logger->output(LOG_ERROR, "product=%d mode=%d parseParamLine failed! %#x %d %d", m_type, m_mode, p[0], p[2], p[3]);
					ret = ITA_INVALIDE_PARAMLINE;
				}		
			}
		}
		else if (ITA_120_TIMO == m_type)
		{
			if (ITA_MCU_X16 == m_mode)
			{
				//MCU+120模组，解析温传数据，快门状态由ITA库控制。
				short* pFrame = (short*)paramLine;
				/*short p0 = *pFrame;
				short pVersion = *(pFrame + 1);
				short pWidth = *(pFrame + 2);
				short pHeight = *(pFrame + 3);*/
				short orinalSt = *(pFrame + 8);
				if (m_isCold)
					mtParams->fOrinalShutterTemp = getTemp(orinalSt);
				short st = *(pFrame + 9);
				mtParams->fRealTimeShutterTemp = getTemp(st);
				short jt = *(pFrame + 10);
				mtParams->fRealTimeLensTemp = getTemp(jt);
				short jpm = *(pFrame + 11);
				mtParams->fRealTimeFpaTemp = getFPATemp(jpm);
				//必须解析本底状态标记，NUC由MCU完成，通过参数行解析结果通知NUC完成。
				//如果不解析，那么nuc状态一直为2，采本底接口中途返回，一直不能打开快门。
				short shutterDownMark = *(pFrame + 12);
				//short bMark = *(pFrame + 13);	//CB360产品这个标志位无效，一直为0。
				/*解析测温范围,发射率，目标距离*/
				mtParams->mtType = *(pFrame + 48);
				mtParams->fEmiss = ((pFrame[50]) & 0xff) / 100.0f;
				mtParams->fDistance = pFrame[84] / 10.0f;
				if (shutterStatus)
				{
					//mtParams->bNucShutterFlag = (unsigned char)bMark;
					mtParams->bOrdinaryShutter = (unsigned char)shutterDownMark;
				}
				if (mtParams->mtType == 0 || mtParams->mtType == 1)
				{
					////低温校温参数
					/*mtParams->fCoefB1 = (short)getBitsValue(framePrame, 150, 2, 0, 16);
					mtParams->fCoefB2 = (short)getBitsValue(framePrame, 152, 2, 0, 16);
					mtParams->fCoefC1 = (short)getBitsValue(framePrame, 154, 2, 0, 16);
					mtParams->fCoefC2 = (short)getBitsValue(framePrame, 156, 2, 0, 16);*/
					mtParams->fNearB = *(pFrame + 79);
					mtParams->fNearKf = *(pFrame + 80);
					/*short nearKf ：160+161字节
					short nearB：158+159字节
					short farKf：166+167字节
					short farB：164+165字节*/
					mtParams->fFarB = *(pFrame + 82);	//人体测温新增参数
					mtParams->fFarKf = *(pFrame + 83);	//人体测温新增参数
					mtParams->fNearB2 = *(pFrame + 85);
					mtParams->fNearKf2 = *(pFrame + 86);
					mtParams->fFarB2 = *(pFrame + 87);
					mtParams->fFarKf2 = *(pFrame + 88);
					mtParams->fLowShutterCorrCoff = (float)(*(pFrame + 97));
					mtParams->fLowLensCorrK = (float)(*(pFrame + 98));
				}
				else {
					mtParams->fNearB = *(pFrame + 89);
					mtParams->fNearKf = *(pFrame + 90);
					mtParams->fFarB = *(pFrame + 91);
					mtParams->fFarKf = *(pFrame + 92);
					mtParams->fNearB2 = *(pFrame + 93);
					mtParams->fNearKf2 = *(pFrame + 94);
					mtParams->fFarB2 = *(pFrame + 95);
					mtParams->fFarKf2 = *(pFrame + 96);
					mtParams->fHighShutterCorrCoff = (float)(*(pFrame + 99)) ;
					mtParams->fHighLensCorrK = (float)(*(pFrame + 100));
				}
				isCorrChanged(mtParams->fHighShutterCorrCoff, mtParams->fHighLensCorrK, mtParams->fLowShutterCorrCoff, mtParams->fLowLensCorrK);
				mtParams->bShutterCorrection = *(pFrame + 101);
				mtParams->bLensCorrection = *(pFrame + 102);
				//Kf2范围为：0.9~1.1，B2范围为-3~3。如果没有二次校过温，KF2=1，B2=0。
				if (mtParams->fNearKf2 < 6000 || mtParams->fNearKf2 > 14000)
				{
					mtParams->fNearKf2 = 10000;
					mtParams->fNearB2 = 0;
				}
				if (mtParams->fFarKf2 < 6000 || mtParams->fFarKf2  > 14000)
				{
					mtParams->fFarKf2 = 10000;
					mtParams->fFarB2 = 0;
				}
				if (mtParams->bLogPrint)
				{
					m_logger->output(LOG_DEBUG, "NearB:%.2f,NearKf:%.2f,FarB:%.2f,FarKf:%.2f,NearB2:%.2f,NearKf2:%.2f,FarB2:%.2f,FarKf2:%.2f,fEmiss:%.2f,fDistance:%.2f,%.2f,%.2f,%.2f,%.2f",
													mtParams->fNearB, mtParams->fNearKf, mtParams->fFarB, mtParams->fFarKf, mtParams->fNearB2, mtParams->fNearKf2, mtParams->fFarB2, mtParams->fFarKf2, mtParams->fEmiss,mtParams->fDistance,
									mtParams->fLowShutterCorrCoff, mtParams->fLowLensCorrK, mtParams->fHighShutterCorrCoff, mtParams->fHighLensCorrK);
				}
			}
		}
		else
		{
			m_logger->output(LOG_ERROR, "parseParamLine m_type=%d", m_type);
			ret = ITA_UNSUPPORT_OPERATION;
		}
	}
	else
	{
		/*模拟数据测试*/
		mtParams->fRealTimeFpaTemp = 31.5;
		mtParams->fRealTimeShutterTemp = 30.5;
		mtParams->fRealTimeLensTemp = 30;
	}
	if (mtParams->bLogPrint)
		m_logger->output(LOG_DEBUG, "paramLine p=%d m=%d  F=%f S=%f L=%f %d %hhu %hhu %d",
			m_type, m_mode, mtParams->fRealTimeFpaTemp, mtParams->fRealTimeShutterTemp, mtParams->fRealTimeLensTemp, m_avgB, mtParams->bOrdinaryShutter, mtParams->bNucShutterFlag, m_isCoffChange);
	return ret;
}

ITA_RESULT Parser::parseTECLine(unsigned char *paramLine, int len, IMAGE_MEASURE_STRUCT *tecParams)
{
	/*
	int nGear;												//测温档位, 0:低温档, 1:常温挡
	float fRealTimeShutterTemp;								//实时快门温度
	float fOrinalShutterTemp;								//开机快门温
	float fRealTimeLensTemp;								//实时镜筒温
	float fCurrentLensTemp;									//最近一次打快门时的镜筒温
	unsigned char bOrdinaryShutter;//普通快门标志位
	unsigned char bNucShutterFlag;//NUC快门标志位
	*/
	static int count = 0;
	short prefix, length, *line, productT;
	if (!paramLine || !tecParams)
		return ITA_NULL_PTR_ERR;
	if (len <= 0)
		return ITA_ARG_OUT_OF_RANGE;
	if (m_type < ITA_HANDHELD_TEC)
		return ITA_UNSUPPORT_OPERATION;
	line = (short *)paramLine;
	prefix = *line;
	length = *(line + 1);
	productT = *(line + 68);  //ZC08A产品型号
	if (0x55AA == prefix && 0x38 == length)
	{
		//ZU08D产品
		//float fpaTemp = (*(line + 7))/100.0;
		tecParams->avgB = *(line + 17);
		tecParams->fRealTimeShutterTemp = (float)((*(line + 24)) / 100.0);
		tecParams->fCurrentLensTemp = (float)((*(line + 25)) / 100.0);
		//float currentShutterTemp = (*(line + 26)) / 100.0;
		tecParams->fOrinalShutterTemp = (float)((*(line + 27)) / 100.0);
		tecParams->bOrdinaryShutter = (*(line + 28)) & 0xFF;
		tecParams->fRealTimeLensTemp = (float)((*(line + 37)) / 100.0);
		tecParams->nGear = *(line + 43);
		tecParams->fAmbient = tecParams->fOrinalShutterTemp;
		//解析测温范围和镜头类型
		tecParams->mtType = *(line + 18);
		tecParams->lensType = (*(line + 15)) & 0x3;
	}
	else if ((short)0xAA55 == prefix && (short)0xBB55 == *(line + 128))	//必须保留short否则不等。
	{
		//ZC07A产品
		tecParams->fRealTimeShutterTemp = (float)((*(line + 33)) / 100.0);
		tecParams->fOrinalShutterTemp = (float)((*(line + 31)) / 100.0);
		tecParams->fRealTimeLensTemp = (float)((*(line + 32)) / 100.0);
		unsigned char shutterFlag = (*(line + 11)) >> 8 & 0x3;
		if (!tecParams->bOrdinaryShutter && 1 == shutterFlag)
		{	//快门刚闭合时记录镜筒温
			tecParams->fCurrentLensTemp = tecParams->fRealTimeLensTemp;
		}else if(!tecParams->fCurrentLensTemp)
			tecParams->fCurrentLensTemp = tecParams->fRealTimeLensTemp;
		tecParams->bOrdinaryShutter = shutterFlag;
		short nGear = (*(line + 21)) >> 4 & 0xF;
		//ZC07A档位是1、2，算法只对0、1做处理，这里兼容下，否则反算Y16为0。
		if (nGear > 0)
			nGear--;
		tecParams->nGear = nGear;
		tecParams->fAmbient = tecParams->fOrinalShutterTemp;
		//解析测温范围和镜头类型
		tecParams->mtType = (*(line + 21)) & 0x3;
		tecParams->lensType = (*(line + 26)) >> 4 & 0xF;
	}
	else if ((short)0xAA55 == prefix && 0x4308 == productT)	//必须保留short否则不等。
	{
		//ZC08A产品
		tecParams->fRealTimeShutterTemp = (float)((*(line + 28)) / 100.0);
		tecParams->fOrinalShutterTemp = (float)((*(line + 27)) / 100.0);
		tecParams->fRealTimeLensTemp = (float)((*(line + 29)) / 100.0);
		unsigned char shutterFlag = (*(line + 25)) >> 1 & 0x3;
		tecParams->fCurrentLensTemp = (float)((*(line + 31)) / 100.0);
		tecParams->bOrdinaryShutter = shutterFlag;
		short nGear = (*(line + 26)) >> 3 & 0xF;
		//ZC08A档位是1、2，算法只对0、1做处理，这里兼容下，否则反算Y16为0。
		if (nGear > 0)
			nGear--;
		tecParams->nGear = nGear;
		tecParams->fAmbient = tecParams->fOrinalShutterTemp;
		//解析测温范围和镜头类型
		tecParams->mtType = (*(line + 26)) & 0x7;
		//ZC08A范围是0、1、2，算法只对大于0做环温修正，这里兼容下。
		tecParams->mtType++;
		tecParams->lensType = (*(line + 70)) >> 1 & 0xF;
		//免校温镜头
		//int ltt = (*(line + 120)) & 0xFF;
		m_last_lltt = m_lltt;
		m_lltt = (*(line + 120)) >> 8 & 0x1;
		//下面是旧协议，废弃。
		//tecParams->fRealTimeShutterTemp = (float)((*(line + 18)) / 100.0);
		//tecParams->fOrinalShutterTemp = (float)((*(line + 17)) / 100.0);
		//tecParams->fRealTimeLensTemp = (float)((*(line + 19)) / 100.0);
		//unsigned char shutterFlag = (*(line + 15)) >> 1 & 0x3;
		//tecParams->fCurrentLensTemp = (float)((*(line + 21)) / 100.0);
		////if (!tecParams->bOrdinaryShutter && 1 == shutterFlag)
		////{	//快门刚闭合时记录镜筒温
		////	tecParams->fCurrentLensTemp = tecParams->fRealTimeLensTemp;
		////}
		////else if (!tecParams->fCurrentLensTemp)
		////	tecParams->fCurrentLensTemp = tecParams->fRealTimeLensTemp;
		//tecParams->bOrdinaryShutter = shutterFlag;
		//tecParams->nGear = (*(line + 16)) >> 3 & 0xF;
		//tecParams->fAmbient = tecParams->fOrinalShutterTemp;
		////解析测温范围和镜头类型
		//tecParams->mtType = (*(line + 16)) & 0x7;
		//tecParams->lensType = (*(line + 50)) >> 1 & 0xF;
	}
	else if ((short)0xAA55 == prefix)
	{
		//统一参数行规范//按照标准协议解析
		short * p = (short *)paramLine;
		unsigned short b = p[2], c = p[3];
		short totalLen = p[1]; //单位short
		unsigned char contType, contLen;
		if (!count)
		{
			unsigned char p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12;
			unsigned short w, h;
			p1 = b >> 8; //0:未定义 1:安霸;  2:海思;   3:高通;  4:imx6;
			p2 = b >> 4 & 0xF; //0:未定义  1:FPGA;   2:芯晟ASIC;   3:ASIC1.0;  4:ASIC2.0  5:CPLD
			p3 = b & 0xF; //0:网络测温;   1:手持测温;   2:模组测温;
			p4 = c >> 12; //协议版本号
			p5 = c >> 6 & 0x3F;
			p6 = c & 0x3F;
			b = p[4];
			c = p[5];
			p7 = b >> 8; //产品型号
			p8 = b & 0xFF;
			p9 = c >> 8;
			p10 = c & 0xFF;
			w = p[9];
			h = p[10];
			b = p[11];
			p11 = b >> 8;	//传输格式 0:X16+参数行；1:Y16+参数行；2:Y16 + 参数行 + Y8；
			p12 = b & 0xFF;	//帧频
			m_logger->output(LOG_INFO, "parseTECLine platform:%d %d %d v:%d %d %d proj:%c %c %d %c, %d %d %d %d",
				p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, w, h, p11, p12);
		}
		count++;
		p = (short *)paramLine + 15;
		while (totalLen > 0)
		{
			contType = p[0] >> 8;
			contLen = p[0] & 0xFF; //单位short
			if (contType == CONTENT_MT_GENERAL)
			{
				//解析测温信息
				tecParams->lensType = p[1] >> 8 & 0xF; //前端识别
				tecParams->nGear = p[1] >> 4 & 0xF; //前端计算
				tecParams->mtType = p[1] & 0xF; //用户切换发命令给前端
				//tecParams->sCurrentShutterValue = p[2]; //
				m_avgB = p[3];
				tecParams->avgB = m_avgB;
				tecParams->fOrinalShutterTemp = (float)(p[9] / 100.0);
				tecParams->fRealTimeShutterTemp = (float)(p[11] / 100.0);
				tecParams->fRealTimeLensTemp = (float)(p[12] / 100.0);
				//tecParams->fRealTimeFpaTemp = (float)(p[13] / 100.0);
				//tecParams->fCurrentShutterTemp = (float)(p[15] / 100.0); //
				tecParams->fCurrentLensTemp = (float)(p[17] / 100.0); //
				tecParams->fAmbient = tecParams->fOrinalShutterTemp;
				if (tecParams->bLogPrint)
					m_logger->output(LOG_DEBUG, "d1 %#x %#x %#x %#x", p[0], p[1], p[2], p[3]);
			}
			else if (contType == CONTENT_STATUS)
			{
				//解析快门状态
				tecParams->bOrdinaryShutter = p[1] & 0x1;
				tecParams->bNucShutterFlag = p[1] >> 3 & 0x1;
				if (tecParams->bLogPrint)
					m_logger->output(LOG_DEBUG, "d2 %#x %#x %#x %#x", p[0], p[1], p[2], p[3]);
				m_burnInfo.burnSwitch = p[15] & 0x1;
				m_burnInfo.burnFlag = p[15] >> 1 & 0x1;
				m_burnInfo.burnDelayTime = p[15] >> 2 & 0x1FF;
				m_burnInfo.burnThreshold = p[16] & 0xFF;
				if (tecParams->bLogPrint) {
					m_logger->output(LOG_DEBUG, "bs:%d,bf:%d,bdt:%d,bt:%d", m_burnInfo.burnSwitch, m_burnInfo.burnFlag, m_burnInfo.burnDelayTime, m_burnInfo.burnThreshold);
				}
				break;
			}
			p += contLen + 1;
			totalLen -= contLen;
		}
	}
	else
	{
		if (tecParams->bLogPrint)
			m_logger->output(LOG_WARN, "invalide %#x %#x %#x %#x", line[0], line[1], line[2], line[3]);
		return ITA_INVALIDE_PARAMLINE;
	}
	if (tecParams->bLogPrint)
		m_logger->output(LOG_DEBUG, "TECLine l=%d  r=%d g=%d S=%f L=%f %d %hhu %hhu lltt=%d last=%d",
			tecParams->lensType, tecParams->mtType, tecParams->nGear, tecParams->fRealTimeShutterTemp, tecParams->fRealTimeLensTemp, m_avgB, tecParams->bOrdinaryShutter, tecParams->bNucShutterFlag, m_lltt, m_last_lltt);
	return ITA_OK;
}

ITA_RESULT Parser::getSensorTemp(int cameraID, ITARegistry * registry, MtParams * mtParams)
{
	ITA_SENSOR_VALUE sensorValue;
	int ADValue;
	float tempValue;
	ITA_RESULT result;

	result = registry->GetSensorTemp(cameraID, ITA_SHUTTER_TEMP, &sensorValue, &ADValue, &tempValue, registry->userParam);
	if (result < 0)
	{
		m_logger->output(LOG_ERROR, "GetSensorShutterTemp %d", result);
		return result;
	}
	if (ITA_TEMP_VALUE == sensorValue)
	{
		mtParams->fRealTimeShutterTemp = tempValue;
	}
	else if (ITA_AD_VALUE == sensorValue)
	{
		mtParams->fRealTimeShutterTemp = getTemp((short)ADValue);
	}
	result = registry->GetSensorTemp(cameraID, ITA_LENS_TEMP, &sensorValue, &ADValue, &tempValue, registry->userParam);
	if (result < 0)
	{
		m_logger->output(LOG_ERROR, "GetSensorLensTemp %d", result);
		return result;
	}
	if (ITA_TEMP_VALUE == sensorValue)
	{
		mtParams->fRealTimeLensTemp = tempValue;
	}
	else if (ITA_AD_VALUE == sensorValue)
	{
		mtParams->fRealTimeLensTemp = getTemp((short)ADValue);
	}
	result = registry->GetSensorTemp(cameraID, ITA_FPA_TEMP, &sensorValue, &ADValue, &tempValue, registry->userParam);
	if (result < 0)
	{
		m_logger->output(LOG_ERROR, "GetSensorFPATemp %d", result);
		return result;
	}
	if (ITA_TEMP_VALUE == sensorValue)
	{
		mtParams->fRealTimeFpaTemp = tempValue;
	}
	else if (ITA_AD_VALUE == sensorValue)
	{
		mtParams->fRealTimeFpaTemp = getFPATemp((short)ADValue);
	}
	float fpa_shutter_temp_error;
	float lens_shutter_temp_error;
	float fpa_lens_temp_error;
	fpa_shutter_temp_error = mtParams->fRealTimeFpaTemp - mtParams->fRealTimeShutterTemp;
	lens_shutter_temp_error = mtParams->fRealTimeLensTemp - mtParams->fRealTimeShutterTemp;
	fpa_lens_temp_error = mtParams->fRealTimeLensTemp - mtParams->fRealTimeFpaTemp;
	if ((fabs(fpa_shutter_temp_error) > 15) || (fabs(lens_shutter_temp_error) > 15) || (fabs(fpa_lens_temp_error) > 15))
	{
		m_logger->output(LOG_ERROR, "GetSensorTemp FpaTemp=%f ShutterTemp=%f LensTemp=%f",
			mtParams->fRealTimeFpaTemp, mtParams->fRealTimeShutterTemp, mtParams->fRealTimeLensTemp);
	}
	if(mtParams->bLogPrint)
		m_logger->output(LOG_DEBUG, "GetSensor Fpa=%f Shutter=%f Lens=%f", 
			mtParams->fRealTimeFpaTemp, mtParams->fRealTimeShutterTemp, mtParams->fRealTimeLensTemp);
	return result;
}

bool Parser::isLensChanged()
{
	bool ret = (m_last_lltt == m_lltt) ? false : true;
	return ret;
}

void Parser::isCorrChanged(float HighSCoff, float HighLCork, float LowSCoff, float LowLCork)
{
	if (m_lastHighSCoff != HighSCoff || m_lastHighLCork != HighLCork || m_lastLowSCoff != LowSCoff || m_lastLowLCork != LowLCork)
	{
		m_isCoffChange = true;
	}
	else
	{
		m_isCoffChange = false;
	}
}

float Parser::getTemp(short ad)
{
	float temp = 0.0f;
	switch (m_type)
	{
	case ITA_256_TIMO:
	case ITA_120_TIMO:
	{
		double temp1 = -(0.000000000040235 * ad * ad * ad);

		double temp2 = (0.000001218402729 * ad * ad);

		double temp3 = -(0.018218076216914 * ad);

		double y = temp1 + temp2 + temp3 + 127.361304901973000;

		temp = (float)((y * 100) / 100.0f);
		break;
	}
	default:
		break;
	}
	return temp;
}

float Parser::getFPATemp(short ad)
{
	float temp = 0.0f;
	switch (m_type)
	{
	case ITA_256_TIMO:
	{
		//float Tt = (float)(1.25 * ad - 16100);   //精度为0.01(即32℃的焦温Tt值为3200)
		float Tt = (float)(1.25 * ad - 15800);   //与采集端保持一致，解决焦温低3℃的问题。
		temp = Tt / 100;
		break;
	}
	case ITA_120_TIMO:
	{
		double y = -0.0201 * ad + 371.29;
		temp = (float)((y * 100) / 100.0f);
		break;
	}
	default:
		break;
	}
	return temp;
}
/*读取一行数据，返回该行的长度。*/
int Parser::readLine(unsigned char *buf, int dataSize, unsigned char *pLineBuf, int lineBufSize)
{
	int len = 0;
	for (int i = 0; i < dataSize; i++)
	{
		if (i >= lineBufSize)
		{
			len = i;
			break;
		}
		pLineBuf[i] = buf[i];
		if (pLineBuf[i] == '\n')
		{
			len = i;
			break;
		}
	}
	return len + 1;
}

int Parser::stringToInt(char *line, int *a, char * s)
{
	const char* po = NULL;
	const char* pm = NULL;
	const char* ret = NULL;
	char LineBuf[MAX_CONFIG_LINE_SIZE];
	//char LineBuf1[MAX_CONFIG_LINE_SIZE];
	
	memset( LineBuf, 0x00, MAX_CONFIG_LINE_SIZE );
	//memset( LineBuf1, 0x00, MAX_CONFIG_LINE_SIZE );
	
	ret = strstr(line,s);
	
	po =  strstr(line,"=");
	pm = strstr(line,";");

	//printf("st:%d po:%d pm:%d\n", ret, po, pm );
	if (ret != NULL && po != NULL && pm != NULL && pm > ( po + 1 ) )
	{
		strncpy( LineBuf, po + 1, pm - (po + 1) );
		*a =  atoi(LineBuf);
		/*strncpy( LineBuf1, ret, po - ret );
		printf("%s:%d\n",LineBuf1, *a);*/
		return 0;
	}
	else
	{
		////printf("%s error! \n", line.data());
		return -1;
	}
}

int Parser::stringToFloat(char *line, float *a, char * s)
{
	const char* po = NULL ;
	const char* pm = NULL;
	const char* ret = NULL;
	char LineBuf[MAX_CONFIG_LINE_SIZE];
	//char LineBuf1[MAX_CONFIG_LINE_SIZE];
	
	memset( LineBuf, 0x00, MAX_CONFIG_LINE_SIZE );
	//memset( LineBuf1, 0x00, MAX_CONFIG_LINE_SIZE );
	
	ret = strstr( line, s );
	
	po =  strstr(line,"=");
	pm = strstr(line,";");
	
	//printf("st:%d po:%d pm:%d\n", ret, po, pm );
	if (ret != NULL && po != NULL && pm != NULL && pm > ( po + 1 ) )
	{
		strncpy( LineBuf, po + 1, pm - (po + 1) );
		*a = (float)atof(LineBuf);
		/*strncpy( LineBuf1, ret, po - ret );
		printf("%s:%f\n",LineBuf1, *a);*/
		return 0;
	}
	else
	{
		////printf("%s error! \n", line.data());
		return -1;
	}
}

int Parser::stringToDistanceRange(char *line, float *a, float *b, char * s)
{
	const char* po = NULL ;
	const char* pm = NULL;
	const char* ret = NULL;
	char aa[MAX_CONFIG_LINE_SIZE];
	char bb[MAX_CONFIG_LINE_SIZE];
	//char cc[MAX_CONFIG_LINE_SIZE];
	
	memset( aa, 0x00, MAX_CONFIG_LINE_SIZE );
	memset( bb, 0x00, MAX_CONFIG_LINE_SIZE );
	//memset( cc, 0x00, MAX_CONFIG_LINE_SIZE );
	
	ret = strstr(line,s);
	
	po =  strstr(line,"=");
	pm = strstr(line,";");
	
	//printf("st:%d po:%d pm:%d\n", ret, po, pm );
	if (ret != NULL && po != NULL && pm != NULL && pm > ( po + 1 ) )
	{
		//string obj = line.substr(po + 1, pm - (po + 1));
		const char* pq = strstr(line,"-");
		
		strncpy( aa, po+1, pq -( po + 1 ) );
		strncpy( bb, pq+1, pm - ( pq + 1) );
		//strncpy( cc, ret,  po -  ret  );
		*a = stof(aa);
		*b = stof(bb);
		//printf("%s:%f %f\n", cc, *a, *b);
		return 0;
	}
	else
	{
		////printf("%s error! \n", line.data());
		return -1;
	}
}

int Parser::stringToCharArray(char *line, char *p, int pLen, char * s)
{
	const char* po = NULL ;
	const char* pm = NULL;
	const char* ret = NULL;
	char LineBuf[MAX_CONFIG_LINE_SIZE];
	
	memset( LineBuf, 0x00, MAX_CONFIG_LINE_SIZE );
	ret = strstr(line,s);
	po =  strstr( line, "=" );
	pm = strstr( line, ";" );
	//printf("st:%d po:%d pm:%d\n", ret, po, pm );
	if (ret != NULL && po != NULL && pm != NULL && pm > ( po + 1 ) )
	{
		strncpy(LineBuf,po + 1, pm - (po + 1) );
		if (pLen >= (  pm - (po + 1) ) )
		{
			memcpy(p, LineBuf, pm - (po + 1));
			p[ pm - po ] = 0;
			//printf("%s\r\n", p);
		}
		else
			m_logger->output(LOG_INFO, "stringToCharArray error. %d %d %s", pLen, (int)( pm - (po + 1) ), LineBuf);
		//m_logger->output(LOG_INFO, "%s:%s", ret, p);
		return 0;
	}
	else
	{
		////printf("%s error! \n", line.data());
		return -1;
	}
}
//从内置flash中读取数据包
ITA_RESULT Parser::readBuiltInFlash()
{
	//先初始化
	ITA_RESULT ret = m_flash120->P25QInit();
	if (ITA_OK != ret)
	{
		m_logger->output(LOG_ERROR, "readBuiltInFlash:P25QInit failed.%d", ret);
		return ret;
	}
	//先读INFO
	unsigned char flashInfo[548] = { 0 }; //512+36
	unsigned char lowFlag, highFlag, humanFlag;
	m_flash120->setDebug(1);
	ret = m_flash120->P25QReadInfo(flashInfo, 548);
	m_flash120->setDebug(0);
	if (ITA_OK == ret)
	{
		m_versionV = flashInfo[255];
		for (int i = 0; i < 17; i++)
		{
			m_logger->output(LOG_DEBUG, "%c", flashInfo[i]);
		}
		m_logger->output(LOG_INFO, "--");
		for (int i = 243; i < 255; i++)
		{
			m_logger->output(LOG_DEBUG, "%c", flashInfo[i]);
		}
		m_logger->output(LOG_INFO, "--");
		for (int i = 255; i < 290; i++)
		{
			m_logger->output(LOG_DEBUG, "%d", flashInfo[i]);
		}
		m_logger->output(LOG_INFO, "readBuiltInFlash versionV=%d", m_versionV);
		if (m_versionV != 2)
		{//旧规范
			memcpy(&m_lowPackageLen, flashInfo + 256 + 16, 4);
			memcpy(&m_highPackageLen, flashInfo + 256 + 20, 4);
			lowFlag = flashInfo[256 + 1];
			highFlag = flashInfo[256 + 2];
			//检查是否存在常温档数据包
			if (1 == lowFlag && m_lowPackageLen > 0)
			{
				m_lowPackage = (unsigned char *)porting_calloc_mem(m_lowPackageLen, 1, ITA_PARSER_MODULE);
				ret = m_flash120->P25QReadCurve(1, m_lowPackage, m_lowPackageLen);
				if (ITA_OK != ret)
				{
					m_logger->output(LOG_ERROR, "P25QReadLowCurve error. %d", ret);
				}
			}
			else
			{
				m_logger->output(LOG_INFO, "readBuiltInFlash:No low package.%d %d", lowFlag, m_lowPackageLen);
			}
			//检查是否存在高温档数据包
			if (1 == highFlag && m_highPackageLen > 0)
			{
				m_highPackage = (unsigned char *)porting_calloc_mem(m_highPackageLen, 1, ITA_PARSER_MODULE);
				ret = m_flash120->P25QReadCurve(64, m_highPackage, m_highPackageLen);
				if (ITA_OK != ret)
				{
					m_logger->output(LOG_ERROR, "P25QReadHighCurve error. %d", ret);
				}
			}
			else
			{
				m_logger->output(LOG_INFO, "readBuiltInFlash:No high package.%d %d", highFlag, m_highPackageLen);
			}
		}
		else
		{//新规范
			lowFlag = flashInfo[256 + 1];
			highFlag = flashInfo[256 + 2];
			humanFlag = flashInfo[256 + 3];
			memcpy(&m_lowPackageLen, flashInfo + 256 + 16, 4);
			memcpy(&m_highPackageLen, flashInfo + 256 + 20, 4);
			memcpy(&m_humanPackageLen, flashInfo + 256 + 24, 4);
			memcpy(&m_lowStartSector, flashInfo + 256 + 28, 2);
			memcpy(&m_highStartSector, flashInfo + 256 + 30, 2);
			memcpy(&m_humanStartSector, flashInfo + 256 + 32, 2);
			memcpy(&m_correctionFactors, flashInfo + 512, 36);
			m_logger->output(LOG_INFO, "readBuiltInFlash info:%d %d %d %d %d %d %d %d %d %f %f %f %f %f %f %f %f %f", 
				lowFlag, highFlag, humanFlag, m_lowPackageLen, m_highPackageLen, m_humanPackageLen, m_lowStartSector, m_highStartSector, m_humanStartSector,
				m_correctionFactors[0], m_correctionFactors[1], m_correctionFactors[2], m_correctionFactors[3], m_correctionFactors[4], m_correctionFactors[5], 
				m_correctionFactors[6], m_correctionFactors[7], m_correctionFactors[8]);
			//检查是否存在常温档数据包
			if (1 == lowFlag && m_lowPackageLen > 0 && m_lowStartSector > 0 && m_lowStartSector < 128)
			{
				m_lowPackage = (unsigned char *)porting_calloc_mem(m_lowPackageLen, 1, ITA_PARSER_MODULE);
				ret = m_flash120->P25QReadCurve(m_lowStartSector, m_lowPackage, m_lowPackageLen);
				if (ITA_OK != ret)
				{
					m_logger->output(LOG_ERROR, "P25QReadLowCurve error. %d", ret);
				}
			}
			else
			{
				m_logger->output(LOG_INFO, "readBuiltInFlash:No low package.%d %d", lowFlag, m_lowPackageLen);
			}
			//检查是否存在高温档数据包
			if (1 == highFlag && m_highPackageLen > 0 && m_highStartSector > 0 && m_highStartSector < 128)
			{
				m_highPackage = (unsigned char *)porting_calloc_mem(m_highPackageLen, 1, ITA_PARSER_MODULE);
				ret = m_flash120->P25QReadCurve(m_highStartSector, m_highPackage, m_highPackageLen);
				if (ITA_OK != ret)
				{
					m_logger->output(LOG_ERROR, "P25QReadHighCurve error. %d", ret);
				}
			}
			else
			{
				m_logger->output(LOG_INFO, "readBuiltInFlash:No high package.%d %d", highFlag, m_highPackageLen);
			}
			//检查是否存在人体档数据包
			if (1 == humanFlag && m_humanPackageLen > 0 && m_humanStartSector > 0 && m_humanStartSector < 128)
			{
				m_humanPackage = (unsigned char *)porting_calloc_mem(m_humanPackageLen, 1, ITA_PARSER_MODULE);
				ret = m_flash120->P25QReadCurve(m_humanStartSector, m_humanPackage, m_humanPackageLen);
				if (ITA_OK != ret)
				{
					m_logger->output(LOG_ERROR, "P25QReadHumanCurve error. %d", ret);
				}
			}
			else
			{
				m_logger->output(LOG_INFO, "readBuiltInFlash:No human package.%d %d", humanFlag, m_humanPackageLen);
			}
		}
		
	}
	else
	{
		m_logger->output(LOG_ERROR, "P25QReadInfo error. %d", ret);
	}
	return ret;
}

ITA_RESULT Parser::readFlashPackage(unsigned char * buf, int offset, int size, ITA_RANGE range)
{
	if (!m_lowPackage && !m_humanPackage)
	{
		readBuiltInFlash();
	}
	if (ITA_INDUSTRY_LOW == range || ITA_HUMAN_BODY == range)
	{
		if (m_versionV != 2)
		{//旧规范
			if (m_lowPackage && offset + size <= m_lowPackageLen)
			{
				memcpy(buf, m_lowPackage + offset, size);
			}
			else
			{
				m_logger->output(LOG_ERROR, "readFlashPackage failed.%#x %d %d %d %d", m_lowPackage, range, m_lowPackageLen, offset, size);
				return ITA_READ_PACKAGE_ERROR;
			}
		}
		else
		{//新规范
			if (ITA_INDUSTRY_LOW == range)
			{
				if (m_lowPackage && offset + size <= m_lowPackageLen)
				{
					memcpy(buf, m_lowPackage + offset, size);
				}
				else
				{
					m_logger->output(LOG_ERROR, "readFlashPackage failed.%#x %d %d %d %d", m_lowPackage, range, m_lowPackageLen, offset, size);
					return ITA_READ_PACKAGE_ERROR;
				}
			}
			else
			{
				if (m_humanPackage && offset + size <= m_humanPackageLen)
				{
					memcpy(buf, m_humanPackage + offset, size);
				}
				else
				{
					m_logger->output(LOG_ERROR, "readFlashPackage failed.%#x %d %d %d %d", m_humanPackage, range, m_humanPackageLen, offset, size);
					return ITA_READ_PACKAGE_ERROR;
				}
			}
		}
	}
	else if(ITA_INDUSTRY_HIGH == range)
	{
		if (m_highPackage && offset + size <= m_highPackageLen)
		{
			memcpy(buf, m_highPackage + offset, size);
		}
		else
		{
			m_logger->output(LOG_ERROR, "readFlashPackage failed.%#x %d %d %d %d", m_highPackage, range, m_highPackageLen, offset, size);
			return ITA_READ_PACKAGE_ERROR;
		}
	}
	else
	{
		m_logger->output(LOG_ERROR, "readFlashPackage failed.%d %d", ITA_UNSUPPORT_OPERATION, range);
		return ITA_UNSUPPORT_OPERATION;
	}
	return ITA_OK;
}

ITA_RESULT Parser::writeFlashPackage(unsigned char * buf, int offset, int size, ITA_RANGE range)
{
	ITA_RESULT ret = ITA_OK;
	if (!m_lowPackage && !m_humanPackage)
	{
		readBuiltInFlash();
	}
	if (ITA_INDUSTRY_LOW == range || ITA_HUMAN_BODY == range)
	{
		if (m_versionV != 2)
		{//旧规范
			if (m_lowPackage && offset + size <= m_lowPackageLen)
			{
				//先拷贝到Flash数据包缓存
				memcpy(m_lowPackage + offset, buf, size);
				//再写入Flash
				ret = m_flash120->P25QWriteCurve(m_lowStartSector, m_lowPackage, m_lowPackageLen);
				if (ITA_OK != ret)
				{
					m_logger->output(LOG_ERROR, "P25QWriteLowCurve error. %d", ret);
				}
			}
			else
			{
				m_logger->output(LOG_ERROR, "writeFlashPackage failed.%#x %d %d %d %d", m_lowPackage, range, m_lowPackageLen, offset, size);
				return ITA_WRITE_PACKAGE_ERROR;
			}
		}
		else
		{//新规范
			if (ITA_INDUSTRY_LOW == range)
			{
				if (m_lowPackage && offset + size <= m_lowPackageLen)
				{
					//先拷贝到Flash数据包缓存
					memcpy(m_lowPackage + offset, buf, size);
					//再写入Flash
					ret = m_flash120->P25QWriteCurve(m_lowStartSector, m_lowPackage, m_lowPackageLen);
					if (ITA_OK != ret)
					{
						m_logger->output(LOG_ERROR, "P25QWriteLowCurve error. %d", ret);
					}
				}
				else
				{
					m_logger->output(LOG_ERROR, "writeFlashPackage failed.%#x %d %d %d %d", m_lowPackage, range, m_lowPackageLen, offset, size);
					return ITA_WRITE_PACKAGE_ERROR;
				}
			}
			else
			{
				if (m_humanPackage && offset + size <= m_humanPackageLen)
				{
					//先拷贝到Flash数据包缓存
					memcpy(m_humanPackage + offset, buf, size);
					//再写入Flash
					ret = m_flash120->P25QWriteCurve(m_humanStartSector, m_humanPackage, m_humanPackageLen);
					if (ITA_OK != ret)
					{
						m_logger->output(LOG_ERROR, "P25QWriteHumanCurve error. %d", ret);
					}
				}
				else
				{
					m_logger->output(LOG_ERROR, "writeFlashPackage failed.%#x %d %d %d %d", m_humanPackage, range, m_humanPackageLen, offset, size);
					return ITA_WRITE_PACKAGE_ERROR;
				}
			}
		}
	}
	else if (ITA_INDUSTRY_HIGH == range)
	{
		if (m_highPackage && offset + size <= m_highPackageLen)
		{
			//先拷贝到Flash数据包缓存
			memcpy(m_highPackage + offset, buf, size);
			//再写入Flash
			ret = m_flash120->P25QWriteCurve(m_highStartSector, m_highPackage, m_highPackageLen);
			if (ITA_OK != ret)
			{
				m_logger->output(LOG_ERROR, "P25QWriteHighCurve error. %d", ret);
			}
		}
		else
		{
			m_logger->output(LOG_ERROR, "writeFlashPackage failed.%#x %d %d %d %d", m_highPackage, range, m_highPackageLen, offset, size);
			return ITA_WRITE_PACKAGE_ERROR;
		}
	}
	else
	{
		m_logger->output(LOG_ERROR, "writeFlashPackage failed.%d %d", ITA_UNSUPPORT_OPERATION, range);
		return ITA_UNSUPPORT_OPERATION;
	}
	m_logger->output(LOG_INFO, "writeFlashPackage range=%d ret=%d", range, ret);
	return ret;
}

void Parser::setBuiltInFlash(int type, ITARegistry * registry, int cameraID)
{
	if (type > 0)
	{
		m_builtInFlash = type;
		m_flash120 = new Flash120(registry, cameraID);
		m_flash120->setLogger(m_logger);
		m_logger->output(LOG_INFO, "setBuiltInFlash %d.", type);
		readBuiltInFlash();
	}
	else
	{
		m_logger->output(LOG_WARN, "setBuiltInFlash ignore.%d", type);
	}
}

ITA_RESULT Parser::parseFlashPackage(int cameraID, ITARegistry * registry, ITA_RANGE range)
{
	ITA_RESULT result;
	int offsetB = 1400;
	if (!registry || !registry->InternalFlashPrepare || !registry->InternalFlashDone 
		|| !registry->SPIWrite || !registry->SPITransmit)
		return ITA_HAL_UNREGISTER;
	m_logger->output(LOG_INFO, "parseFlashPackage range=%d", range);
	m_range = range;
	//256和120模组数据包格式不同，探测器参数区分别是1400和30字节。
	if (ITA_120_TIMO == m_type)
		offsetB = 30;
	if (!headerArray)
		headerArray = (unsigned char *)porting_calloc_mem(MAX_PACKAGE_HEADER, 1, ITA_PARSER_MODULE);
	//读取数据包头
	result = readFlashPackage(headerArray, 0, MAX_PACKAGE_HEADER, range);
	if (ITA_OK != result)
	{
		m_logger->output(LOG_ERROR, "readFlashPackage ret=%d", result);
		return result;
	}
	//在更新包头之前先释放之前的内存。
	if (jwTableArray)
	{
		porting_free_mem(jwTableArray);
		jwTableArray = NULL;
	}
	//人体和工业曲线长度不同，导入不同的数据包要在更新包头之前先释放之前的内存。
	/*if (m_4CurvesData)
	{
		porting_free_mem(m_4CurvesData);
		m_4CurvesData = NULL;
	}*/
	if (!m_4CurvesData)
	{
		m_4CurvesData = (short*)porting_calloc_mem(MAX_CURVE_LENGTH * 4, sizeof(short), ITA_PARSER_MODULE);
	}
	//解析数据包头
	memcpy((void *)&m_ph, headerArray, sizeof(PackageHeader));
	if (m_ph.usHeadLength <= 0 || m_ph.usHeadLength > MAX_PACKAGE_HEADER
		|| m_ph.ucFocusNumber <= 0 || m_ph.ucFocusNumber > MAX_FOCUS_NUMBER
		|| m_ph.usWidth != m_width || m_ph.usHeight != m_height)
	{
		//未导入数据包的模组jwTableNumber=255
		m_logger->output(LOG_ERROR, "readFlashPackage: Wrong header data. HeaderLen=%d,  FocusNumber=%d, Width=%d, Height=%d", m_ph.usHeadLength, m_ph.ucFocusNumber, m_ph.usWidth, m_ph.usHeight);
		return ITA_WRONG_PACKAGE_HEADER;
	}
	//读取探测器参数
	readFlashPackage((unsigned char *)&m_dp, m_ph.usHeadLength, sizeof(DetectorParameter), range);
	//解析焦温个数
	//为了满足高温环境下的测温精度，采集模组数据的时候有8个焦温。
	//8个焦温对应的温度是15，20，25，30，35，40, 45, 50。
	jwTableNumber = m_ph.ucFocusNumber;
	m_logger->output(LOG_INFO, "read jwTableNumber = %d, ucGearMark = %d %d %d %d %s", jwTableNumber, m_ph.ucGearMark, m_ph.focusType, m_ph.lensType, m_ph.mtType, m_ph.cModuleCode);
	//解析焦温表。如果高低温档焦温数量不同的话，切换范围时可能死机。所以先释放再重新申请。
	if (!jwTableArray)
		jwTableArray = (short *)porting_calloc_mem(m_ph.usFocusArrayLength, 1, ITA_PARSER_MODULE);
	//偏移216 + 1400
	readFlashPackage((unsigned char *)jwTableArray, m_ph.usHeadLength + offsetB, m_ph.usFocusArrayLength, range);
	for (int i = 0; i < jwTableNumber; i++)
	{
		m_logger->output(LOG_INFO, "jw %d = %d", i, jwTableArray[i]);
	}
	//m_logger->output(LOG_INFO, "read JW table end");
	//解析曲线。如果高低温档曲线长度不同的话，切换范围时可能死机。所以先释放再重新申请。
	//读600*8*2*2B（远近共16条曲线，256模组人体测温项目只采了0.5米的曲线只有8条）。CURVE_LENGTH * mDH.ucFocusNumber * mDH.ucDistanceNumber * sizeof(short)
	//int curveDataLength = jwTableNumber * 2 * ONE_CURVE_DATA_LENGTH / 2;
	//if (!m_4CurvesData)
	//	m_4CurvesData = (short *)porting_calloc_mem(m_ph.usCurveTemperatureNumber * m_ph.ucDistanceNumber * 2, sizeof(short), ITA_PARSER_MODULE);// m_ph.usCurveDataLength);																																			 //偏移216 + 1400 + ph.usFocusArrayLength 初始读取1档位曲线
	if (m_ph.usCurveTemperatureNumber * m_ph.ucDistanceNumber * 2 > MAX_CURVE_LENGTH * 4) {
		porting_free_mem(m_4CurvesData);
		m_4CurvesData = (short*)porting_calloc_mem(m_ph.usCurveTemperatureNumber * m_ph.ucDistanceNumber * 2, sizeof(short), ITA_PARSER_MODULE);
	}
	readFlashPackage((unsigned char *)m_4CurvesData, m_ph.usHeadLength + offsetB + m_ph.usFocusArrayLength, m_ph.usCurveTemperatureNumber * m_ph.ucDistanceNumber * 2 * sizeof(short), range);
	m_logger->output(LOG_INFO, "read CurveData end. 4 curvesLen=%d totalLen=%d", 8 * m_ph.usCurveTemperatureNumber, m_ph.usCurveDataLength);
	//X16模式下需要解析K数据
	if (ITA_X16 == m_mode || ITA_MCU_X16 == m_mode)
	{
		//初始读取0档位的K。高低温档一帧K长度固定。
		//偏移216 + 1400 + ph.usFocusArrayLength + ph.usCurveDataLength
		readFlashPackage((unsigned char *)m_kArray, m_ph.usHeadLength + offsetB + m_ph.usFocusArrayLength + m_ph.usCurveDataLength, m_ph.usKMatLength / m_ph.ucFocusNumber, range);
		m_logger->output(LOG_INFO, "read K Data end. len=%d", m_ph.usKMatLength);
	}
	return ITA_OK;
}

ITA_RESULT Parser::getCorrectionFactors(float * pCorrectionFactors)
{
	memcpy(pCorrectionFactors, m_correctionFactors, 36);
	return ITA_OK;
}

unsigned short * Parser::getCurrentK()
{
	return m_kArray;
}

short * Parser::getCurves()
{
	return m_4CurvesData;
}

int Parser::getCurvesLen()
{
	//返回曲线长度，单位short。
	if (m_type < ITA_HANDHELD_TEC)
	{
		if (m_ph.usCurveTemperatureNumber > 0 && m_ph.ucDistanceNumber > 0)
		{
			return m_ph.usCurveTemperatureNumber * m_ph.ucDistanceNumber * 2;
		}
	}
	else
	{
		if (m_4CurvesData)
			return MAX_TEC_CURVE_SIZE;
	}
	return 0;
}

ITA_RESULT Parser::setCurves(short * curveData, int curveLen)
{
	if (curCurveLen != curveLen)
	{
		curCurveLen = curveLen;
		if (!m_4CurvesData) {
			m_4CurvesData = (short*)porting_calloc_mem(curveLen, sizeof(short), ITA_PARSER_MODULE);
		}
		else {
			porting_free_mem(m_4CurvesData);
			m_4CurvesData = (short*)porting_calloc_mem(curveLen, sizeof(short), ITA_PARSER_MODULE);
		}
	}
	memcpy(m_4CurvesData, curveData, curveLen * 2);
	return ITA_OK;
}

unsigned char * Parser::getReservedData(int * length, bool isSubB, short bAverage)
{
	//模组产品离线测温需要保存数据包头和焦温表
	if (jwTableNumber > 0)
	{
		if (curGJwNumber != jwTableNumber)
		{
			curGJwNumber = jwTableNumber;
			if (!m_reservedData)
			{
				//增加焦温个数，本地均值开关，本底均值共3个参数，参数类型分别为int,unsigned char和short，同时在尾部增加MAX_FOCUS_NUMBER(100)个short数据。
				m_reservedData = (unsigned char*)porting_calloc_mem(sizeof(PackageHeader) + jwTableNumber * sizeof(short) + sizeof(int) + sizeof(unsigned char) + sizeof(short) + sizeof(short) * MAX_FOCUS_NUMBER, 1, ITA_PARSER_MODULE);
			}
			else {
				/*焦温数量变化，重新申请内存大小*/
				porting_free_mem(m_reservedData);
				m_reservedData = (unsigned char*)porting_calloc_mem(sizeof(PackageHeader) + jwTableNumber * sizeof(short) + sizeof(int) + sizeof(unsigned char) + sizeof(short) + sizeof(short) * MAX_FOCUS_NUMBER, 1, ITA_PARSER_MODULE);
			}
		}
		memcpy(m_reservedData, &m_ph, sizeof(PackageHeader));
		if (!jwTableArray) {
			m_logger->output(LOG_ERROR, "jwTableArray is nullptr");
			return NULL;
		}
		memcpy(m_reservedData + sizeof(PackageHeader), &jwTableNumber, sizeof(int));
		memcpy(m_reservedData + sizeof(PackageHeader) + sizeof(int), jwTableArray, jwTableNumber * sizeof(short));
		unsigned char m_isSubB = isSubB ? 1 : 0;
		memcpy(m_reservedData + sizeof(PackageHeader) + sizeof(int) + jwTableNumber * sizeof(short), &m_isSubB, sizeof(unsigned char));
		memcpy(m_reservedData + sizeof(PackageHeader) + sizeof(int) + jwTableNumber * sizeof(short) + sizeof(unsigned char), &bAverage, sizeof(short));
		*length = sizeof(PackageHeader) + sizeof(int) + jwTableNumber * sizeof(short) + sizeof(unsigned char) + sizeof(short) + sizeof(short) * MAX_FOCUS_NUMBER;
		//m_logger->output(LOG_INFO, "getReservedData %d %d %d %d %d", sizeof(PackageHeader), jwTableNumber, isSubB, bAverage, *length);
		return m_reservedData;
	}
	*length = 0;
	return NULL;
}

ITA_RESULT Parser::setReservedData(unsigned char * data, int length, bool* isSubB, short* bAverage)
{
	memcpy(&m_ph, data, sizeof(PackageHeader));
	jwTableNumber = (length - sizeof(PackageHeader)) / 2;
	if (jwTableNumber < MAX_FOCUS_NUMBER)
	{
		if (!jwTableArray)
		{
			jwTableArray = (short*)porting_calloc_mem(jwTableNumber, sizeof(short), ITA_PARSER_MODULE);
		}
		memcpy(jwTableArray, data + sizeof(PackageHeader), jwTableNumber * sizeof(short));
	}
	else {
		jwTableNumber = *(int*)(data + sizeof(PackageHeader));
		if (jwTableNumber != curJwNumber)
		{
			curJwNumber = jwTableNumber;
			if (!jwTableArray)
			{
				jwTableArray = (short*)porting_calloc_mem(jwTableNumber, sizeof(short), ITA_PARSER_MODULE);
			}
			else {
				porting_free_mem(jwTableArray);
				jwTableArray = (short*)porting_calloc_mem(jwTableNumber, sizeof(short), ITA_PARSER_MODULE);
			}
		}
		memcpy(jwTableArray, data + sizeof(PackageHeader) + sizeof(int), jwTableNumber * sizeof(short));
		unsigned char m_isSubB;
		memcpy(&m_isSubB, data + sizeof(PackageHeader) + sizeof(int) + jwTableNumber * sizeof(short), sizeof(unsigned char));
		*isSubB = (m_isSubB == 1) ? true : false;
		*bAverage = *(short*)(data + sizeof(PackageHeader) + sizeof(int) + jwTableNumber * sizeof(short) + sizeof(unsigned char));
		m_logger->output(LOG_INFO, "setReservedData %d %d %d %d %d", sizeof(PackageHeader), jwTableNumber, *isSubB, *bAverage, length);
	}
	return ITA_OK;
}

short * Parser::getFPArray()
{
	return jwTableArray;
}

PackageHeader * Parser::getPH()
{
	return &m_ph;
}

DetectorParameter * Parser::getDP()
{
	return &m_dp;
}

ITA_RESULT Parser::correctBadPoint(ITA_POINT point)
{
	int i = point.y * m_width + point.x;
	/*最高位置1*/
	m_kArray[i] |= 0x8000;
	return ITA_OK;
}

ITA_RESULT Parser::saveBadPoints(ITA_POINT * pointsArray, int size, int cameraID, ITARegistry *registry, ITA_RANGE range)
{
	ITA_RESULT ret = ITA_OK;
	if (jwTableNumber <= 0)
	{
		return ITA_NO_PACKAGE;
	}
	//写一次的方式
	unsigned short *allkArray = (unsigned short *)porting_calloc_mem(m_frameSize * jwTableNumber, sizeof(unsigned short), ITA_PARSER_MODULE);
	int pos, offset = m_ph.usHeadLength + 1400 + m_ph.usFocusArrayLength + m_ph.usCurveDataLength;
	unsigned short *kArray;
	m_logger->output(LOG_INFO, "saveBadPoints begin. jwNumber=%d, range=%d", jwTableNumber, range);
	if (ITA_120_TIMO == m_type)
		offset = m_ph.usHeadLength + 30 + m_ph.usFocusArrayLength + m_ph.usCurveDataLength;
	//读取所有K
	if (!m_builtInFlash)
	{
		if (registry->ReadPackageData)
			ret = registry->ReadPackageData(cameraID, (unsigned char *)allkArray, offset, m_ph.usKMatLength, range, registry->userParam);
	}
	else
		ret = readFlashPackage((unsigned char *)allkArray, offset, m_ph.usKMatLength, range);
	if (ret != ITA_OK)
	{
		porting_free_mem(allkArray);
		m_logger->output(LOG_ERROR, "saveBadPoints ret=%d", ret);
		return ret;
	}
	//依次读当前测温范围每个档位的K，标记列表中的坏点
	for (int i = 0; i < jwTableNumber; i++)
	{
		kArray = (unsigned short *)((unsigned char *)allkArray + i * m_ph.usKMatLength / m_ph.ucFocusNumber);
		for (int j = 0; j < size; j++)
		{
			pos = pointsArray[j].y * m_width + pointsArray[j].x;
			/*最高位置1*/
			kArray[pos] |= 0x8000;
		}
		m_logger->output(LOG_INFO, "saveBadPoints i=%d count=%d", i, size);
	}
	if (!m_builtInFlash)
		ret = registry->WritePackageData(cameraID, (unsigned char *)allkArray, offset, m_ph.usKMatLength, range, registry->userParam);
	else
		ret = writeFlashPackage((unsigned char *)allkArray, offset, m_ph.usKMatLength, range);
	porting_free_mem(allkArray);
	m_logger->output(LOG_INFO, "saveBadPoints end. ret=%d", ret);
	//写多次的方式
	//unsigned short *kArray = (unsigned short *)porting_calloc_mem(m_frameSize, sizeof(unsigned short), ITA_PARSER_MODULE);
	//int pos, offset = m_ph.usHeadLength + 1400 + m_ph.usFocusArrayLength + m_ph.usCurveDataLength;
	//if (ITA_120_TIMO == m_type)
	//	offset = m_ph.usHeadLength + 30 + m_ph.usFocusArrayLength + m_ph.usCurveDataLength;
	////依次读当前测温范围每个档位的K，标记列表中的坏点
	//for (int i = 0; i < jwTableNumber; i++)
	//{
	//	registry->ReadPackageData(cameraID, (unsigned char *)kArray, offset + i * m_ph.usKMatLength / m_ph.ucFocusNumber, m_ph.usKMatLength / m_ph.ucFocusNumber, range, registry->userParam);
	//	for (int j = 0; j < size; j++)
	//	{
	//		pos = pointsArray[j].y * m_width + pointsArray[j].x;
	//		kArray[pos] = 8192;
	//	}
	//	ret = registry->WritePackageData(cameraID, (unsigned char *)kArray, offset + i * m_ph.usKMatLength / m_ph.ucFocusNumber, m_ph.usKMatLength / m_ph.ucFocusNumber, range, registry->userParam);
	//	m_logger->output(LOG_INFO, "saveBadPoints i=%d count=%d", i, size);
	//}
	//porting_free_mem(kArray);
	return ret;
}

short Parser::getAvgB()
{
	return m_avgB;
}

void Parser::setConfEncrypt(bool isEncrypt)
{
	m_isConfEncrypt = isEncrypt;
	m_logger->output(LOG_INFO, "setConfEncrypt %d", isEncrypt);
}

void Parser::setChStatus(unsigned char status)
{
	m_isCold = (status == 1 ? true : false);
}

ITA_RESULT Parser::isColdOrHot(int cameraID, ITARegistry * registry, MtParams * mtParams, int period, unsigned char* status)
{
	ITA_RESULT ret;
	CRC32 crc32Obj;
	unsigned int crc1, crc2;
	ITAPrivateData pd = { 0 };
	//读private data，如果热机开机，则读取上次关机时存储的开机快门温。
	if (!registry || !registry->ReadPrivateData)
	{
		m_logger->output(LOG_WARN, "isColdOrHot failed.%d", ITA_HAL_UNREGISTER);
		return ITA_HAL_UNREGISTER;
	}
	if (!mtParams)
	{
		m_logger->output(LOG_WARN, "isColdOrHot mtParams null.");
		return ITA_NULL_PTR_ERR;
	}
	unsigned char *dataBuf = (unsigned char *)porting_calloc_mem(MAX_PRIVATE_DATA_SIZE, 1, ITA_PARSER_MODULE);
	//读取保存的私有数据，并校验正确性。
	ret = registry->ReadPrivateData(cameraID, dataBuf, MAX_PRIVATE_DATA_SIZE, registry->userParam);
	if (ITA_OK == ret)
	{
		crc1 = crc32Obj.calcCRC32(dataBuf, sizeof(ITAPrivateData));
		crc2 = *(unsigned int *)(dataBuf + sizeof(ITAPrivateData));
		if (crc1 == crc2)
		{
			memcpy(&pd, dataBuf, sizeof(ITAPrivateData));
			long long myTime = porting_get_time_second();
			if (myTime - pd.second < period && myTime - pd.second >0 || !m_isCold)
			{
				if (pd.orinalShutterTemp >= 1.0 && pd.orinalShutterTemp <= 100.0)
				{
					m_isCold = false;
					mtParams->fOrinalShutterTemp = pd.orinalShutterTemp;
					m_logger->output(LOG_INFO, "isColdOrHot:hot.%f %d %lld %lld", mtParams->fOrinalShutterTemp, myTime - pd.second, myTime, pd.second);
				}
				else
					m_logger->output(LOG_INFO, "isColdOrHot:hot.Illegal temp:%f time:%d %lld %lld", mtParams->fOrinalShutterTemp, myTime - pd.second, myTime, pd.second);
			}
			else
				m_logger->output(LOG_INFO, "isColdOrHot:cold.%d %lld %lld", myTime - pd.second, myTime, pd.second);
		}
		else
		{
			m_logger->output(LOG_INFO, "ReadPrivateData check failed. %#x %#x", crc1, crc2);
		}
	}
	else
	{
		m_logger->output(LOG_ERROR, "ReadPrivateData error code:%d", ret);
	}
	*status = m_isCold ? 1 : 0;
	porting_free_mem(dataBuf);
	return ret;
}

ITA_RESULT Parser::saveColdOrHot(int cameraID, ITARegistry * registry, MtParams * mtParams)
{
	//保存关机时间，如果冷机开机，那么保存开机快门温。
	//格式：ITAPrivateData+crc32
	ITA_RESULT ret;
	CRC32 crc32Obj;
	unsigned int crc1, crc2;
	ITAPrivateData pd = { 0 };
	if (!registry || !registry->ReadPrivateData || !registry->WritePrivateData)
	{
		m_logger->output(LOG_WARN, "saveColdOrHot failed.%d", ITA_HAL_UNREGISTER);
		return ITA_HAL_UNREGISTER;
	}
	if (!mtParams)
	{
		m_logger->output(LOG_WARN, "saveColdOrHot mtParams null.");
		return ITA_NULL_PTR_ERR;
	}
	unsigned char *dataBuf = (unsigned char *)porting_calloc_mem(MAX_PRIVATE_DATA_SIZE, 1, ITA_PARSER_MODULE);
	//读取保存的私有数据，并校验正确性。
	ret = registry->ReadPrivateData(cameraID, dataBuf, MAX_PRIVATE_DATA_SIZE, registry->userParam);
	if (ITA_OK == ret)
	{
		crc1 = crc32Obj.calcCRC32(dataBuf, sizeof(ITAPrivateData));
		crc2 = *(unsigned int *)(dataBuf + sizeof(ITAPrivateData));
		if (crc1 == crc2)
		{
			memcpy(&pd, dataBuf, sizeof(ITAPrivateData));
			m_logger->output(LOG_INFO, "ReadPrivateData check success.%f %d", pd.orinalShutterTemp, pd.second);
		}
		else
		{
			m_logger->output(LOG_INFO, "ReadPrivateData check failed. %#x %#x", crc1, crc2);
		}
	}
	else
	{
		m_logger->output(LOG_ERROR, "ReadPrivateData error code:%d", ret);
	}
	//更新关机时间
	pd.second = porting_get_time_second();
	//如果冷开机，就保存开机快门温。如果热开机，就不变。
	if (m_isCold && mtParams->fOrinalShutterTemp >= 1.0 && mtParams->fOrinalShutterTemp <= 100.0)
	{
		pd.orinalShutterTemp = mtParams->fOrinalShutterTemp;
		m_logger->output(LOG_INFO, "saveColdOrHot update orinalShutterTemp %f", pd.orinalShutterTemp);
	}
	memcpy(dataBuf, &pd, sizeof(ITAPrivateData));
	//计算crc32
	crc1 = crc32Obj.calcCRC32(dataBuf, sizeof(ITAPrivateData));
	*(unsigned int *)(dataBuf + sizeof(ITAPrivateData)) = crc1;
	ret = registry->WritePrivateData(cameraID, dataBuf, MAX_PRIVATE_DATA_SIZE, registry->userParam);
	if (ITA_OK == ret)
	{
		m_logger->output(LOG_INFO, "WritePrivateData success.");
	}
	else
	{
		m_logger->output(LOG_ERROR, "WritePrivateData error code:%d", ret);
	}
	porting_free_mem(dataBuf);
	return ret;
}

int Parser::getlltt()
{
	return m_lltt;
}

bool Parser::getCoffStatus()
{
	return m_isCoffChange;
}

ITA_BURN_INFO Parser::getBurnInfo()
{
	return m_burnInfo;
}

ITA_RESULT Parser::saveK(int camaraId, unsigned short* m_gain_mat, ITARegistry* registry, int m_curGear, short temp)
{
	ITA_RESULT ret = ITA_OK;
	int offsetB = 1400;
	if (ITA_120_TIMO == m_type) {
		offsetB = 30;
	}
	int offset = m_ph.usHeadLength + offsetB + m_ph.usFocusArrayLength + m_ph.usCurveDataLength;
	int singleFocusLen = m_ph.usFocusArrayLength / m_ph.ucFocusNumber;
	int g = m_curGear;
	if (m_curGear >= m_ph.ucFocusNumber && m_ph.ucFocusNumber > 0)
	{
		g = m_ph.ucFocusNumber - 1;
	}
	if (!m_builtInFlash)
	{
		ret = registry->WritePackageData(camaraId, (unsigned char*)m_gain_mat, offset + g * m_ph.usKMatLength / m_ph.ucFocusNumber, m_ph.usKMatLength / m_ph.ucFocusNumber, m_range, registry->userParam);
		ret = registry->WritePackageData(camaraId, (unsigned char*)&temp, m_ph.usHeadLength + offsetB + g * singleFocusLen, singleFocusLen,m_range, registry->userParam);
		if (ret == ITA_OK) {
			m_logger->output(LOG_INFO, "WritePackageData success!");
		}
		else {
			m_logger->output(LOG_INFO, "WritePackageData failed!");
		}
	}
	else {
		ret = writeFlashPackage((unsigned char*)m_gain_mat, offset + g * m_ph.usKMatLength / m_ph.ucFocusNumber, m_ph.usKMatLength / m_ph.ucFocusNumber, m_range);
		ret = writeFlashPackage((unsigned char*)&temp, m_ph.usHeadLength + offsetB + g * singleFocusLen, singleFocusLen, m_range);
		if (ret == ITA_OK) {
			m_logger->output(LOG_INFO, "WriteFlashPackage success!");
		}
	}
	return ret;
}
