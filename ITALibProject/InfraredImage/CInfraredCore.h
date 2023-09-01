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

//����Ƕ��ʽҪ���޸Ĵ����ʽ��������Ƕ��ʽ��
#define AlgOpt
/**************��ʼ��������*****/
#define Clip16(x) (x < -32767) ? -32767 : ((x > 32767) ? 32767 : x)//������
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

//����궨��
#define FRAME_SMOOTH_NUM        (1)//(100)
#define LAST_PARA               (1)
#define GRAYLEVEL_16BIT         (65536) 
#define GRAY_WEIGHT_TABLE_LEN	512
#define TTF_WEIGHT_TABLE_LEN	128

//PseudoColorTable0Ϊ��˾����α��,��26��;
//PseudoColorTable1ΪĿǰģ������ʹ��α�ʣ�Ϊ13����������120ģ���256ģ�顣
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
	PSEUDO_NAME_WHITEHOT = 0, //����
	PSEUDO_NAME_FULGURITE, //����
	PSEUDO_NAME_IRONRED, //����
	PSEUDO_NAME_HOTIRON, //����
	PSEUDO_NAME_MEDICAL, //ҽ��
	PSEUDO_NAME_ARCTIC, //����
	PSEUDO_NAME_RAINBOW1, //�ʺ�1
	PSEUDO_NAME_RAINBOW2, //�ʺ�2
	PSEUDO_NAME_BLACKHOT, //����
	PSEUDO_NAME_BODYDRT, //����ɸ��
	PSEUDO_NAME_TINT1, // ���1
	PSEUDO_NAME_BLUEHOT, // ����
	PSEUDO_NAME_GREENHOT, //����
	PSEUDO_NAME_MEDICAL0, //ҽ��0
	PSEUDO_NAME_MEDICAL1, //ҽ��1
	PSEUDO_NAME_MEDICAL2, //ҽ��2
	PSEUDO_NAME_RAINBOW3, //�ʺ�3
	PSEUDO_NAME_TINT2, //���2
	PSEUDO_NAME_TINT3, //���3
	PSEUDO_NAME_ICEFIRE, //����
	PSEUDO_NAME_REVERSEICEFIRE, //������
	PSEUDO_NAME_STEPS, //�ֲ�
	PSEUDO_NAME_SEPIA, // ���
	PSEUDO_NAME_WINTER, //����
	PSEUDO_NAME_HIGHLIGHT, //����
	PSEUDO_NAME_AMBER, //����
};
#endif

#ifdef PseudoColorTable1
#define PSEUDONUM (13)
extern const unsigned char  PseudoColorTable[PSEUDONUM][256][4];
extern const unsigned char  PseudoColorTableYUV[PSEUDONUM][256][4];
enum ePSEUDO_NAME
{
	PSEUDO_NAME_WHITEHOT = 0, //����
	PSEUDO_NAME_FULGURITE, //����
	PSEUDO_NAME_IRONRED, //����
	PSEUDO_NAME_HOTIRON, //����
	PSEUDO_NAME_MEDICAL, //ҽ��
	PSEUDO_NAME_ARCTIC, //����
	PSEUDO_NAME_RAINBOW1, //�ʺ�1
	PSEUDO_NAME_RAINBOW2, //�ʺ�2
	PSEUDO_NAME_BLACKHOT, //����
	PSEUDO_NAME_BODYDRT, //����ɸ��
	PSEUDO_NAME_TINT, // ���
	PSEUDO_NAME_BLUEHOT, // ����
	PSEUDO_NAME_GREENHOT,//����
};
#endif

// Dynamic range transform type ��̬��Χת������
enum eDRT_TYPE
{
	DRT_TYPE_LINEAR = 0,         //����
	DRT_TYPE_PLATHE,        //ƽֱ̨��ͼ����
	DRT_TYPE_MIX_PHE_LIN,   //��ϵ���
	DRT_TYPE_MANUALTONE, //�ֶ�����
	DRT_TYPE_PERSONMEDICAL, //����ҽ�Ƶ��⣬��ʹ������ҽ��ɫ��ʱ�㷨�Զ��л����õ��ⷽʽ��Ŀǰ�����û����š�
};

//ͼ��ת��ʽ
enum eFLIP_TYPE
{// ��ֵ����
	FlipX = 0,  //ˮƽ����
	FlipY,       //��ֱ����
	FlipXY      //ˮƽ�Ӵ�ֱ����
};

//ͼ����ת��ʽ
enum eROTATE_TYPE
{// ��ֵ����
	ROTATE90 = 0,  //ˮƽ����
	ROTATE180,       //��ֱ����
	ROTATE270      //ˮƽ�Ӵ�ֱ����
};

//ͼ��Ŵ�ʽ
enum Interpolation
{// ��ֵ����
	Near = 0,
	Bilinear,
	BilinearFast,
};
enum eGAMMACOR_TYPE
{
	GAMMACOR_TYPE_SINGLE = 0,
	GAMMACOR_TYPE_DOUBLE,
};
//���ͼ������
#define COLORIMGTYPENUM (12)
enum eCOLORIMG_TYPE
{
	COLORIMG_TYPE_BGR = 0,  //BGR��ʽͼ��
	COLORIMG_TYPE_RGBA,     //RGBA��ʽͼ��
	COLORIMG_TYPE_RGB,      //RGB��ʽͼ��
	COLORIMG_TYPE_RGB565,   //RGB565��ʽͼ��
	COLORIMG_TYPE_YUV422YUYV,   //YUV422��ʽYUYV
	COLORIMG_TYPE_YUV422UYVY,   //YUV422��ʽUYVY
	COLORIMG_TYPE_YUV422Plane,   //YUV422��ʽYUV422Plane
	COLORIMG_TYPE_YVU422Plane,   //YUV422��ʽYVU422Plane
	COLORIMG_TYPE_YUV420NV12,   //YUV420��ʽNV12
	COLORIMG_TYPE_YUV420NV21,   //YUV420��ʽNV21
	COLORIMG_TYPE_YUV420PLANE,   //YUV420��ʽYUV420Plane
	COLORIMG_TYPE_YVU420PLANE,   //YUV420��ʽYVU420Plane
};

// NUC�Ǿ���У���㷨����
struct stNUCPara
{
	bool  b_nuc_switch;       // NUC Switch
};

// NUC�Ǿ���У���㷨����
struct stRPBPPara
{
	bool  b_rpbp_switch;       // �����滻 Switch
	bool b_rpbp_updatalist_switch;
	unsigned char b_rpbp_auto_switch;//�Զ�ȥ����
};

struct stDRTPara
{
	bool b_drt_switch; //���⿪��
	bool b_drt_exterhist_switch;//�Ƿ�ʹ���ⲿֱ��ͼ����
	int n_drt_type;  //��������

	int n_discard_upratio;	      // ���׵����
	int n_discard_downratio;	  // ���׵����

	int n_linear_contrast;        // �ԱȶȲ���	
	int n_linear_brightness;      // ���ȵ��ڲ���	
	int n_linear_restrain_rangethre; //����������

	int n_heq_midvalue;
	int n_heq_plat_thresh;
	int n_heq_range_max;

	//�ֶ��������
	short n_manltone_maxy16;
	short n_manltone_miny16;

	//����ҽ�Ƶ������
	short n_personmedical_y16_base;
	short n_personmedical_y16_high;

	//��ϵ����������
	int n_mix_ThrHigh;//����ֵ
	int n_mix_ThrLow;//����ֵ
};

struct stGammaCorPara
{
	bool b_gmc_switch;
	int b_gmc_type;
	float f_gmc_gamma;
};

struct stTimeFilterPara
{
	bool b_tff_switch;               //  �㷨��������
	int n_tff_std;					 //  ��׼��
	int n_tff_win_size_w;             //  ���ڴ�С
	int n_tff_win_size_h;
};

struct stRSPara
{
	bool  b_rs_switch;           // ȥ�������㷨����
	int  n_stripe_win_width;     // ȥ�������˲����ڿ��
	int  n_stripe_weight_thresh; // ȥ������Ȩ������
	int  n_stripe_std;           // ȥ�����Ʒ���
	int  n_stripe_detail_thresh;        // �������ж���ֵ
};
/**************��ʼ��������*****/
struct stHSMPara
{
	bool  b_hsm_switch;           // �����㷨����
	int  n_hsm_weight;     // ֡���ֵȨ��
	int  n_hsm_stayThr;     // ��̬��ֵ
	int  n_hsm_stayWeight;     // ��̬֡���ֵȨ��

	int n_hsm_interval;//����������
	bool b_midfilter_switch;//��ֵ�˲�����
	int n_midfilter_winSize;//��ֵ�˲�����
	int n_delta_upthr;//֡���ֵ����ֵ����
	int n_delta_downthr;//֡���ֵ����ֵ����
	int n_point_thr;//����������ֵ

	float f_hsm_k;//���������K
	float f_hsm_b;//��������B
};
/*********************************/
// RN��remove noise�� ȥ���㷨����
enum eRN_TYPE
{
	RN_TYPE_GAUSSIAN = 0,         //��˹
	RN_TYPE_GREYSCALE,        //���߻Ҷ�
	RN_TYPE_BILATERAL        //˫���˲�
};

// RN��remove noise�� ȥ���㷨����
struct stRNPara
{
	bool b_rn_switch;	     // ȥ���㷨����
	int n_rn_arith_type;     // ȥ���㷨����
	int n_dist_std; 	     // �ռ��˹�˲���׼��
	int n_gray_std; 	     // ���߻Ҷ��˲���׼��
	int n_win_size;          // ȥ���˲�����
};

// ͼ����
struct stImgFlipPara
{
	bool b_flip_switch;
	int nFlipType;
};

// ͼ����
struct stImgRotatePara
{
	bool b_rotate_switch;
	int nRotateType;
};


// ��sharpen�㷨����
struct stSPPara
{
	bool b_sp_switch;        // �񻯿���
	float n_sp_laplace_weight;//��ϵ��
};

// IIEͼ����ǿ�㷨����
struct stIIEPara
{
	bool b_iie_switch;	    // IIE�㷨Switch
	int n_iie_pos_win_size; 	// ��ǿ��˹�˲����ڴ�С
	int n_iie_enhance_coef;	// ��ǿ�㷨ϸ������ϵ��
	int n_iie_gauss_std;
	unsigned char n_iie_detail_thr; //ϸ����ֵ
};

struct stY8ADJUSTBCPara
{
	bool b_adjustbc_switch;
	int n_adjustbc_bright;
	int n_adjustbc_contrast;
};

// ͼ��Ŵ�
struct stImgZoomPara
{
	float fZoom;
	int n_ZoomType;
};

struct EqualLineMode 
{
	//���������ͣ������� EQUAL_LINE_High = 0��EQUAL_LINE_Low = 1��
	//EQUAL_LINE_HighLow = 2��EQUAL_LINE_Middle = 3
	int type;  
	//���õĸ�Y16
	short highY16; 
	//���õĵ�Y16
	short lowY16; 
	//���õĵ�������ɫ colorΪ���ֽ�int�ͣ�
	//r = (color >> 16) & 0xff,g = (color >> 8) & 0xff,b = color & 0xff.
	int color;  
	//���õĵ�������һ��ɫ
	int otherColor; 
};

//�������������
enum EqualLineType
{
	EQUAL_LINE_High = 0,
	EQUAL_LINE_Low,
	EQUAL_LINE_HighLow,
	EQUAL_LINE_Middle,
};

// α��ɫ
struct stPseudoColorPara
{
	bool b_psdclr_switch;    // α��ɫ����
	int t_psdclr_type;     // α��ɫ����
	int n_colorimg_tpye;   //���ͼ������
	int n_colorimg_len;  //���α��ͼ���ֽ���
	bool b_psdclr_equallinemode; //������ģʽ
	EqualLineMode s_equalline_para;
};

//ȥ�����㷨����
enum eDGG_TYPE
{
	DGG_TYPE_FIX = 0,  //�̶�����ϵ��
	DGG_TYPE_VAR1,     //���ڿ��������¼���ϵ��
	DGG_TYPE_VAR2,     //������һ�ο����¼���ϵ��
};

struct stDGGTPara
{
	float fStartShutterT;	//����������
	float fLastLastShutterT;//�ϴο�����
	float fLastShutterT;	//��ǰ������
	float fRealShutterT;	//ʵʱ������
	float fDelta;			//�ɼ�ģ���Ӧ���豸����
};

struct stDGGPara
{
	bool b_dgg_switch;  	    //ȥ�����㷨����
	bool b_cgm_switch;          //�õ�����ģ�忪��
	int n_dgg_arith_type;       //ȥ�����㷨����
	stDGGTPara stDggTPara;
};

//����ͼ�������
struct stIrPara
{
	stNUCPara st_nuc_para;    //NUC
	stRPBPPara st_rpbp_para;  //�����滻
	stTimeFilterPara    st_tff_para;       // ʱ���˲�
	stRSPara    st_rvs_para;       // ȥ������
	stRSPara    st_rhs_para;       // ȥ������
	/**************��ʼ��������*****/
	stHSMPara   st_hsm_para;//������
	/**********************************/
	stRNPara    st_rn_para;        // �����˲�
	stImgFlipPara st_flip_para;  //ͼ��ת
	stImgRotatePara st_rotate_para;  //ͼ����ת
	stIIEPara    st_iie_para;       // IIE��ǿϵ��
	stDRTPara  st_drt_para;				// ����
	stGammaCorPara st_gmc_para;
	stSPPara     st_sp_para;        // ��
	stY8ADJUSTBCPara st_y8adjustbc_para;
	stImgZoomPara st_zoom_para;         //ͼ��Ŵ����
	stPseudoColorPara st_psdclr_coef;    // α��ɫ
	stDGGPara  st_dgg_para;        // ȥ�����㷨
};

//����ͼ������
struct stIfrImage
{
	int n_width;
	int n_height;
	int n_len;
	int n_channels;

	short *pus_data;	   //��ʾʲô 
	unsigned char *puc_data;
};

struct stIrContent
{
	stIfrImage pst_src_img;    //�������ݺ���ͼ��
	stIfrImage pst_dst_img;    //������ݺ���ͼ��

	unsigned short *pus_shutter_base;  //���ű���
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

	//����ͼ�����ݳ�ʼ��
	void IRContentInitialize();
	//����ͼ����������ʼ��
	void IRParaInitialize();

	//�Ǿ���У��
	void NUCbyTwoPoint();

	//�����滻����
	int GetBadPointList(unsigned short* nBadPointList, unsigned short* pus_gain_mat, int n_width, int n_height);
	int ReplaceBadPoint(short* pus_src, unsigned short* nBadPointList, unsigned short* pus_gain_mat, int n_width, int n_height);

	//����ʱ���˲�Ȩ�ر�
	void GetTFFGuassWeight();
	//�߽���չ����
	void PadMatrix(short *pus_pad_mat, short *pus_mat, int n_width, int n_height, int n_win_size_width, int n_win_size_height);
	//ʱ���˲�����
	void TimeNoiseFliter(short* pus_dst, short* pus_src, int nWidth, int nHeight, int WinSizeW, int WinSizeH);
	//ʱ���˲���װ����
	void TimeFF();

	//ȥ������
	void FixedPoint_GrayFilter_16bit_RVN(short *pus_dst, unsigned short *pus_weight, short *pus_src, int n_width, int n_height, int* gray_weight_table, int n_win_width, int n_win_height);
	void RemoveVerStripe(short *pus_dst, short *pus_src, int width, int height, int win_width, int* gray_weight_table, int weight_thresh, int detail_thresh);
	void RemoveSVN();
	//ȥ����
	void FixedPoint_GrayFilter_16bit_RHN(short *pus_dst, unsigned short *pus_weight, short *pus_src, int n_width, int n_height, int* gray_weight_table, int n_win_width, int n_win_height);
	void RemoveHorStripe(short* pus_dst, short *pus_src, int width, int height, int win_height, int* gray_weight_table, int weight_thresh, int detail_thresh);
	void RemoveSHN();
	/**************��ʼ��������*****/
	void HSM();
	void meanTHr(short* pus_src, short* pus_dst, int n_width, int n_height, int winSize, int thr, int deltaUpthr, int deltaDownthr);
	/*******************************/
	//�����˲�
	void SpaceFilter();
	void GetGaussGrayWeightTable(int *pa_gray_weight_table, int n_len, int n_gray_std);
	void GetGuassPosWeightTable(int* a_pos_weight_table, int n_win_wid, int n_win_hei, int std);
	void GaussianFilter_16bit(short *pus_dst, short *pus_src, int n_width, int n_height, int n_win_wid, int* pos_weight_table);
	void FixedPoint_GrayFilter_16bit(short *pus_dst, short *pus_src, int n_width, int n_height, int* gray_weight_table, int n_win_width, int n_win_height);

	//ͼ����
	void ImgFlip();

	//ͼ����ת
	void ImgRotation();

	//����
	void ModelDRT();

	//ϸ����ǿ
	void FixedPoint_IIE(unsigned char *puc_dst, short *pus_src, int n_width, int n_height);
	void IIE();

	//����
	void GetHist(unsigned short* pHist, int* pnMaxOri, int* pnMinOri, int* pnMaxDisc, int* pnMinDisc, int* pGrayMean, short *pusSrcImg, int n_width,
		int n_height, int nHistSize, int nUpDiscardRatio, int nDownDiscardRatio);
	//���Ե���
	void DRC_LinearAutoBC(unsigned char* puc_dst, short *pus_src, int n_width, int n_height, int nUpDiscardRatio, int nDownDiscardRatio, int nBrightExp, int nContrastExp, int nRestrainRangeThre);
	//ƽֱ̨��ͼ����
	void DRC_PlatHistEqualize(unsigned char* puc_dst, short *pus_src, int n_width, int n_height, int nPlatThresh, int nUpDiscardRatio, int nDownDiscardRatio, int nEqhistMax, int nMidValue);
	//��ϵ���
	void DRC_Mix(unsigned char* puc_dst, short *pus_src, int n_width, int n_height, int nMixContrastExp, int nMixBrightExp, int nRestrainRangeThre, int n_mixrange, int nMidValue, int nPlatThresh, int nUpDiscardRatio, int nDownDiscardRatio,int nMixThrHigh, int nMixThrLow);
	
	//�ֶ�����
	void DRC_ManualTone(unsigned char* puc_dst, short *pus_src, int n_width, int n_height, short nTmaxToY16, short nTminToY16);
	//����ҽ�Ƶ���
	void DRC_PersonMedical(unsigned char* puc_dst, short *ps_src, int n_width, int n_height, short Y16_Tbase, short Y16_high);
		
	//GammaУ��
	void ImgGetGammaTable(unsigned char *GammaTable, float fGamma);
	void ImgGammaCorrect(unsigned char* nPucDst, unsigned char* nPucSrc, int nLen, unsigned char *GammaTable);

	//Y8������˹��
	void LaplaceSharpen(unsigned char *pus_dst, unsigned char *pus_src, int n_width, int n_height, float weight);
	void LaplaceSharpen();

	//Y8��ƫ
	void ImgY8AdjustBC();
	void Y8AdjustBC(unsigned char* puc_dst, unsigned char* puc_src, int n_width, int n_height, int nContrastexp, int nBrightexp);

	//ͼ��Ŵ�
	void ImgResize();
	void GetTable(int srcH, int srcW, double scaleH, double scaleW, unsigned short* Table);
	int ImgBilinearFast(unsigned char * DstBuf, unsigned char * SrcBuf, int srcH, int srcW, double scaleH, double scaleW, unsigned short* Table);
	
	//α��
	void PseudoColorMap();

	//ȥ����
	void CalGuoGaiModel();
	void DeGuoGai();
	
	void EqualLine(unsigned char *puc_dst, unsigned char *y8Arr, short* y16Arr, int height, int width, int pseudo_color_type, int colorimg_tpye, EqualLineMode* s_equal_line_para);
	//����Ȩ�أ�imgBigger�õ���y16�Ŵ�
	void getWx(double wx[4], double x);
	void getWy(double wy[4], double y);
	//������չ
	void padMatrixChar(short *pus_pad_mat, short *pus_mat, int n_width, int n_height, int n_win_size_width, int n_win_size_height);

	//�Զ�ȥ���� zm11a��Ŀʹ��
	void RemoveFrameBadPoint();
	
private:

	//��������ṹ��
	stIrPara IrPara;
	int gWidth, gHeight;  //����ͼ����
	float gZoom;//����ͼ��Ŵ���

	float nBadPointRate;
	int nBadPointNumThr;
	unsigned short* gBadPointList;

	//ʱ���˲�ȫ�ֱ���
	int g_nFFTWeightTable[TTF_WEIGHT_TABLE_LEN];//ʱ���˲�Ȩ��
	short* g_pusTffLastFrame;
	int nFilterFrame;       //֡������
	//ʱ���˲��ֲ�ָ��
	short *diff;
	short *pus_src_pad;  //ʱ���˲������˲����øõ�ַ
	short *pus_last_pad;

	//ȥ�����㷨ȫ�ֱ���
	int g_gray_weight_table_Ver[GRAY_WEIGHT_TABLE_LEN];
	int g_gray_weight_table_Hor[GRAY_WEIGHT_TABLE_LEN];
	//ȥ�������㷨�ֲ�ָ��
	short *pus_img_low;    //�˲���ĵ�Ƶͼ��
	unsigned short *pus_img_weight;
	short *g_img_high;        //ϸ��ͼ�� 
	short *g_ver_mean;  //�о�ֵ����
	unsigned short *g_col_num;
	short *g_row_mean;     //�о�ֵ����
	/**************��ʼ��������*****/
	//������ֲ�ָ��
	short *pus_pre;//ǰһ֡Y16����
	short* pus_deta;//ǰһ֡Y16����
	unsigned short* hsmAhist;//�������ֱ��ͼ����
	/**********************************/
	//�����˲�ȫ�ֱ���
	int g_pos_weight_table[GRAY_WEIGHT_TABLE_LEN];
	int g_gray_weight_table[GRAY_WEIGHT_TABLE_LEN];

	//����ȫ�ֱ���
	int g_nOffset;
	int g_nMaxContrast;  //�Աȶ�����ֵ 
	int gCurFrameNum = 0;
	int g_nRangeSmooth[FRAME_SMOOTH_NUM] = { 0 };
	unsigned char* puc_dst_lin;
	unsigned char* puc_dst_phe;

	//ϸ����ǿ
	int g_IDE_pos_weight_table[GRAY_WEIGHT_TABLE_LEN];
	short  *ps_detail_img; // ϸ��ͼ��
	unsigned char  *puc_base_img; // 8bit��Ƶ
	short *pus_base_img; // 16bit��Ƶ

	//GammaУ��
	unsigned char nGammaTable[256];
	//������
	short* y16ArrZoom;
	
	//ͼ��Ŵ�
	unsigned short* ResizeTable;

	//����м����ݿ���
	bool b_outdata_y16_switch, b_outdata_y8_switch;

	//ȥ�����㷨
	short* g_guogai_model;

#ifdef AlgOpt
	long int *mat_weight_sum;
	long int *mat_value_sum;
#endif // AlgOpt
	//Y16�Ŵ��ܡ����㳬�ֱ����¶Ⱦ���ʱʹ�á�
	short *m_paddingSrc;
	int m_paddingLen;
public:

	//����м����ݶ���ָ��
	short *pus_output_y16; //y16���ݲ����������ڴ棬ʹ�����洫����ڴ棬����һ�ο����ͽ�ʡ�ڴ档
	int n_outdata_y16_width, n_outdata_y16_height;
	unsigned char * puc_output_y8; //y8���ݲ����������ڴ棬ʹ�����洫����ڴ棬����һ�ο����ͽ�ʡ�ڴ档
	int n_outdata_y8_width, n_outdata_y8_height;
	
	//ͼ������
	stIrContent IRContent;

	//����ͼ����������
	void InfraredImageProcess();

	//////������ȡ������
	//�Ǿ���У��
	long get_nuc_switch(bool* pVal);
	long put_nuc_switch(bool newVal);

	//�����滻
	long get_rpbp_updatalist_switch(bool* pVal);
	long put_rpbp_updatalist_switch(bool newVal);
	long get_rpbp_switch(bool* pVal);
	long put_rpbp_switch(bool newVal);
	long get_rpbp_auto_switch(unsigned char* value);
	long put_rpbp_auto_switch(unsigned char value);

	//ʱ���˲� ����ֻ���ű�׼��
	long get_tff_switch(bool* pVal);
	ITA_RESULT put_tff_switch(bool newVal);
	long get_tff_std(int* pVal);
	long put_tff_std(int newVal);

	//ȥ�����㷨
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
	//ȥ�����㷨
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
	/**************��ʼ��������*****/
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
	//�����˲�
	long get_rn_switch(bool* pVal);
	long put_rn_switch(bool newVal);
	long get_rn_arith_type(int* pVal);
	long put_rn_arith_type(int newVal);
	long get_rn_dist_std(int* pVal);
	long put_rn_dist_std(int newVal);
	long get_rn_gray_std(int* pVal);
	long put_rn_gray_std(int newVal);

	//ͼ����
	long get_flip_switch(bool* pVal);
	ITA_RESULT put_flip_switch(bool newVal);
	long get_flip_type(int* pVal);
	ITA_RESULT put_flip_type(int newVal);

	//ͼ����ת
	long get_rotate_switch(bool* pVal);
	ITA_RESULT put_rotate_switch(bool newVal);
	long get_rotate_type(int* pVal);
	ITA_RESULT put_rotate_type(int newVal);

	//�������
	long get_drt_switch(bool* pVal);
	ITA_RESULT put_drt_switch(bool newVal);
	long get_drt_type(int* pVal);
	ITA_RESULT put_drt_type(int newVal);
	long get_discard_upratio(int* pVal);
	long put_discard_upratio(int newVal);
	long get_discard_downratio(int* pVal);
	long put_discard_downratio(int newVal);
	//���Ե������
	long get_linear_brightness(int* pVal);
	long put_linear_brightness(int newVal);
	long get_linear_contrast(int* pVal);
	long put_linear_contrast(int newVal);
	long get_linear_restrain_rangethre(int* pVal);
	long put_linear_restrain_rangethre(int newVal);
	//ƽֱ̨��ͼ����
	long get_heq_plat_thresh(int* pVal);
	long put_heq_plat_thresh(int newVal);
	long get_heq_range_max(int* pVal);
	long put_heq_range_max(int newVal);
	long get_heq_midvalue(int* pVal);
	long put_heq_midvalue(int newVal);
	//�ֶ�����
	long get_manultone_maxy16(int* pVal);
	long put_manultone_maxy16(int newVal);
	long get_manultone_miny16(int* pVal);
	long put_manultone_miny16(int newVal);
	//����ҽ�Ƶ���
	long get_personmedical_y16tbase(int* pVal);
	long put_personmedical_y16tbase(int newVal);
	long get_personmedical_y16high(int* pVal);
	long put_personmedical_y16high(int newVal);
	//��ϵ����ϱ���
	long get_mix_ThrHigh(int* pVal);
	long put_mix_ThrHigh(int newVal);
	long get_mix_ThrLow(int* pVal);
	long put_mix_ThrLow(int newVal);


	//ϸ����ǿ����
	long get_iie_switch(bool* pVal);
	long put_iie_switch(bool newVal);
	long get_iie_enhance_coef(int* pVal);
	long put_iie_enhance_coef(int newVal);
	long get_iie_pos_std(int* pVal);
	long put_iie_pos_std(int newVal);
	long get_iie_detail_thr(unsigned char* pVal);
	long put_iie_detail_thr(unsigned char newVal);

	//GammaУ��
	long get_gmc_switch(bool* pVal);
	long put_gmc_switch(bool newVal);
	long get_gmc_gamma(float* pVal);
	long put_gmc_gamma(float newVal);
	long get_gmc_gammatype(int* pVal);
	long put_gmc_gammatype(int newVal);

	//��
	long get_sp_switch(bool* pVal);
	long put_sp_switch(bool newVal);
	long get_sp_laplace_weight_y16(float* pVal);
	long put_sp_laplace_weight_y16(float newVal);

	//Y8��ƫ
	long get_y8adjustbc_switch(bool* pVal);
	long put_y8adjustbc_switch(bool newVal);
	long get_y8adjustbc_contrast(int* pVal);
	long put_y8adjustbc_contrast(int newVal);
	long get_y8adjustbc_bright(int* pVal);
	long put_y8adjustbc_bright(int newVal);

	//ͼ��Ŵ��������
	long get_ImgZoom(float* pVal);
	ITA_RESULT put_ImgZoom(float newVal);
	long get_ImgZoomType(int* pVal);
	long put_ImgZoomType(int newVal);
	
	//α��
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

	//ȥ����
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

	//����м����ݿ���
	long get_output_y8_switch(bool* pVal);
	//y8���ݲ����������ڴ棬ʹ�����洫����ڴ棬����һ�ο����ͽ�ʡ�ڴ档
	long put_output_y8_switch(bool newVal, unsigned char *y8Data);
	long get_output_y16_switch(bool* pVal);
	//y16���ݲ����������ڴ棬ʹ�����洫����ڴ棬����һ�ο����ͽ�ʡ�ڴ档
	long put_output_y16_switch(bool newVal, short *y16Data);

	//˫�������Բ�ֵ��y16�Ŵ�
	void imgBigger(short * src, int srcWidth, int srcHeight, short * dst, int dstWidth, int dstHeight);
	void imgSmaller(short* src, int srcWidth, int srcHeight, short* dst, int dstWidth, int dstHeight);

	void tvRang2FullRange(unsigned char* nDst,const unsigned char* nSrc, int width, int height);

	CustomPalette* customPalette;
	int getPaletteTotalNum();
	bool isCustomPalette;

	GuideLog* m_logger;
};
#endif // !CINFRARED_CORE_H