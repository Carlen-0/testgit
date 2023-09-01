/*************************************************************
Copyright (C), 2021--2022, Wuhan Guide Sensmart Tech Co., Ltd
File name   : Timo120.cpp
Author      : Guide Sensmart RD. wangyan
Version     : 1.0
Date        : 2021/11/15
Description : Configure Timo 120 detector.
*************************************************************/
#include "Timo120.h"
#include <string>

Timo120::Timo120(ITARegistry * registry, int cameraID, ITA_RANGE range)
{
	m_range = range;
	m_debug = false;

	m_cameraID = cameraID;
	m_registry = registry;
	//120ģ�������ʱ�ӣ�����8MHz��
	m_clkHz = 8;
	m_logger = NULL;
}

Timo120::~Timo120()
{
}

ITA_RESULT Timo120::DetectorInit(int clkHz, unsigned char validMode, unsigned char dataMode, unsigned char frameFrequency, DetectorParameter *dp)
{
	ITA_RESULT ret = ITA_OK;
	if (clkHz >= 1000000)
	{
		m_clkHz = clkHz / 1000000;
		m_logger->output(LOG_INFO, "DetectorInit clkHz=%d %d %d %d %d", m_clkHz, clkHz, validMode, dataMode, frameFrequency);
	}
	else
	{
		m_logger->output(LOG_ERROR, "DetectorInit clkHz=%d %d %d %d %d", m_clkHz, clkHz, validMode, dataMode, frameFrequency);
		return ITA_ILLEGAL_PAPAM_ERR;
	}
	DetectorConf regData[21] = {
		{ 0x10,0x2f },
		{ 0x11,0x00 },
		{ 0x12,0x00 },
		{ 0x13,0x00 },
		{ 0x14,0x00 },
		{ 0x15,0x01 },
		{ 0x16,0xc9 },
		{ 0x17,0x80 },
		{ 0x18,0x1d },
		{ 0x19,0x34 }, //7bit 0x32
		{ 0x1a,0x38 },
		{ 0x1b,0x30 },
		{ 0x1c,0x0c },
		{ 0x20,0xfa }, //int   320 --100  190 --50         
		{ 0x21,0x00 },
		{ 0x22,0xd0 },  //TLINE //d0
		{ 0x23,0x07 }, //07   5M                                      
		{ 0x29,0x5F },  //NUC level 5F
		{ 0x2A,0x30 },  //30
		{ 0xf0,0x6d },
		{ 0xf1,0x24 },
	};
	//�����ݰ��ж�ȡ̽��������
	if (dp && dp->dps[20])
	{
		for (int i = 0; i < 21; i++)
		{
			m_logger->output(LOG_INFO, "DetectorInit %d %#x %#x %#x", i, regData[i].addr, regData[i].regValue, dp->dps[i]);
			regData[i].regValue = dp->dps[i];
		}
	}
	//cal frame rate
	unsigned short frameControl = (unsigned short)(clkHz / (frameFrequency * (90 + 2 + 8))); //Reg 0x1c ��bit4�� ��bit3�� keep 1
	regData[15].regValue = frameControl & 0x00ff;
	regData[16].regValue = (frameControl >> 8) & 0x00ff;
	m_logger->output(LOG_INFO, "DetectorInit frameControl=%d, %#x, %#x, %d, %d", frameControl, regData[15].regValue, regData[16].regValue, clkHz, frameFrequency);
	ret = sensorRegInitial(regData);
	return ret;
}

int Timo120::DoNUC()
{
	/*ITA֧���¼�֪ͨ���ơ���һЩ������û�����֪��NUC�����ź�AF���¼��ķ��������û��д�����ʱ��ע
	��NotifyEvent������ITA����Щ�¼�����ʱ����������NotifyEvent֪ͨ�û����û����յ��¼���������Ӧ��
	ͼ��ֲ���������֡������ͼ������»�����ƴ�ӡ�Ϊʲô�ӵײ㻺���ȡ����������ԭʼ����֡���ų���
	���߼������©���⣬����һ�����ܵ�ԭ������д̽�����Ĵ���������ʱ���ȣ���ȡ��������������֡��
	ƽ̨ISPӲ���ɼ�Camera���������ַ�ʽ��֡�жϣ���֡���������У����п������жϡ���������жϷ�ʽ��
	ʱ���ȿ��ܻ�©�������С����ߴ򾲵�����ٵ����ж��źţ��������水��ƴ�ӵ�֡���ݶ��Ǵ�λ�ġ�֡
	�жϷ�ʽ�򲻴��ڴ����⡣ITA��NUC������̽�����Ĵ�������ȫ���������û����յ�ITA_NUC_BEGIN�¼�ʱ��
	��DVP����ͨ��ֹͣ�������ݣ����յ�ITA_NUC_END�¼����ٴ�DVP����ͨ��������ȡ���ݣ���֤��ȡ������
	������֡��*/
	if (m_registry->NotifyEvent)
		m_registry->NotifyEvent(m_cameraID, ITA_NUC_BEGIN, 0, m_registry->userParam);
	startNUC();
	porting_thread_sleep(480);
	stopNUC();
	porting_thread_sleep(480);
	if (m_registry->NotifyEvent)
		m_registry->NotifyEvent(m_cameraID, ITA_NUC_END, 0, m_registry->userParam);
	return 0;
}

int Timo120::ChangeGears(ITA_RANGE range, unsigned char ucGain, unsigned char ucInt, unsigned char ucRes, bool isNUC)
{
	m_logger->output(LOG_INFO, "Detector ChangeGears range=%d %d %d %d", range, ucGain, ucInt, ucRes);
	if (!ucGain && !ucInt && !ucRes)
	{	//���û�ж������ݰ�������Ĭ��ֵ������̽�������ܳ�ԭʼ���ݡ�
		ucGain = 2;
		ucInt = 44;
		ucRes = 9;
		m_logger->output(LOG_WARN, "Detector ChangeGears default %d %d %d", ucGain, ucInt, ucRes);
	}
	configInt(ucInt);
	configGain(ucGain);
	configRes(ucRes);
	//��ʼ��ʱ��Ҫ��NUC���л����·�Χʱ���ﲻ����NUC���ⲿ�����ֶ�NUC��
	if(isNUC)
		DoNUC();
	return 0;
}

void Timo120::setLogger(GuideLog * logger)
{
	m_logger = logger;
}

void Timo120::setDebug(bool debug)
{
	m_debug = debug;
}
/*
int sensor_reg_read(uint8_t reg_addr, uint8_t * val)
{
int ret;
uint8_t tx[4] = { 0xBC, 0x00, 0x00, 0x00 };
uint8_t rx[4] = { 0x00, 0x00, 0x00, 0x00 };
tx[1] = reg_addr;
ret = HAL_SPI_TransmitReceive(&hspi1, tx, rx, 4, SPI_TIMEOUT);
*val = rx[3];
return ret;
}

int sensor_reg_write(uint8_t reg_addr, uint8_t val)
{
int ret;
uint8_t tx[3] = { 0xAF, 0x00, 0x00 };
tx[1] = reg_addr;
tx[2] = val;
ret = HAL_SPI_Transmit(&hspi1, tx, 3, SPI_TIMEOUT);
return ret;
}
*/
ITA_RESULT Timo120::sensorWriteReg(unsigned char addr, unsigned char data)
{
	/*SPI_FLASH_CS_LOW();
	SPI_SendByte(0xaf);
	SPI_SendByte(addr);
	SPI_SendByte(data);

	SPI_FLASH_CS_HIGH();*/
	ITA_RESULT ret = ITA_OK;
	unsigned char tx[3] = { 0xAF, 0x00, 0x00 };
	tx[1] = addr;
	tx[2] = data;
	if (m_registry && m_registry->SPIWrite)
		ret = m_registry->SPIWrite(m_cameraID, tx, 3, m_registry->userParam);
	else
		ret = ITA_HAL_UNREGISTER;
	m_logger->output(LOG_INFO, "sensorWriteReg ret=%d", ret);
	return ret;
}

ITA_RESULT Timo120::sensorReadReg(unsigned char addr, unsigned char *data)
{
	/*SPI_FLASH_CS_LOW();

	SPI_SendByte(0xbc);
	SPI_SendByte(addr);
	SPI_SendByte(0xff);

	Reg_Data = SPI_Read_Byte(0xff);

	SPI_FLASH_CS_HIGH();*/
	ITA_RESULT ret = ITA_OK;
	unsigned char tx[4] = { 0xBC, 0x00, 0x00, 0x00 };
	unsigned char rx[4] = { 0x00, 0x00, 0x00, 0x00 };
	tx[1] = addr;
	if (m_registry && m_registry->SPITransmit)
		ret = m_registry->SPITransmit(m_cameraID, tx, rx, 4, m_registry->userParam);
	else
		ret = ITA_HAL_UNREGISTER;
	*data = rx[3];
	m_logger->output(LOG_INFO, "sensorReadReg ret=%d %d %d", ret, addr, *data);
	return ret;
}

ITA_RESULT Timo120::sensorRegInitial(DetectorConf * regData)
{
	unsigned char i, spi_data_wr, spi_addr_wr;
	ITA_RESULT ret = ITA_OK;
	for (i = 0; i < 21; i++)
	{
		spi_data_wr = regData[i].regValue;
		spi_addr_wr = regData[i].addr;
		ret = sensorWriteReg(spi_addr_wr, spi_data_wr);
		if (ITA_OK != ret)
		{
			m_logger->output(LOG_ERROR, "Timo120 WriteReg ret=%d", ret);
			return ret;
		}
	}
	for (i = 0; i < 21; i++)
	{
		//spi_data_wr = regData[i].regValue;
		spi_addr_wr = regData[i].addr;
		ret = sensorReadReg(spi_addr_wr, &spi_data_wr);
		if (ITA_OK != ret)
		{
			m_logger->output(LOG_ERROR, "Timo120 ReadReg ret=%d", ret);
			return ret;
		}
		m_logger->output(LOG_DEBUG, "Timo120 ReadReg i=%d addr=%x, data=%x", i, spi_addr_wr, spi_data_wr);
	}
	return ret;
}

ITA_RESULT Timo120::startNUC(void)
{
	unsigned char RegData = 0;
	ITA_RESULT ret;
	ret = sensorReadReg(0x18, &RegData);
	if (ITA_OK != ret)
	{
		m_logger->output(LOG_ERROR, "startNUC ret=%d 1", ret);
		return ret;
	}
	m_logger->output(LOG_DEBUG, "Timo120 startNUC 0x18 data=%d", RegData);
	RegData = RegData & 0xfe;
	ret = sensorWriteReg(0x18, RegData);
	if (ITA_OK != ret)
	{
		m_logger->output(LOG_ERROR, "startNUC ret=%d 2", ret);
		return ret;
	}
	ret = sensorReadReg(0x1B, &RegData);
	if (ITA_OK != ret)
	{
		m_logger->output(LOG_ERROR, "startNUC ret=%d 3", ret);
		return ret;
	}
	m_logger->output(LOG_DEBUG, "Timo120 startNUC 0x1B data=%d", RegData);
	RegData = RegData | 0x80;
	ret = sensorWriteReg(0x1B, RegData);
	if (ITA_OK != ret)
	{
		m_logger->output(LOG_ERROR, "startNUC ret=%d 4", ret);
		return ret;
	}
	ret = sensorReadReg(0x1c, &RegData);
	if (ITA_OK != ret)
	{
		m_logger->output(LOG_ERROR, "startNUC ret=%d 5", ret);
		return ret;
	}
	m_logger->output(LOG_DEBUG, "Timo120 startNUC 0x1c data=%d", RegData);
	RegData = RegData | 0x80;
	ret = sensorWriteReg(0x1c, RegData);
	if (ITA_OK != ret)
	{
		m_logger->output(LOG_ERROR, "startNUC ret=%d 6", ret);
		return ret;
	}
	return ret;
}

ITA_RESULT Timo120::stopNUC(void)
{
	unsigned char RegData = 0;
	ITA_RESULT ret;
	ret = sensorReadReg(0x18, &RegData);
	if (ITA_OK != ret)
	{
		m_logger->output(LOG_ERROR, "stopNUC ret=%d 1", ret);
		return ret;
	}
	m_logger->output(LOG_DEBUG, "Timo120 stopNUC 0x18 data=%d", RegData);
	RegData = RegData | 0x01;
	ret = sensorWriteReg(0x18, RegData);
	if (ITA_OK != ret)
	{
		m_logger->output(LOG_ERROR, "stopNUC ret=%d 2", ret);
		return ret;
	}
	return ret;
}

ITA_RESULT Timo120::configGain(unsigned char value)
{
	unsigned char Temp = 0;
	ITA_RESULT ret = ITA_OK;
	if (value <= 7 && value >= 0)
	{
		unsigned char data;
		ret = sensorReadReg(0x10, &data);
		if (ITA_OK != ret)
		{
			m_logger->output(LOG_ERROR, "configGain ret=%d 1", ret);
			return ret;
		}
		m_logger->output(LOG_DEBUG, "Timo120 configGain 0x10 data=%d", data);
		Temp = data & 0x8f;
		Temp = (Temp | (value << 4));
		ret = sensorWriteReg(0x10, Temp);
		if (ITA_OK != ret)
		{
			m_logger->output(LOG_ERROR, "configGain ret=%d 2", ret);
			return ret;
		}
	}
	else
		ret = ITA_ARG_OUT_OF_RANGE;
	return ret;
}

ITA_RESULT Timo120::configInt(unsigned short value)
{
	unsigned char INT_H, INT_L;
	unsigned short Temp = 0;
	ITA_RESULT ret = ITA_OK;
	Temp = (unsigned short)(value * m_clkHz);
	INT_L = Temp % 256;
	INT_H = (unsigned char)((Temp - INT_L) / 256);
	ret = sensorWriteReg(0x20, INT_L);
	if (ITA_OK != ret)
	{
		m_logger->output(LOG_ERROR, "configInt ret=%d 1", ret);
		return ret;
	}
	ret = sensorWriteReg(0x21, INT_H);
	if (ITA_OK != ret)
	{
		m_logger->output(LOG_ERROR, "configInt ret=%d 2", ret);
		return ret;
	}
	m_logger->output(LOG_DEBUG, "Timo120 configInt ret=%d", ret);
	return ret;
}

ITA_RESULT Timo120::configRes(unsigned char value)
{
	unsigned char Temp = 0;
	ITA_RESULT ret = ITA_OK;
	if (value <= 15 && value >= 0)
	{
		unsigned char data;
		ret = sensorReadReg(0x16, &data);
		if (ITA_OK != ret)
		{
			m_logger->output(LOG_ERROR, "configRes ret=%d 1", ret);
			return ret;
		}
		m_logger->output(LOG_DEBUG, "Timo120 configRes 0x16 data=%d", data);
		Temp = data & 0xf0;
		Temp = (Temp | value);
		ret = sensorWriteReg(0x16, Temp);
		if (ITA_OK != ret)
		{
			m_logger->output(LOG_ERROR, "configRes ret=%d 2", ret);
			return ret;
		}
	}
	else
		ret = ITA_ARG_OUT_OF_RANGE;
	return ret;
}
