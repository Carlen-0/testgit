#ifndef CINFRARED_CORE_H
#define CINFRARED_CORE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../Base/Config.h"
#include "../ERROR.h"
#include "CustomPalette.h"
#include "../Module/GuideLog.h"

//按照嵌入式要求修改代码格式，适用于嵌入式端
#define AlgOpt
/**************初始化气体检测*****/
#define Clip16(x) (x < -32767) ? -32767 : ((x > 32767) ? 32767 : x)//气体检测
/**********************************/
#define ClipU8(x) (x < 0) ? 0 : ((x > 255) ? 255 : x)
#define RGB2Y(R, G, B)  ( 16  + 0.183f * (R) + 0.614f * (G) + 0.062f * (B) )
#define RGB2U(R, G, B)  ( 128 - 0.101f * (R) - 0.339f * (G) + 0.439f * (B) )
#define RGB2V(R, G, B)  ( 128 + 0.439f * (R) - 0.399f * (G) - 0.040f * (B) )
#define YUV2R(Y, U, V) ( 1.164f *((Y) - 16) + 1.792f * ((V) - 128) )
#define YUV2G(Y, U, V) ( 1.164f *((Y) - 16) - 0.213f *((U) - 128) - 0.534f *((V) - 128) )
#define YUV2B(Y, U, V) ( 1.164f *((Y) - 16) + 2.114f *((U) - 128))

#ifndef max_ir
#define max_ir(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min_ir
#define min_ir(a,b)            (((a) < (b)) ? (a) : (b))
#endif

//调光宏定义
#define FRAME_SMOOTH_NUM        (1)//(100)
#define LAST_PARA               (1)
#define GRAYLEVEL_16BIT         (65536) 
#define GRAY_WEIGHT_TABLE_LEN	512
#define TTF_WEIGHT_TABLE_LEN	128

//PseudoColorTable0为公司完整伪彩,共26条;
//PseudoColorTable1为目前模组正在使用伪彩，为13条，适用于120模组和256模组。
#ifdef PALETTE_TIMO
#define PseudoColorTable1
#else
#ifdef PALETTE_ALL
#define PseudoColorTable0
#endif // PALETTE_ALL
#endif // PALETTE_TIMO

#ifdef PseudoColorTable0
#define PSEUDONUM (26)
extern const unsigned char  PseudoColorTable[PSEUDONUM][256][4];
extern const unsigned char  PseudoColorTableYUV[PSEUDONUM][256][4];
enum ePSEUDO_NAME
{
	PSEUDO_NAME_WHITEHOT = 0, //白热
	PSEUDO_NAME_FULGURITE, //熔岩
	PSEUDO_NAME_IRONRED, //铁红
	PSEUDO_NAME_HOTIRON, //热铁
	PSEUDO_NAME_MEDICAL, //医疗
	PSEUDO_NAME_ARCTIC, //北极
	PSEUDO_NAME_RAINBOW1, //彩虹1
	PSEUDO_NAME_RAINBOW2, //彩虹2
	PSEUDO_NAME_BLACKHOT, //黑热
	PSEUDO_NAME_BODYDRT, //人体筛查
	PSEUDO_NAME_TINT1, // 描红1
	PSEUDO_NAME_BLUEHOT, // 蓝热
	PSEUDO_NAME_GREENHOT, //绿热
	PSEUDO_NAME_MEDICAL0, //医疗0
	PSEUDO_NAME_MEDICAL1, //医疗1
	PSEUDO_NAME_MEDICAL2, //医疗2
	PSEUDO_NAME_RAINBOW3, //彩虹3
	PSEUDO_NAME_TINT2, //描红2
	PSEUDO_NAME_TINT3, //描红3
	PSEUDO_NAME_ICEFIRE, //冰火
	PSEUDO_NAME_REVERSEICEFIRE, //反冰火
	PSEUDO_NAME_STEPS, //分层
	PSEUDO_NAME_SEPIA, // 深褐
	PSEUDO_NAME_WINTER, //冬季
	PSEUDO_NAME_HIGHLIGHT, //高亮
	PSEUDO_NAME_AMBER, //琥珀
};
#endif

#ifdef PseudoColorTable1
#define PSEUDONUM (13)
extern const unsigned char  PseudoColorTable[PSEUDONUM][256][4];
extern const unsigned char  PseudoColorTableYUV[PSEUDONUM][256][4];
enum ePSEUDO_NAME
{
	PSEUDO_NAME_WHITEHOT = 0, //白热
	PSEUDO_NAME_FULGURITE, //熔岩
	PSEUDO_NAME_IRONRED, //铁红
	PSEUDO_NAME_HOTIRON, //热铁
	PSEUDO_NAME_MEDICAL, //医疗
	PSEUDO_NAME_ARCTIC, //北极
	PSEUDO_NAME_RAINBOW1, //彩虹1
	PSEUDO_NAME_RAINBOW2, //彩虹2
	PSEUDO_NAME_BLACKHOT, //黑热
	PSEUDO_NAME_BODYDRT, //人体筛查
	PSEUDO_NAME_TINT, // 描红
	PSEUDO_NAME_BLUEHOT, // 蓝热
	PSEUDO_NAME_GREENHOT,//绿热
};
#endif

// Dynamic range transform type 动态范围转换类型
enum eDRT_TYPE
{
	DRT_TYPE_LINEAR = 0,         //线性
	DRT_TYPE_PLATHE,        //平台直方图均衡
	DRT_TYPE_MIX_PHE_LIN,   //混合调光
	DRT_TYPE_MANUALTONE, //手动调光
	DRT_TYPE_PERSONMEDICAL, //人体医疗调光，在使用人体医疗色带时算法自动切换到该调光方式，目前不对用户开放。
};

//图像翻转方式
enum eFLIP_TYPE
{// 插值类型
	FlipX = 0,  //水平镜像
	FlipY,       //垂直镜像
	FlipXY      //水平加垂直镜像
};

//图像旋转方式
enum eROTATE_TYPE
{// 插值类型
	ROTATE90 = 0,  //水平镜像
	ROTATE180,       //垂直镜像
	ROTATE270      //水平加垂直镜像
};

//图像放大方式
enum Interpolation
{// 插值类型
	Near = 0,
	Bilinear,
	BilinearFast,
};
enum eGAMMACOR_TYPE
{
	GAMMACOR_TYPE_SINGLE = 0,
	GAMMACOR_TYPE_DOUBLE,
};
//输出图像类型
#define COLORIMGTYPENUM (12)
enum eCOLORIMG_TYPE
{
	COLORIMG_TYPE_BGR = 0,  //BGR格式图像
	COLORIMG_TYPE_RGBA,     //RGBA格式图像
	COLORIMG_TYPE_RGB,      //RGB格式图像
	COLORIMG_TYPE_RGB565,   //RGB565格式图像
	COLORIMG_TYPE_YUV422YUYV,   //YUV422格式YUYV
	COLORIMG_TYPE_YUV422UYVY,   //YUV422格式UYVY
	COLORIMG_TYPE_YUV422Plane,   //YUV422格式YUV422Plane
	COLORIMG_TYPE_YVU422Plane,   //YUV422格式YVU422Plane
	COLORIMG_TYPE_YUV420NV12,   //YUV420格式NV12
	COLORIMG_TYPE_YUV420NV21,   //YUV420格式NV21
	COLORIMG_TYPE_YUV420PLANE,   //YUV420格式YUV420Plane
	COLORIMG_TYPE_YVU420PLANE,   //YUV420格式YVU420Plane
};

// NUC非均匀校正算法参数
struct stNUCPara
{
	bool  b_nuc_switch;       // NUC Switch
};

// NUC非均匀校正算法参数
struct stRPBPPara
{
	bool  b_rpbp_switch;       // 坏点替换 Switch
	bool b_rpbp_updatalist_switch;
	unsigned char b_rpbp_auto_switch;//自动去坏点
};

struct stDRTPara
{
	bool b_drt_switch; //调光开关
	bool b_drt_exterhist_switch;//是否使用外部直方图开关
	int n_drt_type;  //调光类型

	int n_discard_upratio;	      // 上抛点比例
	int n_discard_downratio;	  // 下抛点比例

	int n_linear_contrast;        // 对比度参数	
	int n_linear_brightness;      // 亮度调节参数	
	int n_linear_restrain_rangethre; //均匀面抑制

	int n_heq_midvalue;
	int n_heq_plat_thresh;
	int n_heq_range_max;

	//手动调光参数
	short n_manltone_maxy16;
	short n_manltone_miny16;

	//人体医疗调光参数
	short n_personmedical_y16_base;
	short n_personmedical_y16_high;

	//混合调光比例参数
	int n_mix_ThrHigh;//高阈值
	int n_mix_ThrLow;//低阈值
};

struct stGammaCorPara
{
	bool b_gmc_switch;
	int b_gmc_type;
	float f_gmc_gamma;
};

struct stTimeFilterPara
{
	bool b_tff_switch;               //  算法开启开关
	int n_tff_std;					 //  标准差
	int n_tff_win_size_w;             //  窗口大小
	int n_tff_win_size_h;
};

struct stRSPara
{
	bool  b_rs_switch;           // 去竖条纹算法开关
	int  n_stripe_win_width;     // 去竖条纹滤波窗口宽度
	int  n_stripe_weight_thresh; // 去竖条纹权重噪声
	int  n_stripe_std;           // 去竖条纹方差
	int  n_stripe_detail_thresh;        // 竖条纹判断阈值
};
/**************初始化气体检测*****/
struct stHSMPara
{
	bool  b_hsm_switch;           // 气体算法开关
	int  n_hsm_weight;     // 帧间差值权重
	int  n_hsm_stayThr;     // 静态阈值
	int  n_hsm_stayWeight;     // 静态帧间差值权重

	int n_hsm_interval;//缓冲区域间隔
	bool b_midfilter_switch;//中值滤波开关
	int n_midfilter_winSize;//中值滤波窗口
	int n_delta_upthr;//帧间差值的阈值上限
	int n_delta_downthr;//帧间差值的阈值下限
	int n_point_thr;//单点噪声阈值

	float f_hsm_k;//线性拉伸的K
	float f_hsm_b;//线性拉伸B
};
/*********************************/
// RN（remove noise） 去噪算法参数
enum eRN_TYPE
{
	RN_TYPE_GAUSSIAN = 0,         //高斯
	RN_TYPE_GREYSCALE,        //单边灰度
	RN_TYPE_BILATERAL        //双边滤波
};

// RN（remove noise） 去噪算法参数
struct stRNPara
{
	bool b_rn_switch;	     // 去噪算法开关
	int n_rn_arith_type;     // 去噪算法类型
	int n_dist_std; 	     // 空间高斯滤波标准差
	int n_gray_std; 	     // 单边灰度滤波标准差
	int n_win_size;          // 去噪滤波窗口
};

// 图像镜像
struct stImgFlipPara
{
	bool b_flip_switch;
	int nFlipType;
};

// 图像镜像
struct stImgRotatePara
{
	bool b_rotate_switch;
	int nRotateType;
};


// 锐化sharpen算法参数
struct stSPPara
{
	bool b_sp_switch;        // 锐化开关
	float n_sp_laplace_weight;//锐化系数
};

// IIE图像增强算法参数
struct stIIEPara
{
	bool b_iie_switch;	    // IIE算法Switch
	int n_iie_pos_win_size; 	// 增强高斯滤波窗口大小
	int n_iie_enhance_coef;	// 增强算法细节增益系数
	int n_iie_gauss_std;
	unsigned char n_iie_detail_thr; //细节阈值
};

struct stY8ADJUSTBCPara
{
	bool b_adjustbc_switch;
	int n_adjustbc_bright;
	int n_adjustbc_contrast;
};

// 图像放大
struct stImgZoomPara
{
	float fZoom;
	int n_ZoomType;
};

struct EqualLineMode 
{
	//等温线类型，共四种 EQUAL_LINE_High = 0，EQUAL_LINE_Low = 1，
	//EQUAL_LINE_HighLow = 2，EQUAL_LINE_Middle = 3
	int type;  
	//设置的高Y16
	short highY16; 
	//设置的低Y16
	short lowY16; 
	//设置的等温线颜色 color为四字节int型，
	//r = (color >> 16) & 0xff,g = (color >> 8) & 0xff,b = color & 0xff.
	int color;  
	//设置的等温线另一颜色
	int otherColor; 
};

//定义等温线类型
enum EqualLineType
{
	EQUAL_LINE_High = 0,
	EQUAL_LINE_Low,
	EQUAL_LINE_HighLow,
	EQUAL_LINE_Middle,
};

// 伪彩色
struct stPseudoColorPara
{
	bool b_psdclr_switch;    // 伪彩色开关
	int t_psdclr_type;     // 伪彩色类型
	int n_colorimg_tpye;   //输出图像类型
	int n_colorimg_len;  //输出伪彩图像字节数
	bool b_psdclr_equallinemode; //等温线模式
	EqualLineMode s_equalline_para;
};

//去锅盖算法类型
enum eDGG_TYPE
{
	DGG_TYPE_FIX = 0,  //固定锅盖系数
	DGG_TYPE_VAR1,     //基于开机快门温计算系数
	DGG_TYPE_VAR2,     //基于上一次快门温计算系数
};

struct stDGGTPara
{
	float fStartShutterT;	//开机快门温
	float fLastLastShutterT;//上次快门温
	float fLastShutterT;	//当前快门温
	float fRealShutterT;	//实时快门温
	float fDelta;			//采集模板对应的设备温升
};

struct stDGGPara
{
	bool b_dgg_switch;  	    //去锅盖算法开关
	bool b_cgm_switch;          //得到锅盖模板开关
	int n_dgg_arith_type;       //去锅盖算法类型
	stDGGTPara stDggTPara;
};

//红外图像处理参数
struct stIrPara
{
	stNUCPara st_nuc_para;    //NUC
	stRPBPPara st_rpbp_para;  //坏点替换
	stTimeFilterPara    st_tff_para;       // 时域滤波
	stRSPara    st_rvs_para;       // 去竖条纹
	stRSPara    st_rhs_para;       // 去横条纹
	/**************初始化气体检测*****/
	stHSMPara   st_hsm_para;//气体检测
	/**********************************/
	stRNPara    st_rn_para;        // 空域滤波
	stImgFlipPara st_flip_para;  //图像翻转
	stImgRotatePara st_rotate_para;  //图像旋转
	stIIEPara    st_iie_para;       // IIE增强系数
	stDRTPara  st_drt_para;				// 调光
	stGammaCorPara st_gmc_para;
	stSPPara     st_sp_para;        // 锐化
	stY8ADJUSTBCPara st_y8adjustbc_para;
	stImgZoomPara st_zoom_para;         //图像放大参数
	stPseudoColorPara st_psdclr_coef;    // 伪彩色
	stDGGPara  st_dgg_para;        // 去锅盖算法
};

//红外图像内容
struct stIfrImage
{
	int n_width;
	int n_height;
	int n_len;
	int n_channels;

	short *pus_data;	   //表示什么 
	unsigned char *puc_data;
};

struct stIrContent
{
	stIfrImage pst_src_img;    //输入数据红外图像
	stIfrImage pst_dst_img;    //输出数据红外图像

	unsigned short *pus_shutter_base;  //快门本地
	unsigned short *pus_gain_mat;      //K
	unsigned short *pus_x16_mat;       //X16
};

class CInfraredCore
{
public:
	CInfraredCore();
	CInfraredCore(int w, int h, float fZoom);
	void FinalRelease();
	~CInfraredCore();
	void PseudoColor(unsigned char* puc_dst, unsigned char* puc_src, int height, int width, int pseudo_color_type, int colorimg_tpye);
	void setLogger(GuideLog* logger);
	bool getGainMat(unsigned short* pn_gain_mat, unsigned short* pus_high_base, unsigned short* pus_low_base,int n_width, int n_height);
	void Resize(unsigned char* puc_dst, unsigned char* puc_src, int n_width, int n_height, float fZoom, int ZoomType);
	template<class T>
	void Flip(T pus_dst, T pus_src, int n_width, int n_height, int nfliptype);
	template<class T>
	void Rotation(T pus_dst, T pus_src, int n_width, int n_height, int nrotationtype);
protected:

	//红外图像内容初始化
	void IRContentInitialize();
	//红外图像成像参数初始化
	void IRParaInitialize();

	//非均匀校正
	void NUCbyTwoPoint();

	//坏点替换功能
	int GetBadPointList(unsigned short* nBadPointList, unsigned short* pus_gain_mat, int n_width, int n_height);
	int ReplaceBadPoint(short* pus_src, unsigned short* nBadPointList, unsigned short* pus_gain_mat, int n_width, int n_height);

	//计算时域滤波权重表
	void GetTFFGuassWeight();
	//边界扩展函数
	void PadMatrix(short *pus_pad_mat, short *pus_mat, int n_width, int n_height, int n_win_size_width, int n_win_size_height);
	//时域滤波函数
	void TimeNoiseFliter(short* pus_dst, short* pus_src, int nWidth, int nHeight, int WinSizeW, int WinSizeH);
	//时域滤波封装函数
	void TimeFF();

	//去竖条纹
	void FixedPoint_GrayFilter_16bit_RVN(short *pus_dst, unsigned short *pus_weight, short *pus_src, int n_width, int n_height, int* gray_weight_table, int n_win_width, int n_win_height);
	void RemoveVerStripe(short *pus_dst, short *pus_src, int width, int height, int win_width, int* gray_weight_table, int weight_thresh, int detail_thresh);
	void RemoveSVN();
	//去横纹
	void FixedPoint_GrayFilter_16bit_RHN(short *pus_dst, unsigned short *pus_weight, short *pus_src, int n_width, int n_height, int* gray_weight_table, int n_win_width, int n_win_height);
	void RemoveHorStripe(short* pus_dst, short *pus_src, int width, int height, int win_height, int* gray_weight_table, int weight_thresh, int detail_thresh);
	void RemoveSHN();
	/**************初始化气体检测*****/
	void HSM();
	void meanTHr(short* pus_src, short* pus_dst, int n_width, int n_height, int winSize, int thr, int deltaUpthr, int deltaDownthr);
	/*******************************/
	//空域滤波
	void SpaceFilter();
	void GetGaussGrayWeightTable(int *pa_gray_weight_table, int n_len, int n_gray_std);
	void GetGuassPosWeightTable(int* a_pos_weight_table, int n_win_wid, int n_win_hei, int std);
	void GaussianFilter_16bit(short *pus_dst, short *pus_src, int n_width, int n_height, int n_win_wid, int* pos_weight_table);
	void FixedPoint_GrayFilter_16bit(short *pus_dst, short *pus_src, int n_width, int n_height, int* gray_weight_table, int n_win_width, int n_win_height);

	//图像镜像
	void ImgFlip();

	//图像旋转
	void ImgRotation();

	//调光
	void ModelDRT();

	//细节增强
	void FixedPoint_IIE(unsigned char *puc_dst, short *pus_src, int n_width, int n_height);
	void IIE();

	//调光
	void GetHist(unsigned short* pHist, int* pnMaxOri, int* pnMinOri, int* pnMaxDisc, int* pnMinDisc, int* pGrayMean, short *pusSrcImg, int n_width,
		int n_height, int nHistSize, int nUpDiscardRatio, int nDownDiscardRatio);
	//线性调光
	void DRC_LinearAutoBC(unsigned char* puc_dst, short *pus_src, int n_width, int n_height, int nUpDiscardRatio, int nDownDiscardRatio, int nBrightExp, int nContrastExp, int nRestrainRangeThre);
	//平台直方图调光
	void DRC_PlatHistEqualize(unsigned char* puc_dst, short *pus_src, int n_width, int n_height, int nPlatThresh, int nUpDiscardRatio, int nDownDiscardRatio, int nEqhistMax, int nMidValue);
	//混合调光
	void DRC_Mix(unsigned char* puc_dst, short *pus_src, int n_width, int n_height, int nMixContrastExp, int nMixBrightExp, int nRestrainRangeThre, int n_mixrange, int nMidValue, int nPlatThresh, int nUpDiscardRatio, int nDownDiscardRatio,int nMixThrHigh, int nMixThrLow);
	
	//手动调光
	void DRC_ManualTone(unsigned char* puc_dst, short *pus_src, int n_width, int n_height, short nTmaxToY16, short nTminToY16);
	//人体医疗调光
	void DRC_PersonMedical(unsigned char* puc_dst, short *ps_src, int n_width, int n_height, short Y16_Tbase, short Y16_high);
		
	//Gamma校正
	void ImgGetGammaTable(unsigned char *GammaTable, float fGamma);
	void ImgGammaCorrect(unsigned char* nPucDst, unsigned char* nPucSrc, int nLen, unsigned char *GammaTable);

	//Y8拉普拉斯锐化
	void LaplaceSharpen(unsigned char *pus_dst, unsigned char *pus_src, int n_width, int n_height, float weight);
	void LaplaceSharpen();

	//Y8纠偏
	void ImgY8AdjustBC();
	void Y8AdjustBC(unsigned char* puc_dst, unsigned char* puc_src, int n_width, int n_height, int nContrastexp, int nBrightexp);

	//图像放大
	void ImgResize();
	void GetTable(int srcH, int srcW, double scaleH, double scaleW, unsigned short* Table);
	int ImgBilinearFast(unsigned char * DstBuf, unsigned char * SrcBuf, int srcH, int srcW, double scaleH, double scaleW, unsigned short* Table);
	
	//伪彩
	void PseudoColorMap();

	//去锅盖
	void CalGuoGaiModel();
	void DeGuoGai();
	
	void EqualLine(unsigned char *puc_dst, unsigned char *y8Arr, short* y16Arr, int height, int width, int pseudo_color_type, int colorimg_tpye, EqualLineMode* s_equal_line_para);
	//计算权重，imgBigger用到，y16放大。
	void getWx(double wx[4], double x);
	void getWy(double wy[4], double y);
	//矩阵扩展
	void padMatrixChar(short *pus_pad_mat, short *pus_mat, int n_width, int n_height, int n_win_size_width, int n_win_size_height);

	//自动去坏点 zm11a项目使用
	void RemoveFrameBadPoint();
	
private:

	//红外参数结构体
	stIrPara IrPara;
	int gWidth, gHeight;  //红外图像宽高
	float gZoom;//红外图像放大倍数

	float nBadPointRate;
	int nBadPointNumThr;
	unsigned short* gBadPointList;

	//时域滤波全局变量
	int g_nFFTWeightTable[TTF_WEIGHT_TABLE_LEN];//时域滤波权重
	short* g_pusTffLastFrame;
	int nFilterFrame;       //帧计数器
	//时域滤波局部指针
	short *diff;
	short *pus_src_pad;  //时域滤波空域滤波公用该地址
	short *pus_last_pad;

	//去条纹算法全局变量
	int g_gray_weight_table_Ver[GRAY_WEIGHT_TABLE_LEN];
	int g_gray_weight_table_Hor[GRAY_WEIGHT_TABLE_LEN];
	//去竖条纹算法局部指针
	short *pus_img_low;    //滤波后的低频图像
	unsigned short *pus_img_weight;
	short *g_img_high;        //细节图像 
	short *g_ver_mean;  //列均值数组
	unsigned short *g_col_num;
	short *g_row_mean;     //列均值数组
	/**************初始化气体检测*****/
	//气体检测局部指针
	short *pus_pre;//前一帧Y16数据
	short* pus_deta;//前一帧Y16数据
	unsigned short* hsmAhist;//气体检测的直方图数组
	/**********************************/
	//空域滤波全局变量
	int g_pos_weight_table[GRAY_WEIGHT_TABLE_LEN];
	int g_gray_weight_table[GRAY_WEIGHT_TABLE_LEN];

	//调光全局变量
	int g_nOffset;
	int g_nMaxContrast;  //对比度限制值 
	int gCurFrameNum = 0;
	int g_nRangeSmooth[FRAME_SMOOTH_NUM] = { 0 };
	unsigned char* puc_dst_lin;
	unsigned char* puc_dst_phe;

	//细节增强
	int g_IDE_pos_weight_table[GRAY_WEIGHT_TABLE_LEN];
	short  *ps_detail_img; // 细锟斤拷图锟斤拷
	unsigned char  *puc_base_img; // 8bit锟斤拷频
	short *pus_base_img; // 16bit锟斤拷频

	//Gamma校正
	unsigned char nGammaTable[256];
	//等温线
	short* y16ArrZoom;
	
	//图像放大
	unsigned short* ResizeTable;

	//输出中间数据开关
	bool b_outdata_y16_switch, b_outdata_y8_switch;

	//去锅盖算法
	short* g_guogai_model;

#ifdef AlgOpt
	long int *mat_weight_sum;
	long int *mat_value_sum;
#endif // AlgOpt
	//Y16放大功能。计算超分辨率温度矩阵时使用。
	short *m_paddingSrc;
	int m_paddingLen;
public:

	//输出中间数据对外指针
	short *pus_output_y16; //y16数据不用再申请内存，使用外面传入的内存，减少一次拷贝和节省内存。
	int n_outdata_y16_width, n_outdata_y16_height;
	unsigned char * puc_output_y8; //y8数据不用再申请内存，使用外面传入的内存，减少一次拷贝和节省内存。
	int n_outdata_y8_width, n_outdata_y8_height;
	
	//图像数据
	stIrContent IRContent;

	//红外图像处理主函数
	void InfraredImageProcess();

	//////参数获取与设置
	//非均匀校正
	long get_nuc_switch(bool* pVal);
	long put_nuc_switch(bool newVal);

	//坏点替换
	long get_rpbp_updatalist_switch(bool* pVal);
	long put_rpbp_updatalist_switch(bool newVal);
	long get_rpbp_switch(bool* pVal);
	long put_rpbp_switch(bool newVal);
	long get_rpbp_auto_switch(unsigned char* value);
	long put_rpbp_auto_switch(unsigned char value);

	//时域滤波 参数只开放标准差
	long get_tff_switch(bool* pVal);
	ITA_RESULT put_tff_switch(bool newVal);
	long get_tff_std(int* pVal);
	long put_tff_std(int newVal);

	//去条纹算法
	long get_rvs_switch(bool* pVal);
	long put_rvs_switch(bool newVal);
	long get_rvs_win_width(int* pVal);
	long put_rvs_win_width(int newVal);
	long get_rvs_weight_thresh(int* pVal);
	long put_rvs_weight_thresh(int newVal);
	long get_rvs_detail_thresh(int* pVal);
	long put_rvs_detail_thresh(int newVal);
	long get_rvs_std(int* pVal);
	long put_rvs_std(int newVal);
	//去横纹算法
	long get_rhs_switch(bool* pVal);
	long put_rhs_switch(bool newVal);
	long get_rhs_win_width(int* pVal);
	long put_rhs_win_width(int newVal);
	long get_rhs_weight_thresh(int* pVal);
	long put_rhs_weight_thresh(int newVal);
	long get_rhs_detail_thresh(int* pVal);
	long put_rhs_detail_thresh(int newVal);
	long get_rhs_std(int* pVal);
	long put_rhs_std(int newVal);
	/**************初始化气体检测*****/
	long get_hsm_switch(bool* pVal);
	long put_hsm_switch(bool newVal);
	long get_hsm_weight(int* pVal);
	ITA_RESULT put_hsm_weight(int newVal);
	long get_hsm_stayThr(int* pVal);
	long put_hsm_stayThr(int newVal);
	long get_hsm_stayWeight(int* pVal);
	long put_hsm_stayWeight(int newVal);

	long put_hsm_interval(int newVal);
	long get_hsm_interval(int* pVal);
	long get_hsm_midfilterSwitch(bool* pVal);
	long put_hsm_midfilterSwitch(bool newVal);
	long get_hsm_midfilterWinSize(int* pVal);
	long put_hsm_midfilterWinSize(int newVal);
	long get_hsm_delta_upthr(int* pVal);
	long put_hsm_delta_upthr(int newVal);
	long get_hsm_delta_downthr(int* pVal);
	long put_hsm_delta_downthr(int newVal);
	long get_hsm_point_thr(int* pVal);
	long put_hsm_point_thr(int newVal);
	/**************************************/
	//空域滤波
	long get_rn_switch(bool* pVal);
	long put_rn_switch(bool newVal);
	long get_rn_arith_type(int* pVal);
	long put_rn_arith_type(int newVal);
	long get_rn_dist_std(int* pVal);
	long put_rn_dist_std(int newVal);
	long get_rn_gray_std(int* pVal);
	long put_rn_gray_std(int newVal);

	//图像镜像
	long get_flip_switch(bool* pVal);
	ITA_RESULT put_flip_switch(bool newVal);
	long get_flip_type(int* pVal);
	ITA_RESULT put_flip_type(int newVal);

	//图像旋转
	long get_rotate_switch(bool* pVal);
	ITA_RESULT put_rotate_switch(bool newVal);
	long get_rotate_type(int* pVal);
	ITA_RESULT put_rotate_type(int newVal);

	//调光参数
	long get_drt_switch(bool* pVal);
	ITA_RESULT put_drt_switch(bool newVal);
	long get_drt_type(int* pVal);
	ITA_RESULT put_drt_type(int newVal);
	long get_discard_upratio(int* pVal);
	long put_discard_upratio(int newVal);
	long get_discard_downratio(int* pVal);
	long put_discard_downratio(int newVal);
	//线性调光参数
	long get_linear_brightness(int* pVal);
	long put_linear_brightness(int newVal);
	long get_linear_contrast(int* pVal);
	long put_linear_contrast(int newVal);
	long get_linear_restrain_rangethre(int* pVal);
	long put_linear_restrain_rangethre(int newVal);
	//平台直方图调光
	long get_heq_plat_thresh(int* pVal);
	long put_heq_plat_thresh(int newVal);
	long get_heq_range_max(int* pVal);
	long put_heq_range_max(int newVal);
	long get_heq_midvalue(int* pVal);
	long put_heq_midvalue(int newVal);
	//手动调光
	long get_manultone_maxy16(int* pVal);
	long put_manultone_maxy16(int newVal);
	long get_manultone_miny16(int* pVal);
	long put_manultone_miny16(int newVal);
	//人体医疗调光
	long get_personmedical_y16tbase(int* pVal);
	long put_personmedical_y16tbase(int newVal);
	long get_personmedical_y16high(int* pVal);
	long put_personmedical_y16high(int newVal);
	//混合调光混合比例
	long get_mix_ThrHigh(int* pVal);
	long put_mix_ThrHigh(int newVal);
	long get_mix_ThrLow(int* pVal);
	long put_mix_ThrLow(int newVal);


	//细节增强参数
	long get_iie_switch(bool* pVal);
	long put_iie_switch(bool newVal);
	long get_iie_enhance_coef(int* pVal);
	long put_iie_enhance_coef(int newVal);
	long get_iie_pos_std(int* pVal);
	long put_iie_pos_std(int newVal);
	long get_iie_detail_thr(unsigned char* pVal);
	long put_iie_detail_thr(unsigned char newVal);

	//Gamma校正
	long get_gmc_switch(bool* pVal);
	long put_gmc_switch(bool newVal);
	long get_gmc_gamma(float* pVal);
	long put_gmc_gamma(float newVal);
	long get_gmc_gammatype(int* pVal);
	long put_gmc_gammatype(int newVal);

	//锐化
	long get_sp_switch(bool* pVal);
	long put_sp_switch(bool newVal);
	long get_sp_laplace_weight_y16(float* pVal);
	long put_sp_laplace_weight_y16(float newVal);

	//Y8纠偏
	long get_y8adjustbc_switch(bool* pVal);
	long put_y8adjustbc_switch(bool newVal);
	long get_y8adjustbc_contrast(int* pVal);
	long put_y8adjustbc_contrast(int newVal);
	long get_y8adjustbc_bright(int* pVal);
	long put_y8adjustbc_bright(int newVal);

	//图像放大参数设置
	long get_ImgZoom(float* pVal);
	ITA_RESULT put_ImgZoom(float newVal);
	long get_ImgZoomType(int* pVal);
	long put_ImgZoomType(int newVal);
	
	//伪彩
	long get_psdclr_switch(bool* pVal);
	long put_psdclr_switch(bool newVal);
	long get_psdclr_type(int* pVal);
	long put_psdclr_type(int newVal);
	long get_colorimg_tpye(int* pVal);
	long put_colorimg_tpye(int newVal);
	long get_colorimg_bytenum(int* pVal);
	long get_psdclr_equallinemode(bool* pVal);
	long put_psdclr_equallinemode(bool newVal);
	long get_psdclr_equallinepara(EqualLineMode* pVal);
	long put_psdclr_equallinepara(EqualLineMode newVal);

	//去锅盖
	ITA_RESULT putCalGuoGaiSwitch(bool newVal);
	ITA_RESULT getCalGuoGaiSwitch(bool* pVal);
	ITA_RESULT getGuoGaiModel(short* nModel, int nWidth, int nHeight);
	ITA_RESULT putGuoGaiModel(short* nNewModel, int nWidth, int nHeight);
	ITA_RESULT putGuoGaiSwitch(bool newVal);
	ITA_RESULT getGuoGaiSwitch(bool* pVal);
	ITA_RESULT putGuoGaiArithType(int newVal);
	ITA_RESULT getGuoGaiArithType(int* pVal);
	ITA_RESULT putGuoGaiTPara(stDGGTPara* sDggTPara);
	ITA_RESULT getGuoGaiTPara(stDGGTPara* sDggTPara);

	//输出中间数据开关
	long get_output_y8_switch(bool* pVal);
	//y8数据不用再申请内存，使用外面传入的内存，减少一次拷贝和节省内存。
	long put_output_y8_switch(bool newVal, unsigned char *y8Data);
	long get_output_y16_switch(bool* pVal);
	//y16数据不用再申请内存，使用外面传入的内存，减少一次拷贝和节省内存。
	long put_output_y16_switch(bool newVal, short *y16Data);

	//双三次线性插值，y16放大。
	void imgBigger(short * src, int srcWidth, int srcHeight, short * dst, int dstWidth, int dstHeight);
	void imgSmaller(short* src, int srcWidth, int srcHeight, short* dst, int dstWidth, int dstHeight);

	void tvRang2FullRange(unsigned char* nDst,const unsigned char* nSrc, int width, int height);

	CustomPalette* customPalette;
	int getPaletteTotalNum();
	bool isCustomPalette;

	GuideLog* m_logger;
};
#endif // !CINFRARED_CORE_H