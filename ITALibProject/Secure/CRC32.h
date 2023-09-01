/*************************************************
Copyright (C), 2020-2030, GuideIR Tech. Co., Ltd.
File name	: CRC32.h
Author		: Guide Sensmart RD
Version		: 1.0
Date		: 2020/4/16
Description	:
*************************************************/
#ifndef CRC32_H
#define CRC32_H

class CRC32
{
public:
	CRC32();
	~CRC32();

	unsigned int calcCRC32(unsigned char *data, int len);

private:
	void makeCRCTable();

private:
	unsigned int m_POLYNOMIAL;
	unsigned int m_table[256];
};

#endif // !CRC32_H
