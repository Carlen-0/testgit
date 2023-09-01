#include"ImageFusion.h"

#ifdef IMAGE_FUSION_EDITION

CImgFusion::CImgFusion(ITA_PIXEL_FORMAT format, ITA_FIELD_ANGLE lensType, int nIrW, int nIrH, int nVisW, int nVisH, unsigned char detail)
{
	eFusImgType = format;
	//m_resolutionType = resolution;
	sIrImg.nWidth = nIrW;
	sIrImg.nHeight = nIrH;
	sVisImg.nWidth = nVisW;
	sVisImg.nHeight = nVisH;
	sFusImg.nWidth = nIrW;
	sFusImg.nHeight = nIrH;
	//图像参数设置

	m_lensType = lensType;
	nFusDetail = detail;

	if ((eFusImgType == ITA_RGB888))
	{
		sIrImg.nLen = sIrImg.nWidth * sIrImg.nHeight * 3;
		sVisImg.nLen = sVisImg.nWidth * sVisImg.nHeight * 3;
		sFusImg.nLen = sIrImg.nLen;
	}
	else if (eFusImgType == ITA_YUV422_YUYV)
	{
		sIrImg.nLen = sIrImg.nWidth * sIrImg.nHeight * 2;
		sVisImg.nLen = sVisImg.nWidth * sVisImg.nHeight * 2;
		sFusImg.nLen = sIrImg.nLen;
	}
	else if (eFusImgType == ITA_YUV420_YUYV || eFusImgType == ITA_YUV420_UYVY || eFusImgType == ITA_YUV420_Plane || eFusImgType == ITA_YVU420_Plane)
	{
		sIrImg.nLen = sIrImg.nWidth * sIrImg.nHeight * 3 / 2;
		sVisImg.nLen = sVisImg.nWidth * sVisImg.nHeight * 3 / 2;
		sFusImg.nLen = sIrImg.nLen;
	}

	//图像数据内存申请
	sIrImg.nImgData = new unsigned char[sIrImg.nLen];
	memset(sIrImg.nImgData, 0, sIrImg.nLen * sizeof(unsigned char));
	sVisImg.nImgData = new unsigned char[sVisImg.nLen];
	memset(sVisImg.nImgData, 0, sVisImg.nLen * sizeof(unsigned char));
	sFusImg.nImgData = new unsigned char[sFusImg.nLen];
	memset(sFusImg.nImgData, 0, sFusImg.nLen * sizeof(unsigned char));

	//RGB格式图像融合内存申请
	m_VisImgGray = new unsigned char[sVisImg.nWidth * sVisImg.nHeight];
	memset(m_VisImgGray, 0, sVisImg.nWidth * sVisImg.nHeight * sizeof(unsigned char));
	nFusImgGray = new unsigned char[sIrImg.nLen];
	memset(nFusImgGray, 0, sIrImg.nLen * sizeof(unsigned char));
	nIrImgYUV = new unsigned char[sIrImg.nLen * 3];
	memset(nIrImgYUV, 0, sIrImg.nLen * 3 * sizeof(unsigned char));

	//仿射部分内存申请
	m_fTransTable = new double[sIrImg.nWidth * sIrImg.nHeight * 2];
	memset(m_fTransTable, 0, sIrImg.nWidth * sIrImg.nHeight * 2 * sizeof(double));
	m_fTransTable_pip = new double[sIrImg.nWidth * sIrImg.nHeight * 2];
	memset(m_fTransTable_pip, 0, sIrImg.nWidth * sIrImg.nHeight * 2 * sizeof(double));
	m_VisAffineImgGray = new unsigned char[sIrImg.nWidth * sIrImg.nHeight];
	memset(m_VisAffineImgGray, 0, sIrImg.nWidth * sIrImg.nHeight * sizeof(unsigned char));
	m_step = 0.1;
	
	m_logger = new GuideLog();

}

ITA_RESULT CImgFusion::CheckDirPath(const char* path)
{
	std::string myPath = (const char*)path;
	int ret;
	if (!path)
	{
		return ITA_NULL_PTR_ERR;
	}
	if (myPath.length() > 128)
		return ITA_BUF_OVERFLOW;
	if ((ret = (int)myPath.find('*')) >= 0)
	{
		return ITA_ILLEGAL_PATH;
	}
	else if ((ret = (int)myPath.find('?')) >= 0)
	{
		return ITA_ILLEGAL_PATH;
	}
	else if ((ret = (int)myPath.find('"')) >= 0)
	{
		return ITA_ILLEGAL_PATH;
	}
	else if ((ret = (int)myPath.find('<')) >= 0)
	{
		return ITA_ILLEGAL_PATH;
	}
	else if ((ret = (int)myPath.find('>')) >= 0)
	{
		return ITA_ILLEGAL_PATH;
	}
	else if ((ret = (int)myPath.find('|')) >= 0)
	{
		return ITA_ILLEGAL_PATH;
	}
	if ((myPath.find_last_of("/")+1) == myPath.length() || (myPath.find_last_of("\\") + 1) == myPath.length())
	{
		return ITA_OK;
	}
	else {
		return ITA_ILLEGAL_PATH;
	}
	
}

ITA_RESULT CImgFusion::ImgFusionInit(const char* fileDir)
{
	m_fileDir = fileDir;
	ITA_RESULT ret = ITA_OK;
	ret = CheckDirPath(fileDir);
	if (ret != ITA_OK)
	{
		return ret;
	}
	SetParams();
	double value = 0;
	std::ifstream inFile(m_fileDir + matrix_name, std::ios::in);
	if (inFile.is_open())
	{
		m_matrix.clear();
		while (inFile >> value) {
			// 打印整行字符串
			//std::cout << value1 << std::endl;
			m_matrix.push_back(value);
		}
	}
	inFile.close();
	warp_mat = cv::Mat::ones(cv::Size(3, 2), CV_64FC1);
	warp_mat.at<double>(0, 0) = m_matrix[0];
	warp_mat.at<double>(0, 1) = m_matrix[1];
	warp_mat.at<double>(1, 0) = m_matrix[2];
	warp_mat.at<double>(1, 1) = m_matrix[3];

	double a1 = m_matrix[0];
	double a2 = m_matrix[1];
	double b1 = m_matrix[2];
	double b2 = m_matrix[3];
	m_ADet = a1 * b2 - b1 * a2;
	if (m_ADet == 0) {
		m_ADet = 1;
	}
	m_ADet = 1.0 / m_ADet;
	m_fVISToIRAffineMat[0][0] = b2 * m_ADet;
	m_fVISToIRAffineMat[1][1] = a1 * m_ADet;
	m_fVISToIRAffineMat[0][1] = -b1 * m_ADet;
	m_fVISToIRAffineMat[1][0] = -a2 * m_ADet;
	m_fVISToIRAffineMat[0][2] = 0;
	m_fVISToIRAffineMat[1][2] = 0;
	m_fVISToIRAffineMat[2][2] = 1;
	
	inFile.open(m_fileDir + x_name, std::ios::in);
	value = 0;
	if (inFile.is_open())
	{
		m_xi.clear();
		while (inFile >> value) {
			// 打印整行字符串
			//std::cout << value << std::endl;
			m_xi.push_back(value);
		}
	}
	inFile.close();

	inFile.open(m_fileDir + y_name, std::ios::in);
	value = 0;
	if (inFile.is_open())
	{
		m_yi.clear();
		while (inFile >> value) {
			// 打印整行字符串
			//std::cout << value << std::endl;
			m_yi.push_back(value);
		}
	}
	return ret;
}

CImgFusion::~CImgFusion()
{
	if (sIrImg.nImgData != NULL)
	{
		delete[] sIrImg.nImgData;
		sIrImg.nImgData = NULL;
	}
	if (sVisImg.nImgData != NULL)
	{
		delete[] sVisImg.nImgData;
		sVisImg.nImgData = NULL;
	}
	if (sFusImg.nImgData != NULL)
	{
		delete[] sFusImg.nImgData;
		sFusImg.nImgData = NULL;
	}

	if (m_VisImgGray != NULL)
	{
		delete[] m_VisImgGray;
		m_VisImgGray = NULL;
	}
	if (nFusImgGray != NULL)
	{
		delete[] nFusImgGray;
		nFusImgGray = NULL;
	}
	if (nIrImgYUV != NULL)
	{
		delete[] nIrImgYUV;
		nIrImgYUV = NULL;
	}
	if (m_fTransTable != NULL)
	{
		delete[] m_fTransTable;
		m_fTransTable = NULL;
	}
	if (m_fTransTable_pip != NULL) {
		delete[] m_fTransTable_pip;
		m_fTransTable_pip = NULL;
	}
	if (m_VisAffineImgGray != NULL)
	{
		delete[] m_VisAffineImgGray;
		m_VisAffineImgGray = NULL;
	}
	if (m_logger) {
		delete m_logger;
		m_logger = NULL;
	}
}

void CImgFusion::SetParams() {
	if (sIrImg.nWidth == 1280 || sIrImg.nWidth == 1024) {
		if (m_lensType == 1) {
			matrix_name = "matrix.csv";
			x_name = "x.csv";
			y_name = "y.csv";
			m_min_distance = 0.3;
			m_max_distance = 10;
		}
		else if (m_lensType == 7) {
			matrix_name = "matrix_wide_angle.csv";
			x_name = "x_wide_angle.csv";
			y_name = "y_wide_angle.csv";
			m_min_distance = 0.3;
			m_max_distance = 10;
		}
		else if (m_lensType == 8) {
			matrix_name = "matrix_telephoto.csv";
			x_name = "x_telephoto.csv";
			y_name = "y_telephoto.csv";
			m_min_distance = 0.5;
			m_max_distance = 10;
		}
		else if (m_lensType == 9) {
			matrix_name = "matrix_longfocus.csv";
			x_name = "x_longfocus.csv";
			y_name = "y_longfocus.csv";
			m_min_distance = 3;
			m_max_distance = 13;
		}
	}
	else if (sIrImg.nWidth == 640) {
		if (m_lensType == 1) {
			matrix_name = "matrix.csv";
			x_name = "x.csv";
			y_name = "y.csv";
			m_min_distance = 0.3;
			m_max_distance = 8;
		}
		else if (m_lensType == 7) {
			matrix_name = "matrix_wide_angle.csv";
			x_name = "x_wide_angle.csv";
			y_name = "y_wide_angle.csv";
			m_min_distance = 0.3;
			m_max_distance = 8;
		}
		else if (m_lensType == 8) {
			matrix_name = "matrix_telephoto.csv";
			x_name = "x_telephoto.csv";
			y_name = "y_telephoto.csv";
			m_min_distance = 0.6;
			m_max_distance = 10;
		}
		else if (m_lensType == 9) {
			matrix_name = "matrix_longfocus.csv";
			x_name = "x_longfocus.csv";
			y_name = "y_longfocus.csv";
			m_min_distance = 5;
			m_max_distance = 13;
		}
	}
	else if (sIrImg.nWidth == 480 || sIrImg.nWidth == 384) {
		if (m_lensType == 1) {
			matrix_name = "matrix.csv";
			x_name = "x.csv";
			y_name = "y.csv";
			m_min_distance = 0.3;
			m_max_distance = 6.4;
		}
		else if (m_lensType == 7) {
			matrix_name = "matrix_wide_angle.csv";
			x_name = "x_wide_angle.csv";
			y_name = "y_wide_angle.csv";
			m_min_distance = 0.3;
			m_max_distance = 5;
		}
		else if (m_lensType == 8) {
			matrix_name = "matrix_telephoto.csv";
			x_name = "x_telephoto.csv";
			y_name = "y_telephoto.csv";
			m_min_distance = 0.6;
			m_max_distance = 8;
		}
		else if (m_lensType == 9) {
			matrix_name = "matrix_longfocus.csv";
			x_name = "x_longfocus.csv";
			y_name = "y_longfocus.csv";
			m_min_distance = 5;
			m_max_distance = 11.2;
		}
	}
	else if (sIrImg.nWidth == 320) {
		if (m_lensType == ITA_ANGLE_25) {
			matrix_name = "matrix.csv";
			x_name = "x.csv";
			y_name = "y.csv";
			m_min_distance = 1.2;
			m_max_distance = 6.4;
		}
	}
}

ITA_RESULT CImgFusion::GetImgData(unsigned char* mIrSrc, int nIrW, int nIrH, unsigned char* nVisSrc, int nVisW, int nVisH)
{
	if ((nIrW != sIrImg.nWidth) || (nIrH != sIrImg.nHeight) || (nVisH != sVisImg.nHeight) || (nVisW != sVisImg.nWidth))
	{
		return ITA_ERROR;
	}

	memcpy(sIrImg.nImgData, mIrSrc, sIrImg.nLen * sizeof(unsigned char));
	memcpy(sVisImg.nImgData, nVisSrc, sVisImg.nLen * sizeof(unsigned char));
	return ITA_OK;
}


void CImgFusion::ImgRegAndFusYUV()
{
	//提取可见光亮度分量 
	memcpy(m_VisImgGray, sVisImg.nImgData, sVisImg.nWidth * sVisImg.nHeight * sizeof(unsigned char));
	memset(m_VisAffineImgGray, 0, sIrImg.nWidth * sIrImg.nHeight * sizeof(unsigned char));
	//可见光图像仿射变换
	AffineTrans_old(m_VisAffineImgGray, sIrImg.nWidth, m_VisImgGray, sVisImg.nWidth, sVisImg.nHeight);
	//红外亮度与可见光亮度融合
	ImgFusionY(nFusImgGray, m_VisAffineImgGray, sIrImg.nImgData, sIrImg.nWidth, sIrImg.nHeight, nFusDetail);
	memcpy(sFusImg.nImgData, sIrImg.nImgData, sIrImg.nLen * sizeof(unsigned char));
	memcpy(sFusImg.nImgData, nFusImgGray, sIrImg.nWidth * sIrImg.nHeight * sizeof(unsigned char));
}

void CImgFusion::ImgFusionY(unsigned char* nDst, unsigned char* nVisSrc, unsigned char* nIrSrc, int nWidth, int nHeight, unsigned char nDetail)
{
	unsigned char nVisImgLow;
	short nTemp;
	int i, j, nSum, nIndex, nIndexRow;
	int nVisHf;

	memcpy(nDst, nIrSrc, nWidth * nHeight * sizeof(unsigned char));
	nIndexRow = 0;
	for (i = 1; i < (nHeight - 1); i++)
	{
		nIndexRow += nWidth;
		for (j = 1; j < (nWidth - 1); j++)
		{
			nIndex = nIndexRow + j;
			nSum = *(nVisSrc + nIndex - nWidth - 1) + *(nVisSrc + nIndex - nWidth) + *(nVisSrc + nIndex - nWidth + 1)
				+ *(nVisSrc + nIndex - 1) + *(nVisSrc + nIndex) + *(nVisSrc + nIndex + 1)
				+ *(nVisSrc + nIndex + nWidth - 1) + *(nVisSrc + nIndex + nWidth);

			nVisImgLow = (unsigned char)(nSum >> 3);
			nVisHf = nDetail * (*(nVisSrc + nIndex) - nVisImgLow);

			nTemp = (short)(nVisHf + *(nIrSrc + nIndex));

			if (nTemp > 235)
				*(nDst + nIndex) = 235;
			else if (nTemp < 16)
				*(nDst + nIndex) = 16;
			else
				*(nDst + nIndex) = (unsigned char)nTemp;
		}
	}
}


void CImgFusion::AffineTrans_old(unsigned char* nDst, int nDstW, unsigned char* nSrc, int nSrcW, int nSrcH)
{
	int i, j, nIndex, nIndexRow;
	double x0, y0;
#ifdef BILSCALINGTYPE
	double fTemp;
	double fDisW, fDisH;
	for (j = nAffineMinH; j < nAffineMaxH; j++)
	{
		nIndexRow = j * nDstW;
		for (i = nAffineMinW; i < nAffineMaxW; i++)
		{
			//计算仿射前对映坐标点
			x0 = *(fTransTable + 2 * (nIndexRow + i));
			y0 = *(fTransTable + 2 * (nIndexRow + i) + 1);
			//要考虑到可能b3和a3是负数，这样的点要排除（对应于平移出界的情况）。
			if (x0 >= 0 && x0 < nSrcW && y0 >= 0 && y0 < nSrcH)
			{
				nIndex = (int)y0 * nSrcW + (int)x0;
				fDisW = x0 - (int)x0;
				fDisH = y0 - (int)y0;
				fTemp = (1 - fDisW) * (1 - fDisH) * (*(nSrc + nIndex)) + fDisW * (1 - fDisH) * (*(nSrc + nIndex + 1)) +
					(1 - fDisW) * fDisH * (*(nSrc + nIndex + nSrcW)) + fDisW * fDisH * (*(nSrc + nIndex + nSrcW + 1));
				*(nDst + nIndexRow + i) = (unsigned char)fTemp;
			}
		}
	}
#else
	for (j = nAffineMinH; j < nAffineMaxH; j++)
	{
		nIndexRow = j * nDstW;
		for (i = nAffineMinW; i < nAffineMaxW; i++)
		{
			//计算仿射前对映坐标点
			x0 = *(m_fTransTable + 2 * (nIndexRow + i));
			y0 = *(m_fTransTable + 2 * (nIndexRow + i) + 1);
			//要考虑到可能b3和a3是负数，这样的点要排除（对应于平移出界的情况）。
			if (x0 >= 0 && x0 < nSrcW && y0 >= 0 && y0 < nSrcH)
			{
				nIndex = (int)y0 * nSrcW + (int)x0;
				*(nDst + nIndexRow + i) = (unsigned char)(*(nSrc + nIndex));
			}
		}
	}
#endif
}

void CImgFusion::VLAndIRFusYUV() {
	//提取可见光亮度分量
	memcpy(m_VisImgGray, sVisImg.nImgData, sVisImg.nWidth * sVisImg.nHeight * sizeof(unsigned char));
	memset(m_VisAffineImgGray, 0, sIrImg.nWidth * sIrImg.nHeight * sizeof(unsigned char));
	//可见光图像仿射变换
	m_VisImgGrayMat = cv::Mat(sVisImg.nHeight, sVisImg.nWidth, CV_8UC1, m_VisImgGray);
	cv::GaussianBlur(m_VisImgGrayMat, m_VisImgGrayMat, cv::Size(3, 3), 0, 0); // 3x3卷积核
	m_VisAffineImgGrayMat = cv::Mat(sIrImg.nHeight, sIrImg.nWidth, CV_8UC1, m_VisAffineImgGray);
	m_IrImgMat = cv::Mat(sIrImg.nHeight, sIrImg.nWidth, CV_8UC1, sIrImg.nImgData);
	cv::warpAffine(m_VisImgGrayMat, m_VisAffineImgGrayMat, warp_mat, m_VisAffineImgGrayMat.size(), cv::INTER_LANCZOS4);
	m_VisAffineImgGrayMat.copyTo(m_VisImgGrayLowMat);
	cv::blur(m_VisAffineImgGrayMat, m_VisImgGrayLowMat, cv::Size(5, 5)); // 3x3卷积核
	m_VisImgGrayMat = nFusDetail * (m_VisAffineImgGrayMat - m_VisImgGrayLowMat) + m_IrImgMat;
	memcpy(sFusImg.nImgData, sIrImg.nImgData, sIrImg.nLen * sizeof(unsigned char));
	memcpy(sFusImg.nImgData, m_VisImgGrayMat.data, sIrImg.nWidth * sIrImg.nHeight * sizeof(unsigned char));
}

void CImgFusion::PutFusImgData(unsigned char* mFusDst) {
	memcpy(mFusDst, sFusImg.nImgData, sFusImg.nLen * sizeof(unsigned char));
}

void CImgFusion::RefreshAffineMat(double distance) {
	int index = (int)round((distance - m_min_distance) / m_step);
	int max_index = (int)round((m_max_distance - m_min_distance) / m_step);
	if (index > max_index) {
		index = max_index;
	}
	else if (index <= 0) {
		index = 0;
	}
	warp_mat.at<double>(0, 2) = m_xi[index] + m_X_offset;
	warp_mat.at<double>(1, 2) = m_yi[index] + m_Y_offset;
}

void CImgFusion::SetXOffset(double x_offset) {
	m_X_offset = x_offset;
}

void CImgFusion::SetYOffset(double y_offset) {
	m_Y_offset = y_offset;
}

double CImgFusion::GetXOffset() {
	return m_X_offset;
}


double CImgFusion::GetYOffset() {
	return m_Y_offset;
}


void CImgFusion::SetAffineMat(double distance) {
	double step = 0.1;
	int index = (int)round((distance - 0.1) / step);
	if (index > 199)
		index = 199;
	//根据新仿射矩阵更新仿射查找表
	int i, j;
	int nSrcW = sVisImg.nWidth;
	int nSrcH = sVisImg.nHeight;
	int nDstW = sIrImg.nWidth;
	int nDstH = sIrImg.nHeight;

	////设置仿射矩阵新值，根据距离换算而来
	double a1 = matrix[0];
	double a2 = matrix[1];
	double a3 = xi[index];
	double b1 = matrix[2];
	double b2 = matrix[3];
	double b3 = yi[index];

	//计算仿射变换后图像的坐标范围
	//int xmin, xmax, ymin, ymax;
	nAffineMaxW = (int)round(gdmax(gdmax(gdmax(0, a2 * (nSrcH - 1)), a1 * (nSrcW - 1)), a1 * (nSrcW - 1) + a2 * (nSrcH - 1)) + a3 + 0.5);
	nAffineMinW = (int)round(gdmin(gdmin(gdmin(0, a2 * (nSrcH - 1)), a1 * (nSrcW - 1)), a1 * (nSrcW - 1) + a2 * (nSrcH - 1)) + a3 + 0.5);
	nAffineMaxH = (int)round(gdmax(gdmax(gdmax(0, b2 * (nSrcH - 1)), b1 * (nSrcW - 1)), b1 * (nSrcW - 1) + b2 * (nSrcH - 1)) + b3 + 0.5);
	nAffineMinH = (int)round(gdmin(gdmin(gdmin(0, b2 * (nSrcH - 1)), b1 * (nSrcW - 1)), b1 * (nSrcW - 1) + b2 * (nSrcH - 1)) + b3 + 0.5);

	if (nAffineMinW < 0)
		nAffineMinW = 0;
	if (nAffineMaxW > nDstW)
		nAffineMaxW = nDstW;
	if (nAffineMinH < 0)
		nAffineMinH = 0;
	if (nAffineMaxH > nDstH)
		nAffineMaxH = nDstH;
	double AInverse[3][3] = { 0 };
	double ADet_old = a1 * b2 - b1 * a2;
	AInverse[0][0] = b2 / ADet_old;
	AInverse[1][1] = a1 / ADet_old;
	AInverse[0][1] = -b1 / ADet_old;
	AInverse[1][0] = -a2 / ADet_old;
	AInverse[2][0] = round((a2 * b3 - b2 * a3) / ADet_old);
	AInverse[2][1] = round((b1 * a3 - a1 * b3) / ADet_old);
	AInverse[0][2] = 0;
	AInverse[1][2] = 0;
	AInverse[2][2] = 1;
	memset(m_fTransTable, 0, sIrImg.nWidth * sIrImg.nHeight * 2 * sizeof(double));
	for (j = nAffineMinH; j < nAffineMaxH; j++)
	{
		for (i = nAffineMinW; i < nAffineMaxW; i++)
		{
			//计算仿射前对映坐标点
			m_fTransTable[2 * (j * nDstW + i) + 0] = AInverse[0][0] * i + AInverse[1][0] * j + AInverse[2][0];
			m_fTransTable[2 * (j * nDstW + i) + 1] = AInverse[0][1] * i + AInverse[1][1] * j + AInverse[2][1];
		}
	}
}

int CImgFusion::DrawBiggerCycle(unsigned char* ir_yuv, unsigned char* fus_yuv, int n_ir_height, int n_ir_width)
{
	cv::Mat mat;
	
	return 1;
}

//double转string
std::string CImgFusion::ConvertToString(double d)
{
	std::ostringstream os;
	if (os << d)
		return os.str();
	return "invalid conversion";
}

//string to double
double CImgFusion::ConvertFromString(std::string str)
{
	std::istringstream iss(str);
	double x;
	if (iss >> x)
		return x;
	return 0.0;
}

void CImgFusion::SetDetectParams(ITA_IMAGE_TYPE imgform,int k) {
	params.filterByCircularity = true;
	////斑点的最小圆度
	params.minCircularity = 0.8f;
	params.filterByArea = true;
	params.filterByColor = true;	//斑点颜色的限制变量
	params.blobColor = 0;			//255表示只提取白色斑点，0表示只提取黑色斑点
	params.minThreshold = 20;
	params.maxThreshold = 200;

	if (sIrImg.nWidth == 1280 || sIrImg.nWidth == 1024) {
		if (imgform == ITA_VL) {
			if (k >= 2) {
				params.maxArea = 10000;
				params.minArea = 300;
			}
			if (k >= 6) {
				params.maxArea = 10000;
				params.minArea = 60;
			}
		}
		else {
			params.maxArea = 8000;
			params.minArea = 100;
			if (k >= 2) {
				params.maxArea = 60000;
				params.minArea = 600;
			}
			if (k >= 6) {
				params.maxArea = 20000;
				params.minArea = 200;
			}
		}
	}
	else if (sIrImg.nWidth == 640) {
		if (imgform == ITA_VL) {
			if (m_lensType == ITA_ANGLE_25 || m_lensType == ITA_ANGLE_45) {
				params.maxArea = 8000;
				params.minArea = 150;
				if (k >= 3) {
					params.minArea = 80;
				}
			}
			else if (m_lensType == ITA_ANGLE_15) {
				params.maxArea = 1000;
				params.minArea = 20;
				if (k >= 2) {
					params.minArea = 80;
				}
			}
			else if (m_lensType == ITA_ANGLE_6) {
				params.maxArea = 1000;
				params.minArea = 30;
			}
		}
		else {
			params.maxArea = 80000;
			params.minArea = 400;
			if (m_lensType == ITA_ANGLE_25) {
				if (k >= 3) {
					params.minArea = 300;
				}
			}
			else if (m_lensType == ITA_ANGLE_45) {
				if (k >= 3) {
					params.minArea = 100;
				}
			}
			else if (m_lensType == ITA_ANGLE_15) {
				if (k < 2) {
					params.minArea = 80;
				}
			}
		}
	}
	else if (sIrImg.nWidth == 480 || sIrImg.nWidth == 384) {
		if (imgform == ITA_VL) {
			if (m_lensType == ITA_ANGLE_25 || m_lensType == ITA_ANGLE_45) {
				params.maxArea = 8000;
				params.minArea = 100;
				if (k >= 3) {
					params.minArea = 80;
				}
			}
			else if (m_lensType == ITA_ANGLE_15) {
				params.maxArea = 1000;
				params.minArea = 20;
				if (k >= 2) {
					params.minArea = 80;
				}
			}
			else if (m_lensType == ITA_ANGLE_15) {
				params.maxArea = 1000;
				params.minArea = 30;
			}
		}
		else {
			params.maxArea = 8000;
			params.minArea = 120;
			if (k >= 5) {
				if (m_lensType == ITA_ANGLE_25) {
					params.minArea = 40;
				}
				else if (m_lensType == ITA_ANGLE_45) {
					params.minArea = 60;
				}
			}
		}
	}
	else if (sIrImg.nWidth == 320) {
		if (imgform == ITA_VL) {
			if (m_lensType == ITA_ANGLE_25) {
				params.maxArea = 8000;
				params.minArea = 150;
				if (k >= 3) {
					params.minArea = 20;
				}
			}
		}
		else {
			params.maxArea = 80000;
			params.minArea = 400;
			if (m_lensType == ITA_ANGLE_25) {
				if (k >= 3) {
					params.minArea = 80;
				}
			}
		}
	}
}

bool CImgFusion::XMoreThan(const cv::Point2f& p1, const cv::Point2f& p2) {
	return p1.x > p2.x;
}

bool CImgFusion::YLessThan(const cv::Point2f& p1, const cv::Point2f& p2) {
	return p1.y < p2.y;
}

void CImgFusion::SortPoints(std::vector<cv::Point2f>& centerCiclePoints, cv::Size patternSize) {
	std::vector<cv::Point2f> points_temp = centerCiclePoints;
	int size = (int)(patternSize.width * patternSize.height);

	std::sort(points_temp.begin(), points_temp.end(), XMoreThan);
	for (int i = 0; i < size; i += patternSize.width) {
		std::sort(points_temp.begin() + i, points_temp.begin() + i + patternSize.width, YLessThan);
	}
	centerCiclePoints = points_temp;
}

void CImgFusion::GetCenterPoint(std::vector<cv::Point2f>& VL_Points, std::vector<cv::Point2f>& IR_Points, int k) {
	VL_Points = m_vl_centerCiclePointList[k];
	IR_Points = m_ir_centerCiclePointList[k];
}

void CImgFusion::ReverseCalAffineTransTable(int n)
{
	m_vl_centerCiclePoints = m_vl_centerCiclePointList[n];
	m_ir_centerCiclePoints = m_ir_centerCiclePointList[n];
	double a1, a2, a3, b1, b2, b3;
	a1 = m_matrix[0];
	a2 = m_matrix[1];
	a3 = 0;
	b1 = m_matrix[2];
	b2 = m_matrix[3];
	b3 = 0;

	double fixed_x = 0;
	double fixed_y = 0;
	double len = 65536 * 1024;

	//最小的偏移距离
	for (int k = 0; k < m_vl_centerCiclePoints.size(); ++k) {
		double dis = 0;
		double ak3 = m_ir_centerCiclePoints[k].x - a2 * m_vl_centerCiclePoints[k].y - a1 * m_vl_centerCiclePoints[k].x;
		double bk3 = m_ir_centerCiclePoints[k].y - b2 * m_vl_centerCiclePoints[k].y - b1 * m_vl_centerCiclePoints[k].x;

		for (int m = 0; m < m_vl_centerCiclePoints.size(); ++m) {
			fixed_x = a1 * m_vl_centerCiclePoints[m].x + a2 * m_vl_centerCiclePoints[m].y + ak3;
			fixed_y = b1 * m_vl_centerCiclePoints[m].x + b2 * m_vl_centerCiclePoints[m].y + bk3;
			dis += pow((fixed_x - m_ir_centerCiclePoints[m].x), 2) + pow((fixed_y - m_ir_centerCiclePoints[m].y), 2);
		}
		if (dis < len) {
			len = dis;
			a3 = ak3;
			b3 = bk3;
		}
	}
	m_VISToIRMatrix_all[n][2] = a3;
	m_VISToIRMatrix_all[n][5] = b3;
	SetW_H_Mat(a1, a2, a3, b1, b2, b3);
}

std::vector<double> CImgFusion::GetDistance() {
	if (sIrImg.nWidth == 1280 || sIrImg.nWidth == 1024) {
		if (m_lensType == 1) {
			detectSize = m_dr1280_1024_standard.size();
			return m_dr1280_1024_standard;
		}
		else if (m_lensType == 7) {
			detectSize = m_dr1280_1024_wideangle.size();
			return m_dr1280_1024_wideangle;
		}
		else if (m_lensType == 8) {
			detectSize = m_dr1280_1024_telephoto.size();
			return m_dr1280_1024_telephoto;
		}
		else if (m_lensType == 9) {
			detectSize = m_dr1280_1024_longfocus.size();
			return m_dr1280_1024_longfocus;
		}
	}
	else if (sIrImg.nWidth == 640) {
		if (m_lensType == 1) {
			detectSize = m_dr640_512_standard.size();
			return m_dr640_512_standard;
		}
		else if (m_lensType == 7) {
			detectSize = m_dr640_512_wideangle.size();
			return m_dr640_512_wideangle;
		}
		else if (m_lensType == 8) {
			detectSize = m_dr640_512_telephoto.size();
			return m_dr640_512_telephoto;
		}
		else if (m_lensType == 9) {
			detectSize = m_dr640_512_longfocus.size();
			return m_dr640_512_longfocus;
		}
	}
	else if (sIrImg.nWidth == 480 || sIrImg.nWidth == 384) {
		if (m_lensType == 1) {
			detectSize = m_dr480_360_standard.size();
			return m_dr480_360_standard;
		}
		else if (m_lensType == 7) {
			detectSize = m_dr480_360_wideangle.size();
			return m_dr480_360_wideangle;
		}
		else if (m_lensType == 8) {
			detectSize = m_dr480_360_telephoto.size();
			return m_dr480_360_telephoto;
		}
		else if (m_lensType == 9) {
			detectSize = m_dr480_360_longfocus.size();
			return m_dr480_360_longfocus;
		}
	}
	else if (sIrImg.nWidth == 320) {
		if (m_lensType == 1) {
			detectSize = static_cast<int>(m_dr320_256_standard.size());
			return m_dr320_256_standard;
		}
	}
}

//判断与0的关系，大于0为1，小于0为-1
int CImgFusion::Sign(double x) {
	if (x > 0)
		return 1;
	else if (x < 0)
		return -1;
	else
		return 0;
}

void CImgFusion::Pchipslopes(std::vector<double>& x_axis, std::vector<double>& slopes, std::vector<double>& derivative) {
	int n = static_cast<int>(x_axis.size());
	derivative.resize(n);
	//存储mulnum大于0 的索引
	std::vector<int> k;
	for (int i = 0; i < n - 2; ++i) {
		double mulnum = Sign(slopes[i]) * Sign(slopes[i + 1]);
		if (mulnum > 0)
			k.push_back(i);
	}
	//x坐标的梯度差
	std::vector<double> h(n - 1, 0);
	for (int i = 0; i < n - 1; ++i) {
		h[i] = x_axis[i + 1] - x_axis[i];
	}

	//累积梯度差
	std::vector<double> hs(k.size(), 0);
	for (int i = 0; i < k.size(); ++i) {
		hs[i] = h[k[i]] + h[k[i] + 1];
	}

	std::vector<double> w1(k.size(), 0);
	std::vector<double> w2(k.size(), 0);
	std::vector<double> dmax(k.size(), 0);
	std::vector<double> dmin(k.size(), 0);
	for (int i = 0; i < (int)k.size(); ++i) {
		w1[i] = (h[k[i]] + hs[i]) / (3 * hs[i]);
		w2[i] = (hs[i] + h[k[i] + 1]) / (3 * hs[i]);
		dmax[i] = gdmax(abs(slopes[k[i]]), abs(slopes[k[i] + 1]));
		dmin[i] = gdmin(abs(slopes[k[i]]), abs(slopes[k[i] + 1]));
		derivative[k[i] + 1] = dmin[i] / (w1[i] * (slopes[k[i]] / dmax[i]) + w2[i] * (slopes[k[i] + 1] / dmax[i]));
	}
	derivative[0] = ((2 * h[0] + h[1]) * slopes[0] - h[0] * slopes[1]) / (h[0] + h[1]);
	if (Sign(derivative[0]) != Sign(slopes[0]))
		derivative[0] = 0;
	else if ((Sign(slopes[0]) != Sign(slopes[1])) && (abs(derivative[0]) > abs(3 * slopes[0])))
		derivative[0] = 3 * slopes[0];

	derivative[n - 1] = ((2 * h[n - 2] + h[n - 3]) * slopes[n - 2] - h[n - 2] * slopes[n - 3]) / (h[n - 2] + h[n - 3]);
	if (Sign(derivative[n - 1]) != Sign(slopes[n - 2]))
		derivative[n - 1] = 0;
	else if (Sign(slopes[n - 2]) != Sign(slopes[n - 3]) && ((abs(derivative[n - 1]) > abs(3 * slopes[n - 2]))))
		derivative[n - 1] = 3 * slopes[n - 2];
}

//分段三次赫米特插值生成映射表
void CImgFusion::Interp1HermitePchip(std::vector<double>& x_axis, std::vector<double>& y_axis, std::vector<double>& x_range_data, std::vector<double>& y_range_data) {
	int size_x = static_cast<int>(x_axis.size());
	int size_y = static_cast<int>(y_axis.size());
	//检查输入的数据是否一一对应
	if (size_x != size_y)
		std::cerr << "the length of distance and data must be same!";
	int len = size_x;

	//slope △y/△x,插值处计算点的导数
	std::vector<double> slopes;
	for (int i = 1; i < len; ++i) {
		double slope = (y_axis[i] - y_axis[i - 1]) / (x_axis[i] - x_axis[i - 1]);
		slopes.push_back(slope);
	}

	std::vector<double> derivative;
	Pchipslopes(x_axis, slopes, derivative);

	//range范围内间隔的所有插值点总数
	int size_ydata = (int)x_range_data.size();
	y_range_data.resize(size_ydata);
	double processValue1 = 0, processValue2 = 0, processValue3 = 0, processValue4 = 0;
	//生成插值范围以内各个点值
	for (int i = 0; i < size_ydata; ++i) {
		int xIndex = 0;
		for (int j = 1; j < size_x + 1; ++j) {
			if (x_range_data[i] - x_axis[j - 1] >= 0)
				xIndex = j;
		}
		//std::cout << "xIndex = " << xIndex << std::endl;
		if (xIndex < 1)
			xIndex = 0;
		else if (xIndex >= len)
			xIndex = len - 2;
		else
			xIndex += -1;
		processValue1 = ((1 + 2 * (x_range_data[i] - x_axis[xIndex]) / (x_axis[xIndex + 1] - x_axis[xIndex])) * y_axis[xIndex] + (x_range_data[i] - x_axis[xIndex]) * derivative[xIndex]);
		processValue2 = ((x_range_data[i] - x_axis[xIndex + 1]) / (x_axis[xIndex] - x_axis[xIndex + 1]));
		processValue3 = ((1 + 2 * (x_range_data[i] - x_axis[xIndex + 1]) / (x_axis[xIndex] - x_axis[xIndex + 1])) * y_axis[xIndex + 1] + (x_range_data[i] - x_axis[xIndex + 1]) * derivative[xIndex + 1]);
		processValue4 = ((x_range_data[i] - x_axis[xIndex]) / (x_axis[xIndex + 1]
			- x_axis[xIndex]));
		y_range_data[i] = processValue1 * processValue2 * processValue2 + processValue3 * processValue4 * processValue4;
	}
}

ITA_RESULT CImgFusion::Getinterp1HermitePchip() {
	std::vector<double> distance = GetDistance();
	if (m_VISToIRMatrix_all.size() != detectSize)
	{
		return ITA_ERROR;
	}
	//插值范围
	int num_step = static_cast<int>(floor((m_max_distance - m_min_distance) / m_step) + 1);
	std::vector<double> range(num_step, 0);
	for (int i = 0; i < num_step; ++i) {
		range[i] = m_min_distance + i * m_step;
	}

	for (int i = 0; i < m_VISToIRMatrix_all.size(); ++i) {
		m_x.push_back(m_VISToIRMatrix_all[i][2]);
		m_y.push_back(m_VISToIRMatrix_all[i][5]);
	}

	//计算插值范围内，步长的所有差值
	Interp1HermitePchip(distance, m_x, range, m_xi);
	std::ofstream opt;
	opt.open(m_fileDir + x_name, std::ios::out | std::ios::trunc);
	for (int i = 0; i < num_step; ++i) {
		opt << m_xi[i] << std::endl;
	}
	opt << std::endl;
	opt.close();

	Interp1HermitePchip(distance, m_y, range, m_yi);
	opt.open(m_fileDir + y_name, std::ios::out | std::ios::trunc);
	for (int i = 0; i < num_step; ++i) {
		opt << m_yi[i] << std::endl;
	}
	opt << std::endl;
	opt.close();

	return ITA_OK;
}

ITA_RESULT CImgFusion::DetectPoint(unsigned char* yuv420, int width, int height, ITARectangle rect, int k, ITA_IMAGE_TYPE imgType)
{
	cv::Mat img = cv::Mat(height,width,CV_8UC3,yuv420);
	std::vector<cv::Point2f>::iterator iter;
	if (img.empty())
	{
		printf("mat is nullptr");
		return ITA_ARG_OUT_OF_RANGE;
	}
	int rSt, rEd, cSt, cEd;
	rSt = rect.leftTop.x;
	rEd = rect.rightDown.x;
	cSt = rect.leftTop.y;
	cEd = rect.rightDown.y;
	int winW1, winH1;
	float dnRate1 = 2.0f;
	winW1 = (int)(2 / dnRate1 + 0.5);
	winH1 = (int)(2 / dnRate1 + 0.5);
	cv::Size boardSize(2, 3);
	SetDetectParams(imgType, k);
	detector = cv::SimpleBlobDetector::create(params);
	if (img.channels() == 3) {
		cv::cvtColor(img, img, cv::COLOR_BGR2GRAY);
	}
	img = img(cv::Range(cSt, cEd),cv::Range(rSt, rEd));
	if (imgType == 1) {
		img = 255 - img;
	}
	std::vector<cv::KeyPoint> keypoints;
	detector->detect(img, keypoints);
	if (keypoints.size() != boardSize.area())
	{
		printf("detect failed");
		return ITA_ARG_OUT_OF_RANGE;
	}
	bool found = cv::findCirclesGrid(img, boardSize, centerCiclePoints, cv::CALIB_CB_SYMMETRIC_GRID | cv::CALIB_CB_CLUSTERING, detector);
	if (found == 1)
	{
		cornerSubPix(img, centerCiclePoints, cv::Size(winW1, winH1),
			cv::Size(-1, -1), cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 30, 0.5));
		SortPoints(centerCiclePoints, boardSize);
		
		for (iter = centerCiclePoints.begin(); iter != centerCiclePoints.end(); iter++) {
			iter->x = iter->x + rSt;
			iter->y = iter->y + cSt;
		}

		if (imgType == 0) {
			m_vl_centerCiclePoints = centerCiclePoints;
			m_vl_centerCiclePointList.push_back(centerCiclePoints);
		}
		else {
			m_ir_centerCiclePoints = centerCiclePoints;
			m_ir_centerCiclePointList.push_back(centerCiclePoints);
		}
	}else {
		if (imgType == 0) {
			printf("VLS circle centerPoint extract failed");
			return ITA_ERROR;
		}
		else {
			printf("IR circle centerPoint extract failed");
			return ITA_ERROR;
		}
	}
	return ITA_OK;
}


void CImgFusion::AverageAllMatrix() {
	double a1 = 0, a2 = 0, b1 = 0, b2 = 0;
	int num_matrix = (int)m_VISToIRMatrix_all.size();
	for (int i = 0; i < num_matrix; ++i) {
		a1 += m_VISToIRMatrix_all[i][0];
		a2 += m_VISToIRMatrix_all[i][1];
		b1 += m_VISToIRMatrix_all[i][3];
		b2 += m_VISToIRMatrix_all[i][4];
	}

	m_matrix[0] = a1 / num_matrix;
	m_matrix[1] = a2 / num_matrix;
	m_matrix[2] = b1 / num_matrix;
	m_matrix[3] = b2 / num_matrix;

	//输出平均矩阵
	std::ofstream opt;
	opt.open(m_fileDir + matrix_name, std::ios::out | std::ios::trunc);
	opt << m_matrix[0] << std::endl;
	opt << m_matrix[1] << std::endl;
	opt << m_matrix[2] << std::endl;
	opt << m_matrix[3] << std::endl;
	opt << std::endl;
	opt.close();
}

void CImgFusion::AffineTransTable() {
	//生成仿射变换矩阵
	Get_TransMatrix_With_N_Point(m_vl_centerCiclePoints, m_ir_centerCiclePoints, m_fVISToIRAffineMat);

	double a1, a2, a3, b1, b2, b3;

	a1 = m_fVISToIRAffineMat[0][0];
	a2 = m_fVISToIRAffineMat[0][1];
	a3 = m_fVISToIRAffineMat[0][2];
	b1 = m_fVISToIRAffineMat[1][0];
	b2 = m_fVISToIRAffineMat[1][1];
	b3 = m_fVISToIRAffineMat[1][2];
	m_VISToIRMatrixs.clear();
	for (int m = 0; m < 3; ++m) {
		for (int n = 0; n < 3; ++n) {

			m_VISToIRMatrixs.push_back(m_fVISToIRAffineMat[m][n]);
		}
	}
	m_VISToIRMatrix_all.push_back(m_VISToIRMatrixs);
	SetW_H_Mat(a1, a2, a3, b1, b2, b3);
}

void CImgFusion::Get_TransMatrix_With_N_Point(std::vector<cv::Point2f> VISPoint, std::vector<cv::Point2f> IRPoint, double VISToIRMatrix[3][3]) {
	int i, j, k;
	int nPointNum = (int)VISPoint.size();

	std::vector<cv::Point2f> uv;
	std::vector<cv::Point2f> xy;
	std::vector<std::vector<double>> normMatrix1, normMatrix2, normMatrix2inv;
	normMatrix1.resize(3);
	normMatrix2.resize(3);
	normMatrix2inv.resize(3);
	for (i = 0; i < 3; i++) {
		normMatrix1[i].resize(3);
		normMatrix1[i][0] = 0;
		normMatrix1[i][1] = 0;
		normMatrix1[i][2] = 0;

		normMatrix2[i].resize(3);
		normMatrix2[i][0] = 0;
		normMatrix2[i][1] = 0;
		normMatrix2[i][2] = 0;

		normMatrix2inv[i].resize(3);
		normMatrix2inv[i][0] = 0;
		normMatrix2inv[i][1] = 0;
		normMatrix2inv[i][2] = 0;
	}
	uv = NormControlPoints(VISPoint, normMatrix1);
	xy = NormControlPoints(IRPoint, normMatrix2);


	GetMatrixInverse_3x3(normMatrix2inv, normMatrix2);

	//最少传入3个不共线的点
	int minRequiredNonCollinearPairs = 3;
	std::vector<std::vector<double>> X1_, X1_Inver;
	X1_.resize(VISPoint.size());
	for (i = 0; i < VISPoint.size(); i++) {
		X1_[i].resize(3);
		X1_[i][0] = xy[i].x;
		X1_[i][1] = xy[i].y;
		X1_[i][2] = 1;
	}
	std::vector<std::vector<double>> Tinv1;
	Tinv1.resize(3);
	for (i = 0; i < 3; i++) {
		Tinv1[i].resize(3);
		Tinv1[i][0] = 0;
		Tinv1[i][1] = 0;
		Tinv1[i][2] = 0;
	}

	std::vector<cv::Point2f> U = uv;

	std::vector<std::vector<double>> A1;
	A1.resize(5);
	for (i = 0; i < 5; i++) {
		A1[i].resize(3);
		A1[i][0] = (double)xy[i].x;
		A1[i][1] = (double)xy[i].y;
		A1[i][2] = (double)1;
	}
	std::vector<std::vector<double>> U1;
	std::vector<std::vector<double>> S1;
	std::vector<std::vector<double>> V1;
	svd_self(A1, U1, S1, V1);
	int rank_S = 0;
	for (i = 0; i < U1.size(); i++) {
		if (U1[i][0] != 0) {
			rank_S++;
		}
	}

	if (rank_S < minRequiredNonCollinearPairs) {
		return;
	}
	else {
		GetGeneralizedInverse(X1_Inver, X1_);

		for (i = 0; i < 3; i++) {
			for (j = 0; j < 2; j++) {
				for (k = 0; k < nPointNum; k++) {
					if (j == 0) {
						Tinv1[i][j] += X1_Inver[i][k] * U[k].x;
					}
					else {
						Tinv1[i][j] += X1_Inver[i][k] * U[k].y;
					}
				}
			}
		}
		Tinv1[0][2] = 0;
		Tinv1[1][2] = 0;
		Tinv1[2][2] = 1;

		std::vector<std::vector<double>> result0;
		MatMulti(result0, Tinv1, normMatrix1);
		MatMulti(Tinv1, normMatrix2inv, result0);

		std::vector<std::vector<double>> T1;
		T1.resize(3);
		for (i = 0; i < 3; i++) {
			T1[i].resize(3);
			T1[i][0] = 0;
			T1[i][1] = 0;
			T1[i][2] = 0;
		}
		GetMatrixInverse_3x3(T1, Tinv1);

		for (i = 0; i < 3; i++) {
			for (j = 0; j < 2; j++) {
				VISToIRMatrix[j][i] = T1[i][j];
			}
		}
		VISToIRMatrix[2][0] = 0;
		VISToIRMatrix[2][1] = 0;
		VISToIRMatrix[2][2] = 1;
	}
}

int CImgFusion::MatMulti(std::vector<std::vector<double>>& fResult, std::vector<std::vector<double>> A, std::vector<std::vector<double>> B) {
	int i, j, k;
	fResult.resize(A.size());
	for (i = 0; i < A.size(); i++) {
		fResult[i].resize(B[0].size());
		for (j = 0; j < B[0].size(); j++) {
			fResult[i][j] = 0;
		}
	}

	//第一个矩阵的列数与第二个矩阵的行数相等时，两个矩阵才能相乘；
	if (A[0].size() == B.size()) {
		//a的行数
		for (i = 0; i < A.size(); i++) {
			//b的列数
			for (j = 0; j < B[0].size(); j++) {
				//a的列数
				for (k = 0; k < A[0].size(); k++) {
					fResult[i][j] += A[i][k] * B[k][j];
				}
			}
		}
		return 0;
	}
	else {
		return 1;
	}
}

void CImgFusion::GetGeneralizedInverse(std::vector<std::vector<double>>& A1Inver, std::vector<std::vector<double>> A1) {
	std::vector<std::vector<double>> U1;
	std::vector<std::vector<double>> S1;
	std::vector<std::vector<double>> V1;

	svd_self(A1, U1, S1, V1);

	std::vector<double> U1_plus;
	U1_plus.resize(U1.size());
	for (int i = 0; i < U1_plus.size(); i++) {
		U1_plus[i] = (1 / U1[i][0]) * (1 / U1[i][0]);
	}

	int h, w;
	w = int(A1.size());
	h = int(A1[1].size());
	A1Inver.resize(h);
	for (int i = 0; i < h; i++) {
		A1Inver[i].resize(w);
		for (int j = 0; j < w; j++)
			A1Inver[i][j] = 0;
	}

	int K = int(V1.size());
	std::vector<std::vector<double>> temp, temp_mp;
	int tempw = gdmin(w, h);
	for (int i = 0; i < K; i++) {
		//double temp[3][3] = { 0 };
		temp.resize(tempw);
		for (int m = 0; m < tempw; m++) {
			temp[m].resize(tempw);
			for (int n = 0; n < tempw; n++)
				temp[m][n] = 0;
		}

		for (int m = 0; m < tempw; m++) {
			for (int n = 0; n < tempw; n++)
			{
				temp[n][m] = V1[i][m] * V1[i][n] * U1_plus[i];
			}
		}

		double ftemp = 0;
		temp_mp.resize(h);
		for (int m = 0; m < h; m++) {
			temp_mp[m].resize(w);
			for (int n = 0; n < w; n++)
				temp_mp[m][n] = 0;
		}

		for (int m = 0; m < h; m++) {
			for (int n = 0; n < w; n++) {
				ftemp = 0;
				for (int num = 0; num < tempw; num++) {
					ftemp = ftemp + temp[m][num] * A1[n][num];
				}
				temp_mp[m][n] = ftemp;
				A1Inver[m][n] += temp_mp[m][n];
			}
		}
	}
}


int CImgFusion::svd_self(const std::vector<std::vector<double>> matSrc,
	std::vector<std::vector<double>>& matD, std::vector<std::vector<double>>& matU, std::vector<std::vector<double>>& matVt) {
	int m = int(matSrc.size());
	int n = int(matSrc[0].size());
	for (const auto& sz : matSrc) {
		if (n != sz.size()) {
			fprintf(stderr, "matrix dimension dismatch\n");
			return -1;
		}
	}

	bool at = false;
	if (m < n) {
		std::swap(m, n);
		at = true;
	}

	matD.resize(n);
	for (int i = 0; i < n; ++i) {
		matD[i].resize(1, (double)0);
	}
	matU.resize(m);
	for (int i = 0; i < m; ++i) {
		matU[i].resize(m, (double)0);
	}
	matVt.resize(n);
	for (int i = 0; i < n; ++i) {
		matVt[i].resize(n, (double)0);
	}
	std::vector<std::vector<double>> tmp_u = matU, tmp_v = matVt;

	std::vector<std::vector<double>> tmp_a, tmp_a_;
	if (!at)
		transpose(matSrc, tmp_a);
	else
		tmp_a = matSrc;

	if (m == n) {
		tmp_a_ = tmp_a;
	}
	else {
		tmp_a_.resize(m);
		for (int i = 0; i < m; ++i) {
			tmp_a_[i].resize(m, (double)0);
		}
		for (int i = 0; i < n; ++i) {
			tmp_a_[i].assign(tmp_a[i].begin(), tmp_a[i].end());
		}
	}
	JacobiSVD(tmp_a_, matD, tmp_v);

	if (!at) {
		transpose(tmp_a_, matU);
		matVt = tmp_v;
	}
	else {
		transpose(tmp_v, matU);
		matVt = tmp_a_;
	}

	return 0;
}

template<typename _Tp>
inline _Tp CImgFusion::hypot_(_Tp a, _Tp b) {
	a = std::abs(a);
	b = std::abs(b);
	if (a > b) {
		b /= a;
		return a * std::sqrt(1 + b * b);
	}
	if (b > 0) {
		a /= b;
		return b * std::sqrt(1 + a * a);
	}
	return 0;
}

template<typename _Tp>
void CImgFusion::JacobiSVD(std::vector<std::vector<_Tp>>& At,
	std::vector<std::vector<_Tp>>& _W, std::vector<std::vector<_Tp>>& Vt) {
	double minval = FLT_MIN;
	_Tp eps = (_Tp)(FLT_EPSILON * 2);
	const int m = (int)(At[0].size());
	const int n = (int)(_W.size());

	const int n1 = (int)(m); // urows
	std::vector<double> W(n, 0.);

	for (int i = 0; i < n; i++) {
		double sd{ 0. };
		for (int k = 0; k < m; k++) {
			_Tp t = At[i][k];
			sd += (double)t * t;
		}
		W[i] = sd;

		for (int k = 0; k < n; k++)
			Vt[i][k] = 0;
		Vt[i][i] = 1;
	}

	int max_iter = gdmax(m, 30);
	for (int iter = 0; iter < max_iter; iter++) {
		bool changed = false;
		_Tp c, s;

		for (int i = 0; i < n - 1; i++) {
			for (int j = i + 1; j < n; j++) {
				_Tp* Ai = At[i].data(), * Aj = At[j].data();
				double a = W[i], p = 0, b = W[j];

				for (int k = 0; k < m; k++)
					p += (double)Ai[k] * Aj[k];

				if (std::abs(p) <= eps * std::sqrt((double)a * b))
					continue;

				p *= 2;
				double beta = a - b;
				double gamma = hypot_((double)p, beta);
				if (beta < 0) {
					double delta = (gamma - beta) * 0.5;
					s = (_Tp)std::sqrt(delta / gamma);
					c = (_Tp)(p / (gamma * s * 2));
				}
				else {
					c = (_Tp)std::sqrt((gamma + beta) / (gamma * 2));
					s = (_Tp)(p / (gamma * c * 2));
				}

				a = b = 0;
				for (int k = 0; k < m; k++) {
					_Tp t0 = c * Ai[k] + s * Aj[k];
					_Tp t1 = -s * Ai[k] + c * Aj[k];
					Ai[k] = t0; Aj[k] = t1;

					a += (double)t0 * t0; b += (double)t1 * t1;
				}
				W[i] = a; W[j] = b;

				changed = true;

				_Tp* Vi = Vt[i].data(), * Vj = Vt[j].data();

				for (int k = 0; k < n; k++) {
					_Tp t0 = c * Vi[k] + s * Vj[k];
					_Tp t1 = -s * Vi[k] + c * Vj[k];
					Vi[k] = t0;
					Vj[k] = t1;
				}
			}
		}

		if (!changed)
			break;
	}

	for (int i = 0; i < n; i++) {
		double sd{ 0. };
		for (int k = 0; k < m; k++) {
			_Tp t = At[i][k];
			sd += (double)t * t;
		}
		W[i] = std::sqrt(sd);
	}

	for (int i = 0; i < n - 1; i++) {
		int j = i;
		for (int k = i + 1; k < n; k++) {
			if (W[j] < W[k])
				j = k;
		}
		if (i != j) {
			std::swap(W[i], W[j]);

			for (int k = 0; k < m; k++)
				std::swap(At[i][k], At[j][k]);

			for (int k = 0; k < n; k++)
				std::swap(Vt[i][k], Vt[j][k]);
		}
	}

	for (int i = 0; i < n; i++)
		_W[i][0] = (_Tp)W[i];

	//srand(time(nullptr));

	for (int i = 0; i < n1; i++) {
		double sd = i < n ? W[i] : 0;

		for (int ii = 0; ii < 100 && sd <= minval; ii++) {
			const _Tp val0 = (_Tp)(1. / m);
			for (int k = 0; k < m; k++) {
				unsigned int rng = rand() % 4294967295; // 2^32 - 1
				_Tp val = (rng & 256) != 0 ? val0 : -val0;
				At[i][k] = val;
			}
			for (int iter = 0; iter < 2; iter++) {
				for (int j = 0; j < i; j++) {
					sd = 0;
					for (int k = 0; k < m; k++)
						sd += At[i][k] * At[j][k];
					_Tp asum = 0;
					for (int k = 0; k < m; k++) {
						_Tp t = (_Tp)(At[i][k] - sd * At[j][k]);
						At[i][k] = t;
						asum += std::abs(t);
					}
					asum = asum > eps * 100 ? 1 / asum : 0;
					for (int k = 0; k < m; k++)
						At[i][k] *= asum;
				}
			}

			sd = 0;
			for (int k = 0; k < m; k++) {
				_Tp t = At[i][k];
				sd += (double)t * t;
			}
			sd = std::sqrt(sd);
		}

		_Tp s = (_Tp)(sd > minval ? 1 / sd : 0.);
		for (int k = 0; k < m; k++)
			At[i][k] *= s;
	}
}

template<typename _Tp>
int CImgFusion::transpose(const std::vector<std::vector<_Tp>>& src, std::vector<std::vector<_Tp>>& dst) {
	int m = (int)(src.size());
	int n = (int)(src[0].size());

	dst.resize(n);
	for (int i = 0; i < n; ++i) {
		dst[i].resize(m);
	}

	for (int y = 0; y < n; ++y) {
		for (int x = 0; x < m; ++x) {
			dst[y][x] = src[x][y];
		}
	}
	return 0;
}

void CImgFusion::GetMatrixInverse_3x3(std::vector<std::vector<double>>& fDst, std::vector<std::vector<double>> fSrc) {
	double a1 = fSrc[0][0];
	double a2 = fSrc[0][1];
	double a3 = fSrc[0][2];
	double b1 = fSrc[1][0];
	double b2 = fSrc[1][1];
	double b3 = fSrc[1][2];
	double c1 = fSrc[2][0];
	double c2 = fSrc[2][1];
	double c3 = fSrc[2][2];

	double fADet = a1 * b2 - b1 * a2;
	if (fADet != 0) {
		fDst[0][0] = double((b2 * c3 - b3 * c2) / fADet);
		fDst[0][1] = double(-(a2 * c3 - a3 * c2) / fADet);
		fDst[0][2] = double((a2 * b3 - b2 * a3) / fADet);
		fDst[1][0] = double(-(b1 * c3 - c1 * b3) / fADet);
		fDst[1][1] = double((a1 * c3 - a3 * c1) / fADet);
		fDst[1][2] = double((b1 * a3 - a1 * b3) / fADet);
		fDst[2][0] = double((b1 * c2 - b2 * c1) / fADet);
		fDst[2][1] = double(-(a1 * c2 - a2 * c1) / fADet);
		fDst[2][2] = double((a1 * b2 - a2 * b1) / fADet);
	}
	else {
		//不存在逆矩阵
		fDst[0][0] = fDst[1][1] = fDst[2][2] = fDst[1][0] = fDst[0][1] = fDst[0][2] = fDst[1][2] = fDst[2][0] = fDst[2][1] = 0;
	}
}

std::vector<cv::Point2f> CImgFusion::NormControlPoints(std::vector<cv::Point2f> SelectedPoint, std::vector<std::vector<double>>& NormMatrixInv) {
	int i;
	//计算所选点的中心点
	cv::Point2f gdCenter = { 0 };
	for (i = 0; i < SelectedPoint.size(); i++) {
		gdCenter.x += SelectedPoint[i].x;
		gdCenter.y += SelectedPoint[i].y;
	}
	gdCenter.x /= SelectedPoint.size();
	gdCenter.y /= SelectedPoint.size();

	//计算相对中心的偏移量
	std::vector<cv::Point2f> gdPtsnorm(SelectedPoint.size());
	for (i = 0; i < SelectedPoint.size(); i++) {
		gdPtsnorm[i].x = SelectedPoint[i].x - gdCenter.x;
		gdPtsnorm[i].y = SelectedPoint[i].y - gdCenter.y;
	}

	//求解标准逆矩阵
	double sumOfPointDistancesFromOriginSquared = 0;
	for (i = 0; i < SelectedPoint.size(); i++) {
		sumOfPointDistancesFromOriginSquared += (gdPtsnorm[i].x * gdPtsnorm[i].x + gdPtsnorm[i].y * gdPtsnorm[i].y);
	}
	double fScaleFactor = sqrt(2 * SelectedPoint.size()) / sqrt(sumOfPointDistancesFromOriginSquared);

	for (i = 0; i < SelectedPoint.size(); i++) {
		gdPtsnorm[i].x *= (float)fScaleFactor;
		gdPtsnorm[i].y *= (float)fScaleFactor;
	}

	NormMatrixInv[0][0] = 1 / fScaleFactor;
	NormMatrixInv[1][1] = 1 / fScaleFactor;
	NormMatrixInv[2][0] = gdCenter.x;
	NormMatrixInv[2][1] = gdCenter.y;
	NormMatrixInv[2][2] = 1;
	return gdPtsnorm;
}

void CImgFusion::SetW_H_Mat(double a1, double a2, double a3, double b1, double b2, double b3) {
	int i, j;
	int nSrcW = sVisImg.nWidth;
	int nSrcH = sVisImg.nHeight;
	int nDstW = sIrImg.nWidth;
	int nDstH = sIrImg.nHeight;
	//计算仿射变换后图像的坐标范围
	//int xmin, xmax, ymin, ymax;
	m_nAffineMaxW = (int)round(gdmax(gdmax(gdmax(0, a2 * (nSrcH - 1)), a1 * (nSrcW - 1)), a1 * (nSrcW - 1) + a2 * (nSrcH - 1)) + a3 + 0.5);
	m_nAffineMinW = (int)round(gdmin(gdmin(gdmin(0, a2 * (nSrcH - 1)), a1 * (nSrcW - 1)), a1 * (nSrcW - 1) + a2 * (nSrcH - 1)) + a3 + 0.5);
	m_nAffineMaxH = (int)round(gdmax(gdmax(gdmax(0, b2 * (nSrcH - 1)), b1 * (nSrcW - 1)), b1 * (nSrcW - 1) + b2 * (nSrcH - 1)) + b3 + 0.5);
	m_nAffineMinH = (int)round(gdmin(gdmin(gdmin(0, b2 * (nSrcH - 1)), b1 * (nSrcW - 1)), b1 * (nSrcW - 1) + b2 * (nSrcH - 1)) + b3 + 0.5);

	if (m_nAffineMinW < 0)
		m_nAffineMinW = 0;
	if (m_nAffineMaxW > nDstW)
		m_nAffineMaxW = nDstW;
	if (m_nAffineMinH < 0)
		m_nAffineMinH = 0;
	if (m_nAffineMaxH > nDstH)
		m_nAffineMaxH = nDstH;
	double ADet_old = a1 * b2 - b1 * a2;
	AInverse[0][0] = b2 / ADet_old;
	AInverse[1][1] = a1 / ADet_old;
	AInverse[0][1] = -b1 / ADet_old;
	AInverse[1][0] = -a2 / ADet_old;
	AInverse[2][0] = round((a2 * b3 - b2 * a3) / ADet_old);
	AInverse[2][1] = round((b1 * a3 - a1 * b3) / ADet_old);
	AInverse[0][2] = 0;
	AInverse[1][2] = 0;
	AInverse[2][2] = 1;
	memset(m_fTransTable, 0, sIrImg.nWidth * sIrImg.nHeight * 2 * sizeof(double));
	for (j = m_nAffineMinH; j < m_nAffineMaxH; j++)
	{
		for (i = m_nAffineMinW; i < m_nAffineMaxW; i++)
		{
			//计算仿射前对映坐标点
			m_fTransTable[2 * (j * nDstW + i) + 0] = AInverse[0][0] * i + AInverse[1][0] * j + AInverse[2][0];
			m_fTransTable[2 * (j * nDstW + i) + 1] = AInverse[0][1] * i + AInverse[1][1] * j + AInverse[2][1];
		}
	}
}

ITA_RESULT CImgFusion::SetLogger(const char* path)
{
	ITA_RESULT ret = m_debugger.checkPath((const char*)path);
	if (ret == ITA_OK) {
		m_logger->setPath(path);
		m_logger->output(LOG_INFO, "debugger %s",(const char*)path);
		return ITA_OK;
	}
	else {
		m_logger->output(LOG_ERROR, "ITA_DEBUGGING_PATH ret=%d", ret);
		return ITA_ERROR;
	}
}

void CImgFusion::CalculateRect(double distance, ITARectangle* rect) {
	int index = (int)round((distance - m_min_distance) / m_step);
	int max_index = (int)round((m_max_distance - m_min_distance) / m_step);
	if (index > max_index) {
		index = max_index;
	}
	else if (index <= 0) {
		index = 0;
	}
	//根据新仿射矩阵更新仿射查找表
	int i, j;
	//根据新仿射矩阵更新仿射查找表
	int nDstW = sIrImg.nWidth;
	int nDstH = sIrImg.nHeight;
	m_nAffineMinW = 0;
	m_nAffineMaxW = nDstW;
	m_nAffineMinH = 0;
	m_nAffineMaxH = nDstH;
	double a1 = m_matrix[0];
	double a2 = m_matrix[1];
	double b1 = m_matrix[2];
	double b2 = m_matrix[3];
	m_ADet = a1 * b2 - b1 * a2;
	if (m_ADet == 0) {
		m_ADet = 1;
	}
	m_ADet = 1.0 / m_ADet;
	m_fVISToIRAffineMat[0][0] = b2 * m_ADet;
	m_fVISToIRAffineMat[1][1] = a1 * m_ADet;
	m_fVISToIRAffineMat[0][1] = -b1 * m_ADet;
	m_fVISToIRAffineMat[1][0] = -a2 * m_ADet;
	m_fVISToIRAffineMat[0][2] = 0;
	m_fVISToIRAffineMat[1][2] = 0;
	m_fVISToIRAffineMat[2][2] = 1;
	m_fVISToIRAffineMat[2][0] = (int)((m_matrix[1] * (m_yi[index] + m_Y_offset) - m_matrix[3] * (m_xi[index] + m_X_offset)) * m_ADet + 0.5);
	m_fVISToIRAffineMat[2][1] = (int)((m_matrix[2] * (m_xi[index] + m_X_offset) - m_matrix[0] * (m_yi[index] + m_Y_offset)) * m_ADet + 0.5);
	memset(m_fTransTable, 0, sIrImg.nWidth * sIrImg.nHeight * 2 * sizeof(double));
	for (j = m_nAffineMinH; j < m_nAffineMaxH; j++) {
		for (i = m_nAffineMinW; i < m_nAffineMaxW; i++) {
			//计算仿射前对映坐标点
			m_fTransTable[2 * (j * nDstW + i) + 0] = m_fVISToIRAffineMat[0][0] * i + m_fVISToIRAffineMat[1][0] * j + m_fVISToIRAffineMat[2][0];
			m_fTransTable[2 * (j * nDstW + i) + 1] = m_fVISToIRAffineMat[0][1] * i + m_fVISToIRAffineMat[1][1] * j + m_fVISToIRAffineMat[2][1];
		}
	}
	rect->leftTop.x = (int)(*(m_fTransTable));
	rect->leftTop.y = (int)(*(m_fTransTable + 1));
	rect->rightDown.x = (int)(*(m_fTransTable + 2 * ((m_nAffineMaxH - 1) * nDstW + m_nAffineMaxW - 1)));
	rect->rightDown.y = (int)(*(m_fTransTable + 2 * ((m_nAffineMaxH - 1) * nDstW + m_nAffineMaxW - 1) + 1));
}

#endif //IMAGE_FUSION_EDITION