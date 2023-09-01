/*************************************************************
Copyright (C), 2021--2022, Wuhan Guide Sensmart Tech Co., Ltd
File name   : ITAWrapper.cpp
Author      : Guide Sensmart RD. wangyan
Version     : 1.0
Date        : 2021/11/9
Description : ITA SDK management module.
*************************************************************/
#include <string>
#include <algorithm>
#include "ITAWrapper.h"
#include "../Module/GuideLog.h"

#ifdef ULTIMATE_EDITION

extern int g_memArray[ITA_NONE_MODULE];

ITAWrapper::ITAWrapper(ITA_MODE mode, ITA_RANGE range, int width, int height, int cameraID)
{
	m_mode = mode;
	m_range = range;
	m_width = width;
	m_height = height;
	m_realW = width;
	m_realH = height;
	//默认无反向处理，256模组需要反向处理。
	m_reverse = false;
	if(120 == width)
		m_productType = ITA_120_TIMO;
	else
	{
		m_productType = ITA_256_TIMO;	//从配置文件读取
		m_reverse = true;
	}
	m_cameraID = cameraID;
	m_scale = 1.0;
	m_rotate = ITA_ROTATE_NONE;
	m_isRotate = false;
	m_flip = ITA_FLIP_NONE;
	m_pixelFormat = ITA_RGB888;
	m_paletteIndex = ITA_WHITE_HEAT;
	m_registry = NULL;
	m_detector = NULL;
	m_timo120 = NULL;
	m_processor = NULL;
	m_isCollectBackground = false;
	m_backgroundCount = 0;
	m_skipCount = 0;
	m_firstNUCing = true;
	m_nucStatus = 0;
	//重复快门纠错机制。默认0关闭。1，用户打开nuc重复快门机制；2，即将重复快门；3，正在快门。快门动作完成后切到1状态。
	m_nucAnotherShutter = 0;
	m_nucRepeatCorrection = 0; //如果两种纠错同时设置，那么只用做ITA_NUC_REPEAT_CORRECTION。
	m_nucRepeatCount = 0;
	m_lastSensorTime = 0;
	m_loop = false;
	m_frameSize = m_width*m_height;
	m_fpaGear = -1;	//初始赋无效值
	m_isGearManual = false;
	m_gearManual = -1;	//手动档位，与当前档位m_fpaGear比较，不等就切换。
	m_isChangeRange = false;
	m_isChangeGear = false;
	m_subAvgB = false; //个别产品例如芯晟ASIC算法差异导致Y16减去本底均值之后才能测温。默认不用减。
	m_y16TempMatrix = NULL;
	m_tempMatrixLen = 0;
	m_avgB = 0;
	g_currentTime = porting_get_ms();
	m_logger = new GuideLog();
	m_monitor = new Monitor(m_productType);
	m_monitor->setLogger(m_logger);
	//默认打开快门和NUC策略，由用户根据需要主动关闭。
	m_monitor->closeShutterPolicy(false);
	if (ITA_X16 == mode || ITA_MCU_X16 == mode)
	{
		m_bArray = (short *)porting_calloc_mem(m_frameSize, sizeof(unsigned short), ITA_WRAPPER_MODULE);
	}
	else
	{
		m_bArray = NULL;
		if (ITA_Y16 == mode)
		{
			//ASIC Y16模式下可以关闭快门和NUC策略，ITA不用启动时做一次NUC。
			m_monitor->closeShutterPolicy(true);
			m_firstNUCing = false;
		}
	}
	//裸模组一次快门约360+170ms。默认丢掉前5帧再采集4帧本底共9帧。
	m_shutterFrameNumber = 9; //用户使用ITA_SINGLE_STEP_TIME设置。
	m_bDelayFrame = 5; //避免快门留影。用户使用ITA_B_DELAY设置。
	m_shutterCloseDelay = 100; //这一组延时参数用户可以用ITA_STEP_DELAY_TIME设置。
	m_shutterOpenDelay = 70;
	if (ITA_MCU_X16 == mode)
	{
		//MCU一次快门时间约520+800ms。USB协议有延时和缓存数据。
		m_shutterFrameNumber = 13; //本底采集完打开快门再抛4帧MCU USB缓存数据，避免闪屏。
		m_shutterCloseDelay = 400; //MCU USB协议有延时，低于400出现快门留影。
		m_shutterOpenDelay = 400; //USB协议有延时，低于400会出现快门打不开的情况。
	}
	m_parser = new Parser(m_mode, width, height, m_productType);
	m_mtBase = new MTWrapper(m_range, width, height, m_productType);
	memset(&m_config, 0, sizeof(ITAConfig));
	//m_wrapper->getMtParams()->productType = m_productType;
	m_mtBase->setLogger(m_logger);
	m_parser->setLogger(m_logger);
	m_af = new AutoFocus();
	m_af->setLogger(m_logger);
	m_processor = new ImageProcessor(m_mode, m_width, m_height, m_scale, m_pixelFormat);
	m_processor->setLogger(m_logger);
	m_processor->setDebugger(&m_debugger);
	//在这里设置初K
	if (ITA_X16 == m_mode || ITA_MCU_X16 == m_mode)
	{
		m_processor->setCurrentK(m_parser->getCurrentK(), m_frameSize * 2);
	}
	//设置图像默认参数。
	m_processor->setPaletteIndex(ITA_WHITE_HEAT);
	m_processor->setDimmingType(ITA_DRT_LINEAR, NULL);
	m_processor->setContrast(255);
	m_processor->setBrightness(70);

	m_debug = false;
	m_closeImage = false;	//默认打开图像处理，由用户根据需要主动关闭。
	m_imgDst = NULL;
	m_y16Data = NULL;
	m_reCalcSize = false;
	/*自动校温功能，模组类产品算法建议使用ZX03C校温方案，4个参数可以根据项目调整具体值。*/
	m_isAutoCollect = 0;	//记录采集状态：0，初始空状态；1，准备采集；2，正在采集；3，一次标定黑体采集完成。
	m_shutterCollectInterval = 10;	//上次快门之后间隔多少秒开始打快门采集
	m_collectInterval = 1;	//相邻两次采集间隔时间
	m_collectNumberOfTimes = 15;	//一次快门之后需要采集的次数
	//m_shutterNumberOfTimes = 1;	//每个黑体采集完成需要的快门次数
	//m_shutterTimes = 0;		//一次黑体采集周期内快门计数，<= m_shutterNumberOfTimes。
	m_collectTimes = 0;		//一次黑体采集周期内采集Y16计数，<= m_collectNumberOfTimes。
	m_collectCount = 0;		//第几个黑体采集，一般3次，30 33 36.
	m_badPointsCount = 0;
	badPointTotalNumber = 30;
	m_badPointsArray = NULL; 
	m_badPoint.x = -1;
	m_badPoint.y = -1;
	m_maxFrameRate = 25; //探测器配置的帧率
	m_fps = m_maxFrameRate;
	m_ispTime = 15; //估算平台处理一帧15ms
	m_lastIspTime = 15;
	m_userIspTime = 0; //用户设置的平均帧处理时间。因为实时计算的帧时间很不稳定，影响实际帧率。
	/*另外一种方案，连续处理用户指定帧率后连续丢帧以满足帧率。用帧间隔时间无法满足帧率，时间无法精准控制。*/
	m_userFps = 0; //用户设置的帧率
	m_nucStartTime = 0;
	m_isCalcCentral = false; //计算中心温
	m_isAF = false; //用户是否开始使用AF，如果是就逐帧传Y16。
	/*用户设置的探测器参数*/
	m_validMode = 0;//0：探测器VSYNC和HSYNC高有效；1：探测器VSYNC和HSYNC低有效。默认为0。
	m_dataMode = 1;//0：先发16位数据的高8bits；1：先发16位数据的低8bits。默认为1。
	m_frameFrequency = 25;//探测器帧频，单位fps。默认25。
	m_builtInFlash = 0;  //1：120 90°模组P25Q内置Flash。
	memset(&guoGaiPara, 0, sizeof(stDGGTPara));
	m_gain_mat = NULL;
	m_collectK = NULL;
	m_startReview = NULL;
	m_coldHotDelay = 3600;
	m_coldHotStatus = 1;
	m_asicStatus = 1;
#ifdef SOURCE_COPY
	m_x16Data = NULL;	//源数据的拷贝，原则上不需要。
#endif // SOURCE_COPY
	m_laserAF = NULL;

	m_toolBox = new ToolBox();
	m_toolBox->setLogger(m_logger);
}

ITAWrapper::~ITAWrapper()
{
	m_logger->output(LOG_INFO, "ITAWrapper destroy.");
	if (m_isAF)
		m_isAF = false;
	//doISP接口返回后才能析构，否则释放m_monitor后doISP中使用时死机。避免用锁，MCU上锁不好实现。
	while (m_loop)
	{
		porting_thread_sleep(1);
		m_logger->output(LOG_INFO, "Waiting to end a process.");
	}
	m_logger->output(LOG_INFO, "End a process.");
	//先保存关机信息
	m_parser->saveColdOrHot(m_cameraID, m_registry, m_mtBase->getMtParams());
	//先销毁m_monitor，避免自动快门死机。
	if (m_monitor)
	{
		delete m_monitor;
		m_monitor = NULL;
	}
	m_logger->output(LOG_INFO, "delete monitor.");
	if (m_af)
	{
		delete m_af;
		m_af = NULL;
	}
	m_logger->output(LOG_INFO, "delete af.");
	if (m_detector != NULL)
	{
		delete m_detector;
		m_detector = NULL;
	}
	if (m_timo120 != NULL)
	{
		delete m_timo120;
		m_timo120 = NULL;
	}
	if (m_processor)
	{
		delete m_processor;
		m_processor = NULL;
	}
	m_logger->output(LOG_INFO, "delete processor.");
	if (m_parser)
	{
		delete m_parser;
		m_parser = NULL;
	}
	if (m_mtBase)
	{
		delete m_mtBase;
		m_mtBase = NULL;
	}
	if (m_bArray)
	{
		porting_free_mem(m_bArray);
		m_bArray = NULL;
	}
	if (m_registry)
	{
		porting_free_mem(m_registry);
		m_registry = NULL;
	}
	if (m_config.functionConfig)
	{
		porting_free_mem(m_config.functionConfig);
		m_config.functionConfig = NULL;
	}
	if (m_config.mtConfig)
	{
		porting_free_mem(m_config.mtConfig);
		m_config.mtConfig = NULL;
	}
	if (m_config.ispConfig)
	{
		porting_free_mem(m_config.ispConfig);
		m_config.ispConfig = NULL;
	}
	m_config.ispConfigCount = 0;
	m_config.mtConfigCount = 0;
	if (m_imgDst)
	{
		porting_free_mem(m_imgDst);
		m_imgDst = NULL;
	}
	if (m_y16Data)
	{
		porting_free_mem(m_y16Data);
		m_y16Data = NULL;
	}
	if (m_y16TempMatrix)
	{
		porting_free_mem(m_y16TempMatrix);
		m_y16TempMatrix = NULL;
	}
	if (m_laserAF)
	{
		delete m_laserAF;
		m_laserAF = NULL;
	}
	if (m_gain_mat)
	{
		porting_free_mem(m_gain_mat);
		m_gain_mat = NULL;
	}
	if (m_badPointsArray) {
		porting_free_mem(m_badPointsArray);
		m_badPointsArray = NULL;
	}
	if (m_collectK)
	{
		porting_free_mem(m_collectK);
		m_collectK = NULL;
	}
	if (m_startReview)
	{
		delete m_startReview;
		m_startReview = NULL;
	}

	if (m_toolBox)
	{
		delete m_toolBox;
		m_toolBox = NULL;
	}

#ifdef SOURCE_COPY
	if (m_x16Data)
	{
		porting_free_mem(m_x16Data);
		m_x16Data = NULL;
	}
#endif // SOURCE_COPY
	//剩余1536B porting_calloc_mem管理数组占用的内存
	/*int sum;
	debugger(ITA_MEMORY_USAGE, &sum);*/
	m_logger->output(LOG_INFO, "delete success.");
	if (m_logger)
	{
		delete m_logger;
		m_logger = NULL;
	}
}

ITA_RESULT ITAWrapper::setRegister(ITARegistry * registry)
{
	ITA_RESULT result = ITA_OK;
	bool flag = false;
	m_logger->output(LOG_INFO, "setRegister %#x", registry);
	if (!registry)
	{
		m_logger->output(LOG_ERROR, "setRegister %#x", registry);
		return ITA_NULL_PTR_ERR;
	}
	if (registry->OutputLog)
		m_logger->registerOutputFunc(registry->OutputLog, m_cameraID, registry->userParam);
	if (!m_registry)
	{
		m_registry = (ITARegistry *)porting_calloc_mem(1, sizeof(ITARegistry), ITA_WRAPPER_MODULE);
	}
	*m_registry = *registry;
	//先解析产品类型，读数据包（格式不同）和初始化探测器都与产品类型相关。
	if (m_registry && m_registry->ReadConfig)
	{
		int len = 0;
		ITA_RESULT ret = m_registry->ReadConfig(m_cameraID, m_parser->getConfBuf(), MAX_CONFIG_FILE_SIZE, &len, m_registry->userParam);
		if (ITA_OK == ret && len > 0)
		{
			m_parser->parseConfig(m_parser->getConfBuf(), len, &m_config);
			if (m_config.functionConfig)
			{
				if (m_config.functionConfig->productTypeS == 1)
				{
					m_productType = (ITA_PRODUCT_TYPE)m_config.functionConfig->productType;
					/*更新产品型号*/
					m_parser->setProductType(m_productType);
					m_monitor->setProductType(m_productType);
					//m_wrapper->getMtParams()->productType = m_productType;
					if (m_productType >= ITA_HANDHELD_TEC)
					{
						delete m_mtBase;
						m_mtBase = new MTTEC(m_range, m_width, m_height, m_productType);
						m_mtBase->setLogger(m_logger);
						if (m_debug)
							m_mtBase->getTECParams()->bLogPrint = 1;
					}
				}
				//检查模组是否内置flash
				if (m_config.functionConfig->internalFlashS == 1)
				{
					m_logger->output(LOG_INFO, "internalFlash=%d", m_config.functionConfig->internalFlash);
					m_builtInFlash = m_config.functionConfig->internalFlash;  //1：120 90°模组P25Q内置Flash。
					m_parser->setBuiltInFlash(m_builtInFlash, m_registry, m_cameraID);
				}
				if (m_config.functionConfig->isSubAvgBS)
				{
					if (!m_config.functionConfig->isSubAvgB)
						m_subAvgB = false;
					else
						m_subAvgB = true;
				}
				//更新校温参数
				if (m_config.functionConfig->collectIntervalS)
					m_collectInterval = m_config.functionConfig->collectInterval;
				if (m_config.functionConfig->shutterCollectIntervalS)
					m_shutterCollectInterval = m_config.functionConfig->shutterCollectInterval;
				if (m_config.functionConfig->collectNumberOfTimesS)
					m_collectNumberOfTimes = m_config.functionConfig->collectNumberOfTimes;
				//更新快门和NUC策略参数
				if (m_config.functionConfig->shutterTempUpperS)
					m_monitor->setShutterTempUpper(m_config.functionConfig->shutterTempUpper);
				if (m_config.functionConfig->nucTempUpperS)
					m_monitor->setNucTempUpper(m_config.functionConfig->nucTempUpper);
				if (m_config.functionConfig->shutterIntervalBottomS)
					m_monitor->setShutterIntervalBottom(m_config.functionConfig->shutterIntervalBottom);
				if (m_config.functionConfig->nucIntervalBottomS)
					m_monitor->setNucIntervalBottom(m_config.functionConfig->nucIntervalBottom);
				//更新自动快门策略参数
				if (m_config.functionConfig->bootTimeToNow1S)
				{
					m_monitor->setShutterParam(m_config.functionConfig->bootTimeToNow1,
						m_config.functionConfig->shutterPeriod1,
						m_config.functionConfig->bootTimeToNow2,
						m_config.functionConfig->shutterPeriod2,
						m_config.functionConfig->bootTimeToNow3,
						m_config.functionConfig->shutterPeriod3);
					flag = true;
				}
				//更新最大帧率
				if (m_config.functionConfig->maxFramerateS)
					m_maxFrameRate = m_config.functionConfig->maxFramerate;
				m_logger->output(LOG_INFO, "ReadConfig OK. product=%d", m_productType);
			}
		}
		else
		{
			m_logger->output(LOG_WARN, "ReadConfig failed. product=%d %d %d %d", m_productType, ret, len, m_cameraID);
		}
		/*用完之后释放内存*/
		m_parser->releaseConfBuf();
	}
	if (!flag)
	{
		//如果配置文件中没有就设置一组默认的参数。根据默认的自动快门策略参数更新快门周期
		m_monitor->setShutterParam(3600, 30, 7200, 60, 7200, 90);
	}
	if (m_registry && m_registry->I2COpen && m_registry->I2CClose
		&& m_registry->I2CRead && m_registry->I2CWrite && !m_detector && ITA_X16 == m_mode 
		&& ITA_256_TIMO == m_productType)
	{
		m_detector = new DetectorWrapper(m_registry, m_cameraID, m_range);
		m_detector->setLogger(m_logger);
		m_detector->setDebug(m_debug);
	}
	if (m_registry && m_registry->SPITransmit && m_registry->SPIWrite
		&& !m_timo120 && ITA_X16 == m_mode && ITA_120_TIMO == m_productType)
	{
		m_timo120 = new Timo120(m_registry, m_cameraID, m_range);
		m_timo120->setLogger(m_logger);
		m_timo120->setDebug(m_debug);
	}
	/*读数据包，放在注册函数中是因为getVersion要读取信息。如果放在prepare，那么会限制用户调用getVersion的时机。*/
	if(!m_builtInFlash)
	{
		ITA_FIELD_ANGLE initLenType = ITA_ANGLE_25;
		if (m_productType < ITA_HANDHELD_TEC)
		{
			/*安卓项目镜头为0的参数包不符合规范，因此初始化解析镜头类型为1的真实参数包*/
			result = m_parser->parsePackageData(m_cameraID, m_registry, m_range, initLenType);
		}
		else {
			result = m_parser->parseCurveData(m_cameraID, m_registry, initLenType, m_range, m_mtBase->getTECParams()->nGear);
		}
    }
	else
		result = m_parser->parseFlashPackage(m_cameraID, m_registry, m_range);
	if (ITA_OK == result)
	{
		/*初始化测温参数，创建CMeasureTempCore实例。*/
		m_mtBase->loadData(m_parser->getPH(), m_parser->getFPArray(), m_parser->getCurves());
		if (m_productType < ITA_HANDHELD_TEC)
			m_monitor->setFPAArray(m_parser->getFPArray(), m_parser->getPH()->ucFocusNumber, m_parser->getPH()->ucRes);
		/*切换测温范围配置寄存器。这里不能做，还没有configureDetector。*/
		/*if (ITA_256_TIMO == m_productType && m_detector)
			m_detector->ChangeGears(m_range);
		else if (ITA_120_TIMO == m_productType && m_timo120)
			m_timo120->ChangeGears(m_range, m_parser->getPH()->ucGain, m_parser->getPH()->ucInt, m_parser->getPH()->ucRes);*/
	}
	//读完数据包之后再载入配置信息，因为镜头类型在数据包中。
	m_mtBase->loadConfig(&m_config, m_range, (ITA_FIELD_ANGLE)m_parser->getPH()->lensType);
	m_processor->loadConfig(&m_config, m_range, (ITA_FIELD_ANGLE)m_parser->getPH()->lensType);
	//X16模式需要读取校温参数，切换测温范围时要更新。Y16数据参数行中不一定有校温参数所以需要读取，用户自行决定是否注册读参数函数。
	if (ITA_X16 == m_mode || ITA_Y16 == m_mode)
		m_parser->parseCalibrateParam(m_cameraID, m_registry, m_range, m_mtBase->getMtParams());
	m_af->setRegister(m_registry, m_cameraID);
	m_parser->isColdOrHot(m_cameraID, m_registry, m_mtBase->getMtParams(),m_coldHotDelay,&m_coldHotStatus);
	return result;
}

ITA_RESULT ITAWrapper::setMeasureRange(ITA_RANGE range)
{
	if (m_range == range)
	{
		return ITA_INACTIVE_CALL;
	}
	//读对应档位数据包
	m_isChangeRange = true;
	m_range = range;
	m_logger->output(LOG_INFO, "setMeasureRange %d", m_range);
	return ITA_OK;
}

ITA_RESULT ITAWrapper::detectorControl(ITA_DC_TYPE type, void * param)
{
	if (!param)
	{
		return ITA_NULL_PTR_ERR;
	}
	int flag = *(int *)param;
	//m_logger->output(LOG_INFO, "detectorControl %d %d", type, flag);
	switch (type)
	{
	case ITA_DETECTOR_VALID_MODE:       //0：探测器VSYNC和HSYNC高有效；1：探测器VSYNC和HSYNC低有效。参数类型：整型。
		if (flag < 0 || flag > 1)
		{
			return ITA_ARG_OUT_OF_RANGE;
		}
		m_validMode = (unsigned char)flag;
		break;
	case ITA_DETECTOR_DATA_MODE:        //0：先发16位数据的高8bits；1：先发16位数据的低8bits。参数类型：整型。
		if (flag < 0 || flag > 1)
		{
			return ITA_ARG_OUT_OF_RANGE;
		}
		m_dataMode = (unsigned char)flag;
		break;
	case ITA_DETECTOR_FRAME_FREQUENCY:  //设置探测器帧频，单位fps。默认25。参数类型：整型。
		if (flag < 1 || flag > 30)	//模组最大帧频30
		{
			return ITA_ARG_OUT_OF_RANGE;
		}
		m_frameFrequency = (unsigned char)flag;
		m_maxFrameRate = m_frameFrequency;
		break;
	default:
		break;
	}
	return ITA_OK;
}

ITA_RESULT ITAWrapper::configureDetector(int clock)
{
	ITA_RESULT ret;
	if (clock < 1000000)
	{
		m_logger->output(LOG_ERROR, "configureDetector clock=%d ret=%d", clock, ITA_ARG_OUT_OF_RANGE);
		return ITA_ARG_OUT_OF_RANGE;
	}
	if (m_mode != ITA_X16)
	{
		return ITA_INACTIVE_CALL;
	}
	m_logger->output(LOG_INFO, "configureDetector clock=%d %d %d %d", clock, m_validMode, m_dataMode, m_frameFrequency);
	if (ITA_256_TIMO == m_productType)
	{
		if (!m_detector)
			return ITA_HAL_UNREGISTER;
		ret = m_detector->DetectorInit(clock, m_validMode, m_dataMode, m_frameFrequency);
		/*切换测温范围配置寄存器*/
		m_detector->ChangeGears(m_range);
		return ret;
	}
	else if (ITA_120_TIMO == m_productType)
	{
		if (!m_timo120)
			return ITA_HAL_UNREGISTER;
		if (clock > 10000000) //120模组时钟上限10M
			return ITA_ARG_OUT_OF_RANGE;
		ret = m_timo120->DetectorInit(clock, m_validMode, m_dataMode, m_frameFrequency, m_parser->getDP());
		/*切换测温范围配置寄存器*/
		m_timo120->ChangeGears(m_range, m_parser->getPH()->ucGain, m_parser->getPH()->ucInt, m_parser->getPH()->ucRes, true);
		return ret;
	}
	else
		return ITA_UNSUPPORT_OPERATION;
}

ITA_RESULT ITAWrapper::prepare(ITAImgInfo * pInfo)
{
	ITA_RESULT result = ITA_OK;
	m_logger->output(LOG_INFO, "prepare mode=%d range=%d w=%d h=%d scale=%f cameraID=%d", m_mode, m_range, m_width, m_height, m_scale, m_cameraID);
	m_processor->getImageInfo(m_rotate, m_pixelFormat, pInfo);
	return result;
}
ITA_RESULT ITAWrapper::doISPTEC(unsigned char *srcData, int srcSize, ITAISPResult *pResult, unsigned char *y8Data, int y8Size)
{
	static unsigned int rateLastTime = 0;
	static int rateCount = 0;
	MONITOR_STATUS status;
	ITA_RESULT result = ITA_OK;
	short *mySrc;
#ifndef CONTINUE_FRAME_DROP
	unsigned int consumeTime = 0;
#endif
	//srcSize不能错，ImageProcessor::process memcpy用到，如果大于实际帧大小会报错heap corruption detected。
	if (srcSize != m_frameSize * 2)
		return ITA_ILLEGAL_PAPAM_ERR;
	m_loop = true;
	g_currentTime = porting_get_ms();
	mySrc = (short *)srcData;
	m_x16CenterValue = *(mySrc + m_frameSize / 2 - m_width / 2);
	//确定返回状态
	if (1 == m_mtBase->getTECParams()->bOrdinaryShutter)
		result = ITA_SHUTTER_CLOSING;
	else if (!pResult)
		result = ITA_NULL_PTR_ERR;
	//每秒获取一次温传
	if (g_currentTime < m_lastSensorTime)
	{
		//表明系统时间被改小，时间线清零。
		m_lastSensorTime = 0;
	}
	if (g_currentTime - m_lastSensorTime > 999 || !m_lastSensorTime)
	{
		m_lastSensorTime = g_currentTime;
		m_fps = rateCount;	//实际帧率计数
		rateCount = 0;
		if (m_debug && m_userFps && m_userFps != m_maxFrameRate)
		{
			m_logger->output(LOG_DEBUG, "isp=%d lisp=%d rateCount=%d userFps=%d", m_ispTime, m_lastIspTime, rateCount, m_userFps);
		}
		/*根据自动快门策略参数更新快门周期*/
		m_monitor->updatePeriod(g_currentTime);
	}
	//控制帧率
	if (m_userFps && m_userFps != m_maxFrameRate)
	{
		unsigned int intervaltemp = 1000 / m_userFps;
#ifdef CONTINUE_FRAME_DROP
		if (rateCount >= m_userFps)
		{
			m_loop = false;
			return ITA_SKIP_FRAME; //此帧不处理
}
#else
		//可以使用用户设置的平均帧处理时间。因为实时计算的帧时间很不稳定，影响实际帧率。
		if (m_userIspTime)
			consumeTime = m_userIspTime;
		else
			consumeTime = (m_lastIspTime + m_ispTime) / 2;
		if ((intervaltemp > consumeTime) && (g_currentTime - rateLastTime < intervaltemp - consumeTime))
		{
			m_loop = false;
			return ITA_SKIP_FRAME; //此帧不处理
		}
		else
		{
			if (m_debug)
				m_logger->output(LOG_DEBUG, "Interval between two frames is %d isp=%d lisp=%d %d count=%d inte=%d cons=%d",
					g_currentTime - rateLastTime, m_ispTime, m_lastIspTime, intervaltemp - (m_lastIspTime + m_ispTime) / 2, rateCount, intervaltemp, consumeTime);
			rateLastTime = g_currentTime;	//记录上帧时间
		}
#endif // CONTINUE_FRAME_DROP
	}
	rateCount++;
	if (!m_closeImage && pResult)
	{
		if (m_reCalcSize)
		{
			//由于返回给用户的pResult要描述正确的img长度，所以改成每次变化都重新申请内存。
			if (m_imgDst)
			{
				porting_free_mem(m_imgDst);
				m_imgDst = NULL;
				pResult->imgDst = NULL;
			}
			//目前Y16没有缩放可以不重新分配，如果缩放则需要。
			/*if (m_y16Data)
			{
			porting_free_mem(m_y16Data);
			m_y16Data = NULL;
			pResult->y16Data = NULL;
			}*/
			m_reCalcSize = false;
		}
		if (!m_imgDst)
		{
			ITAImgInfo pInfo;
			prepare(&pInfo);
			m_imgDst = (unsigned char *)porting_calloc_mem(pInfo.imgDataLen, 1, ITA_WRAPPER_MODULE);
			pResult->info = pInfo;
			pResult->imgDst = m_imgDst;
			if (!pResult->imgDst)
			{
				m_logger->output(LOG_ERROR, "doISPTEC ret=%d", ITA_OUT_OF_MEMORY);
			}
			if (!m_y16Data)
			{	//第一次时分配，后面不释放，退出时再释放。
				m_y16Data = (short *)porting_calloc_mem(pInfo.y16Len, sizeof(short), ITA_WRAPPER_MODULE);
				pResult->y16Data = m_y16Data;
			}
		}
		if (m_isRotate)
		{
			ITAImgInfo pInfo;
			prepare(&pInfo);
			pResult->info = pInfo;
			m_realW = pInfo.w;
			m_realH = pInfo.h;
			m_isRotate = false;
		}
		if (!pResult->imgDst)
		{
			/*这里的判断影响效率，正式版本可以对缩放倍数做限制*/
			result = ITA_OUT_OF_MEMORY;
		}
		else
		{
			//图像处理
			if (!y8Data)
				m_processor->process(mySrc, srcSize, pResult);
			else
				m_processor->copyY8(mySrc, srcSize, pResult, y8Data, y8Size);
		}
	}
	m_lastIspTime = m_ispTime;
	m_ispTime = porting_get_ms() - g_currentTime;
	//快门打开的状态下判断是否校温采集。下位机自动快门策略关不掉时有时会采到闭合时的Y16异常大。
	//记录采集状态：0，初始空状态；1，准备采集；2，正在采集；3，一次标定黑体采集完成。
	if (m_isAutoCollect == 1)
	{
		if (m_asicStatus == 2 && m_mtBase->getMtParams()->bOrdinaryShutter)
		{
			m_asicStatus = 3;
		}
		if (m_asicStatus == 3 && !m_mtBase->getMtParams()->bOrdinaryShutter)
		{
			m_asicStatus = 4;
		}
	}
	if (!m_mtBase->getTECParams()->bOrdinaryShutter && (1 == m_isAutoCollect || 2 == m_isAutoCollect))
	{
		if (1 == m_isAutoCollect)
		{
			if (m_mode != ITA_Y16)
			{
				/*如果大于10s那么直接打快门采集。否则等待。*/
				if (g_currentTime - m_monitor->getLastShutterTime() >= m_shutterCollectInterval * 1000)
				{
					baseControl(ITA_SHUTTER_NOW, 0);
					m_isAutoCollect = 2;
					m_logger->output(LOG_INFO, "AutoCollect=%d", m_isAutoCollect);
				}
			}
			else {
				if (m_registry && m_registry->ShutterControl)
				{
					if (m_asicStatus == 1)
					{
						m_registry->ShutterControl(m_cameraID, ITA_STATUS_COMPENSATE, m_registry->userParam);
						m_asicStatus = 2;
					}
					else if (m_asicStatus == 4)
					{
						m_isAutoCollect = 2;
						m_asicStatus = 1;
						m_logger->output(LOG_INFO, "ITA_Y16 AutoCollect =%d", m_isAutoCollect);
					}		
				}
			}
		}
		else if (2 == m_isAutoCollect)
		{
			/*每隔1s采集一个Y16，需采集15次，然后求15次的平均值。*/
			if ((g_currentTime - (m_monitor->getLastShutterTime() + m_collectInterval * 1000 * m_collectTimes)) >= m_collectInterval * 1000)
			{
				//中心点3*3区域内Y16均值
				//返回的Y16数据未拉伸，有旋转。
				int x, y, sum;
				//int avgB = guideManager->pBaseCore->GD_BASE_CORE_GETAVGB();
				int w = pResult->info.w, h = pResult->info.h;
				x = w / 2 - 1;
				y = h / 2 - 1;
				sum = pResult->y16Data[y*w + x];
				sum += pResult->y16Data[y*w + x - 1];
				sum += pResult->y16Data[y*w + x + 1];
				sum += pResult->y16Data[(y - 1)*w + x];
				sum += pResult->y16Data[(y - 1)*w + x - 1];
				sum += pResult->y16Data[(y - 1)*w + x + 1];
				sum += pResult->y16Data[(y + 1)*w + x];
				sum += pResult->y16Data[(y + 1)*w + x - 1];
				sum += pResult->y16Data[(y + 1)*w + x + 1];
				if (m_subAvgB)
					m_blackY16Array[m_collectTimes] = (short)(sum / 9 - m_avgB);
				else
					m_blackY16Array[m_collectTimes] = (short)(sum / 9);
				m_logger->output(LOG_INFO, "AutoCollect=%d collectCount=%d collectTimes=%d y16=%d", m_isAutoCollect, m_collectCount, m_collectTimes, m_blackY16Array[m_collectTimes]);
				m_collectTimes++;
			}
			if (m_collectTimes >= m_collectNumberOfTimes)
			{
				//一个黑体采集完成，计算15次平均值，回调通知。
				int sum = 0;
				std::sort(m_blackY16Array, m_blackY16Array + 15);
				short midValue = m_blackY16Array[m_collectTimes / 2];
				int actualCount = 0;
				for (unsigned int i = 0; i < m_collectTimes; i++)
				{
					m_logger->output(LOG_INFO, "collectCount=%d collectTimes=%d m_blackY16Array[%d]=%d,midValue=%d", m_collectCount, m_collectTimes, i, m_y16AvgArray[i], midValue);
					if (std::abs(m_blackY16Array[i] - midValue) < 20)
					{
						sum += m_blackY16Array[i];
						actualCount++;
					}
				}
				m_y16AvgArray[m_collectCount] = (short)(sum / (int)actualCount);
				m_logger->output(LOG_INFO, "collectCount=%d collectTimes=%d avg=%d sum=%d,midValue=%d,actualCount=%d", m_collectCount, m_collectTimes, m_y16AvgArray[m_collectCount], sum, midValue, actualCount);
				if (m_collectCBArray[m_collectCount])
				{
					m_collectCBArray[m_collectCount](m_cameraID, m_cbParamArray[m_collectCount]);
				}
				m_isAutoCollect = 3;
				//一次黑体采集周期内采集Y16计数清零
				m_collectTimes = 0;
				//下一个黑体标定计数
				m_collectCount++;
			}
		}
	}
	//判断是否切换测温范围
	if (m_isChangeRange)
	{
		/*读曲线包。TEC产品支持手动切换测温范围，切换完成后如果参数行中的测温范围发生变化但与手动切的范围一致，那么不用再重复切换。
		但是有个风险，手动切换新曲线，测温参数中还是旧的测温范围，直到参数行中更新范围后才正确。所以建议不要手动切范围，统一使用参数行。*/
		result = m_parser->parseCurveData(m_cameraID, m_registry, (ITA_FIELD_ANGLE)m_mtBase->getTECParams()->lensType, m_range, m_mtBase->getTECParams()->nGear);
		if (ITA_OK == result)
		{
			//m_mtBase->getTECParams()->mtType = m_range;//这里不能更新，因为参数行还是旧值会导致马上又切回旧范围。
			/*初始化测温参数，创建CMeasureTempCore实例。*/
			m_mtBase->loadData(m_parser->getPH(), m_parser->getFPArray(), m_parser->getCurves());
			m_mtBase->changeRange(m_range, (ITA_FIELD_ANGLE)m_parser->getPH()->lensType);
			m_mtBase->loadConfig(&m_config, m_range, (ITA_FIELD_ANGLE)m_parser->getPH()->lensType);
			m_processor->loadConfig(&m_config, m_range, (ITA_FIELD_ANGLE)m_parser->getPH()->lensType);
			//X16模式没有参数行，需要读取校温参数，切换测温范围时要更新。
			/*if (ITA_X16 == m_mode)
				m_parser->parseCalibrateParam(m_cameraID, m_registry, m_range, m_mtBase->getMtParams());*/
		}
		m_isChangeRange = false;
	}
	//快门闭合时不能测中心温
	if (m_isCalcCentral && !m_mtBase->getTECParams()->bOrdinaryShutter)
		m_mtBase->refreshFrame(pResult->y16Data, pResult->info.w, pResult->info.h);
	//快门和NUC策略，要在当前帧处理之后。如果快门闭合，那么下一帧开始采集本底。
	status = m_monitor->timeLoopOnce(g_currentTime);
	if (MONITOR_SHUTTER_NOW == status)
		shutterNow();
	else if (MONITOR_NUC_NOW == status)
		nucNow();
	m_loop = false;
	if (ITA_OK != result)
		m_logger->output(LOG_INFO, "ISPTEC %d", result);
	return result;
}

ITA_RESULT ITAWrapper::doISP(unsigned char * srcData, int srcSize, ITAISPResult * pResult, unsigned char *y8Data, int y8Size)
{
	static unsigned int rateLastTime = 0;
	static unsigned int rateCount = 0;
	MONITOR_STATUS status;
	ITA_RESULT result = ITA_OK;
	short *mySrc;
#ifndef CONTINUE_FRAME_DROP
	unsigned int consumeTime = 0;
#endif
	//srcSize不能错，ImageProcessor::process memcpy用到，如果大于实际帧大小会报错heap corruption detected。
    if (srcSize != m_frameSize * 2)
		return ITA_ILLEGAL_PAPAM_ERR;
    if (m_productType >= ITA_HANDHELD_TEC)
		return doISPTEC(srcData, srcSize, pResult, y8Data, y8Size);
	m_loop = true;
	g_currentTime = porting_get_ms();
#ifdef SOURCE_COPY
	if (!m_x16Data)
	{
		m_x16Data = (short *)porting_calloc_mem(m_frameSize, sizeof(unsigned short), ITA_WRAPPER_MODULE);
	}
	memcpy(m_x16Data, srcData, srcSize);
	mySrc = m_x16Data;
#else
	mySrc = (short *)srcData;
#endif // SOURCE_COPY
	m_x16CenterValue = *(mySrc + m_frameSize / 2 - m_width / 2);
	m_mtBase->getMtParams()->sCurrentShutterValue = m_x16CenterValue;	//实时更新该值。切范围时如果仅在快门闭合时更新，那么打印的是NUC之前的值，不是实时值引起误判。
	//确定返回状态
	if (m_firstNUCing)
		result = ITA_FIRST_NUC_NOT_FINISH;
	else if (1 == m_mtBase->getMtParams()->bOrdinaryShutter)
		result = ITA_SHUTTER_CLOSING;
	else if (!pResult)
		result = ITA_NULL_PTR_ERR;
	/*10分钟保存一次关机信息,防止意外断电导致的信息丢失*/
	if (m_monitor->isSaveColdHot(g_currentTime)) {
		m_parser->saveColdOrHot(m_cameraID, m_registry, m_mtBase->getMtParams());
	}
	//每秒获取一次温传
	if (g_currentTime < m_lastSensorTime)
	{
		//表明系统时间被改小，时间线清零。
		m_lastSensorTime = 0;
	}
	if (g_currentTime - m_lastSensorTime > 999 || !m_lastSensorTime)
	{
		if (m_registry && m_registry->GetSensorTemp && ITA_OK == m_parser->getSensorTemp(m_cameraID, m_registry, m_mtBase->getMtParams()))
		{
			if (ITA_120_TIMO == m_productType)
			{
				m_mtBase->getMtParams()->fRealTimeFpaTemp = m_mtBase->smoothFocusTemp(m_mtBase->getMtParams()->fRealTimeFpaTemp);
			}
			m_monitor->updateSensorTemp((int)(m_mtBase->getMtParams()->fRealTimeFpaTemp * 100));
			if (m_mtBase->getMtParams() && m_processor->getPotCoverSwitch())
			{
				m_monitor->updatePotCoverAlgoPara(m_mtBase->getMtParams(), &guoGaiPara);
				m_processor->putGuoGaiPara(&guoGaiPara);
			}
		}
		m_lastSensorTime = g_currentTime;
		//每秒更新温传后计算档位。
		//判断是否需要切档。在快门打开的状态下判断是否需要切档。快门闭合时禁止切档，否则温度异常可能掉10度左右。
		if (!m_mtBase->getMtParams()->bOrdinaryShutter)
		{
			//手动切档打开后，自动切档将失效。
			if (m_isGearManual)
			{
				//手动档位设置有效值，与当前档位m_fpaGear比较，不等就切换。
				if (m_gearManual >= 0 && m_gearManual != m_fpaGear)
				{
					m_isChangeGear = true;
					/*切换焦温档位，探测器配置相同，不用做NUC。*/
					ITA_RESULT ret = m_parser->changeGear(m_cameraID, m_registry, m_gearManual, (ITA_FIELD_ANGLE)m_mtBase->getMtParams()->m_fieldType);
					//X16模式下更新K
					if (ITA_OK == ret && (ITA_X16 == m_mode || ITA_MCU_X16 == m_mode))
					{
						m_processor->setCurrentK(m_parser->getCurrentK(), m_frameSize * 2);
					}
					m_fpaGear = m_gearManual;
					m_isChangeGear = false;
					m_logger->output(LOG_INFO, "manual change gear=%d FpaTemp=%f.", m_gearManual, m_mtBase->getMtParams()->fRealTimeFpaTemp);
				}
			}
			else
			{
				int currentGear = 0;
				//测温库中如果有5个焦温，那么有0 1 2 3 4 5共6个档位。0和5档位的4条曲线是重复的2组。4和5档位共用K。
				ITA_RESULT myResult = m_mtBase->calcFPAGear(&currentGear);
				if (ITA_OK == myResult)
				{
					/*zc08b项目中ITA初始化加载的默认参数包与产线参数包焦温数量不一致，当更新参数行后镜头类型改变，直接切换挡位会导致测温异常，需要重新加载整个参数包来避免此问题。*/
					if (m_fpaGear != currentGear && !m_isChangeRange)
					{
						m_isChangeGear = true;
						//焦温0.01差异引起几秒内频繁切档问题。这里不加时间限制。
						/*切换焦温档位，探测器配置相同，不用做NUC。如果calcFPAGear成功，那么读取数据包成功，所以这里不用判断结果。*/
						m_parser->changeGear(m_cameraID, m_registry, currentGear, (ITA_FIELD_ANGLE)m_mtBase->getMtParams()->m_fieldType);
						//X16模式下更新K
						if (ITA_X16 == m_mode || ITA_MCU_X16 == m_mode)
						{
							m_processor->setCurrentK(m_parser->getCurrentK(), m_frameSize * 2);
						}
						m_fpaGear = currentGear;
						m_logger->output(LOG_INFO, "auto change gear=%d FpaTemp=%f.", currentGear, m_mtBase->getMtParams()->fRealTimeFpaTemp);
						m_isChangeGear = false;
					}
				}
				//如果获取档位失败（没有数据包未初始化测温实例），那么不能测温。
			}
		}
		m_fps = rateCount;	//实际帧率计数
		rateCount = 0;
		if (m_debug && m_userFps && m_userFps != m_maxFrameRate)
		{
			m_logger->output(LOG_DEBUG, "isp=%d lisp=%d rateCount=%d userFps=%d", m_ispTime, m_lastIspTime, rateCount, m_userFps);
		}
		/*根据自动快门策略参数更新快门周期*/
		m_monitor->updatePeriod(g_currentTime);
	}
	//反向处理要在采集本底之前做
	if (m_reverse && (ITA_X16 == m_mode || ITA_MCU_X16 == m_mode))
	{
		int t;
		for (int i = 0; i < m_frameSize; i++)
		{
			//探测器输出的原始数据是反向的，要做处理。
			t = *(mySrc + i);
			t = 16383 - t;
			*(mySrc + i) = (short)t;
		}
	}
	/*校坏点*/
	if (m_badPoint.x > -1)
	{
		m_parser->correctBadPoint(m_badPoint);
		m_processor->setCurrentK(m_parser->getCurrentK(), m_frameSize * 2);
		m_badPoint.x = -1;
		m_badPoint.y = -1;
	}
	//如果快门闭合，并且是X16模式，则采集本底。
	if (m_isCollectBackground)
		collectBackground(mySrc, srcSize);
	if (!m_mtBase->getMtParams()->bOrdinaryShutter && m_collectK && m_collectK->isCollectK())
	{
		m_collectK->collectKData(mySrc);
	}
	//控制帧率
	if (m_userFps && m_userFps != m_maxFrameRate)
	{
		unsigned int intervaltemp = 1000 / m_userFps;
#ifdef CONTINUE_FRAME_DROP
		if (rateCount >= m_userFps)
		{
			m_loop = false;
			return ITA_SKIP_FRAME; //此帧不处理
		}
#else
		//可以使用用户设置的平均帧处理时间。因为实时计算的帧时间很不稳定，影响实际帧率。
		if (m_userIspTime)
			consumeTime = m_userIspTime;
		else
			consumeTime = (m_lastIspTime + m_ispTime) / 2;
		if ((intervaltemp > consumeTime) && (g_currentTime - rateLastTime < intervaltemp - consumeTime))
		{
			m_loop = false;
			return ITA_SKIP_FRAME; //此帧不处理
		}
		else
		{
			if (m_debug)
				m_logger->output(LOG_DEBUG, "Interval between two frames is %d isp=%d lisp=%d %d count=%d inte=%d cons=%d",
					g_currentTime - rateLastTime, m_ispTime, m_lastIspTime, intervaltemp - (m_lastIspTime + m_ispTime) / 2, rateCount, intervaltemp, consumeTime);
			rateLastTime = g_currentTime;	//记录上帧时间
		}
#endif // CONTINUE_FRAME_DROP
	}
	rateCount++;
	if (!m_closeImage && pResult)
	{
		if (m_reCalcSize)
		{
			//由于返回给用户的pResult要描述正确的img长度，所以改成每次变化都重新申请内存。
			if (m_imgDst)
			{
				porting_free_mem(m_imgDst);
				m_imgDst = NULL;
				pResult->imgDst = NULL;
			}
			//目前Y16没有缩放可以不重新分配，如果缩放则需要。
			/*if (m_y16Data)
			{
				porting_free_mem(m_y16Data);
				m_y16Data = NULL;
				pResult->y16Data = NULL;
			}*/
			m_reCalcSize = false;
		}
		if (!m_imgDst)
		{
			ITAImgInfo pInfo;
			prepare(&pInfo);
			m_imgDst = (unsigned char *)porting_calloc_mem(pInfo.imgDataLen, 1, ITA_WRAPPER_MODULE);
			pResult->info = pInfo;
			pResult->imgDst = m_imgDst;
			if (!pResult->imgDst)
			{
				m_logger->output(LOG_ERROR, "doISP ret=%d", ITA_OUT_OF_MEMORY);
			}
			if (!m_y16Data)
			{	//第一次时分配，后面不释放，退出时再释放。
				m_y16Data = (short *)porting_calloc_mem(pInfo.y16Len, sizeof(short), ITA_WRAPPER_MODULE);
				pResult->y16Data = m_y16Data;
			}
		}
		if (m_isRotate)
		{
			ITAImgInfo pInfo;
			prepare(&pInfo);
			pResult->info = pInfo;
			m_realW = pInfo.w;
			m_realH = pInfo.h;
			m_isRotate = false;
		}
		if (!pResult->imgDst)
		{
			/*这里的判断影响效率，正式版本可以对缩放倍数做限制*/
			result = ITA_OUT_OF_MEMORY;
		}
		else
		{
			//图像处理
			if(!y8Data)
				m_processor->process(mySrc, srcSize, pResult);
			else
				m_processor->copyY8(mySrc, srcSize, pResult, y8Data, y8Size);
		}
	}
	m_lastIspTime = m_ispTime;
	m_ispTime = porting_get_ms() - g_currentTime;
	if (!m_mtBase->getMtParams()->bOrdinaryShutter && m_startReview && m_startReview->getReviewStatus())
	{
		//利用中心y16测温
		float temp = 0.0;
		int w = pResult->info.w, h = pResult->info.h;
		short y16 = pResult->y16Data[w * h / 2 + w / 2];
		measureTempPoint(y16, &temp);
		m_startReview->review(temp, g_currentTime);
	}
	//快门打开的状态下判断是否校温采集。下位机自动快门策略关不掉时有时会采到闭合时的Y16异常大。
	//记录采集状态：0，初始空状态；1，准备采集；2，正在采集；3，一次标定黑体采集完成。
	if (m_isAutoCollect == 1)
	{
		if (m_asicStatus == 2 && m_mtBase->getMtParams()->bOrdinaryShutter)
		{
			m_asicStatus = 3;
		}
		if (m_asicStatus == 3 && !m_mtBase->getMtParams()->bOrdinaryShutter)
		{
			m_asicStatus = 4;
		}
	}
	if (!m_mtBase->getMtParams()->bOrdinaryShutter && (1 == m_isAutoCollect || 2 == m_isAutoCollect))
	{
		if (1 == m_isAutoCollect)
		{
			if (m_mode != ITA_Y16)
			{
				/*如果大于10s那么直接打快门采集。否则等待。*/
				if (g_currentTime - m_monitor->getLastShutterTime() >= m_shutterCollectInterval * 1000)
				{
					baseControl(ITA_SHUTTER_NOW, 0);
					m_isAutoCollect = 2;
					m_logger->output(LOG_INFO, "AutoCollect=%d", m_isAutoCollect);
				}
			}
			else {
				if (m_registry && m_registry->ShutterControl)
				{
					if (m_asicStatus == 1)
					{
						m_registry->ShutterControl(m_cameraID, ITA_STATUS_COMPENSATE, m_registry->userParam);
						m_asicStatus = 2;
					}
					else if (m_asicStatus == 4)
					{
						m_isAutoCollect = 2;
						m_asicStatus = 1;
						m_logger->output(LOG_INFO, "ITA_Y16 AutoCollect =%d", m_isAutoCollect);
					}
				}
			}
		}
		else if (2 == m_isAutoCollect)
		{
			/*每隔1s采集一个Y16，需采集15次，然后求15次的平均值。*/
			if ((g_currentTime - (m_monitor->getLastShutterTime() + m_collectInterval * 1000 * m_collectTimes)) >= m_collectInterval * 1000)
			{
				//中心点3*3区域内Y16均值
				//返回的Y16数据未拉伸，有旋转。
				int x, y, sum;
				int w = pResult->info.w, h = pResult->info.h;
				x = w / 2 - 1;
				y = h / 2 - 1;
				sum = pResult->y16Data[y*w + x];
				sum += pResult->y16Data[y*w + x - 1];
				sum += pResult->y16Data[y*w + x + 1];
				sum += pResult->y16Data[(y - 1)*w + x];
				sum += pResult->y16Data[(y - 1)*w + x - 1];
				sum += pResult->y16Data[(y - 1)*w + x + 1];
				sum += pResult->y16Data[(y + 1)*w + x];
				sum += pResult->y16Data[(y + 1)*w + x - 1];
				sum += pResult->y16Data[(y + 1)*w + x + 1];
				if (m_subAvgB)
					m_blackY16Array[m_collectTimes] = (short)(sum / 9 - m_avgB);
				else
					m_blackY16Array[m_collectTimes] = (short)(sum / 9);
				m_logger->output(LOG_INFO, "AutoCollect=%d collectCount=%d collectTimes=%d y16=%d", m_isAutoCollect, m_collectCount, m_collectTimes, m_blackY16Array[m_collectTimes]);
				m_collectTimes++;
			}
			if (m_collectTimes >= m_collectNumberOfTimes)
			{
				//一个黑体采集完成，计算15次平均值，回调通知。
				int sum = 0;
				std::sort(m_blackY16Array, m_blackY16Array + m_collectTimes);
				short midValue = m_blackY16Array[m_collectTimes / 2];
				int actualCount = 0;
				for (unsigned int i = 0; i < m_collectTimes; i++)
				{
					m_logger->output(LOG_INFO, "collectCount=%d collectTimes=%d m_blackY16Array[%d]=%d,midValue=%d", m_collectCount, m_collectTimes, i, m_blackY16Array[i], midValue);
					if (std::abs(m_blackY16Array[i] - midValue) < 20)
					{
						sum += m_blackY16Array[i];
						actualCount++;
					}
				}
				m_y16AvgArray[m_collectCount] = (short)(sum / (int)actualCount);
				m_logger->output(LOG_INFO, "collectCount=%d collectTimes=%d avg=%d sum=%d,midValue=%d,actualCount=%d", m_collectCount, m_collectTimes, m_y16AvgArray[m_collectCount], sum, midValue, actualCount);
				if (m_collectCBArray[m_collectCount])
				{
					m_collectCBArray[m_collectCount](m_cameraID, m_cbParamArray[m_collectCount]);
				}
				m_isAutoCollect = 3;
				//一次黑体采集周期内采集Y16计数清零
				m_collectTimes = 0;
				//下一个黑体标定计数
				m_collectCount++;
			}
		}
	}
	//判断是否切换测温范围
	if (m_isChangeRange)
	{
		/*读数据包*/
		ITA_RESULT readPkStatus = ITA_ERROR;
		if (!m_builtInFlash)
			readPkStatus = m_parser->parsePackageData(m_cameraID, m_registry, m_range, (ITA_FIELD_ANGLE)m_mtBase->getMtParams()->m_fieldType);
		else
			readPkStatus = m_parser->parseFlashPackage(m_cameraID, m_registry, m_range);
		if (ITA_OK == readPkStatus)
		{
			/*初始化测温参数，创建CMeasureTempCore实例。*/
			m_mtBase->loadData(m_parser->getPH(), m_parser->getFPArray(), m_parser->getCurves());
			m_mtBase->changeRange(m_range, (ITA_FIELD_ANGLE)m_parser->getPH()->lensType);
			if (m_parser->getlltt())
			{
				m_mtBase->loadConfig(&m_config, m_range, (ITA_FIELD_ANGLE)m_mtBase->getMtParams()->m_cflType);
			}
			else {
				m_mtBase->loadConfig(&m_config, m_range, (ITA_FIELD_ANGLE)m_parser->getPH()->lensType);
			}
			m_processor->loadConfig(&m_config, m_range, (ITA_FIELD_ANGLE)m_parser->getPH()->lensType);
			m_monitor->setFPAArray(m_parser->getFPArray(), m_parser->getPH()->ucFocusNumber, m_parser->getPH()->ucRes);
			/*切换测温范围配置寄存器*/
			if (ITA_256_TIMO == m_productType && m_detector)
				m_detector->ChangeGears(m_range);
			else if (ITA_120_TIMO == m_productType && m_timo120)
				m_timo120->ChangeGears(m_range, m_parser->getPH()->ucGain, m_parser->getPH()->ucInt, m_parser->getPH()->ucRes, false);
			/*工业测温每档对应两个距离，分一次和二次校温。切档时更新两组校温参数。
			仅二次校温无法满足人体测温精度，所以统一做自动校温。*/
			m_monitor->nucManual(1);
			//切换测温范围后，档位恢复初始值，下一帧切换到正确档位。
			m_fpaGear = -1;
			if (m_isGearManual && -1 == m_gearManual)
			{
				//如果关闭自动切档并且用户没有设置焦温档位，那么默认切到0档。
				m_gearManual = 0;
			}
			//X16模式没有参数行，需要读取校温参数，切换测温范围时要更新。Y16数据参数行中不一定有校温参数所以需要读取，用户自行决定是否注册读参数函数。
			if (ITA_X16 == m_mode || ITA_Y16 == m_mode)
				m_parser->parseCalibrateParam(m_cameraID, m_registry, m_range, m_mtBase->getMtParams());
		}
		m_isChangeRange = false;
	}
	//快门闭合时不能测中心温和AF
	if (m_isCalcCentral && !m_mtBase->getMtParams()->bOrdinaryShutter)
		m_mtBase->refreshFrame(pResult->y16Data, pResult->info.w, pResult->info.h);
#ifdef MULTI_THREAD_FUNCTION
	if (m_isAF && !m_mtBase->getMtParams()->bOrdinaryShutter)
		m_af->refreshFrame(pResult->y16Data, pResult->info.w, pResult->info.h);
#endif // MULTI_THREAD_FUNCTION
	//快门和NUC策略，要在当前帧处理之后。如果快门闭合，那么下一帧开始采集本底。
	status = m_monitor->timeLoopOnce(g_currentTime);
	if (MONITOR_SHUTTER_NOW == status)
		shutterNow();
	else if (MONITOR_NUC_NOW == status)
		nucNow();
	else if (MONITOR_NUC_CLOSED_LOOP == status || MONITOR_NUC_LOOP_REPEAT == status || MONITOR_NUC_RECOVERY == status)
	{
		if (ITA_120_TIMO == m_productType)
		{
			//120模组Res闭环处理。如果失败，5秒后继续，遍历完成后继续重复。
			//ucRes有3种情况：闭环+1，闭环成功后的值保持不变，恢复数据包中的值。
			static unsigned char ucRes = m_parser->getPH()->ucRes;
			if(MONITOR_NUC_CLOSED_LOOP == status)
				ucRes = m_monitor->calcRes(ucRes);
			else if(MONITOR_NUC_RECOVERY == status)
				ucRes = m_parser->getPH()->ucRes;
			if (ucRes)
			{
				m_timo120->ChangeGears(m_range, m_parser->getPH()->ucGain, m_parser->getPH()->ucInt, ucRes, false);
				nucNow();
			}
		}
	}
	m_loop = false;
	if(ITA_OK != result)
		m_logger->output(LOG_INFO, "ISP %d", result);
	return result;
}

ITA_RESULT ITAWrapper::measureTempPoint(short y16, float * surfaceTemp)
{
	ITA_RESULT ret = ITA_OK;
	int count = 0;
	//打快门过程中不能测温。
	/*if (m_mtBase->getMtParams() && 1 == m_mtBase->getMtParams()->bOrdinaryShutter)
	{
		return ITA_INACTIVE_CALL;
	}*/
	//TEC产品快门过程中允许拍照测温。视辉客户TEC产品在打快门过程中不允许测温。
#ifdef SHIHUI_EDITION
	if (m_mtBase->getTECParams() && 1 == m_mtBase->getTECParams()->bOrdinaryShutter)
	{
		return ITA_INACTIVE_CALL;
	}
	if (m_mtBase->getMtParams() && 1 == m_mtBase->getMtParams()->bOrdinaryShutter)
	{
		return ITA_INACTIVE_CALL;
	}
#endif
	if (m_isChangeRange || m_isChangeGear)
	{
		m_logger->output(LOG_INFO, "measureTempPoint is changeRange or changeGear.");
		return ITA_CHANGE_RANGE;
	}
	if(!m_subAvgB)
		ret = m_mtBase->calcTempByY16(y16, surfaceTemp);
	else
		ret = m_mtBase->calcTempByY16(y16 - m_avgB, surfaceTemp);
	return ret;
}

ITA_RESULT ITAWrapper::measureTempMatrix(short * y16Array, int y16W, int y16H, int x, int y, int w, int h, float distance, ITA_MATRIX_TYPE type, float *tempMatrix)
{
	//打快门过程中不能测温。
	/*if (m_mtBase->getMtParams() && 1 == m_mtBase->getMtParams()->bOrdinaryShutter)
	{
		return ITA_INACTIVE_CALL;
	}*/
	//视辉客户产品在打快门过程中不允许测温。
#ifdef SHIHUI_EDITION
	if (m_mtBase->getTECParams() && 1 == m_mtBase->getTECParams()->bOrdinaryShutter)
	{
		return ITA_INACTIVE_CALL;
	}
	if (m_mtBase->getMtParams() && 1 == m_mtBase->getMtParams()->bOrdinaryShutter)
	{
		return ITA_INACTIVE_CALL;
	}
#endif
	if (m_isChangeRange || m_isChangeGear)
	{
		m_logger->output(LOG_INFO, "measureTempPoint is changeRange or changeGear.");
		return ITA_CHANGE_RANGE;
	}
	if (m_subAvgB)
	{
		//注意这里会修改Y16缓存数据
		int len = y16W*y16H;
		if (!m_y16TempMatrix)
		{
			m_logger->output(LOG_INFO, "measureTempMatrix %d %d %d", y16W, y16H, len);
			m_y16TempMatrix = (short *)porting_calloc_mem(len, sizeof(short), ITA_WRAPPER_MODULE);
			m_tempMatrixLen = len * sizeof(short);
		}
		else if(m_tempMatrixLen < len * sizeof(short))
		{
			porting_free_mem(m_y16TempMatrix);
			m_logger->output(LOG_INFO, "measureTempMatrix2 %d %d %d", y16W, y16H, len);
			m_y16TempMatrix = (short *)porting_calloc_mem(len, sizeof(short), ITA_WRAPPER_MODULE);
			m_tempMatrixLen = len * sizeof(short);
		}
		memcpy(m_y16TempMatrix, y16Array, len * 2);
		for (int i = 0; i < len; i++)
		{
			*(m_y16TempMatrix + i) -= m_avgB;
		}
		return m_mtBase->calcTempMatrix(m_y16TempMatrix, y16W, y16H, distance, x, y, w, h, type, tempMatrix);
	}
	else 
		return m_mtBase->calcTempMatrix(y16Array, y16W, y16H, distance, x, y, w, h, type, tempMatrix);
}

ITA_RESULT ITAWrapper::baseControl(ITA_BC_TYPE type, void * param)
{
	ITA_SWITCH *flag = (ITA_SWITCH *)param;
	ITA_RESULT ret = ITA_OK;
	if (!param && ITA_SHUTTER_NOW!=type && ITA_DO_NUC!=type && ITA_CLOSE_SHUTTER!=type 
		&& ITA_OPEN_SHUTTER!=type && ITA_AUTO_FOCUS!=type && ITA_AF_NEAR!=type 
		&& ITA_AF_FAR!=type && ITA_STEP_NEAR!=type && ITA_STEP_FAR!=type)
	{
		ret = ITA_NULL_PTR_ERR;
		//m_logger->output(LOG_ERROR, "baseControl ret=%d", ret);
		return ret;
	}
	//m_logger->output(LOG_INFO, "baseControl type=%d", type);
	switch (type)
	{
	case ITA_SHUTTER_NOW:			//打快门
		if (m_mtBase->getMtParams() && 1 == m_mtBase->getMtParams()->bOrdinaryShutter)
		{
			m_logger->output(LOG_WARN, "ITA_SHUTTER_NOW:Shutter is busying.");
			return ITA_INACTIVE_CALL;
		}
		else
			ret = m_monitor->shutterManual(0); //m_monitor中的快门时间间隔做了限制，默认最低间隔3秒。
		break;
	case ITA_AUTO_SHUTTER:			//控制自动快门。参数类型：ITA_SWITCH。
		ret = m_monitor->autoShutter(*flag);
		break;
	case ITA_SHUTTER_TIME:			//设置自动快门时间，单位秒。默认30秒。参数类型：整型。
	{
		int sec = *(int *)param;
		if (sec > 0)
			ret = m_monitor->setInterval((unsigned int)sec);
		else
			ret = ITA_ILLEGAL_PAPAM_ERR;
		m_logger->output(LOG_INFO, "ITA_SHUTTER_TIME sec=%d ret=%d", sec, ret);
		break;
	}
	case ITA_SINGLE_STEP_TIME:
	{
		int msp = *(int *)param;
		//至少需要9帧360ms
		if (msp > 360)
			m_shutterFrameNumber = msp / 40;
		else
			ret = ITA_ARG_OUT_OF_RANGE;
		m_logger->output(LOG_INFO, "ITA_SINGLE_STEP_TIME shutterFrameNumber=%d p=%d", m_shutterFrameNumber, msp);
		break;
	}
	case ITA_STEP_DELAY_TIME:
	{
		int delayTime = *(int *)param;
		if (delayTime < 50 || delayTime>800)
		{
			ret = ITA_ARG_OUT_OF_RANGE;
		}
		else
		{
			m_shutterCloseDelay = delayTime;
			m_shutterOpenDelay = delayTime;
		}
		m_logger->output(LOG_INFO, "ITA_STEP_DELAY_TIME m_shutterOpenDelay=%d delayTime=%d ret=%d", 
			m_shutterOpenDelay, delayTime, ret);
		break;
	}
	case ITA_B_DELAY:
	{
		int msp = *(int *)param;
		//至少需要5帧
		if (msp >= 5 && msp <= 15)
		{
			m_bDelayFrame = msp;
			m_shutterFrameNumber = m_bDelayFrame + 4; //这里如果不改，本底没采到4帧就退出，本底不对无法成像。
		}
		else
			ret = ITA_ARG_OUT_OF_RANGE;
		m_logger->output(LOG_INFO, "ITA_B_DELAY %d %d %d", m_bDelayFrame, msp, m_shutterFrameNumber);
		break;
	}
	case ITA_GET_SHUTTER_STATUS:		//获取快门状态，快门闭合时用户可以定格画面。参数类型：ITA_SHUTTER_STATUS。在X16使用快门策略的情况下用到。
	{
		if (param)
		{
			if (m_mtBase->getMtParams())
			{
				if (1 == m_mtBase->getMtParams()->bOrdinaryShutter)
					*(ITA_SHUTTER_STATUS *)param = ITA_STATUS_CLOSE;
				else
					*(ITA_SHUTTER_STATUS *)param = ITA_STATUS_OPEN;
			}
			else if (m_mtBase->getTECParams())
			{
				if (1 == m_mtBase->getTECParams()->bOrdinaryShutter)
					*(ITA_SHUTTER_STATUS *)param = ITA_STATUS_CLOSE;
				else
					*(ITA_SHUTTER_STATUS *)param = ITA_STATUS_OPEN;
			}
		}
		else
		{
			ret = ITA_NULL_PTR_ERR;
			m_logger->output(LOG_ERROR, "ITA_GET_SHUTTER_STATUS ret=%d", ret);
		}
		break;
	}
	case ITA_FIRST_NUC_FINISH:		//第一次NUC是否完成。参数类型：ITA_SWITCH。ITA_DISABLE，未完成；ITA_ENABLE，已完成。
		if (m_firstNUCing)
			*flag = ITA_DISABLE;
		else
			*flag = ITA_ENABLE;
		break;
	case ITA_DO_NUC:             //手动NUC
		if (m_mtBase->getMtParams())
		{
			if (1 == m_mtBase->getMtParams()->bOrdinaryShutter)
			{
				m_logger->output(LOG_WARN, "ITA_DO_NUC:Shutter is busying.");
				return ITA_INACTIVE_CALL;
			}
			else
				ret = m_monitor->nucManual(0);
		}
		else
			ret = ITA_UNSUPPORT_OPERATION;
		break;
	case ITA_AUTO_GEARS:
		if (*flag > ITA_ENABLE || *flag < ITA_DISABLE)
			return ITA_ARG_OUT_OF_RANGE;
		if ((ITA_DISABLE == *flag && m_isGearManual) || (ITA_ENABLE == *flag && !m_isGearManual))
		{
			ret = ITA_INACTIVE_CALL;
			m_logger->output(LOG_WARN, "ITA_AUTO_GEARS ITA_INACTIVE_CALL");
			break;
		}
		if (ITA_DISABLE == *flag)
			m_isGearManual = true;
		else
			m_isGearManual = false;
		m_gearManual = -1;
		m_logger->output(LOG_INFO, "ITA_AUTO_GEARS %d", *flag);
		break;
	case ITA_CHANGE_GEAR:
	{
		int count = 0;
		if (!m_isGearManual)
			return ITA_INACTIVE_CALL;
		//如果正在切档就等待，200ms超时。通常最多等待一帧的时间几十毫秒。
		while ((m_isChangeRange || m_isChangeGear) && count < 200)
		{
			porting_thread_sleep(1);
			count++;
			m_logger->output(LOG_INFO, "ITA_CHANGE_GEAR is waiting. isChangeGear=%d, isChangeRange=%d, count=%d", m_isChangeGear, m_isChangeRange, count);
		}
		if (count >= 200)
		{
			ret = ITA_CHANGE_GEAR_TIMEOUT;
			break;
		}			
		//检查档位范围
		int gear = *(int *)param;
		//区分tec和非tec
		if (m_mtBase->getMtParams())
		{
			if (gear >= 0 && gear <= m_parser->getPH()->ucFocusNumber && m_parser->getPH()->ucFocusNumber < MAX_FOCUS_NUMBER)
			{
				if (m_gearManual == gear)
				{
					ret = ITA_INACTIVE_CALL;
					m_logger->output(LOG_WARN, "ITA_CHANGE_GEAR ITA_INACTIVE_CALL");
				}
				else
					m_gearManual = gear;
				m_logger->output(LOG_INFO, "ITA_CHANGE_GEAR. gear=%d ucFocusNumber=%d", gear, m_parser->getPH()->ucFocusNumber);
			}
			else
			{
				ret = ITA_ARG_OUT_OF_RANGE;
				m_logger->output(LOG_ERROR, "ITA_CHANGE_GEAR. ITA_ARG_OUT_OF_RANGE ret=%d, gear=%d ucFocusNumber=%d", ret, gear, m_parser->getPH()->ucFocusNumber);
			}
		}
		else if (m_mtBase->getTECParams())
		{
			if (gear == m_mtBase->getTECParams()->nGear)
			{
				ret = ITA_INACTIVE_CALL;
				m_logger->output(LOG_WARN, "ITA_CHANGE_GEAR ITA_INACTIVE_CALL");
			}
			else {
				m_mtBase->getTECParams()->nGear = gear;
				ITA_RESULT ret = m_parser->parseCurveData(m_cameraID, m_registry, (ITA_FIELD_ANGLE)m_mtBase->getTECParams()->lensType, m_range, gear);
				if (ret == ITA_OK)
				{
					m_mtBase->loadData(m_parser->getPH(), m_parser->getFPArray(), m_parser->getCurves());
					m_mtBase->changeRange(m_range, (ITA_FIELD_ANGLE)m_parser->getPH()->lensType);
				}
			}
		}
		
		break;
	}
	case ITA_SHUTTER_POLICY:		//控制快门、NUC、切档策略。参数类型：ITA_SWITCH。默认打开。在ITA_Y16模式下默认关闭。
		if (*flag > ITA_ENABLE || *flag < ITA_DISABLE)
			return ITA_ARG_OUT_OF_RANGE;
		if (ITA_DISABLE == *flag)
		{
			m_monitor->closeShutterPolicy(true);
			//如果ASIC Y16模式下初始化时用户设置关闭快门策略，那么ITA不用启动时做一次NUC。
			m_firstNUCing = false;
		}
		else
			m_monitor->closeShutterPolicy(false);
		m_logger->output(LOG_INFO, "ITA_SHUTTER_POLICY %d", *flag);
		break;
	case ITA_CLOSE_SHUTTER:
		if (m_mtBase->getMtParams())
		{
			if (m_mtBase->getMtParams()->bOrdinaryShutter)
			{
				m_logger->output(LOG_WARN, "ITA_CLOSE_SHUTTER:Shutter is busying.");
				return ITA_INACTIVE_CALL;
			}
			//闭合快门。用户单步控制快门，不用触发采本底机制。
			if (m_registry && m_registry->ShutterControl)
				ret = m_registry->ShutterControl(m_cameraID, ITA_STATUS_CLOSE, m_registry->userParam);
			else
				ret = ITA_HAL_UNREGISTER;
			m_logger->output(LOG_INFO, "ITA_CLOSE_SHUTTER");
		}
		else
			ret = ITA_UNSUPPORT_OPERATION;
		break;
	case ITA_OPEN_SHUTTER:
		//弹开快门。
		if (m_mtBase->getMtParams())
		{
			if (m_registry && m_registry->ShutterControl)
				ret = m_registry->ShutterControl(m_cameraID, ITA_STATUS_OPEN, m_registry->userParam);
			else
				ret = ITA_HAL_UNREGISTER;
			m_logger->output(LOG_INFO, "ITA_OPEN_SHUTTER");
		}
		else
			ret = ITA_UNSUPPORT_OPERATION;
		break;
	case ITA_IMAGE_PROCESS:			//控制图像处理。参数类型：ITA_SWITCH。默认打开。在X16模式下不能关闭，在Y16模式下可以关闭。例如某类产品通过芯片ISP已经获取YUV，使用ITA仅测温。
		if (*flag > ITA_ENABLE || *flag < ITA_DISABLE)
			return ITA_ARG_OUT_OF_RANGE;
		if (ITA_DISABLE == *flag)
		{
			if(ITA_X16 == m_mode || ITA_MCU_X16 == m_mode)
				return ITA_INACTIVE_CALL;
			if (m_closeImage)
				return ITA_INACTIVE_CALL;
			m_closeImage = true;
			if (m_imgDst)
			{
				porting_free_mem(m_imgDst);
				m_imgDst = NULL;
			}
			if (m_y16Data)
			{
				porting_free_mem(m_y16Data);
				m_y16Data = NULL;
			}
		}
		else
		{
			if (!m_closeImage)
				return ITA_INACTIVE_CALL;
			m_closeImage = false;
		}
		m_logger->output(LOG_INFO, "ITA_IMAGE_PROCESS %d", *flag);
		break;
	case ITA_SET_FRAMERATE:
	{
		int frameRate = *(int *)param;
		if (frameRate > 0 && frameRate <= (int)m_maxFrameRate)
		{
			m_userFps = frameRate; //用户设置的帧率
			m_logger->output(LOG_INFO, "SET_FRAMERATE:%d", frameRate);
		}
		else
			ret = ITA_ARG_OUT_OF_RANGE;
		break;
	}
	case ITA_GET_FRAMERATE:
		if (param)
		{
			if (m_fps > m_maxFrameRate)
				*(int *)param = m_maxFrameRate;
			else
				*(int *)param = m_fps;
		}
		else
			ret = ITA_NULL_PTR_ERR;
		break;
	case ITA_SET_CONSUME:
	{
		int consume = *(int *)param;
		if (consume > 0 && consume < 500)
		{
			m_userIspTime = consume;
			m_logger->output(LOG_INFO, "SET_CONSUME:%d", m_userIspTime);
		}
		else
			ret = ITA_ARG_OUT_OF_RANGE;
		break;
	}
	case ITA_NUC_ANOTHER_SHUTTER:
	{
		if (*flag > ITA_ENABLE || *flag < ITA_DISABLE)
			return ITA_ARG_OUT_OF_RANGE;
		//重复快门纠错机制。默认0关闭。1，用户打开nuc重复快门机制；2，即将重复快门；3，正在快门。快门动作完成后切到1状态。
		if ((ITA_DISABLE == *flag && m_nucAnotherShutter != 1) || (ITA_ENABLE == *flag && m_nucAnotherShutter))
		{
			ret = ITA_INACTIVE_CALL;
			m_logger->output(LOG_WARN, "ITA_NUC_ANOTHER_SHUTTER ITA_INACTIVE_CALL %d %d", *flag, m_nucAnotherShutter);
			break;
		}
		if (ITA_DISABLE == *flag)
			m_nucAnotherShutter = 0;
		else
			m_nucAnotherShutter = 1;
		m_logger->output(LOG_INFO, "ITA_NUC_ANOTHER_SHUTTER %d", *flag);
		break;
	}
	case ITA_NUC_REPEAT_CORRECTION:
	{
		if (*flag > ITA_ENABLE || *flag < ITA_DISABLE)
			return ITA_ARG_OUT_OF_RANGE;
		//重复NUC纠错机制。默认0关闭。1，用户打开nuc重复纠错机制。如果两种纠错同时设置，那么只用做ITA_NUC_REPEAT_CORRECTION。
		if ((ITA_DISABLE == *flag && m_nucRepeatCorrection != 1) || (ITA_ENABLE == *flag && m_nucRepeatCorrection))
		{
			ret = ITA_INACTIVE_CALL;
			m_logger->output(LOG_WARN, "ITA_NUC_REPEAT_CORRECTION ITA_INACTIVE_CALL %d %d", *flag, m_nucRepeatCorrection);
			break;
		}
		if (ITA_DISABLE == *flag)
			m_nucRepeatCorrection = 0;
		else
			m_nucRepeatCorrection = 1;
		m_processor->setRepeatCorrection(m_nucRepeatCorrection);
		m_logger->output(LOG_INFO, "ITA_NUC_REPEAT_CORRECTION %d", *flag);
		break;
	}
	case ITA_NUC_REPEAT_THRESHOLD:
	{
		//阈值可以设置。
		m_processor->setThreshold(param);
		break;
	}
	case ITA_CH_DELAY:
	{
		int delay = *(int*)param;
		if (delay <= 0) {
			ret = ITA_INACTIVE_CALL;
		}
		else {
			m_coldHotDelay = delay;
		}
		break;
	}
	case ITA_SET_CHSTATUS:
	{
		unsigned char chStatus = *(unsigned char*)param;
		if (chStatus < 0 || chStatus > 1) {
			ret = ITA_INACTIVE_CALL;
		}
		m_parser->setChStatus(chStatus);
		break;
	}
	/*	ITA_CONF_ENCRYPT,			//设置配置文件是否加密。参数类型：ITA_SWITCH。ITA_DISABLE，未加密；ITA_ENABLE，加密。*/
	/*case ITA_CONF_ENCRYPT:
	{
		if (*flag > ITA_ENABLE || *flag < ITA_DISABLE)
			return ITA_ARG_OUT_OF_RANGE;
		if (ITA_ENABLE == *flag)
			m_parser->setConfEncrypt(true);
		else
			m_parser->setConfEncrypt(false);
		break;
	}*/
	case ITA_AUTO_FOCUS:			//自动调焦
	case ITA_AF_NEAR:				//调最近焦
	case ITA_AF_FAR:				//调最远焦
	case ITA_STEP_NEAR:				//调整一个时间周期步长近焦
	case ITA_STEP_FAR:				//调整一个时间周期步长远焦
		if (!m_isAF)
			m_isAF = true;			
		ret = m_af->AFControl(type, param);
		break;
	default:
		break;
	}
	return ret;
}

ITA_RESULT ITAWrapper::measureControl(ITA_MC_TYPE type, void * param)
{
	if (ITA_SUB_AVGB == type)
	{
		if (!param)
			return ITA_NULL_PTR_ERR;
		ITA_SWITCH flag = *(ITA_SWITCH *)param;
		if (flag > ITA_ENABLE || flag < ITA_DISABLE)
			return ITA_ARG_OUT_OF_RANGE;
		if (ITA_ENABLE == flag)
			m_subAvgB = true;
		else
			m_subAvgB = false;
		return ITA_OK;
	}
	else if (ITA_GET_AVGB == type)
	{
		if (!param)
			return ITA_NULL_PTR_ERR;
		*(short *)param = m_avgB;
		return ITA_OK;
	}
	else
	{
		if (ITA_CENTRAL_TEMPER == type && !m_isCalcCentral)
		{
			m_isCalcCentral = true; //下一帧开始计算中心温
			return ITA_CENTRAL_NOT_READY;
		}
		//如果计算中心温并且减去本底均值，那么m_mtBase内没有这些信息无法计算。只能在这里计算。
		if (ITA_CENTRAL_TEMPER == type)
		{
			//m_mtBase输出中心点y16
			ITA_RESULT ret = m_mtBase->parametersControl(type, param);
			if (ITA_OK == ret)
			{
				short nCenter = *(short *)param;
				ret = measureTempPoint(nCenter, (float *)param);
			}
			return ret;
		}
		else
			return m_mtBase->parametersControl(type, param);
	}
}

ITA_RESULT ITAWrapper::imageControl(ITA_IC_TYPE type, void * param)
{
	bool *bParam = (bool *)param;
	ITA_RESULT ret = ITA_OK;
	if (!param)
	{
		ret = ITA_NULL_PTR_ERR;
		m_logger->output(LOG_ERROR, "ITAWrapper::imageControl ret=%d", ret);
		return ret;
	}
	m_logger->output(LOG_INFO, "imageControl op=%d", type);
	switch (type)
	{
	case ITA_SET_FORMAT:		//设置成像像素格式。参数类型：ITA_PIXEL_FORMAT。
	{
		ITA_PIXEL_FORMAT format = *(ITA_PIXEL_FORMAT *)param;
		int count = 0;
		if (format > ITA_YVU420_Plane || format < ITA_RGB888)
		{
			ret = ITA_ARG_OUT_OF_RANGE;
			break;
		}
		if (format == m_pixelFormat)
		{
			ret = ITA_INACTIVE_CALL;
			break;
		}
		//doISP接口返回后才能重新计算宽高。
		while (m_loop && count < 200)
		{
			porting_thread_sleep(1);
			count++;
			m_logger->output(LOG_INFO, "ITA_SET_FORMAT:Waiting to end a process.");
		}
		if (count >= 200)
		{
			ret = ITA_WAIT_TIMEOUT;
			break;
		}
		m_pixelFormat = format;
		m_processor->setPixelFormat(m_pixelFormat);
		m_reCalcSize = true;
		break;
	}
	case ITA_SET_PALETTE:		//设置伪彩，范围0-11。参数类型：ITA_PALETTE_TYPE。
	{
		ITA_PALETTE_TYPE paletteIndex = *(ITA_PALETTE_TYPE *)param;
		if (m_paletteIndex == paletteIndex)
			return ITA_INACTIVE_CALL;
		ret = m_processor->setPaletteIndex(paletteIndex);
		if(ITA_OK == ret)
			m_paletteIndex = paletteIndex;
		break;
	}
	case ITA_SET_SCALE:			//缩放倍数。参数类型：浮点。
	{
		float scale = *(float *)param;
		int count = 0;
		if (scale == m_scale)
		{
			ret = ITA_INACTIVE_CALL;
			break;
		}
		else if (scale < 1 || scale > 20)
		{ //放大200倍，申请内存时Android上返回不为空的指针，但实际不可用，所以限制上限20倍。
			ret = ITA_ARG_OUT_OF_RANGE;
			break;
		}
		//doISP接口返回后才能重新申请内存。
		while (m_loop && count < 200)
		{
			porting_thread_sleep(1);
			count++;
			m_logger->output(LOG_INFO, "ITA_SET_SCALE:Waiting to end a process.");
		}
		if (count >= 200)
		{
			ret = ITA_WAIT_TIMEOUT;
			break;
		}
		ret = m_processor->setScale(scale);
		//根据新的尺寸重新申请内存。
		if (ITA_OK == ret)
		{
			m_scale = scale;
			m_reCalcSize = true;
		}
		break;
	}
	case ITA_SET_HSM_WEIGHT:
	{
		int hsmWeight = *(int*)param;
		if (hsmWeight < -1000 || hsmWeight > 1000) {
			ret = ITA_ARG_OUT_OF_RANGE;
			break;
		}
		ISPParam ispParam = m_processor->getISPParam();
		ispParam.hsmWeight = hsmWeight;
		m_processor->setISPParam(ispParam);
		break;
	}
	case ITA_SET_LAPLACE_WEIGHT:
	{
		float laplaceWeight = *(float*)param;
		if (laplaceWeight < 0 || laplaceWeight > 10)
		{
			ret = ITA_ARG_OUT_OF_RANGE;
			break;
		}
		ISPParam ispParam = m_processor->getISPParam();
		ispParam.laplaceWeight = laplaceWeight;
		m_processor->setISPParam(ispParam);
		break;
	}
	case ITA_SET_HSM_STAYTHR:
	{
		ISPParam ispParam = m_processor->getISPParam();
		ispParam.hsmStayThr = *(int*)param;
		if (ispParam.hsmStayThr < 0 || ispParam.hsmStayThr >65535)
		{
			ret = ITA_ARG_OUT_OF_RANGE;
			break;
		}
		m_processor->setISPParam(ispParam);
		break;
	}
	case ITA_SET_HSM_STAYWEIGHT:
	{
		ISPParam ispParam = m_processor->getISPParam();
		ispParam.hsmStayWeight = *(int*)param;
		if (ispParam.hsmStayWeight < -1000 || ispParam.hsmStayWeight > 1000)
		{
			ret = ITA_ARG_OUT_OF_RANGE;
			break;
		}
		m_processor->setISPParam(ispParam);
		break;
	}
	case ITA_HSM_INTERVAL:
	{
		ISPParam ispParam = m_processor->getISPParam();
		ispParam.hsmInterval = *(int*)param;
		if (ispParam.hsmInterval <0)
		{
			ret = ITA_ARG_OUT_OF_RANGE;
			break;
		}
		m_processor->setISPParam(ispParam);
		break;
	}
	case ITA_HSM_MIDFILTER:
	{
		ISPParam ispParam = m_processor->getISPParam();
		ispParam.hsmMidfilter = *(unsigned char*)param;
		m_processor->setISPParam(ispParam);
		break;
	}
	case ITA_HSM_MIDFILTER_WINSIZE:
	{
		ISPParam ispParam = m_processor->getISPParam();
		ispParam.hsmMidfilterWinsize = *(int*)param;
		if (ispParam.hsmMidfilterWinsize <= 0 || ispParam.hsmMidfilterWinsize > 20)
		{
			ret = ITA_ARG_OUT_OF_RANGE;
			break;
		}
		m_processor->setISPParam(ispParam);
		break;
	}
	case ITA_HSM_DELTA_UPTHR:
	{
		ISPParam ispParam = m_processor->getISPParam();
		ispParam.hsmDeltaUpthr = *(int*)param;
		if (ispParam.hsmDeltaUpthr < 0)
		{
			ret = ITA_ARG_OUT_OF_RANGE;
			break;
		}
		m_processor->setISPParam(ispParam);
		break;
	}
	case ITA_HSM_DELTA_DOWNTHR:
	{
		ISPParam ispParam = m_processor->getISPParam();
		ispParam.hsmDeltaDownthr = *(int*)param;
		if (ispParam.hsmDeltaDownthr > 0)
		{
			ret = ITA_ARG_OUT_OF_RANGE;
			break;
		}
		m_processor->setISPParam(ispParam);
		break;
	}
	case ITA_HSM_POINT_THR:
	{
		ISPParam ispParam = m_processor->getISPParam();
		ispParam.hsmPointThr = *(int*)param;
		if (ispParam.hsmPointThr < 0)
		{
			ret = ITA_ARG_OUT_OF_RANGE;
			break;
		}
		m_processor->setISPParam(ispParam);
		break;
	}
	case ITA_SET_CONTRAST:		//设置对比度。对比度0-511。默认255。参数类型：整形。
	{
		int contrast = *(int *)param;
		ret = m_processor->setContrast(contrast);
		break;
	}
	case ITA_SET_BRIGHTNESS:	//设置亮度。亮度0-255。默认70。参数类型：整形。
	{
		int bright = *(int *)param;
		ret = m_processor->setBrightness(bright);
		break;
	}
	case ITA_SET_ROTATE:		//设置旋转方式，RGB和Y16数据均旋转。参数类型：ITA_ROTATE_TYPE。
	{
		ITA_ROTATE_TYPE rotateType = *(ITA_ROTATE_TYPE *)param;
		int count = 0;
		if (rotateType == m_rotate)
		{
			ret = ITA_INACTIVE_CALL;
			break;
		}
		//doISP接口返回后才能重新计算宽高。
		while (m_loop && count < 200)
		{
			porting_thread_sleep(1);
			count++;
			m_logger->output(LOG_INFO, "ITA_SET_ROTATE:Waiting to end a process.");
		}
		if (count >= 200)
		{
			ret = ITA_WAIT_TIMEOUT;
			break;
		}
		ret = m_processor->setRotate(rotateType);
		if (ITA_OK == ret)
		{
			m_rotate = rotateType;
			m_isRotate = true;
		}
		break;
	}
	case ITA_SET_FLIP:			//设置翻转，RGB和Y16数据均翻转。参数类型：ITA_FLIP_TYPE。
	{
		ITA_FLIP_TYPE flipType = *(ITA_FLIP_TYPE *)param;
		if (flipType == m_flip)
			return ITA_INACTIVE_CALL;
		ret = m_processor->setFlip(flipType);
		if(ITA_OK == ret)
			m_flip = flipType;
		break;
	}
	case ITA_SET_REVERSE:		//打开或者关闭反向处理。ITA_X16模式下默认打开。参数类型：bool。
	{
		if (m_reverse == *bParam)
		{
			ret = ITA_INACTIVE_CALL;
		}
		else
		{
			m_reverse = *bParam;
		}
		break;
	}
	case ITA_SET_ZOOM:
	{
		ITA_ZOOM_TYPE zoomType = *(ITA_ZOOM_TYPE*)param;
		if (zoomType > ITA_ZOOM_BILINEARFAST || zoomType < ITA_ZOOM_NEAR) {
			ret = ITA_ARG_OUT_OF_RANGE;
		}
		else {
			ISPParam ispParam = m_processor->getISPParam();
			ispParam.zoomType = zoomType;
			ret = m_processor->setISPParam(ispParam);
		}
		break;
	}
	case ITA_TIME_FILTER:		//时域滤波开关，可以关闭或者打开。参数类型：bool。
	{
		ISPParam ispParam = m_processor->getISPParam();
		ispParam.isTimeFilter = *bParam;
		m_processor->setISPParam(ispParam);
		break;
	}
	case ITA_SPATIAL_FILTER:		//空域滤波开关，可以关闭或者打开。参数类型：bool。
	{
		ISPParam ispParam = m_processor->getISPParam();
		ispParam.isSpatialFilter = *bParam;
		m_processor->setISPParam(ispParam);
		break;
	}
	case ITA_REMOVE_VERTICAL:	//去竖纹，可以关闭或者打开。参数类型：bool。
	{
		ISPParam ispParam = m_processor->getISPParam();
		ispParam.isRemoveVertical = *bParam;
		m_processor->setISPParam(ispParam);
		break;
	}
	case ITA_REMOVE_HORIZONTAL:	//去横纹，可以关闭或者打开。参数类型：bool。
	{
		ISPParam ispParam = m_processor->getISPParam();
		ispParam.isRemoveHorizontal = *bParam;
		m_processor->setISPParam(ispParam);
		break;
	}
	case ITA_HSM:	//气体检测， 可以关闭或者打开。 参数类型：bool
	{
		ISPParam ispParam = m_processor->getISPParam();
		ispParam.isHSM = *bParam;
		m_processor->setISPParam(ispParam);
		break;
	}
	case ITA_SHARPENING:		//锐化，可以关闭或者打开。参数类型：bool。
	{
		ISPParam ispParam = m_processor->getISPParam();
		ispParam.isSharpening = *bParam;
		m_processor->setISPParam(ispParam);
		break;
	}
	case ITA_DETAIL_ENHANCE:	//细节增强，可以关闭或者打开。参数类型：bool。
	{
		ISPParam ispParam = m_processor->getISPParam();
		ispParam.isDetailEnhance = *bParam;
		m_processor->setISPParam(ispParam);
		break;
	}
	case ITA_BLOCK_HISTOGRAM:	//分块直方图，可以关闭或者打开。参数类型：bool。
	{
		ISPParam ispParam = m_processor->getISPParam();
		ispParam.isBlockHistogram = *bParam;
		m_processor->setISPParam(ispParam);
		break;
	}
	case ITA_GAMMA_CORRECTION:		//Gamma校正，可以关闭或者打开。参数类型：bool。
	{
		ISPParam ispParam = m_processor->getISPParam();
		ispParam.isGammaCorrection = *bParam;
		m_processor->setISPParam(ispParam);
		break;
	}
	case ITA_Y8_ADJUSTBC:		//Y8纠偏，可以关闭或者打开。参数类型：bool。
	{
		ISPParam ispParam = m_processor->getISPParam();
		ispParam.isY8AdjustBC = *bParam;
		m_processor->setISPParam(ispParam);
		break;
	}
	case ITA_DRT_Y8:
	{
		ISPParam ispParam = m_processor->getISPParam();
		ispParam.isDrtY8 = *bParam;
		m_processor->setISPParam(ispParam);
		break;
	}
	case ITA_TFF_STD:
	{
		ISPParam ispParam = m_processor->getISPParam();
		ispParam.tffStd = *(int *)param;
		m_processor->setISPParam(ispParam);
		break;
	}
	case ITA_RESTRAIN_RANGE:
	{
		ISPParam ispParam = m_processor->getISPParam();
		ispParam.restrainRange = *(int *)param;
		m_processor->setISPParam(ispParam);
		break;
	}
	case ITA_DISCARD_UPRATIO:
	{
		ISPParam ispParam = m_processor->getISPParam();
		ispParam.discardUpratio = *(int *)param;
		m_processor->setISPParam(ispParam);
		break;
	}
	case ITA_DISCARD_DOWNRATIO:
	{
		ISPParam ispParam = m_processor->getISPParam();
		ispParam.discardDownratio = *(int *)param;
		m_processor->setISPParam(ispParam);
		break;
	}
	case ITA_HEQ_PLAT_THRESH:
	{
		ISPParam ispParam = m_processor->getISPParam();
		ispParam.heqPlatThresh = *(int *)param;
		m_processor->setISPParam(ispParam);
		break;
	}
	case ITA_HEQ_RANGE_MAX:
	{
		ISPParam ispParam = m_processor->getISPParam();
		ispParam.heqRangeMax = *(int *)param;
		m_processor->setISPParam(ispParam);
		break;
	}
	case ITA_HEQ_MIDVALUE:
	{
		ISPParam ispParam = m_processor->getISPParam();
		ispParam.heqMidvalue = *(int *)param;
		m_processor->setISPParam(ispParam);
		break;
	}
	case ITA_IIE_ENHANCE_COEF:
	{
		ISPParam ispParam = m_processor->getISPParam();
		ispParam.iieEnhanceCoef = *(int *)param;
		m_processor->setISPParam(ispParam);
		break;
	}
	case ITA_IIE_GAUSS_STD:
	{
		ISPParam ispParam = m_processor->getISPParam();
		ispParam.iieGaussStd = *(int *)param;
		m_processor->setISPParam(ispParam);
		break;
	}
	case ITA_GMC_GAMMA:
	{
		ISPParam ispParam = m_processor->getISPParam();
		ispParam.gmcGamma = *(float *)param;
		m_processor->setISPParam(ispParam);
		break;
	}
	case ITA_GMC_TYPE:
	{
		ISPParam ispParam = m_processor->getISPParam();
		ispParam.gmcType = *(int *)param;
		m_processor->setISPParam(ispParam);
		break;
	}
	case ITA_MIX_THRLOW:
	{
		ISPParam ispParam = m_processor->getISPParam();
		ispParam.mixThrLow = *(int*)param;
		m_processor->setISPParam(ispParam);
		break;
	}
	case ITA_MIX_THRHIGH:
	{
		ISPParam ispParam = m_processor->getISPParam();
		ispParam.mixThrHigh = *(int*)param;
		m_processor->setISPParam(ispParam);
		break;
	}
	case ITA_LINEAR_RESTRAIN_RANGETHR:
	{
		ISPParam ispParam = m_processor->getISPParam();
		ispParam.linearResrainRange = *(int*)param;
		m_processor->setISPParam(ispParam);
		break;
	}
	case ITA_AUTO_RPBDPS:
	{
		ISPParam ispParam = m_processor->getISPParam();
		ispParam.isAutoRpBdPs = *(unsigned char*)param;
		m_processor->setISPParam(ispParam);
		break;
	}
	default:
		ret = ITA_UNSUPPORT_OPERATION;
		break;
	}
	return ret;
}

ITA_RESULT ITAWrapper::getParamInfo(ITAParamInfo *info)
{
	if (m_mtBase && m_mtBase->getMtParams())
	{
		m_mtBase->getParamInfo(info);
	}
	if (m_processor)
	{
		ISPParam ispParam = m_processor->getISPParam();
		m_processor->getParamInfo(info);
	}
	info->format = m_pixelFormat;
	info->palette = m_paletteIndex;
	info->scale = m_scale;
	info->rotate = m_rotate;
	info->flip = m_flip;
	info->range = m_range;
	if(m_isGearManual)
		info->fpaGear = m_fpaGear;
	info->isAutoGear = !m_isGearManual; //自动切档开关
	info->isReverse = m_reverse; //是否反向的开关
	info->isSubAvgB = m_subAvgB; //减去本底均值的开关
	info->isDebug = m_debug; //调试开关
	info->drtType = m_processor->getDimmingType(); //调光方式
	info->equalLineType = m_processor->getEqualLineType(); //等温线类型
	info->frameRate = m_userFps; //用户设置的帧率
	info->coldHotStatus = m_coldHotStatus;//冷热机状态
	return ITA_OK;
}

ITA_RESULT ITAWrapper::debugger(ITA_DEBUG_TYPE type, void * param)
{
	ITA_SWITCH *flag = (ITA_SWITCH *)param;
	ITA_RESULT ret = ITA_OK;
	if (!param && ITA_SNAPSHOT != type && ITA_START_RECORD != type && ITA_STOP_RECORD != type)
	{
		ret = ITA_NULL_PTR_ERR;
		m_logger->output(LOG_ERROR, "debugger ret=%d", ret);
		return ret;
	}
	switch (type)
	{
	case ITA_SET_DEBUGGING:			//设置是否开启调试。参数类型：ITA_SWITCH。ITA_DISABLE，关闭调试。ITA_ENABLE，开启调试。如果未设置日志路径，那么默认保存在程序同级目录下ITA.log。
		m_logger->output(LOG_INFO, "debugger op=%d %d", type, *flag);
		if (*flag<ITA_DISABLE || *flag>ITA_ENABLE)
		{
			return ITA_ARG_OUT_OF_RANGE;
		}
		if (ITA_DISABLE == *flag)
		{
			m_debug = false;
			if (m_productType < ITA_HANDHELD_TEC)
				m_mtBase->getMtParams()->bLogPrint = 0;
			else
				m_mtBase->getTECParams()->bLogPrint = 0;
		}
		else
		{
			m_debug = true;
			if (m_productType < ITA_HANDHELD_TEC)
				m_mtBase->getMtParams()->bLogPrint = 1;
			else
				m_mtBase->getTECParams()->bLogPrint = 1;
		}
		if(ITA_256_TIMO == m_productType && m_detector)
			m_detector->setDebug(m_debug);
		else if(ITA_120_TIMO == m_productType && m_timo120)
			m_timo120->setDebug(m_debug);
		break;	
	case ITA_DEBUGGING_PATH:		//设置日志保存的完整路径。注意要在ITA_Init之后调用。必须是已存在的目录，例如/home/app/debug.log。Windows例如"D:/Tools/debug.log"。长度不要超过128。参数类型：字符串，以 \0 结束。
	{
		ret = m_debugger.checkPath((const char *)param);
		if (ITA_OK == ret)
		{
			m_logger->setPath((const char *)param);
			m_debugger.control(type, param);
			m_logger->output(LOG_INFO, "debugger op=%d %s", type, (const char *)param);
		}
		else
		{
			m_logger->output(LOG_ERROR, "ITA_DEBUGGING_PATH ret=%d", ret);
		}
		break;
	}
	case ITA_DEBUGGING_MODE:		//设置日志输出模式。1，日志输出到文件，默认输出方式。2，日志输出到控制台。支持用户自定义输出日志，见ITARegistry。
	{
		int mode = *(int *)param;
		m_logger->output(LOG_INFO, "debugger op=%d %d", type, mode);
		if (mode < 1 || mode > 3)
		{
			return ITA_ARG_OUT_OF_RANGE;
		}
		m_logger->setMode((DBG_MODE_T)mode);
		break;
	}
	case ITA_SNAPSHOT:				//拍照保存当前帧图像，存储在程序的同级目录下。
	case ITA_START_RECORD:			//开始录X16/Y16数据。
	case ITA_STOP_RECORD:			//停止录X16/Y16数据。
		ret = m_processor->control(type, param);
		break;
	case ITA_MEMORY_USAGE:			//查询动态内存使用量。参数类型：整型。单位：Byte。
	{
		int sum = 0;
		for (int i = 0; i < ITA_NONE_MODULE; i++)
		{
			sum += g_memArray[i];
			if(m_debug)
				m_logger->output(LOG_INFO, "ITA_MEMORY_USAGE %d %d sum=%d", i, g_memArray[i], sum);
		}
		if(param)
			*(int *)param = sum;
		else
		{
			ret = ITA_NULL_PTR_ERR;
			m_logger->output(LOG_ERROR, "ITA_MEMORY_USAGE ret=%d", ret);
		}
		break;
	}
	case ITA_ISP_CONSUME:
		if (param)
			*(int *)param = m_ispTime;
		else
		{
			ret = ITA_NULL_PTR_ERR;
			m_logger->output(LOG_ERROR, "ITA_ISP_CONSUME ret=%d", ret);
		}
		break;
	default:
		break;
	}
	return ret;
}

ITA_RESULT ITAWrapper::addCustomPalette(int count, unsigned char * data, int dataLen)
{
	if (count <= 0 || !data)
		return ITA_ILLEGAL_PAPAM_ERR;
	return ITA_RESULT();
}

ITA_RESULT ITAWrapper::getColorImage(unsigned char *pY8, int w, int h, unsigned char* pPalette, int paletteLen, unsigned char* pImage, int imgLen)
{
	if (!pY8 || !pPalette || !pImage)
		return ITA_NULL_PTR_ERR;
	if (w <= 0 || h <= 0 || paletteLen <= 0 || paletteLen != 256 * 4 || imgLen <= 0 || imgLen != w * h * 3)
		return ITA_ILLEGAL_PAPAM_ERR;
	return m_processor->getColorImage(pY8, w, h, pPalette, paletteLen, pImage, imgLen);
}

ITA_RESULT ITAWrapper::calcY16ByTemp(float surfaceTemp, short * y16)
{
	if (m_subAvgB)
	{
		short yValue = 0;
		ITA_RESULT ret = m_mtBase->calcY16ByTemp(surfaceTemp, &yValue);
		*y16 = yValue + m_avgB;
		return ret;
	}
	else
		return m_mtBase->calcY16ByTemp(surfaceTemp, y16);
}

ITA_RESULT ITAWrapper::calcY16MatrixByTemp(float * tempMatrix, int w, int h, short * y16Matrix, int matrixSize)
{
	if (w <= 0 || h <= 0 || w * h > matrixSize)
	{
		return ITA_ARG_OUT_OF_RANGE;
	}
	if (m_subAvgB)
	{
		ITA_RESULT ret = m_mtBase->calcY16MatrixByTemp(tempMatrix, w, h, y16Matrix, matrixSize);
		for (int i = 0; i < matrixSize; i++)
		{
			*(y16Matrix + i) += m_avgB;
		}
		return ret;
	}
	else 
		return m_mtBase->calcY16MatrixByTemp(tempMatrix, w, h, y16Matrix, matrixSize);
}

/*
1.2.1.20220621 增加ITA_CalcY16MatrixByTemp。ZC08B项目要用。
1.2.2.20220705 支持标准参数行协议；计算温度矩阵支持减去本底；如果自动校温失败那么恢复原来的kf b值。
1.2.3.20220816 增加气体检测、高灵敏度、超分接口。
1.2.4.20220831 增加ReadCurveDataEx注册函数，支持带有免校温镜头的手持测温和机芯测温产品。增加ITA_ConvertY8接口。
1.2.5.20221012 增加黑体测温功能，用BLACKBODY_EDITION控制。增加ITA_GetColorImage接口的实现。
1.2.6.20221020 支持设置9个图像算法参数。实现接口ITA_CustomPalette。增加消防成像调光，用FIRE_DRC_EDITION控制。
1.2.7.20221101 对用户开放26条伪彩名称；ITA_GetColorImage源数据改为Y8。增加制冷探测器测温模型，用REFRIGER_EDITION控制。
1.2.8.20221206 增加接口ReadPackageDataEx支持不同镜头热插拔读取对应数据包；支持模组测温模型解析标准参数行，范围、镜头变化时自动切换范围；增加采K功能。
*/
ITA_RESULT ITAWrapper::getVersion(ITAVersionInfo * pVersion)
{
	ITA_RESULT result = ITA_OK;
	const char * p = "1.2.8.202302015";
	memcpy(pVersion->version, p, strlen(p));
	if (m_productType < ITA_HANDHELD_TEC)
	{
		pVersion->totalNumberOfGears = m_parser->getPH()->ucFocusNumber;
		if (m_parser->getPH()->ucFocusNumber <= 0 || m_parser->getPH()->ucFocusNumber > MAX_FOCUS_NUMBER)
			result = ITA_WRONG_PACKAGE_HEADER;
	}
	pVersion->totalNumberOfPalettes = m_processor->getPaletteNum();
	if (m_config.functionConfig && strlen(m_config.functionConfig->versionNumber)>0)
	{
		memcpy(pVersion->confVersion, m_config.functionConfig->versionNumber, strlen(m_config.functionConfig->versionNumber));
	}
	memcpy(pVersion->moduleCode, m_parser->getPH()->cModuleCode, 24);

	pVersion->productType = (m_productType < ITA_HANDHELD_TEC) ? 0 : 1;
	m_parser->getCorrectionFactors(pVersion->correctionFactors);
	return result;
}

ITA_RESULT ITAWrapper::updateParamLine(unsigned char * paramLine, int len)
{
	//由Parser解析
	short *pRef = 0;
	if (m_productType >= ITA_HANDHELD_TEC)
	{
		int nGear = m_mtBase->getTECParams()->nGear;
		int nRange = m_mtBase->getTECParams()->mtType;
		int nLensType = m_mtBase->getTECParams()->lensType;
		m_parser->parseTECLine(paramLine, len, m_mtBase->getTECParams());
		m_avgB = m_mtBase->getTECParams()->avgB;
		if(m_debug)
			m_logger->output(LOG_DEBUG, "TECLine %d %d %d %d %d %d %d", nGear, m_mtBase->getTECParams()->nGear, 
				nRange, m_mtBase->getTECParams()->mtType, nLensType, m_mtBase->getTECParams()->lensType, m_range);
		if (nGear != m_mtBase->getTECParams()->nGear
			|| (nRange != m_mtBase->getTECParams()->mtType && m_range!= m_mtBase->getTECParams()->mtType) //比较m_range避免与手动切范围重复
			|| nLensType != m_mtBase->getTECParams()->lensType || m_parser->isLensChanged())
		{	
			m_range = (ITA_RANGE)m_mtBase->getTECParams()->mtType;
			//档位变化时更新参数和曲线
			m_mtBase->calcFPAGear(&nGear);
			m_isChangeRange = true;
			if (nLensType != m_mtBase->getTECParams()->lensType && m_laserAF)
			{//更新曲线
				m_laserAF->updateLens((ITA_FIELD_ANGLE)m_mtBase->getTECParams()->lensType);
			}
			//m_parser->parseCurveData(m_cameraID, m_registry, (ITA_FIELD_ANGLE)m_mtBase->getTECParams()->lensType, m_range, m_mtBase->getTECParams()->nGear);
		}
		return ITA_OK;
	}
	if (ITA_FPGA_Y16 == m_mode && ITA_256_TIMO == m_productType)
	{
		pRef = (short *)paramLine;
		//如果通知FPGA采本底，这里要解析是否本底采集完成。
		if (1 == m_isFPGAB)
		{
			short b = *(pRef + 9);
			unsigned char a = (b >> 15) & 0x1;
			unsigned char c = b & 0xF;
			//串口发送指令后，下一帧不一定马上被标记1正在采集本底。这个是异步的。
			if (1 == a)
			{
				m_fpgaBing = true;
			}
			else if (m_fpgaBing)
			{
				//之前正在采集本底，现在采集完成。
				m_isFPGAB = 2;
				m_fpgaBing = false;
			}
			else
			{
				if (g_currentTime < m_FPGAStartTime)
				{
					//表明系统时间被改小，重置时间线。
					m_FPGAStartTime = 0;
				}
				if (g_currentTime - m_FPGAStartTime > 1500)
				{
					//如果fpga没收到串口指令，那么快门不能恢复。加个超时机制比较保险。timeout to go out
					m_isFPGAB = 2;
					m_fpgaBing = false;
					m_logger->output(LOG_WARN, "MODE_FPGA_Y16 Background collect Timeout:isFPGAB=%d a=%d c=%d b=%#x", m_isFPGAB, a, c, b);
				}
			}
			if (m_debug)
				m_logger->output(LOG_INFO, "MODE_FPGA_Y16 Background collect:isFPGAB=%d a=%d c=%d b=%#x", m_isFPGAB, a, c, b);
		}
	}
	else
	{
		//实时更新温传数据和快门状态。
		/*MCU普通快门时不要解析，MCU NUC时1、2、3状态都需要解析参数行快门状态。*/
		if (ITA_MCU_X16 == m_mode && 0 == m_nucStatus)
		{
			int nRange = m_mtBase->getMtParams()->mtType;
			m_parser->parseParamLine(paramLine, len, m_mtBase->getMtParams(), false);
			if (nRange != m_mtBase->getMtParams()->mtType && m_range != m_mtBase->getMtParams()->mtType)
			{
				m_range = (ITA_RANGE)m_mtBase->getMtParams()->mtType;
				m_isChangeRange = true;
			}
			if (m_parser->getCoffStatus())
			{
				m_mtBase->refreshCoreParaInfo();
			}
			if (m_debug)
			{
				m_logger->output(LOG_DEBUG, "ParamLine MCU %d %d", nRange, m_mtBase->getMtParams()->mtType);
			}
		}	
		else
		{
			int nRange = m_mtBase->getMtParams()->mtType;
			int nLensType = m_mtBase->getMtParams()->m_fieldType;
			m_parser->parseParamLine(paramLine, len, m_mtBase->getMtParams(), true);
			if (m_parser->getCoffStatus())
			{
				m_mtBase->refreshCoreParaInfo();
			}
			if (m_debug)
				m_logger->output(LOG_DEBUG, "ParamLine %d %d %d %d %d",
					nRange, m_mtBase->getMtParams()->mtType, nLensType, m_mtBase->getMtParams()->m_fieldType, m_range);
			if (nRange != m_mtBase->getMtParams()->mtType && m_range != m_mtBase->getMtParams()->mtType) //比较m_range避免与手动切范围重复
			{
				m_range = (ITA_RANGE)m_mtBase->getMtParams()->mtType;
				m_isChangeRange = true;
				//模组测温模型按测温范围读取数据包
			}
			if (nLensType != m_mtBase->getMtParams()->m_fieldType || m_parser->isLensChanged())
			{
				if (m_registry && m_registry->ReadPackageDataEx)
				{
					m_isChangeRange = true; //支持多种镜头热插拔才触发
				}
			}
		}
		if (ITA_120_TIMO == m_productType)
		{
			m_mtBase->getMtParams()->fRealTimeFpaTemp = m_mtBase->smoothFocusTemp(m_mtBase->getMtParams()->fRealTimeFpaTemp);
		}
		m_monitor->updateSensorTemp((int)(m_mtBase->getMtParams()->fRealTimeFpaTemp * 100));
		//更新本底均值
		if (ITA_Y16 == m_mode || ITA_FPGA_Y16 == m_mode)
		{
			m_avgB = m_parser->getAvgB();
			m_monitor->updateAVGB(m_avgB);
		}
		/*m_logger->output(LOG_DEBUG, "updateParamLine. m_mode=%d nucStatus=%d bNucShutterFlag=%d bOrdinaryShutter=%d",
			m_mode, m_nucStatus, m_wrapper->getMtParams()->bNucShutterFlag, m_wrapper->getMtParams()->bOrdinaryShutter);*/
		//Y16时更新快门状态。其它模式由ITA库控制快门状态，不需要参数行。
		if (ITA_Y16 == m_mode)
			m_mtBase->refreshShutterStatus(m_x16CenterValue,m_mode);
		else if (ITA_MCU_X16 == m_mode)
		{
			//此模式NUC由MCU完成，根据参数行状态采本底。普通快门仍然由ITA库完成。
			if (m_nucStatus == 1 && m_mtBase->getMtParams()->bNucShutterFlag == 1)  //记录NUC状态：0，初始空状态；1，开始做NUC；2，正在做NUC；3，NUC已完成。
			{
				//通知采本底
				m_isCollectBackground = true;
				m_backgroundCount = 0;
				m_skipCount = 0;
				m_nucStatus = 2;	//记录NUC状态：0，初始空状态；1，开始做NUC；2，正在做NUC；3，NUC已完成。
				m_mtBase->refreshShutterStatus(m_x16CenterValue,m_mode);
				//if (m_debug)
					m_logger->output(LOG_INFO, "CollectBackground begin. nucStatus=%d m_mode=%d", m_nucStatus, m_mode);
			}
			else if (m_nucStatus == 2 && m_mtBase->getMtParams()->bNucShutterFlag == 0)  //记录NUC状态：0，初始空状态；1，开始做NUC；2，正在做NUC；3，NUC已完成。
			{
				//nuc时mcu主动控制快门，不用ITA打开快门。ITA只用采本底。
				m_isCollectBackground = false;
				m_backgroundCount = 0;
				m_skipCount = 0;
				m_isFPGAB = 0;	//恢复就绪状态
				m_fpgaBing = false;
				if (m_firstNUCing)
					m_firstNUCing = false;
				m_nucStatus = 3;	//记录NUC状态：0，初始空状态；1，开始做NUC；2，正在做NUC；3，NUC已完成。
				m_mtBase->refreshShutterStatus(m_x16CenterValue, m_mode);
				//if (m_debug)
					m_logger->output(LOG_INFO, "CollectBackground end. nucStatus=%d m_mode=%d", m_nucStatus, m_mode);
			}
			else if (m_nucStatus == 3 && m_mtBase->getMtParams()->bOrdinaryShutter == 0)
			{
				//NUC已完成时仍要解析参数行直到快门标志位为0，此时不用再解析参数行快门状态。
				m_nucStatus = 0;
			}
			if (ITA_120_TIMO == m_productType && m_nucStartTime)
			{
				//CB360产品正在做nuc，检查快门状态，过2秒后置0。
				if (g_currentTime - m_nucStartTime >= 2000)
				{
					m_mtBase->getMtParams()->bOrdinaryShutter = 0;	//记录快门状态
					m_nucStartTime = 0;
					if (m_registry && m_registry->ShutterControl)
						m_registry->ShutterControl(m_cameraID, ITA_STATUS_CLOSE, m_registry->userParam);
					porting_thread_sleep(m_shutterCloseDelay);//MCU USB协议有延时，低于300出现快门留影。
					shutterNow();
					if (m_firstNUCing)
						m_firstNUCing = false;
				}
			}
		}
	}
	return ITA_OK;
}

ITA_RESULT ITAWrapper::measureBody(float surfaceTemp, float envirTemp, float * bodyTemp)
{
	return m_mtBase->calcBodyTemp(surfaceTemp, envirTemp, bodyTemp);
}

ITA_RESULT ITAWrapper::setDimmingType(ITA_DRT_TYPE type, ITADRTParam *param)
{
	return m_processor->setDimmingType(type, param);
}

ITA_RESULT ITAWrapper::setEqualLine(ITA_EQUAL_LINE_TYPE type, ITAEqualLineParam param)
{
	return m_processor->setEqualLineType(type, param);
}

ITA_RESULT ITAWrapper::correctBadPoints(ITA_CORRECT_TYPE type, void * param)
{
	ITA_RESULT res = ITA_OK;
	if (!param)
	{
		m_logger->output(LOG_ERROR, "correctBadPoints ret=%d", ITA_NULL_PTR_ERR);
		return ITA_NULL_PTR_ERR;
	}
	if (!m_badPointsArray)
	{
		m_badPointsArray = (ITA_POINT*)porting_calloc_mem(badPointTotalNumber, sizeof(ITA_POINT), ITA_WRAPPER_MODULE);
	}
	switch (type)
	{
	case ITA_TRY_CORRECT_POINT:	//先尝试标定坏点，实时图像确认标定成功后再添加。参数类型：ITA_POINT。
	{
		ITA_POINT *p = (ITA_POINT *)param;
		if (m_badPoint.x > -1)
		{
			//上一个坏点还未标定
			m_logger->output(LOG_WARN, "TRY_CORRECT_POINT is busying. x=%d y=%d", p->x, p->y);
			return ITA_INACTIVE_CALL;
		}
		if (p->x >= 0 && p->y >= 0 && p->x < m_width && p->y < m_height)
		{
			m_badPoint.x = p->x;
			m_badPoint.y = p->y;
			//下一帧生效
		}
		else
		{
			res = ITA_ILLEGAL_PAPAM_ERR;
		}
		m_logger->output(LOG_INFO, "TRY_CORRECT_POINT:x=%d y=%d ret=%d", p->x, p->y, res);
		break;
	}
	case ITA_CORRECT_POINT:		//添加坏点，可以多次添加。参数类型：ITA_POINT。
	{
		ITA_POINT *p = (ITA_POINT *)param;
		if (m_badPointsCount >= badPointTotalNumber)
		{
			badPointTotalNumber *= 2;
			ITA_POINT* temp = (ITA_POINT*)porting_calloc_mem(badPointTotalNumber, sizeof(ITA_POINT), ITA_WRAPPER_MODULE);
			memcpy(temp, m_badPointsArray, sizeof(ITA_POINT) * m_badPointsCount);
			porting_free_mem(m_badPointsArray);
			m_badPointsArray = temp;
			m_logger->output(LOG_INFO, "current badpointCount =%d,badPointTotalNumber =%d", m_badPointsCount, badPointTotalNumber);
		}
		if (p->x >= 0 && p->y >= 0 && p->x < m_width && p->y < m_height)
		{
			m_badPointsArray[m_badPointsCount++] = *p;
		}
		else
		{
			res = ITA_ILLEGAL_PAPAM_ERR;
		}
		m_logger->output(LOG_INFO, "CORRECT_POINT:i=%d x=%d y=%d ret=%d", m_badPointsCount, p->x, p->y, res);
		break;
	}
	case ITA_SAVE_POINTS:		//保存坏点。无参数。待坏点添加完成后，保存。
	{
		if (m_badPointsCount > 0)
		{
			ITA_RANGE *range = (ITA_RANGE *)param;
			res = m_parser->saveBadPoints(m_badPointsArray, m_badPointsCount, m_cameraID, m_registry, *range);
			m_logger->output(LOG_INFO, "SAVE_POINTS:range=%d count=%d ret=%d", *range, m_badPointsCount, res);
		}
		else
		{
			//上一个坏点还未标定
			m_logger->output(LOG_WARN, "SAVE_POINTS:No bad Point. count=%d", m_badPointsCount);
			return ITA_INACTIVE_CALL;
		}
		//恢复初始值
		m_badPointsCount = 0;
		m_badPoint.x = -1;
		m_badPoint.y = -1;
		break;
	}
	default:
		break;
	}
	return res;
}

/*3次标定黑体采集总时间最多：10+15+15+15 = 55秒。最少45秒。*/
ITA_RESULT ITAWrapper::startCollect(ITACollectParam collectParam, ITACollectFinish cb, void * param)
{
	if (!cb || (ITA_COLLECT_DISTANCE == collectParam.type && collectParam.param <= 0))
	{
		m_logger->output(LOG_ERROR, "startCollect ret=%d", ITA_ILLEGAL_PAPAM_ERR);
		return ITA_ILLEGAL_PAPAM_ERR;
	}
	if (1 == m_isAutoCollect || 2 == m_isAutoCollect)
	{
		m_logger->output(LOG_ERROR, "startCollect ret=%d", ITA_INACTIVE_CALL);
		return ITA_INACTIVE_CALL;
	}
	//采集时不能关闭图像处理
	if (m_closeImage)
	{
		m_logger->output(LOG_ERROR, "startCollect:Image processing cannot be turned off. ret=%d", ITA_INACTIVE_CALL);
		return ITA_INACTIVE_CALL;
	}
	m_collectType = collectParam.type;
	m_collectParam = collectParam.param;
	m_logger->output(LOG_INFO, "startCollect %d %f %f %d %d %d", m_collectType, m_collectParam, collectParam.blackTemp,
		m_shutterCollectInterval, m_collectInterval, m_collectNumberOfTimes);
	//先关闭快门和NUC策略
	ITA_SWITCH flag = ITA_DISABLE;
	baseControl(ITA_SHUTTER_POLICY, &flag);
	m_isAutoCollect = 1;	//记录采集状态：0，初始空状态；1，准备采集；2，正在采集；3，一次标定黑体采集完成。
	m_blackTempArray[m_collectCount] = collectParam.blackTemp;
	m_collectCBArray[m_collectCount] = cb;
	m_cbParamArray[m_collectCount] = param;
	m_collectTimes = 0;		//一次黑体采集周期内采集Y16计数，<= m_collectNumberOfTimes。
	return ITA_OK;
}

ITA_RESULT ITAWrapper::startCalibrate(float * kf, float * b, ITACalibrateResult *result)
{
	AutoCorrectTempPara autocorTpara;
	bool ret = false, isNear = false;
	ITA_RESULT rt = ITA_ERROR;

	if (!kf || !b || !result)
	{
		return ITA_NULL_PTR_ERR;
	}
	m_logger->output(LOG_INFO, "startCalibrate AutoCollect=%d collectCount=%d collectType=%d collectParam=%f", 
		m_isAutoCollect, m_collectCount, m_collectType, m_collectParam);
	memset(&autocorTpara, 0, sizeof(AutoCorrectTempPara));
	
	autocorTpara.nBlackBodyNumber = m_collectCount;
	for (int i = 0; i < m_collectCount; i++)
	{
		autocorTpara.TBlackBody[i] = m_blackTempArray[i];
		autocorTpara.Y16[i] = m_y16AvgArray[i];
		m_logger->output(LOG_INFO, "%d TBlack=%f y16=%d", i, autocorTpara.TBlackBody[i], autocorTpara.Y16[i]);
	}
	//记录kf b，如果失败那么恢复原来的值。自动校温算法中会修改kf b。
	float fkf0 = 10000, fb0 = 0, nkf0 = 10000, nb0 = 0;
	if (m_mtBase->getMtParams())
	{
		fkf0 = m_mtBase->getMtParams()->fFarKf;
		fb0 = m_mtBase->getMtParams()->fFarB;
		nkf0 = m_mtBase->getMtParams()->fNearKf;
		nb0 = m_mtBase->getMtParams()->fNearB;
	}
	switch (m_collectType)
	{
	case ITA_COLLECT_DISTANCE:
		if (m_collectParam < 1.0)
		{
			ret = m_mtBase->AutoCorrectTempNear(kf, b, &autocorTpara);
			isNear = true;
		}
		else
			ret = m_mtBase->AutoCorrectTempFar(kf, b, &autocorTpara);
		break;
	case ITA_COLLECT_ENVIRON:
		//在低温10℃、常温23℃环境温度下
		if (m_collectParam > 20)
		{
			ret = m_mtBase->AutoCorrectTempFar(kf, b, &autocorTpara);
		}
		else
		{
			ret = m_mtBase->AutoCorrectTempNear(kf, b, &autocorTpara);
			isNear = true;
		}
		break;
	default:
		break;
	}
	memcpy(result, &autocorTpara, sizeof(AutoCorrectTempPara));
	bool myRes = true;
	if (m_range == ITA_HUMAN_BODY)
	{
		for (int j = 0; j < m_collectCount; j++)
		{
			if (autocorTpara.ErrorCorrect[j]<-0.4 || autocorTpara.ErrorCorrect[j]>0.4)
			{
				m_logger->output(LOG_ERROR, "AutoCorrect %d %d %f", m_range, j, autocorTpara.ErrorCorrect[j]);
#ifndef CALIBRATE_EDITION
				myRes = false;
#endif
				break;
			}
		}
	}
	else
	{
		for (int j = 0; j < m_collectCount; j++)
		{
			if (autocorTpara.TBlackBody[j] <= 20.0)
			{
				//朗驰自动校温有时失败，低于设定的条件值。算法组建议稍微放宽条件。
				/*if (autocorTpara.ErrorCorrect[j]<autocorTpara.TBlackBody[j] * (-0.05)
					|| autocorTpara.ErrorCorrect[j]>autocorTpara.TBlackBody[j] * (0.05))*/
				if (autocorTpara.ErrorCorrect[j]<autocorTpara.TBlackBody[j] * (-0.06)
					|| autocorTpara.ErrorCorrect[j]>autocorTpara.TBlackBody[j] * (0.06))
				{
					m_logger->output(LOG_ERROR, "AutoCorrect %d %d %f %f", m_range, j, autocorTpara.ErrorCorrect[j], autocorTpara.TBlackBody[j]);
#ifndef CALIBRATE_EDITION
					myRes = false;
#endif
					break;
				}
			}
			else
			{
				if (autocorTpara.ErrorCorrect[j]<autocorTpara.TBlackBody[j] * (-0.02)
					|| autocorTpara.ErrorCorrect[j]>autocorTpara.TBlackBody[j] * (0.02))
				{
					m_logger->output(LOG_ERROR, "AutoCorrect %d %d %f %f", m_range, j, autocorTpara.ErrorCorrect[j], autocorTpara.TBlackBody[j]);
#ifndef CALIBRATE_EDITION
					myRes = false;
#endif
					break;
				}
			}
		}
	}

	if (ret && myRes)
	{
		m_logger->output(LOG_INFO, "startCalibrate success. kf=%f b=%f", *kf, *b);
		if (!m_registry || !m_registry->WriteCalibrateParam)
		{
			rt = ITA_HAL_UNREGISTER;
		}
		else
		{
			rt = ITA_OK;
			m_logger->output(LOG_INFO, "startCalibrate save kf=%f b=%f", *kf, *b);
			//保存参数
			ITA_CALIBRATE_TYPE type;
			if (isNear)
			{
				type = ITA_NEAR_KF_AUTO;
				m_registry->WriteCalibrateParam(m_cameraID, m_range, type, *kf, m_registry->userParam);
				type = ITA_NEAR_B_AUTO;
				m_registry->WriteCalibrateParam(m_cameraID, m_range, type, *b, m_registry->userParam);
				if (m_mtBase->getMtParams())
				{
					m_mtBase->getMtParams()->fNearKf = *kf;
					m_mtBase->getMtParams()->fNearB = *b;
				}
			}
			else
			{
				type = ITA_FAR_KF_AUTO;
				m_registry->WriteCalibrateParam(m_cameraID, m_range, type, *kf, m_registry->userParam);
				type = ITA_FAR_B_AUTO;
				m_registry->WriteCalibrateParam(m_cameraID, m_range, type, *b, m_registry->userParam);
				if (m_mtBase->getMtParams())
				{
					m_mtBase->getMtParams()->fFarKf = *kf;
					m_mtBase->getMtParams()->fFarB = *b;
				}
			}
		}
	}
	else
	{
		if (m_mtBase->getMtParams())
		{
			m_logger->output(LOG_INFO, "recovery1 nkf=%f nb=%f fkf=%f fb=%f",
				m_mtBase->getMtParams()->fNearKf, m_mtBase->getMtParams()->fNearB, m_mtBase->getMtParams()->fFarKf, m_mtBase->getMtParams()->fFarB);
			m_mtBase->getMtParams()->fFarKf = fkf0;
			m_mtBase->getMtParams()->fFarB = fb0;
			m_mtBase->getMtParams()->fNearKf = nkf0;
			m_mtBase->getMtParams()->fNearB = nb0;
			m_logger->output(LOG_INFO, "recovery2 nkf=%f nb=%f fkf=%f fb=%f",
				m_mtBase->getMtParams()->fNearKf, m_mtBase->getMtParams()->fNearB, m_mtBase->getMtParams()->fFarKf, m_mtBase->getMtParams()->fFarB);
		}
		m_logger->output(LOG_WARN, "startCalibrate failed. kf=%f b=%f", *kf, *b);
	}
	if (ITA_X16 == m_mode || ITA_MCU_X16 == m_mode)
	{
		//打开快门和NUC策略
		ITA_SWITCH flag = ITA_ENABLE;
		baseControl(ITA_SHUTTER_POLICY, &flag);
	}
	/*自动校温完成后恢复初始状态*/
	m_isAutoCollect = 0;	//记录采集状态：0，初始空状态；1，准备采集；2，正在采集；3，一次标定黑体采集完成。
	//m_shutterTimes = 0;		//一次黑体采集周期内快门计数，<= m_shutterNumberOfTimes。
	m_collectTimes = 0;		//一次黑体采集周期内采集Y16计数，<= m_collectNumberOfTimes。
	m_collectCount = 0;		//第几个黑体采集，一般3次，30 33 36.
	return rt;
}

ITA_RESULT ITAWrapper::calibrateByUser(ITA_RANGE range, ITA_CALIBRATE_TYPE type, float p)
{
	ITA_RESULT ret;
	if (!m_registry || !m_registry->WriteCalibrateParam)
		return ITA_HAL_UNREGISTER;
	ret = m_registry->WriteCalibrateParam(m_cameraID, range, type, p, m_registry->userParam);
	m_logger->output(LOG_INFO, "WCP:%d %d %f %d", range, type, p, m_range);
	if (ITA_OK == ret && range == m_range)
	{
		m_mtBase->setKFB(type, p);
	}
	return ret;
}

ITA_RESULT ITAWrapper::setTECInfo(ITATECInfo info)
{
	if (m_mtBase->getTECParams())
	{
		IMAGE_MEASURE_STRUCT *p = m_mtBase->getTECParams();
		p->bAmbientCorrection = info.bAmbientCorrection;
		p->bAtmosphereCorrection = info.bAtmosphereCorrection;
		p->bB1Correction = info.bB1Correction;
		p->bDistanceCorrection = info.bDistanceCorrection;
		p->bEmissCorrection = info.bEmissCorrection;
		p->bHumidityCorrection = info.bHumidityCorrection;
		p->bLensCorrection = info.bLensCorrection;
		p->bShutterCorrection = info.bShutterCorrection;
		p->bWindowTransmittanceCorrection = info.bWindowTransmittanceCorrection;
		p->fAmbient = info.fAmbient;
		p->fDistance = info.fDistance;
		p->fEmiss = info.fEmiss;
		p->fHumidity = info.fHumidity;
		p->fReflectT = info.fReflectT;
		p->fAtmosphereTemperature = info.fAtmosphereTemperature;
		p->fWindowTemperature = info.fWindowTemperature;
		p->fWindowTransmittance = info.fWindowTransmittance;
		//p->mtType = info.mtType;
		p->nAtmosphereTransmittance = info.nAtmosphereTransmittance;
		p->nB1 = info.nB1;
		p->nB1Offset = info.nB1Offset;
		p->nB2 = info.nB2;
		p->nDistance_a0 = info.nDistance_a0;
		p->nDistance_a1 = info.nDistance_a1;
		p->nDistance_a2 = info.nDistance_a2;
		p->nDistance_a3 = info.nDistance_a3;
		p->nDistance_a4 = info.nDistance_a4;
		p->nDistance_a5 = info.nDistance_a5;
		p->nDistance_a6 = info.nDistance_a6;
		p->nDistance_a7 = info.nDistance_a7;
		p->nDistance_a8 = info.nDistance_a8;
		//p->nGear = info.nGear;
		p->nK1 = info.nK1;
		p->nK2 = info.nK2;
		p->nK3 = info.nK3;
		p->nKF = info.nKF;
		p->nKFOffset = info.nKFOffset;
		p->sY16Offset = info.sY16Offset;
		return ITA_OK;
	}
	else
		return ITA_UNSUPPORT_OPERATION;
}

ITA_RESULT ITAWrapper::getTECInfo(ITATECInfo * pInfo)
{
	if (m_mtBase->getTECParams())
	{
		IMAGE_MEASURE_STRUCT *p = m_mtBase->getTECParams();
		pInfo->bAmbientCorrection = p->bAmbientCorrection;
		pInfo->bAtmosphereCorrection = p->bAtmosphereCorrection;
		pInfo->bB1Correction = p->bB1Correction;
		pInfo->bDistanceCorrection = p->bDistanceCorrection;
		pInfo->bEmissCorrection = p->bEmissCorrection;
		pInfo->bHumidityCorrection = p->bHumidityCorrection;
		pInfo->bLensCorrection = p->bLensCorrection;
		pInfo->bShutterCorrection = p->bShutterCorrection;
		pInfo->bWindowTransmittanceCorrection = p->bWindowTransmittanceCorrection;
		pInfo->fAmbient = p->fAmbient;
		pInfo->fDistance = p->fDistance;
		pInfo->fEmiss = p->fEmiss;
		pInfo->fHumidity = p->fHumidity;
		pInfo->fReflectT = p->fReflectT;
		pInfo->fAtmosphereTemperature = p->fAtmosphereTemperature;
		pInfo->fWindowTemperature = p->fWindowTemperature;
		pInfo->fWindowTransmittance = p->fWindowTransmittance;
		pInfo->mtType = p->mtType;
		pInfo->nAtmosphereTransmittance = p->nAtmosphereTransmittance;
		pInfo->nB1 = p->nB1;
		pInfo->nB1Offset = p->nB1Offset;
		pInfo->nB2 = p->nB2;
		pInfo->nDistance_a0 = p->nDistance_a0;
		pInfo->nDistance_a1 = p->nDistance_a1;
		pInfo->nDistance_a2 = p->nDistance_a2;
		pInfo->nDistance_a3 = p->nDistance_a3;
		pInfo->nDistance_a4 = p->nDistance_a4;
		pInfo->nDistance_a5 = p->nDistance_a5;
		pInfo->nDistance_a6 = p->nDistance_a6;
		pInfo->nDistance_a7 = p->nDistance_a7;
		pInfo->nDistance_a8 = p->nDistance_a8;
		pInfo->nGear = p->nGear;
		pInfo->nK1 = p->nK1;
		pInfo->nK2 = p->nK2;
		pInfo->nK3 = p->nK3;
		pInfo->nKF = p->nKF;
		pInfo->nKFOffset = p->nKFOffset;
		pInfo->sY16Offset = p->sY16Offset;
		pInfo->burnSwitch = m_parser->getBurnInfo().burnSwitch;
		pInfo->burnFlag = m_parser->getBurnInfo().burnFlag;
		pInfo->burnDelayTime = m_parser->getBurnInfo().burnDelayTime;
		pInfo->burnThreshold = m_parser->getBurnInfo().burnThreshold;
		return ITA_OK;
	}
	else
		return ITA_UNSUPPORT_OPERATION;
}

ITA_RESULT ITAWrapper::setMeasureInfo(ITAMeasureInfo info)
{
	if (m_mtBase->getMtParams())
	{
		MTWrapper* mtWrapper = (MTWrapper*)m_mtBase;
		return mtWrapper->setMeasureInfo(info);
	}
	else {
		return ITA_UNSUPPORT_OPERATION;
	}
}

ITA_RESULT ITAWrapper::getMeasureInfo(ITAMeasureInfo* info)
{
	if (m_mtBase->getMtParams())
	{
		MtParams* p = m_mtBase->getMtParams();
		info->bDistanceCompensate = p->bDistanceCompensate;
		info->bEmissCorrection = p->bEmissCorrection;
		info->bTransCorrection = p->bTransCorrection;
		info->bHumidityCorrection = p->bHumidityCorrection;
		info->fNearKf = p->fNearKf;
		info->fNearB = p->fNearB;
		info->fFarKf = p->fFarKf;
		info->fFarB = p->fFarB;
		info->fNearKf2 = p->fNearKf2;
		info->fNearB2 = p->fNearB2;
		info->fFarKf2 = p->fFarKf2;
		info->fFarB2 = p->fFarB2;
		info->fHighShutterCorrCoff = p->fHighShutterCorrCoff;
		info->fLowShutterCorrCoff = p->fLowShutterCorrCoff;
		info->fShutterCorrCoff = p->fShutterCorrCoff;
		info->fHighLensCorrK = p->fHighLensCorrK;
		info->fLowLensCorrK = p->fLowLensCorrK;
		info->fLensCorrK = p->fLensCorrK;
		info->fDistance = p->fDistance;
		info->fEmiss = p->fEmiss;
		info->fTransmit = p->fTransmit;
		info->fHumidity = p->fHumidity;
		info->fReflectTemp = p->fReflectTemp;
		info->fCorrectDistance = p->fCorrectDistance;
		return ITA_OK;
	}
	else {
		return ITA_UNSUPPORT_OPERATION;
	}
}

ITA_RESULT ITAWrapper::addInterceptor(PostHandle postHandle)
{
	return m_af->addInterceptor(postHandle);
}

ITA_RESULT ITAWrapper::processBiCubic(void* dst, void* src, int w, int h, int w_zoom, int h_zoom, ITA_DATA_TYPE dataType)
{
	return m_processor->processBiCubic(dst, src, w, h, w_zoom, h_zoom, dataType);
}

ITA_RESULT ITAWrapper::convertY8(const unsigned char* src, int srcW, int srcH, unsigned char* dst, int dstW, int dstH, int dateLen, ITA_Y8_DATA_MODE datamode)
{
	ITAImgInfo imageInfo;
	memset(&imageInfo, 0, sizeof(ITAImgInfo));
	m_processor->getImageInfo(m_rotate, m_pixelFormat, &imageInfo);
	if (imageInfo.imgW != dstW || imageInfo.imgH != dstH || imageInfo.imgDataLen != dateLen)
	{
		return ITA_ARG_OUT_OF_RANGE;
	}
	if (datamode > ITA_Y8_XINSHENG || datamode < ITA_Y8_NORMAL)
	{
		return ITA_ARG_OUT_OF_RANGE;
	}
	return m_processor->convertY8(src, srcW,srcH,dst,dstW,dstH,dateLen,datamode);
}

ITA_RESULT ITAWrapper::customPalette(ITA_PALETTE_OP_TYPE type, void* param)
{
	return m_processor->customPalette(type, param);
}

ITA_RESULT ITAWrapper::getScene(ITASceneParam *param)
{
	if (m_productType < ITA_HANDHELD_TEC)
	{
		param->mtParam = m_mtBase->getMtParams();
		param->paramLen = sizeof(MtParams);
		param->reservedData = m_parser->getReservedData(&param->reservedLen,m_subAvgB,m_avgB);
		if (m_debug)
		{
			m_logger->output(LOG_INFO, "getScene %f %f", m_mtBase->getMtParams()->fEmiss, m_mtBase->getMtParams()->fReflectTemp);
			std::string filePath = m_debugger.getPath();
			filePath.append("myGetScene.raw");
			HANDLE_T hd = porting_file_open(filePath.data(), "wb+");
			if (hd)
			{
				porting_file_write(hd, (unsigned char *)param->mtParam, param->paramLen);
				porting_file_close(hd);
			}
		}
	}
	else
	{
		param->mtParam = m_mtBase->getTECParams();
		param->paramLen = sizeof(IMAGE_MEASURE_STRUCT);
		param->reservedData = NULL;
		param->reservedLen = 0;
	}
	param->curveData = (unsigned char *)m_parser->getCurves();
	param->curveLen = m_parser->getCurvesLen() * 2;
	if (!param->curveData)
		return ITA_NO_PACKAGE;
	else
		return ITA_OK;
}

ITA_RESULT ITAWrapper::setScene(ITASceneParam param)
{
	m_parser->setCurves((short *)param.curveData, param.curveLen / 2);
	if (m_productType < ITA_HANDHELD_TEC && param.reservedData)
	{
		m_parser->setReservedData((unsigned char *)param.reservedData, param.reservedLen,&m_subAvgB,&m_avgB);
	}
	m_mtBase->setMtParams(param.mtParam, param.paramLen);
	/*初始化测温参数，创建CMeasureTempCore实例。*/
	m_mtBase->loadData(m_parser->getPH(), m_parser->getFPArray(), m_parser->getCurves());
	m_mtBase->refreshCoreParaInfo();
	return ITA_OK;
}

ITA_RESULT ITAWrapper::zoomY16(short * srcY16, int srcW, int srcH, short * dstY16, int dstW, int dstH, float times)
{
	return m_processor->zoomY16(srcY16, srcW, srcH, dstY16, dstW, dstH, times);
}

ITA_RESULT ITAWrapper::isChangeRange(short * pSrc, int width, int height, int range, float areaTh1, float areaTh2, int low2high, int high2low, int * isChange)
{
	if (m_mtBase && m_mtBase->getMtParams()->bOrdinaryShutter == 1)
	{
		*isChange = 0;
		return ITA_SHUTTER_CLOSING;
	}
	if (m_subAvgB)
	{
		//注意这里会修改Y16缓存数据
		int len = width*height;
		if (!m_y16TempMatrix)
		{
			m_logger->output(LOG_INFO, "isChangeRange %d %d %d", width, height, len);
			m_y16TempMatrix = (short *)porting_calloc_mem(len, sizeof(short), ITA_WRAPPER_MODULE);
			m_tempMatrixLen = len * sizeof(short);
		}
		else if (m_tempMatrixLen < len * sizeof(short))
		{
			porting_free_mem(m_y16TempMatrix);
			m_logger->output(LOG_INFO, "isChangeRange2 %d %d %d", width, height, len);
			m_y16TempMatrix = (short *)porting_calloc_mem(len, sizeof(short), ITA_WRAPPER_MODULE);
			m_tempMatrixLen = len * sizeof(short);
		}
		memcpy(m_y16TempMatrix, pSrc, len * 2);
		for (int i = 0; i < len; i++)
		{
			*(m_y16TempMatrix + i) -= m_avgB;
		}
		return m_mtBase->isChangeRange(m_y16TempMatrix, width, height, range, areaTh1, areaTh2, low2high, high2low, isChange);
	}
	else
		return m_mtBase->isChangeRange(pSrc, width, height, range, areaTh1, areaTh2, low2high, high2low, isChange);
}

ITA_RESULT ITAWrapper::isMultiChangeRange(short* pSrc, int width, int height, int range,
	float areaTh1, float areaTh2, int low2mid, int mid2low, int mid2high, int high2mid, int* isChange)
{
	if (m_mtBase && m_mtBase->getMtParams()->bOrdinaryShutter == 1)
	{
		*isChange = 0;
		return ITA_SHUTTER_CLOSING;
	}
	if (m_subAvgB)
	{
		//注意这里会修改Y16缓存数据
		int len = width * height;
		if (!m_y16TempMatrix)
		{
			m_logger->output(LOG_INFO, "isChangeRange %d %d %d", width, height, len);
			m_y16TempMatrix = (short*)porting_calloc_mem(len, sizeof(short), ITA_WRAPPER_MODULE);
			m_tempMatrixLen = len * sizeof(short);
		}
		else if (m_tempMatrixLen < len * sizeof(short))
		{
			porting_free_mem(m_y16TempMatrix);
			m_logger->output(LOG_INFO, "isChangeRange2 %d %d %d", width, height, len);
			m_y16TempMatrix = (short*)porting_calloc_mem(len, sizeof(short), ITA_WRAPPER_MODULE);
			m_tempMatrixLen = len * sizeof(short);
		}
		memcpy(m_y16TempMatrix, pSrc, len * 2);
		for (int i = 0; i < len; i++)
		{
			*(m_y16TempMatrix + i) -= m_avgB;
		}
		return m_mtBase->isMultiChangeRange(m_y16TempMatrix, width, height, range, areaTh1, areaTh2, low2mid, mid2low, mid2high, high2mid, isChange);
	}
	else
		return m_mtBase->isMultiChangeRange(pSrc, width, height, range, areaTh1, areaTh2, low2mid, mid2low, mid2high, high2mid, isChange);
}

int ITAWrapper::collectBackground(short * srcData, int srcSize)
{
	if (srcSize <= 0)
		return ITA_ILLEGAL_PAPAM_ERR;
	/*if (m_debug)
		m_logger->output(LOG_DEBUG, "collectBackground:mode=%d skipCount=%d backgroundCount=%d bDelayFrame=%d nucStatus=%d", m_mode, m_skipCount, m_backgroundCount, m_bDelayFrame, m_nucStatus);*/
	if (ITA_X16 == m_mode || ITA_MCU_X16 == m_mode)
	{
		m_skipCount++;
		//采本底前抛掉5帧，避免采到闭合前的图像。ITA_MCU_X16做nuc时不能抛，因为MCU已经抛掉，这里再抛后面不够采4帧。
		//ITA_MCU_X16打快门时，ITA库单步控制快门，所以仍要抛掉前面5帧。假设使用MCU打快门，那么也不用抛，根据参数行通知直接采本底。
		//如果不抛前面5帧，会引起本底采的不对，间隔时间太短弹开快门的命令也未有效。
		if (m_skipCount <= m_bDelayFrame && (ITA_X16 == m_mode || (ITA_MCU_X16 == m_mode && 0 == m_nucStatus)))
		{
			if (m_debug)
				m_logger->output(LOG_DEBUG, "collectBackground:Skip frame count = %d", m_skipCount);
			return 0;
		}
		if (m_backgroundCount == 0)
			memcpy(m_bArray, srcData, m_frameSize * 2);
		else if (m_backgroundCount <= 3)
		{
			//4帧本底分步求平均
			for (int i = 0; i < m_frameSize; i++)
			{
				*(m_bArray + i) = (*(m_bArray + i) + *(srcData + i)) / 2;
			}
			if (m_backgroundCount == 3)
			{
				//本底采集完成，更新本底。
				ITA_RESULT ret = m_processor->updateB(m_bArray, m_frameSize * 2, m_nucStatus);
				m_avgB = m_processor->getAvgB();
				m_monitor->updateAVGB(m_avgB);
				if (ITA_OK != ret)
				{
					//此次nuc失效，须最多再做2次。
					m_nucRepeatCount++;
				}
				else if (m_nucRepeatCount > 0)
				{
					//上次nuc失效，这次成功。
					m_logger->output(LOG_INFO, "ITA_NUC_REPEAT_CORRECTION successful. %d", m_nucRepeatCount);
					m_nucRepeatCount = 0;
				}
				if (m_debug)
					m_logger->output(LOG_DEBUG, "updateB count = %d, avgB=%d", m_backgroundCount + 1, m_avgB);
			}
		}
	}
	else if (ITA_FPGA_Y16 == m_mode && ITA_256_TIMO == m_productType)
	{
		//first to enter ,other time should not to be done,should keep last update b has been compeleted in outside function before to do next shutter or nuc
		if (m_isFPGAB == 0)
		{
			//SDK快门策略有效，需要通知FPGA采本底。就绪状态下通知一次。TBD
			/*if (GUIDEIR_ERR == UartCmdSend(CMD_UPDATE_BASE, 0, 0))
			{
				m_logger->output(LOG_ERROR, "UartCmdSend CMD_UPDATE_BASE failed!");
			}
			else*/
			{
				if (m_debug)
					m_logger->output(LOG_INFO, "UartCmdSend CMD_UPDATE_BASE success!");
				m_isFPGAB = 1;	//FPGA开始采集本底。采集完成后通过参数行通知ITA。
				m_fpgaBing = false;
				m_FPGAStartTime = g_currentTime;	//记录开始时间
			}
		}
	}
	m_backgroundCount++;
	if (ITA_MCU_X16 == m_mode && 2 == m_nucStatus) //记录NUC状态：0，初始空状态；1，开始做NUC；2，正在做NUC；3，NUC已完成。
	{
		//等待参数行快门状态通知。nuc时mcu主动控制快门，不用ITA打开快门。
		return 0;
	}
	//以下是对MCU产品普通快门的处理，以及裸模组nuc和普通快门的处理。
	//采完4帧本底后打开快门
	if ((ITA_FPGA_Y16 != m_mode && 4 == m_backgroundCount) || m_isFPGAB == 2)
	{
		if (m_debug)
			m_logger->output(LOG_DEBUG, "Shutter Open:shutterFrameNumber=%d %d CloseDelay=%d OpenDelay=%d", 
				m_shutterFrameNumber, m_backgroundCount + m_bDelayFrame, m_shutterCloseDelay, m_shutterOpenDelay);
		//ITA_FPGA_Y16模式也在4帧之后打开快门，与X16一致。
		if(m_mtBase->getMtParams())
			m_mtBase->getMtParams()->bNucShutterFlag = 0;	//记录快门状态
		//弹开快门。
		if (m_registry && m_registry->ShutterControl)
			m_registry->ShutterControl(m_cameraID, ITA_STATUS_OPEN, m_registry->userParam);
		porting_thread_sleep(m_shutterOpenDelay); //USB协议有延时，低于300会出现快门打不开的情况。
	}
	//普通快门时，ITA_MCU_X16 再抛4帧MCU USB缓存数据，避免闪屏。ITA_X16 m_shutterFrameNumber为9不用抛。
	if ((ITA_FPGA_Y16 != m_mode && m_backgroundCount >= m_shutterFrameNumber - m_bDelayFrame) || m_isFPGAB == 2)
	{
		if (m_debug)
			m_logger->output(LOG_DEBUG, "collectBackground:backgroundCount = %d", m_backgroundCount);
		if (m_mtBase->getMtParams())
			m_mtBase->getMtParams()->bOrdinaryShutter = 0;	//记录快门状态
		//更新快门状态
		m_mtBase->refreshShutterStatus(m_x16CenterValue, m_mode);
		m_isCollectBackground = false;
		m_backgroundCount = 0;
		m_skipCount = 0;
		m_isFPGAB = 0;	//恢复就绪状态
		m_fpgaBing = false;
		if (2 == m_nucStatus)
		{
			if (m_firstNUCing)
				m_firstNUCing = false;
			m_nucStatus = 0;// 3;	//记录NUC状态：0，初始空状态；1，开始做NUC；2，正在做NUC；3，NUC已完成。
			//重复NUC纠错机制。如果两种纠错同时设置，那么只用做ITA_NUC_REPEAT_CORRECTION。
			if (1 == m_nucRepeatCorrection)
			{
				if (m_nucRepeatCount >= 3)
				{
					//连续3次nuc均失效，纠错失败。
					m_logger->output(LOG_INFO, "ITA_NUC_REPEAT_CORRECTION failed. %d %d", m_nucRepeatCorrection, m_nucRepeatCount);
					m_nucRepeatCount = 0;
				}
				else if (m_nucRepeatCount > 0)
				{
					//须重复一次nuc
					m_logger->output(LOG_INFO, "Make another nuc after NUC is completed. %d %d", m_nucRepeatCorrection, m_nucRepeatCount);
					//m_monitor中的NUC时间间隔做了限制，默认最低间隔5秒。
					m_monitor->nucManual(1);
				}
			}
			//重复快门纠错机制。默认0关闭。1，用户打开nuc重复快门机制；2，即将重复快门；3，正在快门。快门动作完成后切到1状态。
			else if (1 == m_nucAnotherShutter)
			{
				m_logger->output(LOG_INFO, "Make another shutter after NUC is completed. %d", m_nucAnotherShutter);
				m_nucAnotherShutter = 2;
				//m_monitor中的快门时间间隔做了限制，默认最低间隔3秒。
				m_monitor->shutterManual(1);
			}
		}
		//重复快门纠错机制。默认0关闭。1，用户打开nuc重复快门机制；2，即将重复快门；3，正在快门。快门动作完成后切到1状态。
		if (3 == m_nucAnotherShutter)
		{
			if (m_debug)
				m_logger->output(LOG_DEBUG, "Make another shutter done. %d", m_nucAnotherShutter);
			m_nucAnotherShutter = 1;
		}
		if (m_registry && m_registry->NotifyEvent)
			m_registry->NotifyEvent(m_cameraID, ITA_SHUTTER_END, 0, m_registry->userParam);
		//保证计算的是真实isp time。避免弹开快门延时导致此次m_ispTime计算过大，导致帧率控制异常一直忽略帧。
		g_currentTime = porting_get_ms();
	}
	return 0;
}

int ITAWrapper::shutterNow()
{
	if (m_mtBase->getMtParams() && m_mtBase->getMtParams()->bOrdinaryShutter)
	{
		m_logger->output(LOG_WARN, "shutterNow:Shutter is busying.");
		return -1;
	}
	if (m_registry->NotifyEvent)
		m_registry->NotifyEvent(m_cameraID, ITA_SHUTTER_BEGIN, 0, m_registry->userParam);
	if(m_mtBase->getMtParams())
		m_mtBase->getMtParams()->bOrdinaryShutter = 1;	//记录快门状态
	//重复快门纠错机制。默认0关闭。1，用户打开nuc重复快门机制；2，即将重复快门；3，正在快门。快门动作完成后切到1状态。
	if (2 == m_nucAnotherShutter)
	{
		m_nucAnotherShutter = 3;
	}
	//if (m_debug)
		m_logger->output(LOG_INFO, "Close Shutter.");
	//快门闭合
	if (m_registry && m_registry->ShutterControl)
		m_registry->ShutterControl(m_cameraID, ITA_STATUS_CLOSE, m_registry->userParam);
	porting_thread_sleep(m_shutterCloseDelay);//MCU USB协议有延时，低于300出现快门留影。
	//通知采本底
	m_isCollectBackground = true;
	m_backgroundCount = 0;
	m_skipCount = 0;
	if(m_mtBase->getMtParams())
		m_mtBase->getMtParams()->bNucShutterFlag = 1;	//记录快门状态
	//更新快门状态
	m_mtBase->refreshShutterStatus(m_x16CenterValue,m_mode);
	return 0;
}

int ITAWrapper::nucNow()
{
	if (m_mtBase->getMtParams() && m_mtBase->getMtParams()->bOrdinaryShutter)
	{
		m_logger->output(LOG_WARN, "nucNow:Shutter is busying.");
		return -1;
	}
	if (ITA_MCU_X16 == m_mode && ITA_120_TIMO == m_productType)
	{
		//CB360产品bNucShutterFlag无效，只做nuc不采集本底。
		//快门状态置1，过2秒后置0。
		if(m_mtBase->getMtParams())
			m_mtBase->getMtParams()->bOrdinaryShutter = 1;	//记录快门状态
		m_nucStartTime = g_currentTime;
		if (m_registry && m_registry->ShutterControl)
			m_registry->ShutterControl(m_cameraID, ITA_STATUS_CLOSE, m_registry->userParam);
		porting_thread_sleep(m_shutterCloseDelay);//MCU USB协议有延时，低于300出现快门留影。
		//给MCU发命令做NUC
		if (m_registry && m_registry->DoNUC)
			m_registry->DoNUC(m_cameraID, m_registry->userParam);
		return 0;
	}
	if (ITA_X16 == m_mode || ITA_FPGA_Y16 == m_mode)
	{
		if (m_registry && m_registry->NotifyEvent)
			m_registry->NotifyEvent(m_cameraID, ITA_SHUTTER_BEGIN, 0, m_registry->userParam);
		m_nucStatus = 1;	//记录NUC状态：0，初始空状态；1，开始做NUC；2，正在做NUC；3，NUC已完成。
		if(m_mtBase->getMtParams())
			m_mtBase->getMtParams()->bOrdinaryShutter = 1;	//记录快门状态
		//if (m_debug)
			m_logger->output(LOG_INFO, "NUC Close Shutter.");
		//闭合快门
		if (m_registry && m_registry->ShutterControl)
			m_registry->ShutterControl(m_cameraID, ITA_STATUS_CLOSE, m_registry->userParam);
		//做NUC
		if (ITA_256_TIMO == m_productType && m_detector)
			m_detector->DoNUC();
		else if (ITA_120_TIMO == m_productType && m_timo120)
			m_timo120->DoNUC();
		porting_thread_sleep(200);
		//通知采本底
		m_isCollectBackground = true;
		m_backgroundCount = 0;
		m_skipCount = 0;
		if(m_mtBase->getMtParams())
			m_mtBase->getMtParams()->bNucShutterFlag = 1;	//记录快门状态
		m_nucStatus = 2;	//记录NUC状态：0，初始空状态；1，开始做NUC；2，正在做NUC；3，NUC已完成。
		//更新快门状态
		m_mtBase->refreshShutterStatus(m_x16CenterValue,m_mode);
	}
	else if (ITA_MCU_X16 == m_mode)
	{
		//给MCU发命令做NUC
		if (m_registry && m_registry->DoNUC)
			m_registry->DoNUC(m_cameraID, m_registry->userParam);
		m_nucStatus = 1;	//记录NUC状态：0，初始空状态；1，开始做NUC；2，正在做NUC；3，NUC已完成。
		//通过参数行状态采本底
		//if(m_debug)
			m_logger->output(LOG_INFO, "DoNUC nucStatus=%d m_mode=%d", m_nucStatus, m_mode);
	}
	return 0;
}

ITA_RESULT ITAWrapper::laserAFRegister(ITALaserAFRegistry *registry)
{
	ITA_FIELD_ANGLE lensType;
	//laserAF通常是手持产品的功能
	if (m_productType >= ITA_HANDHELD_TEC)
	{
		lensType = (ITA_FIELD_ANGLE)m_mtBase->getTECParams()->lensType;
	}
	else
	{
		lensType = (ITA_FIELD_ANGLE)m_parser->getPH()->lensType;
	}
	//注册一次，避免资源重新申请带来多线程问题。
	if (!m_laserAF)
	{
		m_laserAF = new CInfraredAF(m_cameraID, registry, lensType);
		return ITA_OK;
	}
	else
		return ITA_INACTIVE_CALL;
}

ITA_RESULT ITAWrapper::laserAF()
{
	if (m_laserAF)
	{
		m_laserAF->LDAF();
		return ITA_OK;
	}
	else
		return ITA_HAL_UNREGISTER;
}

ITA_RESULT ITAWrapper::contrastAF()
{
	if (m_laserAF)
	{
		m_laserAF->CAF();
		return ITA_OK;
	}
	else
		return ITA_HAL_UNREGISTER;
}

ITA_RESULT ITAWrapper::calibrateAF(ITA_AF_CALIBRATE type, int *diff)
{
	ITA_RESULT ret = ITA_OK;
	if (m_laserAF)
	{
		switch (type)
		{
		case ITA_AF_CALIBRATE_DIFF:
			if (m_laserAF->DiffCalibrateAF(diff)==0)
			{
				ret = ITA_CALIBRATION_ERROR;
			}
			break;
		case ITA_AF_CALIBRATE_LASER:
			if (m_laserAF->LaserCalibrateAF(diff)==0)
			{
				ret = ITA_CALIBRATION_ERROR;
			}
			break;
		default:
			ret = ITA_UNSUPPORT_OPERATION;
			break;
		}
		return ret;
	}
	else
		return ITA_HAL_UNREGISTER;
}

ITA_RESULT ITAWrapper::setInfoAF(ITA_AF_INFO type, void *param)
{
	ITA_RESULT ret = ITA_OK;
	if (m_laserAF)
	{
		switch (type)
		{
		case ITA_AF_BASE_INFO:
			m_laserAF->FocusBaseInfoSet((ITA_FOCUS_BASE_CFG_S *)param);
			break;
		case ITA_AF_LASER_INFO:
			m_laserAF->FocusLaserInfoset((ITA_FOCUS_LASER_CFG_S *)param);
			break;
		case ITA_AF_CONTRAST_INFO:
			m_laserAF->FocusContrastInfoset((ITA_FOCUS_CONTRAST_CFG_S *)param);
			break;
		default:
			ret = ITA_UNSUPPORT_OPERATION;
			break;
		}
		return ret;
	}
	else
		return ITA_HAL_UNREGISTER;
}

ITA_RESULT ITAWrapper::collectK(int gear, float blackTemp, ITACollectBaseFinish cb, void* userParam)
{
	ITA_RESULT ret = ITA_ERROR;
	if (gear < 0 && gear > m_parser->getPH()->ucFocusNumber && m_mode != ITA_X16) {
		return ITA_ARG_OUT_OF_RANGE;
	}
	if (!m_collectK)
	{
		m_collectK = new CollectK(m_width, m_height, m_cameraID);
		m_collectK->setLogger(m_logger);
	}
	ret = m_collectK->registryBaseInfo(gear, blackTemp, cb, userParam);
	if (ret != ITA_OK) {
		return ret;
	}
	//采集时不能关闭图像处理
	if (m_closeImage)
	{
		m_logger->output(LOG_ERROR, "collectK:Image processing cannot be turned off. ret=%d", ITA_INACTIVE_CALL);
		return ITA_INACTIVE_CALL;
	}
	//先关闭快门和NUC策略
	ITA_SWITCH flag = ITA_DISABLE;
	baseControl(ITA_SHUTTER_POLICY, &flag);
	return ITA_OK;
}

ITA_RESULT ITAWrapper::saveK(int gear, ITACollectBaseFinish cb, void* param)
{
	ITA_RESULT ret = ITA_ERROR;
	if (gear < 0 || gear >= m_parser->getPH()->ucFocusNumber || m_mode != ITA_X16)
	{
		return ITA_ARG_OUT_OF_RANGE;
	}
	ret = m_collectK->isSaveK(gear);
	if (ret == ITA_OK)
	{
		if (m_registry && m_registry->WritePackageData)
		{
			//利用高低温本底计算K矩阵。
			if (!m_gain_mat)
			{
				m_gain_mat = (unsigned short*)porting_calloc_mem(m_frameSize, sizeof(unsigned short), ITA_WRAPPER_MODULE);
			}
			memset(m_gain_mat, 0, m_frameSize * sizeof(unsigned short));
			//计算K矩阵
			m_processor->getGainMat(m_collectK->getHighBase(), m_collectK->getLowBase(), m_gain_mat, m_width, m_height);
			if (cb) {
				cb(m_cameraID,gear, m_gain_mat, m_frameSize * sizeof(unsigned short), param);
			}
			//计算得到的K矩阵和当前焦温档位的焦温写到用户的文件中去,并更新当前K矩阵。
			if (ITA_X16 == m_mode || ITA_MCU_X16 == m_mode)
			{
				m_processor->setCurrentK(m_gain_mat, m_frameSize * 2);
			}
			short fpaTemp = m_mtBase->getMtParams()->fRealTimeFpaTemp * 100;
			ret = m_parser->saveK(m_cameraID, m_gain_mat, m_registry, gear, fpaTemp);
			m_collectK->initCollectInit();
			m_logger->output(LOG_INFO, "saveK finish!");
			return ret;
		}
	}
	return ret;
}

ITA_RESULT ITAWrapper::potCoverRestrain(ITA_POTCOVER_OP_TYPE type, void* para)
{
	return m_processor->potCoverRestrain(type, para);
}

ITA_RESULT ITAWrapper::startReview(float temper, ITAReviewFinish cb, void* userParam)
{
	if (!m_startReview) {
		m_startReview = new StartReview(m_cameraID, m_logger);
	}
	return m_startReview->init(m_range,m_productType, temper, cb, userParam);
}

/* 自动测温 */
ITA_RESULT ITAWrapper::AutoCorrectTemp(float *pBlackBodyTemp, short *pY16, int arrLength)
{
	return m_mtBase->AutoCorrectTemp(pBlackBodyTemp, pY16, arrLength);
}

/* ToolBox */
ITA_RESULT ITAWrapper::AutoResponseRate(short lowY16, short highY16, int currentINT, int* pOutINT, short responseRate, short precision)
{
	if (!m_toolBox)
	{
		return ITA_NULL_PTR_ERR;
	}

	return m_toolBox->AutoResponseRate(lowY16, highY16, currentINT, pOutINT, responseRate, precision);
}

ITA_RESULT ITAWrapper::AutoGetKMatirx(short* pLowBase, short* pHighBase, int width, int height, unsigned short* pKMatrix)
{
	if (!m_toolBox)
	{
		return ITA_NULL_PTR_ERR;
	}

	return m_toolBox->AutoGetKMatirx(pLowBase, pHighBase, width, height, pKMatrix);
}

ITA_RESULT ITAWrapper::AutoCorrectBadPoint(unsigned short* pKMatrix, int width, int height, unsigned short thresh, int* pResult)
{
	if (!m_toolBox)
	{
		return ITA_NULL_PTR_ERR;
	}

	return m_toolBox->AutoCorrectBadPoint(pKMatrix, width, height, thresh, pResult);
}

ITA_RESULT ITAWrapper::AddBadPoint(unsigned short* pKMatrix, int x, int y, int width, int height)
{
	if (!m_toolBox)
	{
		return ITA_NULL_PTR_ERR;
	}

	return m_toolBox->AddBadPoint(pKMatrix, x, y, width, height);
}

ITA_RESULT ITAWrapper::AddBadLine(unsigned short* pKMatrix, int lineIndex, int width, int height, ITAAddBadLineMode mode)
{
	if (!m_toolBox)
	{
		return ITA_NULL_PTR_ERR;
	}

	return m_toolBox->AddBadLine(pKMatrix, lineIndex, width, height, mode);
}

ITA_RESULT ITAWrapper::IfConsistency(float centerPointTemp, float* pOtherPointTemp, int otherPointNum, ITAIfConsistencyType* pResult)
{
	if (!m_toolBox)
	{
		return ITA_NULL_PTR_ERR;
	}

	return m_toolBox->IfConsistency(centerPointTemp, pOtherPointTemp, otherPointNum, pResult);
}

ITA_RESULT ITAWrapper::GenerateCurve(double* pTempMatrix, short* pY16, int arrLength, int maxTemp, short* pCurve, int curveLength, int* pValidCurveLength)
{
	if (!m_toolBox)
	{
		return ITA_NULL_PTR_ERR;
	}

	return m_toolBox->GenerateCurve(pTempMatrix, pY16, arrLength, maxTemp, pCurve, curveLength, pValidCurveLength);
}

ITA_RESULT ITAWrapper::AutoGetDrift(ITADriftInfo* pDriftInfo, int driftInfoLength, int* pShutterDrift, int* pLenDrift)
{
	if (!m_toolBox)
	{
		return ITA_NULL_PTR_ERR;
	}

	return m_toolBox->AutoGetDrift(pDriftInfo, driftInfoLength, pShutterDrift, pLenDrift);
}

ITA_RESULT ITAWrapper::AutoMeasureDistanceCoff(float* pTempMatrix, double* pDistance, int numOfBlackBody, int numOfDistance,
	int standDistance, int* pDistanceCoff, ITADistanceCoffMode mode)
{
	if (!m_toolBox)
	{
		return ITA_NULL_PTR_ERR;
	}

	return m_toolBox->AutoMeasureDistanceCoff(pTempMatrix, pDistance, numOfBlackBody, numOfDistance, standDistance, pDistanceCoff, mode);
}

ITA_RESULT ITAWrapper::AutoMeasureAmbientCoff(double* pStandardAmbientTemp, float standardAmbientTemp,
	double* pCurrentAmbientTemp, float currentAmbientTemp, int tempMatrixSize, int* pK3, int* pB2)
{
	if (!m_toolBox)
	{
		return ITA_NULL_PTR_ERR;
	}

	return m_toolBox->AutoMeasureAmbientCoff(pStandardAmbientTemp, standardAmbientTemp, pCurrentAmbientTemp, currentAmbientTemp, tempMatrixSize, pK3, pB2);
}

ITA_RESULT ITAWrapper::AutoCheckTemperature(float* pStandTemp, float* pMeasureTemp, int arrLength, int* pNonConformTempIndex, int* pNonConformTempNum)
{
	if (!m_toolBox)
	{
		return ITA_NULL_PTR_ERR;
	}

	return m_toolBox->AutoCheckTemperature(pStandTemp, pMeasureTemp, arrLength, pNonConformTempIndex, pNonConformTempNum);
}

ITA_RESULT ITAWrapper::checkMeasureTempPreCondition()
{
	ITA_RESULT ret = ITA_OK;

	//打快门过程中不能测温。
	/*if (m_mtBase->getMtParams() && 1 == m_mtBase->getMtParams()->bOrdinaryShutter)
	{
		return ITA_INACTIVE_CALL;
	}*/
	//TEC产品快门过程中允许拍照测温。视辉客户TEC产品在打快门过程中不允许测温。
#ifdef SHIHUI_EDITION
	if (m_mtBase->getTECParams() && 1 == m_mtBase->getTECParams()->bOrdinaryShutter)
	{
		return ITA_INACTIVE_CALL;
	}
#endif
	//风险提示：客户测温时可能是多线程调用，测温和切档都会用到曲线。如果在切档过程中正在更新曲线，此时调用测温有冲突。
	//如果正在切档就等待，200ms超时。通常最多等待一帧的时间几十毫秒。
	int count = 0;
	// @FIXME: unsafe
	while ((m_isChangeRange || m_isChangeGear) && count < 200)
	{
		porting_thread_sleep(1);
		count++;
		m_logger->output(LOG_INFO, "measureTempPoint is waiting for temperature range switching to complete. isChangeGear=%d, isChangeRange=%d, count=%d", m_isChangeGear, m_isChangeRange, count);
	}
	return ITA_OK;
}

ITA_RESULT ITAWrapper::AnalysisAdd(ITAShape* shape, int size)
{
	return m_mtBase->AnalysisAdd(shape, size);
}



ITA_RESULT ITAWrapper::AnalysisClear()
{
	return m_mtBase->AnalysisClear();
}

ITA_RESULT ITAWrapper::AnalysisTemp(short* y16Data, int width, int height, ITAAnalysisTemp* analysisTemp, int size)
{
	ITA_RESULT ret = checkMeasureTempPreCondition();
	if (ITA_OK != ret)
	{
		return ret;
	}
	if (width != m_width || height != m_height)
	{
		return ITA_ARG_OUT_OF_RANGE;
	}

	return m_mtBase->AnalysisTemp(y16Data, m_width, m_height, analysisTemp, size, (m_subAvgB ? m_avgB : 0));
}

ITA_RESULT ITAWrapper::AnalysisY16(short* y16Data, int width, int height, ITAAnalysisY16Info* analysisY16Info, int size)
{
	if (width != m_width || height != m_height)
	{
		return ITA_ARG_OUT_OF_RANGE;
	}
	return m_mtBase->AnalysisY16(y16Data, width, height, analysisY16Info, size);
}

ITA_RESULT ITAWrapper::AnalysisGetRangeMask(ITARangeMask* rangeMask, int size)
{
	return m_mtBase->GetRangeMask(rangeMask,size);
}
#endif // ULTIMATE_EDITION

