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
	//Ĭ���޷�����256ģ����Ҫ������
	m_reverse = false;
	if(120 == width)
		m_productType = ITA_120_TIMO;
	else
	{
		m_productType = ITA_256_TIMO;	//�������ļ���ȡ
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
	//�ظ����ž�����ơ�Ĭ��0�رա�1���û���nuc�ظ����Ż��ƣ�2�������ظ����ţ�3�����ڿ��š����Ŷ�����ɺ��е�1״̬��
	m_nucAnotherShutter = 0;
	m_nucRepeatCorrection = 0; //������־���ͬʱ���ã���ôֻ����ITA_NUC_REPEAT_CORRECTION��
	m_nucRepeatCount = 0;
	m_lastSensorTime = 0;
	m_loop = false;
	m_frameSize = m_width*m_height;
	m_fpaGear = -1;	//��ʼ����Чֵ
	m_isGearManual = false;
	m_gearManual = -1;	//�ֶ���λ���뵱ǰ��λm_fpaGear�Ƚϣ����Ⱦ��л���
	m_isChangeRange = false;
	m_isChangeGear = false;
	m_subAvgB = false; //�����Ʒ����о��ASIC�㷨���쵼��Y16��ȥ���׾�ֵ֮����ܲ��¡�Ĭ�ϲ��ü���
	m_y16TempMatrix = NULL;
	m_tempMatrixLen = 0;
	m_avgB = 0;
	g_currentTime = porting_get_ms();
	m_logger = new GuideLog();
	m_monitor = new Monitor(m_productType);
	m_monitor->setLogger(m_logger);
	//Ĭ�ϴ򿪿��ź�NUC���ԣ����û�������Ҫ�����رա�
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
			//ASIC Y16ģʽ�¿��Թرտ��ź�NUC���ԣ�ITA��������ʱ��һ��NUC��
			m_monitor->closeShutterPolicy(true);
			m_firstNUCing = false;
		}
	}
	//��ģ��һ�ο���Լ360+170ms��Ĭ�϶���ǰ5֡�ٲɼ�4֡���׹�9֡��
	m_shutterFrameNumber = 9; //�û�ʹ��ITA_SINGLE_STEP_TIME���á�
	m_bDelayFrame = 5; //���������Ӱ���û�ʹ��ITA_B_DELAY���á�
	m_shutterCloseDelay = 100; //��һ����ʱ�����û�������ITA_STEP_DELAY_TIME���á�
	m_shutterOpenDelay = 70;
	if (ITA_MCU_X16 == mode)
	{
		//MCUһ�ο���ʱ��Լ520+800ms��USBЭ������ʱ�ͻ������ݡ�
		m_shutterFrameNumber = 13; //���ײɼ���򿪿�������4֡MCU USB�������ݣ�����������
		m_shutterCloseDelay = 400; //MCU USBЭ������ʱ������400���ֿ�����Ӱ��
		m_shutterOpenDelay = 400; //USBЭ������ʱ������400����ֿ��Ŵ򲻿��������
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
	//���������ó�K
	if (ITA_X16 == m_mode || ITA_MCU_X16 == m_mode)
	{
		m_processor->setCurrentK(m_parser->getCurrentK(), m_frameSize * 2);
	}
	//����ͼ��Ĭ�ϲ�����
	m_processor->setPaletteIndex(ITA_WHITE_HEAT);
	m_processor->setDimmingType(ITA_DRT_LINEAR, NULL);
	m_processor->setContrast(255);
	m_processor->setBrightness(70);

	m_debug = false;
	m_closeImage = false;	//Ĭ�ϴ�ͼ�������û�������Ҫ�����رա�
	m_imgDst = NULL;
	m_y16Data = NULL;
	m_reCalcSize = false;
	/*�Զ�У�¹��ܣ�ģ�����Ʒ�㷨����ʹ��ZX03CУ�·�����4���������Ը�����Ŀ��������ֵ��*/
	m_isAutoCollect = 0;	//��¼�ɼ�״̬��0����ʼ��״̬��1��׼���ɼ���2�����ڲɼ���3��һ�α궨����ɼ���ɡ�
	m_shutterCollectInterval = 10;	//�ϴο���֮���������뿪ʼ����Ųɼ�
	m_collectInterval = 1;	//�������βɼ����ʱ��
	m_collectNumberOfTimes = 15;	//һ�ο���֮����Ҫ�ɼ��Ĵ���
	//m_shutterNumberOfTimes = 1;	//ÿ������ɼ������Ҫ�Ŀ��Ŵ���
	//m_shutterTimes = 0;		//һ�κ���ɼ������ڿ��ż�����<= m_shutterNumberOfTimes��
	m_collectTimes = 0;		//һ�κ���ɼ������ڲɼ�Y16������<= m_collectNumberOfTimes��
	m_collectCount = 0;		//�ڼ�������ɼ���һ��3�Σ�30 33 36.
	m_badPointsCount = 0;
	badPointTotalNumber = 30;
	m_badPointsArray = NULL; 
	m_badPoint.x = -1;
	m_badPoint.y = -1;
	m_maxFrameRate = 25; //̽�������õ�֡��
	m_fps = m_maxFrameRate;
	m_ispTime = 15; //����ƽ̨����һ֡15ms
	m_lastIspTime = 15;
	m_userIspTime = 0; //�û����õ�ƽ��֡����ʱ�䡣��Ϊʵʱ�����֡ʱ��ܲ��ȶ���Ӱ��ʵ��֡�ʡ�
	/*����һ�ַ��������������û�ָ��֡�ʺ�������֡������֡�ʡ���֡���ʱ���޷�����֡�ʣ�ʱ���޷���׼���ơ�*/
	m_userFps = 0; //�û����õ�֡��
	m_nucStartTime = 0;
	m_isCalcCentral = false; //����������
	m_isAF = false; //�û��Ƿ�ʼʹ��AF������Ǿ���֡��Y16��
	/*�û����õ�̽��������*/
	m_validMode = 0;//0��̽����VSYNC��HSYNC����Ч��1��̽����VSYNC��HSYNC����Ч��Ĭ��Ϊ0��
	m_dataMode = 1;//0���ȷ�16λ���ݵĸ�8bits��1���ȷ�16λ���ݵĵ�8bits��Ĭ��Ϊ1��
	m_frameFrequency = 25;//̽����֡Ƶ����λfps��Ĭ��25��
	m_builtInFlash = 0;  //1��120 90��ģ��P25Q����Flash��
	memset(&guoGaiPara, 0, sizeof(stDGGTPara));
	m_gain_mat = NULL;
	m_collectK = NULL;
	m_startReview = NULL;
	m_coldHotDelay = 3600;
	m_coldHotStatus = 1;
	m_asicStatus = 1;
#ifdef SOURCE_COPY
	m_x16Data = NULL;	//Դ���ݵĿ�����ԭ���ϲ���Ҫ��
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
	//doISP�ӿڷ��غ���������������ͷ�m_monitor��doISP��ʹ��ʱ����������������MCU��������ʵ�֡�
	while (m_loop)
	{
		porting_thread_sleep(1);
		m_logger->output(LOG_INFO, "Waiting to end a process.");
	}
	m_logger->output(LOG_INFO, "End a process.");
	//�ȱ���ػ���Ϣ
	m_parser->saveColdOrHot(m_cameraID, m_registry, m_mtBase->getMtParams());
	//������m_monitor�������Զ�����������
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
	//ʣ��1536B porting_calloc_mem��������ռ�õ��ڴ�
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
	//�Ƚ�����Ʒ���ͣ������ݰ�����ʽ��ͬ���ͳ�ʼ��̽���������Ʒ������ء�
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
					/*���²�Ʒ�ͺ�*/
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
				//���ģ���Ƿ�����flash
				if (m_config.functionConfig->internalFlashS == 1)
				{
					m_logger->output(LOG_INFO, "internalFlash=%d", m_config.functionConfig->internalFlash);
					m_builtInFlash = m_config.functionConfig->internalFlash;  //1��120 90��ģ��P25Q����Flash��
					m_parser->setBuiltInFlash(m_builtInFlash, m_registry, m_cameraID);
				}
				if (m_config.functionConfig->isSubAvgBS)
				{
					if (!m_config.functionConfig->isSubAvgB)
						m_subAvgB = false;
					else
						m_subAvgB = true;
				}
				//����У�²���
				if (m_config.functionConfig->collectIntervalS)
					m_collectInterval = m_config.functionConfig->collectInterval;
				if (m_config.functionConfig->shutterCollectIntervalS)
					m_shutterCollectInterval = m_config.functionConfig->shutterCollectInterval;
				if (m_config.functionConfig->collectNumberOfTimesS)
					m_collectNumberOfTimes = m_config.functionConfig->collectNumberOfTimes;
				//���¿��ź�NUC���Բ���
				if (m_config.functionConfig->shutterTempUpperS)
					m_monitor->setShutterTempUpper(m_config.functionConfig->shutterTempUpper);
				if (m_config.functionConfig->nucTempUpperS)
					m_monitor->setNucTempUpper(m_config.functionConfig->nucTempUpper);
				if (m_config.functionConfig->shutterIntervalBottomS)
					m_monitor->setShutterIntervalBottom(m_config.functionConfig->shutterIntervalBottom);
				if (m_config.functionConfig->nucIntervalBottomS)
					m_monitor->setNucIntervalBottom(m_config.functionConfig->nucIntervalBottom);
				//�����Զ����Ų��Բ���
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
				//�������֡��
				if (m_config.functionConfig->maxFramerateS)
					m_maxFrameRate = m_config.functionConfig->maxFramerate;
				m_logger->output(LOG_INFO, "ReadConfig OK. product=%d", m_productType);
			}
		}
		else
		{
			m_logger->output(LOG_WARN, "ReadConfig failed. product=%d %d %d %d", m_productType, ret, len, m_cameraID);
		}
		/*����֮���ͷ��ڴ�*/
		m_parser->releaseConfBuf();
	}
	if (!flag)
	{
		//��������ļ���û�о�����һ��Ĭ�ϵĲ���������Ĭ�ϵ��Զ����Ų��Բ������¿�������
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
	/*�����ݰ�������ע�ắ��������ΪgetVersionҪ��ȡ��Ϣ���������prepare����ô�������û�����getVersion��ʱ����*/
	if(!m_builtInFlash)
	{
		ITA_FIELD_ANGLE initLenType = ITA_ANGLE_25;
		if (m_productType < ITA_HANDHELD_TEC)
		{
			/*��׿��Ŀ��ͷΪ0�Ĳ����������Ϲ淶����˳�ʼ��������ͷ����Ϊ1����ʵ������*/
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
		/*��ʼ�����²���������CMeasureTempCoreʵ����*/
		m_mtBase->loadData(m_parser->getPH(), m_parser->getFPArray(), m_parser->getCurves());
		if (m_productType < ITA_HANDHELD_TEC)
			m_monitor->setFPAArray(m_parser->getFPArray(), m_parser->getPH()->ucFocusNumber, m_parser->getPH()->ucRes);
		/*�л����·�Χ���üĴ��������ﲻ��������û��configureDetector��*/
		/*if (ITA_256_TIMO == m_productType && m_detector)
			m_detector->ChangeGears(m_range);
		else if (ITA_120_TIMO == m_productType && m_timo120)
			m_timo120->ChangeGears(m_range, m_parser->getPH()->ucGain, m_parser->getPH()->ucInt, m_parser->getPH()->ucRes);*/
	}
	//�������ݰ�֮��������������Ϣ����Ϊ��ͷ���������ݰ��С�
	m_mtBase->loadConfig(&m_config, m_range, (ITA_FIELD_ANGLE)m_parser->getPH()->lensType);
	m_processor->loadConfig(&m_config, m_range, (ITA_FIELD_ANGLE)m_parser->getPH()->lensType);
	//X16ģʽ��Ҫ��ȡУ�²������л����·�ΧʱҪ���¡�Y16���ݲ������в�һ����У�²���������Ҫ��ȡ���û����о����Ƿ�ע�������������
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
	//����Ӧ��λ���ݰ�
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
	case ITA_DETECTOR_VALID_MODE:       //0��̽����VSYNC��HSYNC����Ч��1��̽����VSYNC��HSYNC����Ч���������ͣ����͡�
		if (flag < 0 || flag > 1)
		{
			return ITA_ARG_OUT_OF_RANGE;
		}
		m_validMode = (unsigned char)flag;
		break;
	case ITA_DETECTOR_DATA_MODE:        //0���ȷ�16λ���ݵĸ�8bits��1���ȷ�16λ���ݵĵ�8bits���������ͣ����͡�
		if (flag < 0 || flag > 1)
		{
			return ITA_ARG_OUT_OF_RANGE;
		}
		m_dataMode = (unsigned char)flag;
		break;
	case ITA_DETECTOR_FRAME_FREQUENCY:  //����̽����֡Ƶ����λfps��Ĭ��25���������ͣ����͡�
		if (flag < 1 || flag > 30)	//ģ�����֡Ƶ30
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
		/*�л����·�Χ���üĴ���*/
		m_detector->ChangeGears(m_range);
		return ret;
	}
	else if (ITA_120_TIMO == m_productType)
	{
		if (!m_timo120)
			return ITA_HAL_UNREGISTER;
		if (clock > 10000000) //120ģ��ʱ������10M
			return ITA_ARG_OUT_OF_RANGE;
		ret = m_timo120->DetectorInit(clock, m_validMode, m_dataMode, m_frameFrequency, m_parser->getDP());
		/*�л����·�Χ���üĴ���*/
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
	//srcSize���ܴ�ImageProcessor::process memcpy�õ����������ʵ��֡��С�ᱨ��heap corruption detected��
	if (srcSize != m_frameSize * 2)
		return ITA_ILLEGAL_PAPAM_ERR;
	m_loop = true;
	g_currentTime = porting_get_ms();
	mySrc = (short *)srcData;
	m_x16CenterValue = *(mySrc + m_frameSize / 2 - m_width / 2);
	//ȷ������״̬
	if (1 == m_mtBase->getTECParams()->bOrdinaryShutter)
		result = ITA_SHUTTER_CLOSING;
	else if (!pResult)
		result = ITA_NULL_PTR_ERR;
	//ÿ���ȡһ���´�
	if (g_currentTime < m_lastSensorTime)
	{
		//����ϵͳʱ�䱻��С��ʱ�������㡣
		m_lastSensorTime = 0;
	}
	if (g_currentTime - m_lastSensorTime > 999 || !m_lastSensorTime)
	{
		m_lastSensorTime = g_currentTime;
		m_fps = rateCount;	//ʵ��֡�ʼ���
		rateCount = 0;
		if (m_debug && m_userFps && m_userFps != m_maxFrameRate)
		{
			m_logger->output(LOG_DEBUG, "isp=%d lisp=%d rateCount=%d userFps=%d", m_ispTime, m_lastIspTime, rateCount, m_userFps);
		}
		/*�����Զ����Ų��Բ������¿�������*/
		m_monitor->updatePeriod(g_currentTime);
	}
	//����֡��
	if (m_userFps && m_userFps != m_maxFrameRate)
	{
		unsigned int intervaltemp = 1000 / m_userFps;
#ifdef CONTINUE_FRAME_DROP
		if (rateCount >= m_userFps)
		{
			m_loop = false;
			return ITA_SKIP_FRAME; //��֡������
}
#else
		//����ʹ���û����õ�ƽ��֡����ʱ�䡣��Ϊʵʱ�����֡ʱ��ܲ��ȶ���Ӱ��ʵ��֡�ʡ�
		if (m_userIspTime)
			consumeTime = m_userIspTime;
		else
			consumeTime = (m_lastIspTime + m_ispTime) / 2;
		if ((intervaltemp > consumeTime) && (g_currentTime - rateLastTime < intervaltemp - consumeTime))
		{
			m_loop = false;
			return ITA_SKIP_FRAME; //��֡������
		}
		else
		{
			if (m_debug)
				m_logger->output(LOG_DEBUG, "Interval between two frames is %d isp=%d lisp=%d %d count=%d inte=%d cons=%d",
					g_currentTime - rateLastTime, m_ispTime, m_lastIspTime, intervaltemp - (m_lastIspTime + m_ispTime) / 2, rateCount, intervaltemp, consumeTime);
			rateLastTime = g_currentTime;	//��¼��֡ʱ��
		}
#endif // CONTINUE_FRAME_DROP
	}
	rateCount++;
	if (!m_closeImage && pResult)
	{
		if (m_reCalcSize)
		{
			//���ڷ��ظ��û���pResultҪ������ȷ��img���ȣ����Ըĳ�ÿ�α仯�����������ڴ档
			if (m_imgDst)
			{
				porting_free_mem(m_imgDst);
				m_imgDst = NULL;
				pResult->imgDst = NULL;
			}
			//ĿǰY16û�����ſ��Բ����·��䣬�����������Ҫ��
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
			{	//��һ��ʱ���䣬���治�ͷţ��˳�ʱ���ͷš�
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
			/*������ж�Ӱ��Ч�ʣ���ʽ�汾���Զ����ű���������*/
			result = ITA_OUT_OF_MEMORY;
		}
		else
		{
			//ͼ����
			if (!y8Data)
				m_processor->process(mySrc, srcSize, pResult);
			else
				m_processor->copyY8(mySrc, srcSize, pResult, y8Data, y8Size);
		}
	}
	m_lastIspTime = m_ispTime;
	m_ispTime = porting_get_ms() - g_currentTime;
	//���Ŵ򿪵�״̬���ж��Ƿ�У�²ɼ�����λ���Զ����Ų��Թز���ʱ��ʱ��ɵ��պ�ʱ��Y16�쳣��
	//��¼�ɼ�״̬��0����ʼ��״̬��1��׼���ɼ���2�����ڲɼ���3��һ�α궨����ɼ���ɡ�
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
				/*�������10s��ôֱ�Ӵ���Ųɼ�������ȴ���*/
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
			/*ÿ��1s�ɼ�һ��Y16����ɼ�15�Σ�Ȼ����15�ε�ƽ��ֵ��*/
			if ((g_currentTime - (m_monitor->getLastShutterTime() + m_collectInterval * 1000 * m_collectTimes)) >= m_collectInterval * 1000)
			{
				//���ĵ�3*3������Y16��ֵ
				//���ص�Y16����δ���죬����ת��
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
				//һ������ɼ���ɣ�����15��ƽ��ֵ���ص�֪ͨ��
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
				//һ�κ���ɼ������ڲɼ�Y16��������
				m_collectTimes = 0;
				//��һ������궨����
				m_collectCount++;
			}
		}
	}
	//�ж��Ƿ��л����·�Χ
	if (m_isChangeRange)
	{
		/*�����߰���TEC��Ʒ֧���ֶ��л����·�Χ���л���ɺ�����������еĲ��·�Χ�����仯�����ֶ��еķ�Χһ�£���ô�������ظ��л���
		�����и����գ��ֶ��л������ߣ����²����л��ǾɵĲ��·�Χ��ֱ���������и��·�Χ�����ȷ�����Խ��鲻Ҫ�ֶ��з�Χ��ͳһʹ�ò����С�*/
		result = m_parser->parseCurveData(m_cameraID, m_registry, (ITA_FIELD_ANGLE)m_mtBase->getTECParams()->lensType, m_range, m_mtBase->getTECParams()->nGear);
		if (ITA_OK == result)
		{
			//m_mtBase->getTECParams()->mtType = m_range;//���ﲻ�ܸ��£���Ϊ�����л��Ǿ�ֵ�ᵼ���������лؾɷ�Χ��
			/*��ʼ�����²���������CMeasureTempCoreʵ����*/
			m_mtBase->loadData(m_parser->getPH(), m_parser->getFPArray(), m_parser->getCurves());
			m_mtBase->changeRange(m_range, (ITA_FIELD_ANGLE)m_parser->getPH()->lensType);
			m_mtBase->loadConfig(&m_config, m_range, (ITA_FIELD_ANGLE)m_parser->getPH()->lensType);
			m_processor->loadConfig(&m_config, m_range, (ITA_FIELD_ANGLE)m_parser->getPH()->lensType);
			//X16ģʽû�в����У���Ҫ��ȡУ�²������л����·�ΧʱҪ���¡�
			/*if (ITA_X16 == m_mode)
				m_parser->parseCalibrateParam(m_cameraID, m_registry, m_range, m_mtBase->getMtParams());*/
		}
		m_isChangeRange = false;
	}
	//���űպ�ʱ���ܲ�������
	if (m_isCalcCentral && !m_mtBase->getTECParams()->bOrdinaryShutter)
		m_mtBase->refreshFrame(pResult->y16Data, pResult->info.w, pResult->info.h);
	//���ź�NUC���ԣ�Ҫ�ڵ�ǰ֡����֮��������űպϣ���ô��һ֡��ʼ�ɼ����ס�
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
	//srcSize���ܴ�ImageProcessor::process memcpy�õ����������ʵ��֡��С�ᱨ��heap corruption detected��
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
	m_mtBase->getMtParams()->sCurrentShutterValue = m_x16CenterValue;	//ʵʱ���¸�ֵ���з�Χʱ������ڿ��űպ�ʱ���£���ô��ӡ����NUC֮ǰ��ֵ������ʵʱֵ�������С�
	//ȷ������״̬
	if (m_firstNUCing)
		result = ITA_FIRST_NUC_NOT_FINISH;
	else if (1 == m_mtBase->getMtParams()->bOrdinaryShutter)
		result = ITA_SHUTTER_CLOSING;
	else if (!pResult)
		result = ITA_NULL_PTR_ERR;
	/*10���ӱ���һ�ιػ���Ϣ,��ֹ����ϵ絼�µ���Ϣ��ʧ*/
	if (m_monitor->isSaveColdHot(g_currentTime)) {
		m_parser->saveColdOrHot(m_cameraID, m_registry, m_mtBase->getMtParams());
	}
	//ÿ���ȡһ���´�
	if (g_currentTime < m_lastSensorTime)
	{
		//����ϵͳʱ�䱻��С��ʱ�������㡣
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
		//ÿ������´�����㵵λ��
		//�ж��Ƿ���Ҫ�е����ڿ��Ŵ򿪵�״̬���ж��Ƿ���Ҫ�е������űպ�ʱ��ֹ�е��������¶��쳣���ܵ�10�����ҡ�
		if (!m_mtBase->getMtParams()->bOrdinaryShutter)
		{
			//�ֶ��е��򿪺��Զ��е���ʧЧ��
			if (m_isGearManual)
			{
				//�ֶ���λ������Чֵ���뵱ǰ��λm_fpaGear�Ƚϣ����Ⱦ��л���
				if (m_gearManual >= 0 && m_gearManual != m_fpaGear)
				{
					m_isChangeGear = true;
					/*�л����µ�λ��̽����������ͬ��������NUC��*/
					ITA_RESULT ret = m_parser->changeGear(m_cameraID, m_registry, m_gearManual, (ITA_FIELD_ANGLE)m_mtBase->getMtParams()->m_fieldType);
					//X16ģʽ�¸���K
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
				//���¿��������5�����£���ô��0 1 2 3 4 5��6����λ��0��5��λ��4���������ظ���2�顣4��5��λ����K��
				ITA_RESULT myResult = m_mtBase->calcFPAGear(&currentGear);
				if (ITA_OK == myResult)
				{
					/*zc08b��Ŀ��ITA��ʼ�����ص�Ĭ�ϲ���������߲���������������һ�£������²����к�ͷ���͸ı䣬ֱ���л���λ�ᵼ�²����쳣����Ҫ���¼�����������������������⡣*/
					if (m_fpaGear != currentGear && !m_isChangeRange)
					{
						m_isChangeGear = true;
						//����0.01������������Ƶ���е����⡣���ﲻ��ʱ�����ơ�
						/*�л����µ�λ��̽����������ͬ��������NUC�����calcFPAGear�ɹ�����ô��ȡ���ݰ��ɹ����������ﲻ���жϽ����*/
						m_parser->changeGear(m_cameraID, m_registry, currentGear, (ITA_FIELD_ANGLE)m_mtBase->getMtParams()->m_fieldType);
						//X16ģʽ�¸���K
						if (ITA_X16 == m_mode || ITA_MCU_X16 == m_mode)
						{
							m_processor->setCurrentK(m_parser->getCurrentK(), m_frameSize * 2);
						}
						m_fpaGear = currentGear;
						m_logger->output(LOG_INFO, "auto change gear=%d FpaTemp=%f.", currentGear, m_mtBase->getMtParams()->fRealTimeFpaTemp);
						m_isChangeGear = false;
					}
				}
				//�����ȡ��λʧ�ܣ�û�����ݰ�δ��ʼ������ʵ��������ô���ܲ��¡�
			}
		}
		m_fps = rateCount;	//ʵ��֡�ʼ���
		rateCount = 0;
		if (m_debug && m_userFps && m_userFps != m_maxFrameRate)
		{
			m_logger->output(LOG_DEBUG, "isp=%d lisp=%d rateCount=%d userFps=%d", m_ispTime, m_lastIspTime, rateCount, m_userFps);
		}
		/*�����Զ����Ų��Բ������¿�������*/
		m_monitor->updatePeriod(g_currentTime);
	}
	//������Ҫ�ڲɼ�����֮ǰ��
	if (m_reverse && (ITA_X16 == m_mode || ITA_MCU_X16 == m_mode))
	{
		int t;
		for (int i = 0; i < m_frameSize; i++)
		{
			//̽���������ԭʼ�����Ƿ���ģ�Ҫ������
			t = *(mySrc + i);
			t = 16383 - t;
			*(mySrc + i) = (short)t;
		}
	}
	/*У����*/
	if (m_badPoint.x > -1)
	{
		m_parser->correctBadPoint(m_badPoint);
		m_processor->setCurrentK(m_parser->getCurrentK(), m_frameSize * 2);
		m_badPoint.x = -1;
		m_badPoint.y = -1;
	}
	//������űպϣ�������X16ģʽ����ɼ����ס�
	if (m_isCollectBackground)
		collectBackground(mySrc, srcSize);
	if (!m_mtBase->getMtParams()->bOrdinaryShutter && m_collectK && m_collectK->isCollectK())
	{
		m_collectK->collectKData(mySrc);
	}
	//����֡��
	if (m_userFps && m_userFps != m_maxFrameRate)
	{
		unsigned int intervaltemp = 1000 / m_userFps;
#ifdef CONTINUE_FRAME_DROP
		if (rateCount >= m_userFps)
		{
			m_loop = false;
			return ITA_SKIP_FRAME; //��֡������
		}
#else
		//����ʹ���û����õ�ƽ��֡����ʱ�䡣��Ϊʵʱ�����֡ʱ��ܲ��ȶ���Ӱ��ʵ��֡�ʡ�
		if (m_userIspTime)
			consumeTime = m_userIspTime;
		else
			consumeTime = (m_lastIspTime + m_ispTime) / 2;
		if ((intervaltemp > consumeTime) && (g_currentTime - rateLastTime < intervaltemp - consumeTime))
		{
			m_loop = false;
			return ITA_SKIP_FRAME; //��֡������
		}
		else
		{
			if (m_debug)
				m_logger->output(LOG_DEBUG, "Interval between two frames is %d isp=%d lisp=%d %d count=%d inte=%d cons=%d",
					g_currentTime - rateLastTime, m_ispTime, m_lastIspTime, intervaltemp - (m_lastIspTime + m_ispTime) / 2, rateCount, intervaltemp, consumeTime);
			rateLastTime = g_currentTime;	//��¼��֡ʱ��
		}
#endif // CONTINUE_FRAME_DROP
	}
	rateCount++;
	if (!m_closeImage && pResult)
	{
		if (m_reCalcSize)
		{
			//���ڷ��ظ��û���pResultҪ������ȷ��img���ȣ����Ըĳ�ÿ�α仯�����������ڴ档
			if (m_imgDst)
			{
				porting_free_mem(m_imgDst);
				m_imgDst = NULL;
				pResult->imgDst = NULL;
			}
			//ĿǰY16û�����ſ��Բ����·��䣬�����������Ҫ��
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
			{	//��һ��ʱ���䣬���治�ͷţ��˳�ʱ���ͷš�
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
			/*������ж�Ӱ��Ч�ʣ���ʽ�汾���Զ����ű���������*/
			result = ITA_OUT_OF_MEMORY;
		}
		else
		{
			//ͼ����
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
		//��������y16����
		float temp = 0.0;
		int w = pResult->info.w, h = pResult->info.h;
		short y16 = pResult->y16Data[w * h / 2 + w / 2];
		measureTempPoint(y16, &temp);
		m_startReview->review(temp, g_currentTime);
	}
	//���Ŵ򿪵�״̬���ж��Ƿ�У�²ɼ�����λ���Զ����Ų��Թز���ʱ��ʱ��ɵ��պ�ʱ��Y16�쳣��
	//��¼�ɼ�״̬��0����ʼ��״̬��1��׼���ɼ���2�����ڲɼ���3��һ�α궨����ɼ���ɡ�
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
				/*�������10s��ôֱ�Ӵ���Ųɼ�������ȴ���*/
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
			/*ÿ��1s�ɼ�һ��Y16����ɼ�15�Σ�Ȼ����15�ε�ƽ��ֵ��*/
			if ((g_currentTime - (m_monitor->getLastShutterTime() + m_collectInterval * 1000 * m_collectTimes)) >= m_collectInterval * 1000)
			{
				//���ĵ�3*3������Y16��ֵ
				//���ص�Y16����δ���죬����ת��
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
				//һ������ɼ���ɣ�����15��ƽ��ֵ���ص�֪ͨ��
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
				//һ�κ���ɼ������ڲɼ�Y16��������
				m_collectTimes = 0;
				//��һ������궨����
				m_collectCount++;
			}
		}
	}
	//�ж��Ƿ��л����·�Χ
	if (m_isChangeRange)
	{
		/*�����ݰ�*/
		ITA_RESULT readPkStatus = ITA_ERROR;
		if (!m_builtInFlash)
			readPkStatus = m_parser->parsePackageData(m_cameraID, m_registry, m_range, (ITA_FIELD_ANGLE)m_mtBase->getMtParams()->m_fieldType);
		else
			readPkStatus = m_parser->parseFlashPackage(m_cameraID, m_registry, m_range);
		if (ITA_OK == readPkStatus)
		{
			/*��ʼ�����²���������CMeasureTempCoreʵ����*/
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
			/*�л����·�Χ���üĴ���*/
			if (ITA_256_TIMO == m_productType && m_detector)
				m_detector->ChangeGears(m_range);
			else if (ITA_120_TIMO == m_productType && m_timo120)
				m_timo120->ChangeGears(m_range, m_parser->getPH()->ucGain, m_parser->getPH()->ucInt, m_parser->getPH()->ucRes, false);
			/*��ҵ����ÿ����Ӧ�������룬��һ�κͶ���У�¡��е�ʱ��������У�²�����
			������У���޷�����������¾��ȣ�����ͳһ���Զ�У�¡�*/
			m_monitor->nucManual(1);
			//�л����·�Χ�󣬵�λ�ָ���ʼֵ����һ֡�л�����ȷ��λ��
			m_fpaGear = -1;
			if (m_isGearManual && -1 == m_gearManual)
			{
				//����ر��Զ��е������û�û�����ý��µ�λ����ôĬ���е�0����
				m_gearManual = 0;
			}
			//X16ģʽû�в����У���Ҫ��ȡУ�²������л����·�ΧʱҪ���¡�Y16���ݲ������в�һ����У�²���������Ҫ��ȡ���û����о����Ƿ�ע�������������
			if (ITA_X16 == m_mode || ITA_Y16 == m_mode)
				m_parser->parseCalibrateParam(m_cameraID, m_registry, m_range, m_mtBase->getMtParams());
		}
		m_isChangeRange = false;
	}
	//���űպ�ʱ���ܲ������º�AF
	if (m_isCalcCentral && !m_mtBase->getMtParams()->bOrdinaryShutter)
		m_mtBase->refreshFrame(pResult->y16Data, pResult->info.w, pResult->info.h);
#ifdef MULTI_THREAD_FUNCTION
	if (m_isAF && !m_mtBase->getMtParams()->bOrdinaryShutter)
		m_af->refreshFrame(pResult->y16Data, pResult->info.w, pResult->info.h);
#endif // MULTI_THREAD_FUNCTION
	//���ź�NUC���ԣ�Ҫ�ڵ�ǰ֡����֮��������űպϣ���ô��һ֡��ʼ�ɼ����ס�
	status = m_monitor->timeLoopOnce(g_currentTime);
	if (MONITOR_SHUTTER_NOW == status)
		shutterNow();
	else if (MONITOR_NUC_NOW == status)
		nucNow();
	else if (MONITOR_NUC_CLOSED_LOOP == status || MONITOR_NUC_LOOP_REPEAT == status || MONITOR_NUC_RECOVERY == status)
	{
		if (ITA_120_TIMO == m_productType)
		{
			//120ģ��Res�ջ��������ʧ�ܣ�5��������������ɺ�����ظ���
			//ucRes��3��������ջ�+1���ջ��ɹ����ֵ���ֲ��䣬�ָ����ݰ��е�ֵ��
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
	//����Ź����в��ܲ��¡�
	/*if (m_mtBase->getMtParams() && 1 == m_mtBase->getMtParams()->bOrdinaryShutter)
	{
		return ITA_INACTIVE_CALL;
	}*/
	//TEC��Ʒ���Ź������������ղ��¡��ӻԿͻ�TEC��Ʒ�ڴ���Ź����в�������¡�
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
	//����Ź����в��ܲ��¡�
	/*if (m_mtBase->getMtParams() && 1 == m_mtBase->getMtParams()->bOrdinaryShutter)
	{
		return ITA_INACTIVE_CALL;
	}*/
	//�ӻԿͻ���Ʒ�ڴ���Ź����в�������¡�
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
		//ע��������޸�Y16��������
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
	case ITA_SHUTTER_NOW:			//�����
		if (m_mtBase->getMtParams() && 1 == m_mtBase->getMtParams()->bOrdinaryShutter)
		{
			m_logger->output(LOG_WARN, "ITA_SHUTTER_NOW:Shutter is busying.");
			return ITA_INACTIVE_CALL;
		}
		else
			ret = m_monitor->shutterManual(0); //m_monitor�еĿ���ʱ�����������ƣ�Ĭ����ͼ��3�롣
		break;
	case ITA_AUTO_SHUTTER:			//�����Զ����š��������ͣ�ITA_SWITCH��
		ret = m_monitor->autoShutter(*flag);
		break;
	case ITA_SHUTTER_TIME:			//�����Զ�����ʱ�䣬��λ�롣Ĭ��30�롣�������ͣ����͡�
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
		//������Ҫ9֡360ms
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
		//������Ҫ5֡
		if (msp >= 5 && msp <= 15)
		{
			m_bDelayFrame = msp;
			m_shutterFrameNumber = m_bDelayFrame + 4; //����������ģ�����û�ɵ�4֡���˳������ײ����޷�����
		}
		else
			ret = ITA_ARG_OUT_OF_RANGE;
		m_logger->output(LOG_INFO, "ITA_B_DELAY %d %d %d", m_bDelayFrame, msp, m_shutterFrameNumber);
		break;
	}
	case ITA_GET_SHUTTER_STATUS:		//��ȡ����״̬�����űպ�ʱ�û����Զ����档�������ͣ�ITA_SHUTTER_STATUS����X16ʹ�ÿ��Ų��Ե�������õ���
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
	case ITA_FIRST_NUC_FINISH:		//��һ��NUC�Ƿ���ɡ��������ͣ�ITA_SWITCH��ITA_DISABLE��δ��ɣ�ITA_ENABLE������ɡ�
		if (m_firstNUCing)
			*flag = ITA_DISABLE;
		else
			*flag = ITA_ENABLE;
		break;
	case ITA_DO_NUC:             //�ֶ�NUC
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
		//��������е��͵ȴ���200ms��ʱ��ͨ�����ȴ�һ֡��ʱ�伸ʮ���롣
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
		//��鵵λ��Χ
		int gear = *(int *)param;
		//����tec�ͷ�tec
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
	case ITA_SHUTTER_POLICY:		//���ƿ��š�NUC���е����ԡ��������ͣ�ITA_SWITCH��Ĭ�ϴ򿪡���ITA_Y16ģʽ��Ĭ�Ϲرա�
		if (*flag > ITA_ENABLE || *flag < ITA_DISABLE)
			return ITA_ARG_OUT_OF_RANGE;
		if (ITA_DISABLE == *flag)
		{
			m_monitor->closeShutterPolicy(true);
			//���ASIC Y16ģʽ�³�ʼ��ʱ�û����ùرտ��Ų��ԣ���ôITA��������ʱ��һ��NUC��
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
			//�պϿ��š��û��������ƿ��ţ����ô����ɱ��׻��ơ�
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
		//�������š�
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
	case ITA_IMAGE_PROCESS:			//����ͼ�����������ͣ�ITA_SWITCH��Ĭ�ϴ򿪡���X16ģʽ�²��ܹرգ���Y16ģʽ�¿��Թرա�����ĳ���Ʒͨ��оƬISP�Ѿ���ȡYUV��ʹ��ITA�����¡�
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
			m_userFps = frameRate; //�û����õ�֡��
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
		//�ظ����ž�����ơ�Ĭ��0�رա�1���û���nuc�ظ����Ż��ƣ�2�������ظ����ţ�3�����ڿ��š����Ŷ�����ɺ��е�1״̬��
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
		//�ظ�NUC������ơ�Ĭ��0�رա�1���û���nuc�ظ�������ơ�������־���ͬʱ���ã���ôֻ����ITA_NUC_REPEAT_CORRECTION��
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
		//��ֵ�������á�
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
	/*	ITA_CONF_ENCRYPT,			//���������ļ��Ƿ���ܡ��������ͣ�ITA_SWITCH��ITA_DISABLE��δ���ܣ�ITA_ENABLE�����ܡ�*/
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
	case ITA_AUTO_FOCUS:			//�Զ�����
	case ITA_AF_NEAR:				//�������
	case ITA_AF_FAR:				//����Զ��
	case ITA_STEP_NEAR:				//����һ��ʱ�����ڲ�������
	case ITA_STEP_FAR:				//����һ��ʱ�����ڲ���Զ��
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
			m_isCalcCentral = true; //��һ֡��ʼ����������
			return ITA_CENTRAL_NOT_READY;
		}
		//������������²��Ҽ�ȥ���׾�ֵ����ôm_mtBase��û����Щ��Ϣ�޷����㡣ֻ����������㡣
		if (ITA_CENTRAL_TEMPER == type)
		{
			//m_mtBase������ĵ�y16
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
	case ITA_SET_FORMAT:		//���ó������ظ�ʽ���������ͣ�ITA_PIXEL_FORMAT��
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
		//doISP�ӿڷ��غ�������¼����ߡ�
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
	case ITA_SET_PALETTE:		//����α�ʣ���Χ0-11���������ͣ�ITA_PALETTE_TYPE��
	{
		ITA_PALETTE_TYPE paletteIndex = *(ITA_PALETTE_TYPE *)param;
		if (m_paletteIndex == paletteIndex)
			return ITA_INACTIVE_CALL;
		ret = m_processor->setPaletteIndex(paletteIndex);
		if(ITA_OK == ret)
			m_paletteIndex = paletteIndex;
		break;
	}
	case ITA_SET_SCALE:			//���ű������������ͣ����㡣
	{
		float scale = *(float *)param;
		int count = 0;
		if (scale == m_scale)
		{
			ret = ITA_INACTIVE_CALL;
			break;
		}
		else if (scale < 1 || scale > 20)
		{ //�Ŵ�200���������ڴ�ʱAndroid�Ϸ��ز�Ϊ�յ�ָ�룬��ʵ�ʲ����ã�������������20����
			ret = ITA_ARG_OUT_OF_RANGE;
			break;
		}
		//doISP�ӿڷ��غ�������������ڴ档
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
		//�����µĳߴ����������ڴ档
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
	case ITA_SET_CONTRAST:		//���öԱȶȡ��Աȶ�0-511��Ĭ��255���������ͣ����Ρ�
	{
		int contrast = *(int *)param;
		ret = m_processor->setContrast(contrast);
		break;
	}
	case ITA_SET_BRIGHTNESS:	//�������ȡ�����0-255��Ĭ��70���������ͣ����Ρ�
	{
		int bright = *(int *)param;
		ret = m_processor->setBrightness(bright);
		break;
	}
	case ITA_SET_ROTATE:		//������ת��ʽ��RGB��Y16���ݾ���ת���������ͣ�ITA_ROTATE_TYPE��
	{
		ITA_ROTATE_TYPE rotateType = *(ITA_ROTATE_TYPE *)param;
		int count = 0;
		if (rotateType == m_rotate)
		{
			ret = ITA_INACTIVE_CALL;
			break;
		}
		//doISP�ӿڷ��غ�������¼����ߡ�
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
	case ITA_SET_FLIP:			//���÷�ת��RGB��Y16���ݾ���ת���������ͣ�ITA_FLIP_TYPE��
	{
		ITA_FLIP_TYPE flipType = *(ITA_FLIP_TYPE *)param;
		if (flipType == m_flip)
			return ITA_INACTIVE_CALL;
		ret = m_processor->setFlip(flipType);
		if(ITA_OK == ret)
			m_flip = flipType;
		break;
	}
	case ITA_SET_REVERSE:		//�򿪻��߹رշ�����ITA_X16ģʽ��Ĭ�ϴ򿪡��������ͣ�bool��
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
	case ITA_TIME_FILTER:		//ʱ���˲����أ����Թرջ��ߴ򿪡��������ͣ�bool��
	{
		ISPParam ispParam = m_processor->getISPParam();
		ispParam.isTimeFilter = *bParam;
		m_processor->setISPParam(ispParam);
		break;
	}
	case ITA_SPATIAL_FILTER:		//�����˲����أ����Թرջ��ߴ򿪡��������ͣ�bool��
	{
		ISPParam ispParam = m_processor->getISPParam();
		ispParam.isSpatialFilter = *bParam;
		m_processor->setISPParam(ispParam);
		break;
	}
	case ITA_REMOVE_VERTICAL:	//ȥ���ƣ����Թرջ��ߴ򿪡��������ͣ�bool��
	{
		ISPParam ispParam = m_processor->getISPParam();
		ispParam.isRemoveVertical = *bParam;
		m_processor->setISPParam(ispParam);
		break;
	}
	case ITA_REMOVE_HORIZONTAL:	//ȥ���ƣ����Թرջ��ߴ򿪡��������ͣ�bool��
	{
		ISPParam ispParam = m_processor->getISPParam();
		ispParam.isRemoveHorizontal = *bParam;
		m_processor->setISPParam(ispParam);
		break;
	}
	case ITA_HSM:	//�����⣬ ���Թرջ��ߴ򿪡� �������ͣ�bool
	{
		ISPParam ispParam = m_processor->getISPParam();
		ispParam.isHSM = *bParam;
		m_processor->setISPParam(ispParam);
		break;
	}
	case ITA_SHARPENING:		//�񻯣����Թرջ��ߴ򿪡��������ͣ�bool��
	{
		ISPParam ispParam = m_processor->getISPParam();
		ispParam.isSharpening = *bParam;
		m_processor->setISPParam(ispParam);
		break;
	}
	case ITA_DETAIL_ENHANCE:	//ϸ����ǿ�����Թرջ��ߴ򿪡��������ͣ�bool��
	{
		ISPParam ispParam = m_processor->getISPParam();
		ispParam.isDetailEnhance = *bParam;
		m_processor->setISPParam(ispParam);
		break;
	}
	case ITA_BLOCK_HISTOGRAM:	//�ֿ�ֱ��ͼ�����Թرջ��ߴ򿪡��������ͣ�bool��
	{
		ISPParam ispParam = m_processor->getISPParam();
		ispParam.isBlockHistogram = *bParam;
		m_processor->setISPParam(ispParam);
		break;
	}
	case ITA_GAMMA_CORRECTION:		//GammaУ�������Թرջ��ߴ򿪡��������ͣ�bool��
	{
		ISPParam ispParam = m_processor->getISPParam();
		ispParam.isGammaCorrection = *bParam;
		m_processor->setISPParam(ispParam);
		break;
	}
	case ITA_Y8_ADJUSTBC:		//Y8��ƫ�����Թرջ��ߴ򿪡��������ͣ�bool��
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
	info->isAutoGear = !m_isGearManual; //�Զ��е�����
	info->isReverse = m_reverse; //�Ƿ���Ŀ���
	info->isSubAvgB = m_subAvgB; //��ȥ���׾�ֵ�Ŀ���
	info->isDebug = m_debug; //���Կ���
	info->drtType = m_processor->getDimmingType(); //���ⷽʽ
	info->equalLineType = m_processor->getEqualLineType(); //����������
	info->frameRate = m_userFps; //�û����õ�֡��
	info->coldHotStatus = m_coldHotStatus;//���Ȼ�״̬
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
	case ITA_SET_DEBUGGING:			//�����Ƿ������ԡ��������ͣ�ITA_SWITCH��ITA_DISABLE���رյ��ԡ�ITA_ENABLE���������ԡ����δ������־·������ôĬ�ϱ����ڳ���ͬ��Ŀ¼��ITA.log��
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
	case ITA_DEBUGGING_PATH:		//������־���������·����ע��Ҫ��ITA_Init֮����á��������Ѵ��ڵ�Ŀ¼������/home/app/debug.log��Windows����"D:/Tools/debug.log"�����Ȳ�Ҫ����128���������ͣ��ַ������� \0 ������
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
	case ITA_DEBUGGING_MODE:		//������־���ģʽ��1����־������ļ���Ĭ�������ʽ��2����־���������̨��֧���û��Զ��������־����ITARegistry��
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
	case ITA_SNAPSHOT:				//���ձ��浱ǰ֡ͼ�񣬴洢�ڳ����ͬ��Ŀ¼�¡�
	case ITA_START_RECORD:			//��ʼ¼X16/Y16���ݡ�
	case ITA_STOP_RECORD:			//ֹͣ¼X16/Y16���ݡ�
		ret = m_processor->control(type, param);
		break;
	case ITA_MEMORY_USAGE:			//��ѯ��̬�ڴ�ʹ�������������ͣ����͡���λ��Byte��
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
1.2.1.20220621 ����ITA_CalcY16MatrixByTemp��ZC08B��ĿҪ�á�
1.2.2.20220705 ֧�ֱ�׼������Э�飻�����¶Ⱦ���֧�ּ�ȥ���ף�����Զ�У��ʧ����ô�ָ�ԭ����kf bֵ��
1.2.3.20220816 ���������⡢�������ȡ����ֽӿڡ�
1.2.4.20220831 ����ReadCurveDataExע�ắ����֧�ִ�����У�¾�ͷ���ֲֳ��ºͻ�о���²�Ʒ������ITA_ConvertY8�ӿڡ�
1.2.5.20221012 ���Ӻ�����¹��ܣ���BLACKBODY_EDITION���ơ�����ITA_GetColorImage�ӿڵ�ʵ�֡�
1.2.6.20221020 ֧������9��ͼ���㷨������ʵ�ֽӿ�ITA_CustomPalette����������������⣬��FIRE_DRC_EDITION���ơ�
1.2.7.20221101 ���û�����26��α�����ƣ�ITA_GetColorImageԴ���ݸ�ΪY8����������̽��������ģ�ͣ���REFRIGER_EDITION���ơ�
1.2.8.20221206 ���ӽӿ�ReadPackageDataEx֧�ֲ�ͬ��ͷ�Ȳ�ζ�ȡ��Ӧ���ݰ���֧��ģ�����ģ�ͽ�����׼�����У���Χ����ͷ�仯ʱ�Զ��л���Χ�����Ӳ�K���ܡ�
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
	//��Parser����
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
			|| (nRange != m_mtBase->getTECParams()->mtType && m_range!= m_mtBase->getTECParams()->mtType) //�Ƚ�m_range�������ֶ��з�Χ�ظ�
			|| nLensType != m_mtBase->getTECParams()->lensType || m_parser->isLensChanged())
		{	
			m_range = (ITA_RANGE)m_mtBase->getTECParams()->mtType;
			//��λ�仯ʱ���²���������
			m_mtBase->calcFPAGear(&nGear);
			m_isChangeRange = true;
			if (nLensType != m_mtBase->getTECParams()->lensType && m_laserAF)
			{//��������
				m_laserAF->updateLens((ITA_FIELD_ANGLE)m_mtBase->getTECParams()->lensType);
			}
			//m_parser->parseCurveData(m_cameraID, m_registry, (ITA_FIELD_ANGLE)m_mtBase->getTECParams()->lensType, m_range, m_mtBase->getTECParams()->nGear);
		}
		return ITA_OK;
	}
	if (ITA_FPGA_Y16 == m_mode && ITA_256_TIMO == m_productType)
	{
		pRef = (short *)paramLine;
		//���֪ͨFPGA�ɱ��ף�����Ҫ�����Ƿ񱾵ײɼ���ɡ�
		if (1 == m_isFPGAB)
		{
			short b = *(pRef + 9);
			unsigned char a = (b >> 15) & 0x1;
			unsigned char c = b & 0xF;
			//���ڷ���ָ�����һ֡��һ�����ϱ����1���ڲɼ����ס�������첽�ġ�
			if (1 == a)
			{
				m_fpgaBing = true;
			}
			else if (m_fpgaBing)
			{
				//֮ǰ���ڲɼ����ף����ڲɼ���ɡ�
				m_isFPGAB = 2;
				m_fpgaBing = false;
			}
			else
			{
				if (g_currentTime < m_FPGAStartTime)
				{
					//����ϵͳʱ�䱻��С������ʱ���ߡ�
					m_FPGAStartTime = 0;
				}
				if (g_currentTime - m_FPGAStartTime > 1500)
				{
					//���fpgaû�յ�����ָ���ô���Ų��ָܻ����Ӹ���ʱ���ƱȽϱ��ա�timeout to go out
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
		//ʵʱ�����´����ݺͿ���״̬��
		/*MCU��ͨ����ʱ��Ҫ������MCU NUCʱ1��2��3״̬����Ҫ���������п���״̬��*/
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
			if (nRange != m_mtBase->getMtParams()->mtType && m_range != m_mtBase->getMtParams()->mtType) //�Ƚ�m_range�������ֶ��з�Χ�ظ�
			{
				m_range = (ITA_RANGE)m_mtBase->getMtParams()->mtType;
				m_isChangeRange = true;
				//ģ�����ģ�Ͱ����·�Χ��ȡ���ݰ�
			}
			if (nLensType != m_mtBase->getMtParams()->m_fieldType || m_parser->isLensChanged())
			{
				if (m_registry && m_registry->ReadPackageDataEx)
				{
					m_isChangeRange = true; //֧�ֶ��־�ͷ�Ȳ�βŴ���
				}
			}
		}
		if (ITA_120_TIMO == m_productType)
		{
			m_mtBase->getMtParams()->fRealTimeFpaTemp = m_mtBase->smoothFocusTemp(m_mtBase->getMtParams()->fRealTimeFpaTemp);
		}
		m_monitor->updateSensorTemp((int)(m_mtBase->getMtParams()->fRealTimeFpaTemp * 100));
		//���±��׾�ֵ
		if (ITA_Y16 == m_mode || ITA_FPGA_Y16 == m_mode)
		{
			m_avgB = m_parser->getAvgB();
			m_monitor->updateAVGB(m_avgB);
		}
		/*m_logger->output(LOG_DEBUG, "updateParamLine. m_mode=%d nucStatus=%d bNucShutterFlag=%d bOrdinaryShutter=%d",
			m_mode, m_nucStatus, m_wrapper->getMtParams()->bNucShutterFlag, m_wrapper->getMtParams()->bOrdinaryShutter);*/
		//Y16ʱ���¿���״̬������ģʽ��ITA����ƿ���״̬������Ҫ�����С�
		if (ITA_Y16 == m_mode)
			m_mtBase->refreshShutterStatus(m_x16CenterValue,m_mode);
		else if (ITA_MCU_X16 == m_mode)
		{
			//��ģʽNUC��MCU��ɣ����ݲ�����״̬�ɱ��ס���ͨ������Ȼ��ITA����ɡ�
			if (m_nucStatus == 1 && m_mtBase->getMtParams()->bNucShutterFlag == 1)  //��¼NUC״̬��0����ʼ��״̬��1����ʼ��NUC��2��������NUC��3��NUC����ɡ�
			{
				//֪ͨ�ɱ���
				m_isCollectBackground = true;
				m_backgroundCount = 0;
				m_skipCount = 0;
				m_nucStatus = 2;	//��¼NUC״̬��0����ʼ��״̬��1����ʼ��NUC��2��������NUC��3��NUC����ɡ�
				m_mtBase->refreshShutterStatus(m_x16CenterValue,m_mode);
				//if (m_debug)
					m_logger->output(LOG_INFO, "CollectBackground begin. nucStatus=%d m_mode=%d", m_nucStatus, m_mode);
			}
			else if (m_nucStatus == 2 && m_mtBase->getMtParams()->bNucShutterFlag == 0)  //��¼NUC״̬��0����ʼ��״̬��1����ʼ��NUC��2��������NUC��3��NUC����ɡ�
			{
				//nucʱmcu�������ƿ��ţ�����ITA�򿪿��š�ITAֻ�òɱ��ס�
				m_isCollectBackground = false;
				m_backgroundCount = 0;
				m_skipCount = 0;
				m_isFPGAB = 0;	//�ָ�����״̬
				m_fpgaBing = false;
				if (m_firstNUCing)
					m_firstNUCing = false;
				m_nucStatus = 3;	//��¼NUC״̬��0����ʼ��״̬��1����ʼ��NUC��2��������NUC��3��NUC����ɡ�
				m_mtBase->refreshShutterStatus(m_x16CenterValue, m_mode);
				//if (m_debug)
					m_logger->output(LOG_INFO, "CollectBackground end. nucStatus=%d m_mode=%d", m_nucStatus, m_mode);
			}
			else if (m_nucStatus == 3 && m_mtBase->getMtParams()->bOrdinaryShutter == 0)
			{
				//NUC�����ʱ��Ҫ����������ֱ�����ű�־λΪ0����ʱ�����ٽ��������п���״̬��
				m_nucStatus = 0;
			}
			if (ITA_120_TIMO == m_productType && m_nucStartTime)
			{
				//CB360��Ʒ������nuc��������״̬����2�����0��
				if (g_currentTime - m_nucStartTime >= 2000)
				{
					m_mtBase->getMtParams()->bOrdinaryShutter = 0;	//��¼����״̬
					m_nucStartTime = 0;
					if (m_registry && m_registry->ShutterControl)
						m_registry->ShutterControl(m_cameraID, ITA_STATUS_CLOSE, m_registry->userParam);
					porting_thread_sleep(m_shutterCloseDelay);//MCU USBЭ������ʱ������300���ֿ�����Ӱ��
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
	case ITA_TRY_CORRECT_POINT:	//�ȳ��Ա궨���㣬ʵʱͼ��ȷ�ϱ궨�ɹ�������ӡ��������ͣ�ITA_POINT��
	{
		ITA_POINT *p = (ITA_POINT *)param;
		if (m_badPoint.x > -1)
		{
			//��һ�����㻹δ�궨
			m_logger->output(LOG_WARN, "TRY_CORRECT_POINT is busying. x=%d y=%d", p->x, p->y);
			return ITA_INACTIVE_CALL;
		}
		if (p->x >= 0 && p->y >= 0 && p->x < m_width && p->y < m_height)
		{
			m_badPoint.x = p->x;
			m_badPoint.y = p->y;
			//��һ֡��Ч
		}
		else
		{
			res = ITA_ILLEGAL_PAPAM_ERR;
		}
		m_logger->output(LOG_INFO, "TRY_CORRECT_POINT:x=%d y=%d ret=%d", p->x, p->y, res);
		break;
	}
	case ITA_CORRECT_POINT:		//��ӻ��㣬���Զ����ӡ��������ͣ�ITA_POINT��
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
	case ITA_SAVE_POINTS:		//���滵�㡣�޲����������������ɺ󣬱��档
	{
		if (m_badPointsCount > 0)
		{
			ITA_RANGE *range = (ITA_RANGE *)param;
			res = m_parser->saveBadPoints(m_badPointsArray, m_badPointsCount, m_cameraID, m_registry, *range);
			m_logger->output(LOG_INFO, "SAVE_POINTS:range=%d count=%d ret=%d", *range, m_badPointsCount, res);
		}
		else
		{
			//��һ�����㻹δ�궨
			m_logger->output(LOG_WARN, "SAVE_POINTS:No bad Point. count=%d", m_badPointsCount);
			return ITA_INACTIVE_CALL;
		}
		//�ָ���ʼֵ
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

/*3�α궨����ɼ���ʱ����ࣺ10+15+15+15 = 55�롣����45�롣*/
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
	//�ɼ�ʱ���ܹر�ͼ����
	if (m_closeImage)
	{
		m_logger->output(LOG_ERROR, "startCollect:Image processing cannot be turned off. ret=%d", ITA_INACTIVE_CALL);
		return ITA_INACTIVE_CALL;
	}
	m_collectType = collectParam.type;
	m_collectParam = collectParam.param;
	m_logger->output(LOG_INFO, "startCollect %d %f %f %d %d %d", m_collectType, m_collectParam, collectParam.blackTemp,
		m_shutterCollectInterval, m_collectInterval, m_collectNumberOfTimes);
	//�ȹرտ��ź�NUC����
	ITA_SWITCH flag = ITA_DISABLE;
	baseControl(ITA_SHUTTER_POLICY, &flag);
	m_isAutoCollect = 1;	//��¼�ɼ�״̬��0����ʼ��״̬��1��׼���ɼ���2�����ڲɼ���3��һ�α궨����ɼ���ɡ�
	m_blackTempArray[m_collectCount] = collectParam.blackTemp;
	m_collectCBArray[m_collectCount] = cb;
	m_cbParamArray[m_collectCount] = param;
	m_collectTimes = 0;		//һ�κ���ɼ������ڲɼ�Y16������<= m_collectNumberOfTimes��
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
	//��¼kf b�����ʧ����ô�ָ�ԭ����ֵ���Զ�У���㷨�л��޸�kf b��
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
		//�ڵ���10�桢����23�滷���¶���
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
				//�ʳ��Զ�У����ʱʧ�ܣ������趨������ֵ���㷨�齨����΢�ſ�������
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
			//�������
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
		//�򿪿��ź�NUC����
		ITA_SWITCH flag = ITA_ENABLE;
		baseControl(ITA_SHUTTER_POLICY, &flag);
	}
	/*�Զ�У����ɺ�ָ���ʼ״̬*/
	m_isAutoCollect = 0;	//��¼�ɼ�״̬��0����ʼ��״̬��1��׼���ɼ���2�����ڲɼ���3��һ�α궨����ɼ���ɡ�
	//m_shutterTimes = 0;		//һ�κ���ɼ������ڿ��ż�����<= m_shutterNumberOfTimes��
	m_collectTimes = 0;		//һ�κ���ɼ������ڲɼ�Y16������<= m_collectNumberOfTimes��
	m_collectCount = 0;		//�ڼ�������ɼ���һ��3�Σ�30 33 36.
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
	/*��ʼ�����²���������CMeasureTempCoreʵ����*/
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
		//ע��������޸�Y16��������
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
		//ע��������޸�Y16��������
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
		//�ɱ���ǰ�׵�5֡������ɵ��պ�ǰ��ͼ��ITA_MCU_X16��nucʱ�����ף���ΪMCU�Ѿ��׵����������׺��治����4֡��
		//ITA_MCU_X16�����ʱ��ITA�ⵥ�����ƿ��ţ�������Ҫ�׵�ǰ��5֡������ʹ��MCU����ţ���ôҲ�����ף����ݲ�����ֱ֪ͨ�Ӳɱ��ס�
		//�������ǰ��5֡�������𱾵ײɵĲ��ԣ����ʱ��̫�̵������ŵ�����Ҳδ��Ч��
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
			//4֡���׷ֲ���ƽ��
			for (int i = 0; i < m_frameSize; i++)
			{
				*(m_bArray + i) = (*(m_bArray + i) + *(srcData + i)) / 2;
			}
			if (m_backgroundCount == 3)
			{
				//���ײɼ���ɣ����±��ס�
				ITA_RESULT ret = m_processor->updateB(m_bArray, m_frameSize * 2, m_nucStatus);
				m_avgB = m_processor->getAvgB();
				m_monitor->updateAVGB(m_avgB);
				if (ITA_OK != ret)
				{
					//�˴�nucʧЧ�����������2�Ρ�
					m_nucRepeatCount++;
				}
				else if (m_nucRepeatCount > 0)
				{
					//�ϴ�nucʧЧ����γɹ���
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
			//SDK���Ų�����Ч����Ҫ֪ͨFPGA�ɱ��ס�����״̬��֪ͨһ�Ρ�TBD
			/*if (GUIDEIR_ERR == UartCmdSend(CMD_UPDATE_BASE, 0, 0))
			{
				m_logger->output(LOG_ERROR, "UartCmdSend CMD_UPDATE_BASE failed!");
			}
			else*/
			{
				if (m_debug)
					m_logger->output(LOG_INFO, "UartCmdSend CMD_UPDATE_BASE success!");
				m_isFPGAB = 1;	//FPGA��ʼ�ɼ����ס��ɼ���ɺ�ͨ��������֪ͨITA��
				m_fpgaBing = false;
				m_FPGAStartTime = g_currentTime;	//��¼��ʼʱ��
			}
		}
	}
	m_backgroundCount++;
	if (ITA_MCU_X16 == m_mode && 2 == m_nucStatus) //��¼NUC״̬��0����ʼ��״̬��1����ʼ��NUC��2��������NUC��3��NUC����ɡ�
	{
		//�ȴ������п���״̬֪ͨ��nucʱmcu�������ƿ��ţ�����ITA�򿪿��š�
		return 0;
	}
	//�����Ƕ�MCU��Ʒ��ͨ���ŵĴ����Լ���ģ��nuc����ͨ���ŵĴ���
	//����4֡���׺�򿪿���
	if ((ITA_FPGA_Y16 != m_mode && 4 == m_backgroundCount) || m_isFPGAB == 2)
	{
		if (m_debug)
			m_logger->output(LOG_DEBUG, "Shutter Open:shutterFrameNumber=%d %d CloseDelay=%d OpenDelay=%d", 
				m_shutterFrameNumber, m_backgroundCount + m_bDelayFrame, m_shutterCloseDelay, m_shutterOpenDelay);
		//ITA_FPGA_Y16ģʽҲ��4֮֡��򿪿��ţ���X16һ�¡�
		if(m_mtBase->getMtParams())
			m_mtBase->getMtParams()->bNucShutterFlag = 0;	//��¼����״̬
		//�������š�
		if (m_registry && m_registry->ShutterControl)
			m_registry->ShutterControl(m_cameraID, ITA_STATUS_OPEN, m_registry->userParam);
		porting_thread_sleep(m_shutterOpenDelay); //USBЭ������ʱ������300����ֿ��Ŵ򲻿��������
	}
	//��ͨ����ʱ��ITA_MCU_X16 ����4֡MCU USB�������ݣ�����������ITA_X16 m_shutterFrameNumberΪ9�����ס�
	if ((ITA_FPGA_Y16 != m_mode && m_backgroundCount >= m_shutterFrameNumber - m_bDelayFrame) || m_isFPGAB == 2)
	{
		if (m_debug)
			m_logger->output(LOG_DEBUG, "collectBackground:backgroundCount = %d", m_backgroundCount);
		if (m_mtBase->getMtParams())
			m_mtBase->getMtParams()->bOrdinaryShutter = 0;	//��¼����״̬
		//���¿���״̬
		m_mtBase->refreshShutterStatus(m_x16CenterValue, m_mode);
		m_isCollectBackground = false;
		m_backgroundCount = 0;
		m_skipCount = 0;
		m_isFPGAB = 0;	//�ָ�����״̬
		m_fpgaBing = false;
		if (2 == m_nucStatus)
		{
			if (m_firstNUCing)
				m_firstNUCing = false;
			m_nucStatus = 0;// 3;	//��¼NUC״̬��0����ʼ��״̬��1����ʼ��NUC��2��������NUC��3��NUC����ɡ�
			//�ظ�NUC������ơ�������־���ͬʱ���ã���ôֻ����ITA_NUC_REPEAT_CORRECTION��
			if (1 == m_nucRepeatCorrection)
			{
				if (m_nucRepeatCount >= 3)
				{
					//����3��nuc��ʧЧ������ʧ�ܡ�
					m_logger->output(LOG_INFO, "ITA_NUC_REPEAT_CORRECTION failed. %d %d", m_nucRepeatCorrection, m_nucRepeatCount);
					m_nucRepeatCount = 0;
				}
				else if (m_nucRepeatCount > 0)
				{
					//���ظ�һ��nuc
					m_logger->output(LOG_INFO, "Make another nuc after NUC is completed. %d %d", m_nucRepeatCorrection, m_nucRepeatCount);
					//m_monitor�е�NUCʱ�����������ƣ�Ĭ����ͼ��5�롣
					m_monitor->nucManual(1);
				}
			}
			//�ظ����ž�����ơ�Ĭ��0�رա�1���û���nuc�ظ����Ż��ƣ�2�������ظ����ţ�3�����ڿ��š����Ŷ�����ɺ��е�1״̬��
			else if (1 == m_nucAnotherShutter)
			{
				m_logger->output(LOG_INFO, "Make another shutter after NUC is completed. %d", m_nucAnotherShutter);
				m_nucAnotherShutter = 2;
				//m_monitor�еĿ���ʱ�����������ƣ�Ĭ����ͼ��3�롣
				m_monitor->shutterManual(1);
			}
		}
		//�ظ����ž�����ơ�Ĭ��0�رա�1���û���nuc�ظ����Ż��ƣ�2�������ظ����ţ�3�����ڿ��š����Ŷ�����ɺ��е�1״̬��
		if (3 == m_nucAnotherShutter)
		{
			if (m_debug)
				m_logger->output(LOG_DEBUG, "Make another shutter done. %d", m_nucAnotherShutter);
			m_nucAnotherShutter = 1;
		}
		if (m_registry && m_registry->NotifyEvent)
			m_registry->NotifyEvent(m_cameraID, ITA_SHUTTER_END, 0, m_registry->userParam);
		//��֤���������ʵisp time�����ⵯ��������ʱ���´˴�m_ispTime������󣬵���֡�ʿ����쳣һֱ����֡��
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
		m_mtBase->getMtParams()->bOrdinaryShutter = 1;	//��¼����״̬
	//�ظ����ž�����ơ�Ĭ��0�رա�1���û���nuc�ظ����Ż��ƣ�2�������ظ����ţ�3�����ڿ��š����Ŷ�����ɺ��е�1״̬��
	if (2 == m_nucAnotherShutter)
	{
		m_nucAnotherShutter = 3;
	}
	//if (m_debug)
		m_logger->output(LOG_INFO, "Close Shutter.");
	//���űպ�
	if (m_registry && m_registry->ShutterControl)
		m_registry->ShutterControl(m_cameraID, ITA_STATUS_CLOSE, m_registry->userParam);
	porting_thread_sleep(m_shutterCloseDelay);//MCU USBЭ������ʱ������300���ֿ�����Ӱ��
	//֪ͨ�ɱ���
	m_isCollectBackground = true;
	m_backgroundCount = 0;
	m_skipCount = 0;
	if(m_mtBase->getMtParams())
		m_mtBase->getMtParams()->bNucShutterFlag = 1;	//��¼����״̬
	//���¿���״̬
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
		//CB360��ƷbNucShutterFlag��Ч��ֻ��nuc���ɼ����ס�
		//����״̬��1����2�����0��
		if(m_mtBase->getMtParams())
			m_mtBase->getMtParams()->bOrdinaryShutter = 1;	//��¼����״̬
		m_nucStartTime = g_currentTime;
		if (m_registry && m_registry->ShutterControl)
			m_registry->ShutterControl(m_cameraID, ITA_STATUS_CLOSE, m_registry->userParam);
		porting_thread_sleep(m_shutterCloseDelay);//MCU USBЭ������ʱ������300���ֿ�����Ӱ��
		//��MCU��������NUC
		if (m_registry && m_registry->DoNUC)
			m_registry->DoNUC(m_cameraID, m_registry->userParam);
		return 0;
	}
	if (ITA_X16 == m_mode || ITA_FPGA_Y16 == m_mode)
	{
		if (m_registry && m_registry->NotifyEvent)
			m_registry->NotifyEvent(m_cameraID, ITA_SHUTTER_BEGIN, 0, m_registry->userParam);
		m_nucStatus = 1;	//��¼NUC״̬��0����ʼ��״̬��1����ʼ��NUC��2��������NUC��3��NUC����ɡ�
		if(m_mtBase->getMtParams())
			m_mtBase->getMtParams()->bOrdinaryShutter = 1;	//��¼����״̬
		//if (m_debug)
			m_logger->output(LOG_INFO, "NUC Close Shutter.");
		//�պϿ���
		if (m_registry && m_registry->ShutterControl)
			m_registry->ShutterControl(m_cameraID, ITA_STATUS_CLOSE, m_registry->userParam);
		//��NUC
		if (ITA_256_TIMO == m_productType && m_detector)
			m_detector->DoNUC();
		else if (ITA_120_TIMO == m_productType && m_timo120)
			m_timo120->DoNUC();
		porting_thread_sleep(200);
		//֪ͨ�ɱ���
		m_isCollectBackground = true;
		m_backgroundCount = 0;
		m_skipCount = 0;
		if(m_mtBase->getMtParams())
			m_mtBase->getMtParams()->bNucShutterFlag = 1;	//��¼����״̬
		m_nucStatus = 2;	//��¼NUC״̬��0����ʼ��״̬��1����ʼ��NUC��2��������NUC��3��NUC����ɡ�
		//���¿���״̬
		m_mtBase->refreshShutterStatus(m_x16CenterValue,m_mode);
	}
	else if (ITA_MCU_X16 == m_mode)
	{
		//��MCU��������NUC
		if (m_registry && m_registry->DoNUC)
			m_registry->DoNUC(m_cameraID, m_registry->userParam);
		m_nucStatus = 1;	//��¼NUC״̬��0����ʼ��״̬��1����ʼ��NUC��2��������NUC��3��NUC����ɡ�
		//ͨ��������״̬�ɱ���
		//if(m_debug)
			m_logger->output(LOG_INFO, "DoNUC nucStatus=%d m_mode=%d", m_nucStatus, m_mode);
	}
	return 0;
}

ITA_RESULT ITAWrapper::laserAFRegister(ITALaserAFRegistry *registry)
{
	ITA_FIELD_ANGLE lensType;
	//laserAFͨ�����ֲֳ�Ʒ�Ĺ���
	if (m_productType >= ITA_HANDHELD_TEC)
	{
		lensType = (ITA_FIELD_ANGLE)m_mtBase->getTECParams()->lensType;
	}
	else
	{
		lensType = (ITA_FIELD_ANGLE)m_parser->getPH()->lensType;
	}
	//ע��һ�Σ�������Դ��������������߳����⡣
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
	//�ɼ�ʱ���ܹر�ͼ����
	if (m_closeImage)
	{
		m_logger->output(LOG_ERROR, "collectK:Image processing cannot be turned off. ret=%d", ITA_INACTIVE_CALL);
		return ITA_INACTIVE_CALL;
	}
	//�ȹرտ��ź�NUC����
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
			//���øߵ��±��׼���K����
			if (!m_gain_mat)
			{
				m_gain_mat = (unsigned short*)porting_calloc_mem(m_frameSize, sizeof(unsigned short), ITA_WRAPPER_MODULE);
			}
			memset(m_gain_mat, 0, m_frameSize * sizeof(unsigned short));
			//����K����
			m_processor->getGainMat(m_collectK->getHighBase(), m_collectK->getLowBase(), m_gain_mat, m_width, m_height);
			if (cb) {
				cb(m_cameraID,gear, m_gain_mat, m_frameSize * sizeof(unsigned short), param);
			}
			//����õ���K����͵�ǰ���µ�λ�Ľ���д���û����ļ���ȥ,�����µ�ǰK����
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

/* �Զ����� */
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

	//����Ź����в��ܲ��¡�
	/*if (m_mtBase->getMtParams() && 1 == m_mtBase->getMtParams()->bOrdinaryShutter)
	{
		return ITA_INACTIVE_CALL;
	}*/
	//TEC��Ʒ���Ź������������ղ��¡��ӻԿͻ�TEC��Ʒ�ڴ���Ź����в�������¡�
#ifdef SHIHUI_EDITION
	if (m_mtBase->getTECParams() && 1 == m_mtBase->getTECParams()->bOrdinaryShutter)
	{
		return ITA_INACTIVE_CALL;
	}
#endif
	//������ʾ���ͻ�����ʱ�����Ƕ��̵߳��ã����º��е������õ����ߡ�������е����������ڸ������ߣ���ʱ���ò����г�ͻ��
	//��������е��͵ȴ���200ms��ʱ��ͨ�����ȴ�һ֡��ʱ�伸ʮ���롣
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

