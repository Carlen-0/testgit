#include "CRC32.h"
#include <string.h>

CRC32::CRC32()
{
	m_POLYNOMIAL = 0xEDB88320;
	memset(m_table, 0, 256 * 4);
	makeCRCTable();
}

CRC32::~CRC32()
{

}

unsigned int CRC32::calcCRC32(unsigned char * data, int len)
{
	unsigned int crc = 0;
	crc = ~crc;
	for (int i = 0; i < len; i++)
		crc = (crc >> 8) ^ m_table[(crc ^ data[i]) & 0xff];
	return ~crc;
}

void CRC32::makeCRCTable()
{
	int i, j;
	for (i = 0; i < 256; i++)
		for (j = 0, m_table[i] = i; j < 8; j++)
			m_table[i] = (m_table[i] >> 1) ^ ((m_table[i] & 1) ? m_POLYNOMIAL : 0);
}
