#ifndef IMAGE_FUSION_H
#define IMAGE_FUSION_H
#include "../Base/Config.h"
#include "../ITAFusion.h"
#include"../ITADTD.h"
#ifdef IMAGE_FUSION_EDITION
#include "../Module/GuideLog.h"
#include "../Module/Debugger.h"
#include<cmath>
#include <string.h>
#include <vector>
#include <fstream>
#include <time.h>
#include <iostream>
#include "opencv2/opencv.hpp"
#include "opencv2/calib3d/calib3d.hpp"

struct stImage
{
	unsigned char* nImgData;
	int nWidth;
	int nHeight;
	int nLen;
};
class CImgFusion
{
public:
	//构造函数
	CImgFusion(ITA_PIXEL_FORMAT format, ITA_FIELD_ANGLE lensType, int nIrW, int nIrH, int nVisW, int nVisH, unsigned char detail);
	//析构函数
	~CImgFusion();
public:
	ITA_RESULT ImgFusionInit(const char* fileDir);

	ITA_RESULT CheckDirPath(const char* path);
	//仿射配准
	void AffineTransTable();

	void SetAffineMat(double distance);

	void RefreshAffineMat(double distance);

	void VLAndIRFusYUV();

	//输入待融合红外和可见光图像
	ITA_RESULT GetImgData(unsigned char* mIrSrc, int nIrW, int nIrH, unsigned char* nVisSrc, int nVisW, int nVisH);

	//YUV对齐+融合
	void ImgRegAndFusYUV();

	//输出融合图像
	void PutFusImgData(unsigned char* mFusDst);

	void SetXOffset(double x_offset);
	void SetYOffset(double y_offset);

	double GetXOffset();
	double GetYOffset();

	int DrawBiggerCycle(unsigned char* ir_yuv, unsigned char* fus_yuv, int n_ir_height, int n_ir_width);

	ITA_RESULT DetectPoint(unsigned char* yuv420, int width, int height, ITARectangle  rect, int k, ITA_IMAGE_TYPE imgType);

	std::string ConvertToString(double d);

	double ConvertFromString(std::string str);

	void SetDetectParams(ITA_IMAGE_TYPE imgform, int index);

	void SortPoints(std::vector<cv::Point2f>& centerCiclePoints, cv::Size patternSize);

	static bool XMoreThan(const cv::Point2f& p1, const cv::Point2f& p2);

	static bool YLessThan(const cv::Point2f& p1, const cv::Point2f& p2);

	void AffineTransTable(std::vector<cv::Point2f> VISPoint, std::vector<cv::Point2f> IRPoint);

	void Get_TransMatrix_With_N_Point(std::vector<cv::Point2f> VISPoint, std::vector<cv::Point2f> IRPoint, double VISToIRMatrix[3][3]);

	void SetW_H_Mat(double a1, double a2, double a3, double b1, double b2, double b3);

	std::vector<cv::Point2f> NormControlPoints(std::vector<cv::Point2f> SelectedPoint, std::vector<std::vector<double>>& NormMatrixInv);

	void GetMatrixInverse_3x3(std::vector<std::vector<double>>& fDst, std::vector<std::vector<double>> fSrc);

	void GetGeneralizedInverse(std::vector<std::vector<double>>& A1Inver, std::vector<std::vector<double>> A1);

	void CalculateRect(double distance, ITARectangle* rect);

	int svd_self(const std::vector<std::vector<double>> matSrc,
		std::vector<std::vector<double>>& matD, std::vector<std::vector<double>>& matU, std::vector<std::vector<double>>& matVt);

	int MatMulti(std::vector<std::vector<double>>& fResult, std::vector<std::vector<double>> A, std::vector<std::vector<double>> B);

	template<typename _Tp>
	int transpose(const std::vector<std::vector<_Tp>>& src, std::vector<std::vector<_Tp>>& dst);

	template<typename _Tp>
	static void JacobiSVD(std::vector<std::vector<_Tp>>& At,
		std::vector<std::vector<_Tp>>& _W, std::vector<std::vector<_Tp>>& Vt);

	template<typename _Tp>
	static inline _Tp hypot_(_Tp a, _Tp b);

	// X方向与Y方向上的偏移
	double m_X_offset = 0;
	double m_Y_offset = 0;
	void SetParams();

	void AverageAllMatrix();

	void GetCenterPoint(std::vector<cv::Point2f>& VL_Points, std::vector<cv::Point2f>& IR_Points, int k);

	void ReverseCalAffineTransTable(int n);

	ITA_RESULT Getinterp1HermitePchip();

	std::vector<double> GetDistance();

	void Interp1HermitePchip(std::vector<double>& x_axis, std::vector<double>& y_axis, std::vector<double>& x_range_data, std::vector<double>& y_range_data);

	void Pchipslopes(std::vector<double>& x_axis, std::vector<double>& slopes, std::vector<double>& derivative);

	int Sign(double x);

	ITA_RESULT SetLogger(const char* path);

	std::vector<double> distance_standard_A = { 0.3, 0.6, 1.2, 1.8, 2.4, 3.2, 4.8, 6.4, 9.6 };
	std::vector<double> distance_standard_B = { 0.3, 0.6, 1.2, 1.8, 2.4, 3.2, 4.8, 6.4, 8 };
	std::vector<double> distance_telephoto_A;
	std::vector<double> distance_telephoto_B;

	//标定数据 步频、最近距离、最远距离
	double m_step = 0;
	double m_min_distance = 0;
	double m_max_distance = 0;

	std::vector<double> m_dr1280_1024_standard = { 0.3, 0.6, 1.2, 1.8, 2.4, 3.2, 4.8, 6.4, 9.6 };
	std::vector<double> m_dr1280_1024_wideangle = { 0.3, 0.6, 1.2, 1.8, 2.4, 3.2, 4.8, 6.4, 9.6 };
	std::vector<double> m_dr1280_1024_telephoto = { 3, 3.4, 4, 4.8, 5.6, 6.4, 8, 9.6 };
	std::vector<double> m_dr1280_1024_longfocus = {};

	std::vector<double> m_dr640_512_standard = { 0.3, 0.6, 1.2, 1.8, 2.4, 3.2, 4.8, 6.4, 8 };
	std::vector<double> m_dr640_512_wideangle = { 0.3, 1.2, 1.8, 2.4, 3.2, 4.8, 6.4, 8 };
	std::vector<double> m_dr640_512_telephoto = { 0.6, 1.2, 2.4, 3.2, 4.8, 6.4, 8, 9.6 };
	std::vector<double> m_dr640_512_longfocus = { 4.8, 6.4, 8, 9.6, 11.2, 12.8 };

	std::vector<double> m_dr480_360_standard = { 0.3, 0.6, 1.2, 1.8, 2.4, 3.2, 4.8, 6.4 };
	std::vector<double> m_dr480_360_wideangle = { 0.3, 1.2, 1.8, 2.4, 3.2, 4.8 };
	std::vector<double> m_dr480_360_telephoto = { 0.6, 1.2, 2.4, 3.2, 4.8, 6.4, 8 };
	std::vector<double> m_dr480_360_longfocus = { 4.8, 6.4, 8, 9.6, 11.2 };

	std::vector<double> m_dr384_288_standard = { 0.3, 0.6, 1.2, 1.8, 2.4, 3.2, 4.8, 6.4, 8 };
	std::vector<double> m_dr384_288_wideangle = {};
	std::vector<double> m_dr384_288_telephoto = {};
	std::vector<double> m_dr384_288_longfocus = {};

	std::vector<double> m_dr320_256_standard = { 1.2, 1.5, 1.8, 2.4, 3.2, 4.8, 6.4 };

private:
	double ADet;
	double *m_fTransTable;
	double* m_fTransTable_pip;
	double fVISToIRAffineMat[3][3];
	int nAffineMinW, nAffineMaxW, nAffineMinH, nAffineMaxH;
	//图像融合参数
	stImage sIrImg;
	stImage sVisImg;
	stImage sFusImg;
	unsigned char nFusDetail;
	ITA_PIXEL_FORMAT eFusImgType;

	cv::Mat m_IrImgMat;
	cv::Mat m_VisImgGrayMat;
	cv::Mat m_VisAffineImgGrayMat;
	cv::Mat m_VisImgGrayLowMat;
	//RGB融合指针
	unsigned char* m_VisImgGray;
	unsigned char* nFusImgGray;
	unsigned char* nIrImgYUV;
	unsigned char* m_VisAffineImgGray;
	double m_fVISToIRAffineMat[3][3] = { 0 };
	int m_nAffineMinW, m_nAffineMaxW, m_nAffineMinH, m_nAffineMaxH;
	double AInverse[3][3] = { 0 };
	std::string matrix_name;
	std::string x_name;
	std::string y_name;
	std::string m_fileDir;

	//可见光与红外图像圆心数据
	std::vector<double> m_VISToIRMatrixs;
	std::vector<std::vector<double>> m_VISToIRMatrix_all;
	std::vector<std::vector<cv::Point2f>> m_vl_centerCiclePointList;
	std::vector<std::vector<cv::Point2f>> m_ir_centerCiclePointList;
	ITA_FIELD_ANGLE m_lensType;
	std::vector<cv::Point2f> m_vl_centerCiclePoints;
	std::vector<cv::Point2f> m_ir_centerCiclePoints;
	std::vector<cv::Point2f> centerCiclePoints;

	// 差值后生成的X方向与Y方向上偏移
	std::vector<double> m_xi = { 0 };
	std::vector<double> m_yi = { 0 };
	// 各个距离下X方向与Y方向上的偏移
	std::vector<double> m_x;
	std::vector<double> m_y;

	cv::Mat IR_clone;
	cv::SimpleBlobDetector::Params params;
	cv::Ptr<cv::SimpleBlobDetector> detector;

	std::vector<double> m_matrix = { 2.53072, 0.060606, -0.0596689, 2.53192 };
	int detectSize;

	GuideLog* m_logger;
	Debugger m_debugger;

	cv::Mat warp_mat;
	double m_ADet;
	void ImgFusionY(unsigned char* nDst, unsigned char* nVisSrc, unsigned char* nIrSrc, int nWidth, int nHeight, unsigned char nDetail);

	//根据仿射变换矩阵对可见光图像进行变换
	void AffineTrans_old(unsigned char* nDst, int nDstW, unsigned char* nSrc, int nSrcW, int nSrcH);
	const double matrix[4] = { 2.733121, 0.011588763,  -0.011100178, 2.754214 };
	const double xi[200] = {
		-1014.4774463872, -1017.4200463679, -1020.3873372295, -1023.3790842349, -1026.3950526468, -1029.4350077280, -1032.4987147411,
		-1035.5859389490, -1038.6964456144, -1041.8300000000, -1044.9863673686, -1048.1653129830, -1051.3666021059, -1054.5900000000,
		-1058.1665575562, -1061.9864461158, -1065.3881116175, -1067.7100000000, -1069.0928158522, -1070.1832572127, -1071.0787601946,
		-1071.8767609110, -1072.6746954751, -1073.5700000000, -1074.6066776761, -1075.7293532894, -1076.8863953461, -1078.0261723525,
		-1079.0970528149, -1080.0474052394, -1080.8255981323, -1081.3800000000, -1081.7838479104, -1082.1475873655, -1082.4753995225,
		-1082.7714655389, -1083.0399665717, -1083.2850837783, -1083.5109983160, -1083.7218913420, -1083.9219440135, -1084.1153374879,
		-1084.3062529223, -1084.4988714741, -1084.6973743005, -1084.9059425588, -1085.1287574062, -1085.3700000000, -1085.6290543262,
		-1085.9006379087, -1086.1819322119, -1086.4701187001, -1086.7623788376, -1087.0558940887, -1087.3478459178, -1087.6354157891,
		-1087.9157851671, -1088.1861355159, -1088.4436483000, -1088.6855049836, -1088.9088870311, -1089.1109759068, -1089.2889530750,
		-1089.4400000000, -1089.5750301919, -1089.7068422626, -1089.8354453295, -1089.9608485099, -1090.0830609212, -1090.2020916806,
		-1090.3179499055, -1090.4306447133, -1090.5401852212, -1090.6465805466, -1090.7498398068, -1090.8499721192, -1090.9469866010,
		-1091.0408923696, -1091.1316985424, -1091.2194142366, -1091.3040485695, -1091.3856106586, -1091.4641096211, -1091.5395545743,
		-1091.6119546357, -1091.6813189224, -1091.7476565519, -1091.8109766415, -1091.8712883085, -1091.9286006702, -1091.9829228440,
		-1092.0342639471, -1092.0826330969, -1092.1280394108, -1092.1704920061, -1092.2100000000, -1092.2468490787, -1092.2814172582,
		-1092.3138521503, -1092.3443013671, -1092.3729125206, -1092.3998332229, -1092.4252110858, -1092.4491937215, -1092.4719287418,
		-1092.4935637589, -1092.5142463847, -1092.5341242312, -1092.5533449105, -1092.5720560345, -1092.5904052152, -1092.6085400647,
		-1092.6266081949, -1092.6447572179, -1092.6631347456, -1092.6818883901, -1092.7011657633, -1092.7211144773, -1092.7418821441,
		-1092.7636163756, -1092.7864647839, -1092.8105749810, -1092.8360945789, -1092.8631711895, -1092.8919524249, -1092.9225858972,
		-1092.9552192182, -1092.9900000000, -1093.0272238489, -1093.0670475480, -1093.1094104746, -1093.1542520059, -1093.2015115191,
		-1093.2511283916, -1093.3030420004, -1093.3571917230, -1093.4135169364, -1093.4719570181, -1093.5324513451, -1093.5949392948,
		-1093.6593602443, -1093.7256535711, -1093.7937586521, -1093.8636148649, -1093.9351615865, -1094.0083381942, -1094.0830840652,
		-1094.1593385769, -1094.2370411064, -1094.3161310309, -1094.3965477278, -1094.4782305743, -1094.5611189476, -1094.6451522250,
		-1094.7302697836, -1094.8164110008, -1094.9035152538, -1094.9915219199, -1095.0803703762, -1095.1700000000, -1095.2603501686,
		-1095.3513602592, -1095.4429696490, -1095.5351177154, -1095.6277438354, -1095.7207873865, -1095.8141877458, -1095.9078842905,
		-1096.0018163980, -1096.0959234454, -1096.1901448100, -1096.2844198691, -1096.3786879998, -1096.4728885795, -1096.5669609853,
		-1096.6608445946, -1096.7544787845, -1096.8478029323, -1096.9407564153, -1097.0332786106, -1097.1253088956, -1097.2167866475,
		-1097.3076512435, -1097.3978420608, -1097.4872984768, -1097.5759598686, -1097.6637656135, -1097.7506550887, -1097.8365676715,
		-1097.9214427391, -1098.0052196688, -1098.0878378378, -1098.1692366234, -1098.2493554028, -1098.3281335532, -1098.4055104519,
		-1098.4814254761, -1098.5558180031, -1098.6286274101, -1098.6997930743 };

	const double yi[200] = {
		-1220.9090739222, -1208.9522305530, -1196.4290914948, -1183.4991705717, -1170.3219816073, -1157.0570384255, -1143.8638548500,
		-1130.9019447048, -1118.3308218135, -1106.3100000000, -1094.9989930881, -1084.5573149016, -1075.1444792643, -1066.9200000000,
		-1059.6126218883, -1052.9582881719, -1047.1273103696, -1042.2900000000, -1038.1625548968, -1034.3700060879, -1030.9459148835,
		-1027.9238425940, -1025.3373505294, -1023.2200000000, -1021.4693978134, -1019.9332707765, -1018.5703601983, -1017.3394073876,
		-1016.1991536535, -1015.1083403049, -1014.0257086507, -1012.9100000000, -1011.7625605818, -1010.6149491691, -1009.4712258871,
		-1008.3354508607, -1007.2116842150, -1006.1039860751, -1005.0164165660, -1003.9530358126, -1002.9179039401, -1001.9150810735,
		-1000.9486273379, -1000.0226028582, -999.1410677595, -998.3080821669, -997.5277062054, -996.8040000000, -996.1179078306,
		-995.4482946092, -994.7961014088, -994.1622693027, -993.5477393639, -992.9534526656, -992.3803502810, -991.8293732831,
		-991.3014627451, -990.7975597401, -990.3186053414, -989.8655406219, -989.4393066549, -989.0408445135, -988.6710952708,
		-988.3310000000, -988.0102525631, -987.6978033135, -987.3934780610, -987.0971026155, -986.8085027870, -986.5275043853,
		-986.2539332203, -985.9876151020, -985.7283758402, -985.4760412449, -985.2304371259, -984.9913892932, -984.7587235566,
		-984.5322657261, -984.3118416115, -984.0972770228, -983.8883977699, -983.6850296626, -983.4869985108, -983.2941301246,
		-983.1062503137, -982.9231848881, -982.7447596576, -982.5708004322, -982.4011330218, -982.2355832363, -982.0739768855,
		-981.9161397794, -981.7618977280, -981.6110765410, -981.4635020283, -981.3190000000, -981.1784899004, -981.0428316055,
		-980.9117615727, -980.7850162591, -980.6623321221, -980.5434456189, -980.4280932067, -980.3160113429, -980.2069364847,
		-980.1006050893, -979.9967536141, -979.8951185163, -979.7954362531, -979.6974432818, -979.6008760598, -979.5054710441,
		-979.4109646923, -979.3170934613, -979.2235938087, -979.1302021915, -979.0366550671, -978.9426888928, -978.8480401258,
		-978.7524452233, -978.6556406427, -978.5573628412, -978.4573482760, -978.3553334045, -978.2510546839, -978.1442485714,
		-978.0346515244, -977.9220000000, -977.8074176999, -977.6922066415, -977.5763707558, -977.4599139738, -977.3428402265,
		-977.2251534448, -977.1068575597, -976.9879565023, -976.8684542036, -976.7483545945, -976.6276616060, -976.5063791691,
		-976.3845112148, -976.2620616742, -976.1390344781, -976.0154335576, -975.8912628437, -975.7665262674, -975.6412277597,
		-975.5153712515, -975.3889606738, -975.2619999577, -975.1344930342, -975.0064438341, -974.8778562886, -974.7487343286,
		-974.6190818851, -974.4889028891, -974.3582012716, -974.2269809636, -974.0952458961, -973.9630000000, -973.8302472064,
		-973.6969914462, -973.5632366506, -973.4289867503, -973.2942456765, -973.1590173601, -973.0233057321, -972.8871147235,
		-972.7504482654, -972.6133102886, -972.4757047242, -972.3376355032, -972.1991065566, -972.0601218154, -971.9206852105,
		-971.7808006729, -971.6404721337, -971.4997035239, -971.3584987743, -971.2168618161, -971.0747965803, -970.9323069977,
		-970.7893969994, -970.6460705164, -970.5023314797, -970.3581838203, -970.2136314692, -970.0686783573, -969.9233284156,
		-969.7775855753, -969.6314537671, -969.4849369222, -969.3380389716, -969.1907638461, -969.0431154769, -968.8950977948,
		-968.7467147310, -968.5979702164, -968.4488681819, -968.2994125586 };
};

#endif //IMAGE_FUSION_EDITION

#endif // !IMAGE_FUSION_H