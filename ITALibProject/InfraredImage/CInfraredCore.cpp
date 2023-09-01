#include "PseudoColorTable.h"
#include "../Module/PortingAPI.h"

//默认构造函数
CInfraredCore::CInfraredCore()
{
	gWidth = 256;
	gHeight = 192;
	gZoom = 1;

	nBadPointRate = 0.005f;
	nBadPointNumThr = int(gWidth * gHeight * nBadPointRate);
	gBadPointList = NULL;
	nFilterFrame = 0;
	//调光全局变量
	g_nOffset = 32768;
	g_nMaxContrast = 4;  //对比度限制值 

	g_pusTffLastFrame = NULL;
	diff = NULL;
	pus_src_pad = NULL;
	pus_last_pad = NULL;
	pus_img_low = NULL;
	pus_img_weight = NULL;
	g_img_high = NULL;
	g_ver_mean = NULL;
	g_col_num = NULL;
	g_row_mean = NULL;
	puc_dst_lin = NULL;
	puc_dst_phe = NULL;
	ps_detail_img = NULL; // 细锟斤拷图锟斤拷
	puc_base_img = NULL; // 8bit锟斤拷频
	pus_base_img = NULL; // 16bit锟斤拷频
	pus_output_y16 = NULL;
	puc_output_y8 = NULL;

	ResizeTable = NULL;
	m_paddingSrc = NULL;
	m_paddingLen = 0;
	pus_pre = NULL;
	pus_deta = NULL;
	hsmAhist = NULL;
	customPalette = NULL;
	isCustomPalette = false;
	IRContent.pus_gain_mat = NULL;
	IRContent.pus_shutter_base = NULL;
	IRContent.pus_x16_mat = NULL;
	y16ArrZoom = NULL;
	g_guogai_model = NULL;
	m_logger = NULL;
	IRContentInitialize();
	IRParaInitialize();
}

//构造函数
CInfraredCore::CInfraredCore(int w, int h, float fZoom)
{
	gWidth = w;
	gHeight = h;
	if (fZoom < 1)
		gZoom = 1;
	else
		gZoom = fZoom;
	nBadPointRate = 0.005f;
	nBadPointNumThr = int(gWidth * gHeight * nBadPointRate);
	gBadPointList = NULL;
	nFilterFrame = 0;
	//调光全局变量
	g_nOffset = 32768;
	g_nMaxContrast = 4;  //对比度限制值 

	g_pusTffLastFrame = NULL;
	diff = NULL;
	pus_src_pad = NULL;
	pus_last_pad = NULL;
	pus_img_low = NULL;
	pus_img_weight = NULL;
	g_img_high = NULL;
	g_ver_mean = NULL;
	g_col_num = NULL;
	g_row_mean = NULL;
	puc_dst_lin = NULL;
	puc_dst_phe = NULL;
	ps_detail_img = NULL; // 细锟斤拷图锟斤拷
	puc_base_img = NULL; // 8bit锟斤拷频
	pus_base_img = NULL; // 16bit锟斤拷频

	pus_output_y16 = NULL;
	puc_output_y8 = NULL;

	ResizeTable = NULL;
	m_paddingSrc = NULL;
	pus_pre = NULL;
	pus_deta = NULL;
	hsmAhist = NULL;
	m_paddingLen = 0;

	customPalette = NULL;
	isCustomPalette = false;
	IRContent.pus_gain_mat = NULL;
	IRContent.pus_shutter_base = NULL;
	IRContent.pus_x16_mat = NULL;
	y16ArrZoom = NULL;
	g_guogai_model = NULL;
	m_logger = NULL;
	IRContentInitialize();
	IRParaInitialize();
}

void CInfraredCore::FinalRelease()
{
	//nuc内存释放
	if (IRContent.pus_gain_mat)
	{
		porting_free_mem(IRContent.pus_gain_mat);
		IRContent.pus_gain_mat = NULL;
	}
	if (IRContent.pus_shutter_base)
	{
		porting_free_mem(IRContent.pus_shutter_base);
		IRContent.pus_shutter_base = NULL;
	}
	if (IRContent.pus_x16_mat)
	{
		porting_free_mem(IRContent.pus_x16_mat);
		IRContent.pus_x16_mat = NULL;
	}
	//rpbp内存释放
	if (gBadPointList)
	{
		porting_free_mem(gBadPointList);
		gBadPointList = NULL;
	}
	porting_free_mem(IRContent.pst_src_img.pus_data);
	IRContent.pst_src_img.pus_data = NULL;
	porting_free_mem(IRContent.pst_src_img.puc_data);
	IRContent.pst_src_img.puc_data = NULL;
	porting_free_mem(IRContent.pst_dst_img.pus_data);
	IRContent.pst_dst_img.pus_data = NULL;
	porting_free_mem(IRContent.pst_dst_img.puc_data);
	IRContent.pst_dst_img.puc_data = NULL;
	porting_free_mem(pus_src_pad); //多算法公用内存
	pus_src_pad = NULL;
	//tff内存释放
	if (g_pusTffLastFrame)
	{
		porting_free_mem(g_pusTffLastFrame);
		g_pusTffLastFrame = NULL;
	}
	if (diff)
	{
		porting_free_mem(diff);
		diff = NULL;
	}
	if (pus_last_pad)
	{
		porting_free_mem(pus_last_pad);
		pus_last_pad = NULL;
	}
	//去条纹算法内存释放
	if (pus_img_low)
	{
		porting_free_mem(pus_img_low);
		pus_img_low = NULL;
	}
	if (pus_img_weight)
	{
		porting_free_mem(pus_img_weight);
		pus_img_weight = NULL;
	}
	if (g_img_high)
	{
		porting_free_mem(g_img_high);
		g_img_high = NULL;
	}
	if (g_ver_mean)
	{
		porting_free_mem(g_ver_mean);
		g_ver_mean = NULL;
	}
	if (g_col_num)
	{
		porting_free_mem(g_col_num);
		g_col_num = NULL;
	}
	if (g_row_mean)
	{
		porting_free_mem(g_row_mean);
		g_row_mean = NULL;
	}	
	//气体检测内存释放
	if (pus_pre) {
		porting_free_mem(pus_pre);
		pus_pre = NULL;
	}
	if (pus_deta) {
		porting_free_mem(pus_deta);
		pus_deta = NULL;
	}
	if (hsmAhist) {
		porting_free_mem(hsmAhist);
		hsmAhist = NULL;
	}
	//drt内存释放
	if (puc_dst_lin)
	{
		porting_free_mem(puc_dst_lin);
		puc_dst_lin = NULL;
	}
	if (puc_dst_phe)
	{
		porting_free_mem(puc_dst_phe);
		puc_dst_phe = NULL;
	}
	//iie内存释放
	if (ps_detail_img)
	{
		porting_free_mem(ps_detail_img);
		ps_detail_img = NULL;
	}
	if (puc_base_img)
	{
		porting_free_mem(puc_base_img);
		puc_base_img = NULL;
	}
	if (pus_base_img)
	{
		porting_free_mem(pus_base_img);
		pus_base_img = NULL;
	}
	//等温线模式的内存释放
	if (y16ArrZoom)
	{
		porting_free_mem(y16ArrZoom);
		y16ArrZoom = NULL;
	}
	if (b_outdata_y16_switch)
	{
		//porting_free_mem(pus_output_y16); //不用再申请内存，使用外面传入的内存，减少一次拷贝和节省内存。
		pus_output_y16 = NULL;
	}
	if (b_outdata_y8_switch)
	{
		//porting_free_mem(puc_output_y8); //不用再申请内存，使用外面传入的内存，减少一次拷贝和节省内存。
		puc_output_y8 = NULL;
	}

	if (IrPara.st_zoom_para.n_ZoomType == BilinearFast)
	{
		porting_free_mem(ResizeTable);
		ResizeTable = NULL;
	}
	if (customPalette)
	{
		delete customPalette;
		customPalette = NULL;
	}
	if (g_guogai_model)
	{
		delete g_guogai_model;
		g_guogai_model = NULL;
	}
#ifdef AlgOpt
	porting_free_mem(mat_weight_sum);
	mat_weight_sum = NULL;
	porting_free_mem(mat_value_sum);
	mat_value_sum = NULL;
#endif // AlgOpt
}

//析构函数
CInfraredCore::~CInfraredCore()
{
	FinalRelease();
	if (m_paddingSrc)
	{
		porting_free_mem(m_paddingSrc);
		m_paddingSrc = NULL;
		m_paddingLen = 0;
	}
}


//红外图像内容初始化
void CInfraredCore::IRContentInitialize()
{
	IRContent.pst_src_img.n_width = gWidth;
	IRContent.pst_src_img.n_height = gHeight;
	IRContent.pst_src_img.n_len = gWidth * gHeight;
	IRContent.pst_src_img.n_channels = 1;

	IRContent.pst_dst_img.n_width = gWidth;
	IRContent.pst_dst_img.n_height = gHeight;
	IRContent.pst_dst_img.n_len = gWidth * gHeight;
	IRContent.pst_dst_img.n_channels = 1;

	//设置红外图像相关参数
	//输入数据pst_src_img初始化
	IRContent.pst_src_img.pus_data = (short*)porting_calloc_mem(gWidth * gHeight, sizeof(short), ITA_IMAGE_MODULE);
	IRContent.pst_src_img.puc_data = (unsigned char*)porting_calloc_mem(4 * gWidth * gHeight, sizeof(unsigned char), ITA_IMAGE_MODULE);
	//输出数据pst_dst_img初始化
	IRContent.pst_dst_img.pus_data = (short*)porting_calloc_mem(gWidth * gHeight, sizeof(short), ITA_IMAGE_MODULE);
	IRContent.pst_dst_img.puc_data = (unsigned char*)porting_calloc_mem(4 * gWidth * gHeight, sizeof(unsigned char), ITA_IMAGE_MODULE);

	//时域滤波，去竖纹，高斯空域滤波和高斯灰度滤波公用内存空间
	pus_src_pad = (short*)porting_calloc_mem((gWidth + 15) * (gHeight + 15), sizeof(short), ITA_IMAGE_MODULE);

	//锅盖模板
	g_guogai_model = (short*)porting_calloc_mem(gWidth * gHeight, sizeof(short), ITA_IMAGE_MODULE);

#ifdef AlgOpt
	mat_weight_sum = (long int *)porting_calloc_mem(gWidth * gHeight, sizeof(long int), ITA_IMAGE_MODULE);
	mat_value_sum = (long int *)porting_calloc_mem(gWidth * gHeight, sizeof(long int), ITA_IMAGE_MODULE);
#endif // AlgOpt
}

//红外图像成像参数初始化
void CInfraredCore::IRParaInitialize()
{
	//初始化非均匀校正开关
	IrPara.st_nuc_para.b_nuc_switch = 0;
	if (IrPara.st_nuc_para.b_nuc_switch)
	{
		IRContent.pus_shutter_base = (unsigned short*)porting_calloc_mem(gWidth * gHeight, sizeof(unsigned short), ITA_IMAGE_MODULE);
		IRContent.pus_x16_mat = (unsigned short*)porting_calloc_mem(gWidth * gHeight, sizeof(unsigned short), ITA_IMAGE_MODULE);
	}

	//坏点替换开关
	IrPara.st_rpbp_para.b_rpbp_switch = 0;
	IrPara.st_rpbp_para.b_rpbp_updatalist_switch = 1;  //初始化时默认更新坏点列表
	IrPara.st_rpbp_para.b_rpbp_auto_switch = 0;//自动去坏点功能默认关闭
	if (IrPara.st_rpbp_para.b_rpbp_switch || IrPara.st_nuc_para.b_nuc_switch)
	{
		IRContent.pus_gain_mat = (unsigned short*)porting_calloc_mem(gWidth * gHeight, sizeof(unsigned short), ITA_IMAGE_MODULE);
	}
	if (IrPara.st_rpbp_para.b_rpbp_switch)
	{
		gBadPointList = (unsigned short*)porting_calloc_mem((nBadPointNumThr + 1) * 2, sizeof(unsigned short), ITA_IMAGE_MODULE);
	}

	//初始化时域滤波参数
	IrPara.st_tff_para.b_tff_switch = 0;
	IrPara.st_tff_para.n_tff_std = 10;
	IrPara.st_tff_para.n_tff_win_size_w = 4;
	IrPara.st_tff_para.n_tff_win_size_h = 2;
	GetTFFGuassWeight();
	if (IrPara.st_tff_para.b_tff_switch == 1)
	{
		g_pusTffLastFrame = (short*)porting_calloc_mem(gWidth * gHeight, sizeof(short), ITA_IMAGE_MODULE);
		diff = (short*)porting_calloc_mem(gWidth * gHeight, sizeof(short), ITA_IMAGE_MODULE);
		pus_last_pad = (short*)porting_calloc_mem((gWidth + 15) * (gHeight + 15), sizeof(short), ITA_IMAGE_MODULE);
	}

	////初始化去竖纹参数
	IrPara.st_rvs_para.b_rs_switch = 0;
	IrPara.st_rvs_para.n_stripe_win_width = 9;
	IrPara.st_rvs_para.n_stripe_std = 15;
	IrPara.st_rvs_para.n_stripe_weight_thresh = 3500;
	IrPara.st_rvs_para.n_stripe_detail_thresh = 10;
	GetGaussGrayWeightTable(g_gray_weight_table_Ver, GRAY_WEIGHT_TABLE_LEN, IrPara.st_rvs_para.n_stripe_std);

	////初始化去横纹参数
	IrPara.st_rhs_para.b_rs_switch = 0;
	IrPara.st_rhs_para.n_stripe_win_width = 9;
	IrPara.st_rhs_para.n_stripe_std = 15;
	IrPara.st_rhs_para.n_stripe_weight_thresh = 3500;
	IrPara.st_rhs_para.n_stripe_detail_thresh = 20;
	GetGaussGrayWeightTable(g_gray_weight_table_Hor, GRAY_WEIGHT_TABLE_LEN, IrPara.st_rvs_para.n_stripe_std);
	if ((IrPara.st_rhs_para.b_rs_switch == 1) || (IrPara.st_rvs_para.b_rs_switch == 1))
	{
		//去条纹算法内存申请
		pus_img_low = (short*)porting_calloc_mem(gWidth * gHeight, sizeof(short), ITA_IMAGE_MODULE);    // 滤波后的低频图像
		pus_img_weight = (unsigned short*)porting_calloc_mem(gWidth * gHeight, sizeof(unsigned short), ITA_IMAGE_MODULE);
		g_img_high = (short*)porting_calloc_mem(gWidth * gHeight, sizeof(short), ITA_IMAGE_MODULE);        // 细节图像 
		g_ver_mean = (short*)porting_calloc_mem(gWidth, sizeof(short), ITA_IMAGE_MODULE);  // 列均值数组
		g_col_num = (unsigned short*)porting_calloc_mem(gWidth, sizeof(unsigned short), ITA_IMAGE_MODULE);
		g_row_mean = (short*)porting_calloc_mem(gHeight, sizeof(short), ITA_IMAGE_MODULE);     // 列均值数组
	}
	/**************初始化气体检测*****/
	IrPara.st_hsm_para.b_hsm_switch = 0;
	IrPara.st_hsm_para.n_hsm_weight = 100;
	IrPara.st_hsm_para.n_hsm_stayThr = 30;
	IrPara.st_hsm_para.n_hsm_stayWeight = 10;

	IrPara.st_hsm_para.n_hsm_interval = 50;
	IrPara.st_hsm_para.n_midfilter_winSize = 3;
	IrPara.st_hsm_para.b_midfilter_switch = 0;;//中值滤波开关
	IrPara.st_hsm_para.n_delta_upthr = 5000;//帧间差值的阈值上限
	IrPara.st_hsm_para.n_delta_downthr = -5000;//帧间差值的阈值下限
	IrPara.st_hsm_para.n_point_thr = 5;//单点噪声阈值
	if (IrPara.st_hsm_para.b_hsm_switch == 1)
	{
		IrPara.st_hsm_para.f_hsm_k = (IrPara.st_hsm_para.n_hsm_weight - IrPara.st_hsm_para.n_hsm_stayWeight) * 1.0 / IrPara.st_hsm_para.n_hsm_interval;
		IrPara.st_hsm_para.f_hsm_b = IrPara.st_hsm_para.n_hsm_stayWeight - IrPara.st_hsm_para.f_hsm_k * IrPara.st_hsm_para.n_hsm_stayThr;
	}
	/*if (IrPara.st_hsm_para.b_hsm_switch == 1) {
		pus_pre = (short*)porting_calloc_mem(gWidth * gHeight, sizeof(short), ITA_IMAGE_MODULE);//前一帧Y16数据
	}*/
	/**********************************/
	//初始化空域滤波
	IrPara.st_rn_para.b_rn_switch = 0;
	IrPara.st_rn_para.n_rn_arith_type = 1;  //0高斯空域滤波 1高斯灰度滤波
	IrPara.st_rn_para.n_dist_std = 5;
	IrPara.st_rn_para.n_gray_std = 5;
	IrPara.st_rn_para.n_win_size = 3;
	GetGuassPosWeightTable(g_pos_weight_table, IrPara.st_rn_para.n_win_size, IrPara.st_rn_para.n_win_size, IrPara.st_rn_para.n_dist_std);
	GetGaussGrayWeightTable(g_gray_weight_table, GRAY_WEIGHT_TABLE_LEN, IrPara.st_rn_para.n_gray_std);

	//图像旋转
	IrPara.st_rotate_para.b_rotate_switch = 0; //开关
	IrPara.st_rotate_para.nRotateType = 0; //0顺时针旋转90 1顺时针旋转180 2顺时针旋转270

	//图像翻转参数
	IrPara.st_flip_para.b_flip_switch = 0;  //开关11
	IrPara.st_flip_para.nFlipType = 0;  //0水平翻转 1垂直翻转 2水平+垂直翻转

	//初始化调光参数
	IrPara.st_drt_para.b_drt_switch = true;	//调光，默认打开。如果关闭，那么不处理Y16转Y8，不会生成imgDst图像数据。
	IrPara.st_drt_para.b_drt_exterhist_switch = 0; //是否使用前端统计调光参数
	IrPara.st_drt_para.n_drt_type = 0; //0线性调光 1平台直方图调光 2混合调光 3手动调光 4人体医疗调光
	if (IrPara.st_drt_para.n_drt_type == 2)
	{
		puc_dst_lin = (unsigned char*)porting_calloc_mem(gWidth * gHeight, sizeof(unsigned char), ITA_IMAGE_MODULE);
		puc_dst_phe = (unsigned char*)porting_calloc_mem(gWidth * gHeight, sizeof(unsigned char), ITA_IMAGE_MODULE);
	}
	IrPara.st_drt_para.n_discard_upratio = 1; //抛点千分之一
	IrPara.st_drt_para.n_discard_downratio = 1; //抛点千分之一
	//线性调光参数
	IrPara.st_drt_para.n_linear_brightness = 128;
	IrPara.st_drt_para.n_linear_contrast = 256;
	IrPara.st_drt_para.n_linear_restrain_rangethre = 64;
	//平台直方图调光参数
	IrPara.st_drt_para.n_heq_plat_thresh = 5;
	IrPara.st_drt_para.n_heq_range_max = 230;
	IrPara.st_drt_para.n_heq_midvalue = 128;
	//手动调光参数
	IrPara.st_drt_para.n_manltone_maxy16 = 200;
	IrPara.st_drt_para.n_manltone_miny16 = 100;
	//人体医疗调光参数
	IrPara.st_drt_para.n_personmedical_y16_base = 300;
	IrPara.st_drt_para.n_personmedical_y16_high = 400;

	//混合调光比例参数
	IrPara.st_drt_para.n_mix_ThrHigh = 300;//高阈值
	IrPara.st_drt_para.n_mix_ThrLow = 100;//低阈值

	////初始化IIE参数
	IrPara.st_iie_para.b_iie_switch = 0;	    // IIE算法Switch
	IrPara.st_iie_para.n_iie_pos_win_size = 3;  // 增强空域滤波窗口大小
	IrPara.st_iie_para.n_iie_enhance_coef = 32;	// 增强算法细节增益系数 16为1倍
	IrPara.st_iie_para.n_iie_gauss_std = 5;
	IrPara.st_iie_para.n_iie_detail_thr = 20; //细节阈值

	GetGuassPosWeightTable(g_IDE_pos_weight_table, IrPara.st_iie_para.n_iie_pos_win_size, IrPara.st_iie_para.n_iie_pos_win_size, IrPara.st_iie_para.n_iie_gauss_std);
	if (IrPara.st_iie_para.b_iie_switch == 1)
	{
		//细节增强内存申请
		ps_detail_img = (short*)porting_calloc_mem(gWidth * gHeight, sizeof(short), ITA_IMAGE_MODULE); // 细锟斤拷图锟斤拷
		puc_base_img = (unsigned char*)porting_calloc_mem(gWidth * gHeight, sizeof(unsigned char), ITA_IMAGE_MODULE); // 8bit锟斤拷频
		pus_base_img = (short*)porting_calloc_mem(gWidth * gHeight, sizeof(short), ITA_IMAGE_MODULE); // 16bit锟斤拷频
	}

	//Gamma校正
	IrPara.st_gmc_para.b_gmc_switch = 0;
	IrPara.st_gmc_para.f_gmc_gamma = 1.25;
	IrPara.st_gmc_para.b_gmc_type = 0;
	if (IrPara.st_gmc_para.b_gmc_switch)
	{
		ImgGetGammaTable(nGammaTable, IrPara.st_gmc_para.f_gmc_gamma);
	}

	////初始化锐化参数
	IrPara.st_sp_para.b_sp_switch = 0;
	IrPara.st_sp_para.n_sp_laplace_weight = 1;

	//初始化Y8纠偏参数
	IrPara.st_y8adjustbc_para.b_adjustbc_switch = 0;
	IrPara.st_y8adjustbc_para.n_adjustbc_bright = 128;
	IrPara.st_y8adjustbc_para.n_adjustbc_contrast = 128;

	//图像放大
	IrPara.st_zoom_para.fZoom = gZoom;
	IrPara.st_zoom_para.n_ZoomType = BilinearFast; //Near Bilinear BilinearFast
	if (IrPara.st_zoom_para.fZoom != 1)
	{
		void *pBuf = porting_calloc_mem((int)(4 * gWidth * gHeight * IrPara.st_zoom_para.fZoom * IrPara.st_zoom_para.fZoom), sizeof(unsigned char), ITA_IMAGE_MODULE);
		if (!pBuf)
			return;
		if (IRContent.pst_src_img.puc_data)
			porting_free_mem(IRContent.pst_src_img.puc_data);
		IRContent.pst_src_img.puc_data = (unsigned char*)pBuf;
		pBuf = porting_calloc_mem((int)(4 * gWidth * gHeight * IrPara.st_zoom_para.fZoom * IrPara.st_zoom_para.fZoom), sizeof(unsigned char), ITA_IMAGE_MODULE);
		if (!pBuf)
			return;
		if (IRContent.pst_dst_img.puc_data)
			porting_free_mem(IRContent.pst_dst_img.puc_data);
		IRContent.pst_dst_img.puc_data = (unsigned char*)pBuf;
	}
	if (IrPara.st_zoom_para.n_ZoomType == BilinearFast)
	{
		int srcH = gHeight;
		int srcW = gWidth;
		double scaleH = IrPara.st_zoom_para.fZoom;
		double scaleW = IrPara.st_zoom_para.fZoom;
		int dstW = int(srcW * scaleW);
		int dstH = int(srcH * scaleH);
		ResizeTable = (unsigned short*)porting_calloc_mem((dstH + dstW + 2) * 2, sizeof(unsigned short), ITA_IMAGE_MODULE);
		GetTable(srcH, srcW, scaleH, scaleW, ResizeTable);
	}

	//初始化伪彩参数
	IrPara.st_psdclr_coef.b_psdclr_switch = 1;
	IrPara.st_psdclr_coef.t_psdclr_type = 0; //默认伪彩类型为0即白热
	IrPara.st_psdclr_coef.n_colorimg_tpye = 0;  //0输出bgr格式三通道 1输出rgba格式 2输出rgb格式三通道
	IrPara.st_psdclr_coef.n_colorimg_len = (int)(3 * IRContent.pst_src_img.n_width * IRContent.pst_src_img.n_height * IrPara.st_zoom_para.fZoom * IrPara.st_zoom_para.fZoom);
	//等温线功能
	IrPara.st_psdclr_coef.b_psdclr_equallinemode = 0;
	if ((IrPara.st_psdclr_coef.b_psdclr_equallinemode == 1) && (IrPara.st_psdclr_coef.b_psdclr_switch == 0))
	{
		IrPara.st_psdclr_coef.b_psdclr_switch = 1;
	}
	if (IrPara.st_psdclr_coef.b_psdclr_equallinemode == 1)
	{
		int y16ArrZoomLen = int(IrPara.st_zoom_para.fZoom * IrPara.st_zoom_para.fZoom * gWidth * IRContent.pst_src_img.n_height);
		y16ArrZoom = (short*)porting_calloc_mem(y16ArrZoomLen, sizeof(short), ITA_IMAGE_MODULE);
	}
	IrPara.st_psdclr_coef.s_equalline_para.type = 0;
	IrPara.st_psdclr_coef.s_equalline_para.color = (255<<16);
	IrPara.st_psdclr_coef.s_equalline_para.otherColor = 0;
	IrPara.st_psdclr_coef.s_equalline_para.highY16 = 5900;
	IrPara.st_psdclr_coef.s_equalline_para.lowY16 = 5800;

	//去锅盖算法开关
	IrPara.st_dgg_para.b_dgg_switch = 0;
	IrPara.st_dgg_para.b_cgm_switch = 0;
	IrPara.st_dgg_para.n_dgg_arith_type = 0;
	IrPara.st_dgg_para.stDggTPara.fDelta = 10;
	IrPara.st_dgg_para.stDggTPara.fLastLastShutterT = 20;
	IrPara.st_dgg_para.stDggTPara.fLastShutterT = 21;
	IrPara.st_dgg_para.stDggTPara.fRealShutterT = 22;
	IrPara.st_dgg_para.stDggTPara.fStartShutterT = 20;

	//输出数据开关
	b_outdata_y16_switch = 0;
	b_outdata_y8_switch = 0;
	//不用再申请内存，使用外面传入的内存，减少一次拷贝和节省内存。
	/*if (b_outdata_y16_switch == 1)
	{
		pus_output_y16 = (short*)porting_calloc_mem(gWidth * gHeight, sizeof(short), ITA_IMAGE_MODULE);
	}
	if (b_outdata_y8_switch == 1)
	{
		puc_output_y8 = (unsigned char*)porting_calloc_mem(gWidth * gHeight, sizeof(unsigned char), ITA_IMAGE_MODULE);
	}*/
}

//clock_t start, finish;
//double duration_tff = 0,duration_rvs = 0, duration_rhs = 0, duration_rn = 0, duration_flip = 0, duration_rotate = 0,
//duration_iie = 0, duration_sp = 0, duration_y8adjustbc = 0, duration_zoom = 0;

//红外图像处理主函数
void CInfraredCore::InfraredImageProcess()
{

	IRContent.pst_src_img.n_width = gWidth;
	IRContent.pst_src_img.n_height = gHeight;
	IRContent.pst_src_img.n_len = gWidth * gHeight;
	IRContent.pst_dst_img.n_width = gWidth;
	IRContent.pst_dst_img.n_height = gHeight;
	IRContent.pst_dst_img.n_len = gWidth * gHeight;
	
	//非均匀校正
	if (IrPara.st_nuc_para.b_nuc_switch)
	{
		NUCbyTwoPoint();
		memcpy(IRContent.pst_src_img.pus_data, IRContent.pst_dst_img.pus_data, IRContent.pst_dst_img.n_len * sizeof(short));
	}

	//去锅盖算法
	if (IrPara.st_dgg_para.b_dgg_switch)
	{
		DeGuoGai();
		memcpy(IRContent.pst_src_img.pus_data, IRContent.pst_dst_img.pus_data, IRContent.pst_dst_img.n_len * sizeof(short));
	}

	//坏点替换
	if (IrPara.st_rpbp_para.b_rpbp_switch)
	{
		if (*IRContent.pus_gain_mat != 0)  //如果K矩阵未赋值全为0，则不进入。初K为8192，
		{
			if (IrPara.st_rpbp_para.b_rpbp_updatalist_switch)
			{
				GetBadPointList(gBadPointList, IRContent.pus_gain_mat, IRContent.pst_src_img.n_width, IRContent.pst_src_img.n_height);
				IrPara.st_rpbp_para.b_rpbp_updatalist_switch = 0;
			}
			ReplaceBadPoint(IRContent.pst_src_img.pus_data, gBadPointList,IRContent.pus_gain_mat, IRContent.pst_src_img.n_width, IRContent.pst_src_img.n_height);
		}
	}

	if (IrPara.st_rpbp_para.b_rpbp_auto_switch)
	{
		RemoveFrameBadPoint();
		memcpy(IRContent.pst_src_img.pus_data, IRContent.pst_dst_img.pus_data, IRContent.pst_dst_img.n_len * sizeof(short));
	}

	//时域滤波
	if (IrPara.st_tff_para.b_tff_switch)
	{
		TimeFF();
		memcpy(IRContent.pst_src_img.pus_data, IRContent.pst_dst_img.pus_data, IRContent.pst_dst_img.n_len * sizeof(short));
	}
	
	//去竖纹
	if (IrPara.st_rvs_para.b_rs_switch)
	{
		RemoveSVN();
		memcpy(IRContent.pst_src_img.pus_data, IRContent.pst_dst_img.pus_data, IRContent.pst_dst_img.n_len * sizeof(short));
	}
	
	////去横纹
	if (IrPara.st_rhs_para.b_rs_switch)
	{
		RemoveSHN();
		memcpy(IRContent.pst_src_img.pus_data, IRContent.pst_dst_img.pus_data, IRContent.pst_dst_img.n_len * sizeof(short));
	}
	/**************初始化气体检测*****/
	if (IrPara.st_hsm_para.b_hsm_switch) {
		HSM();
		memcpy(IRContent.pst_src_img.pus_data, IRContent.pst_dst_img.pus_data, IRContent.pst_dst_img.n_len * sizeof(short));
	}
	/**************************************/
	//空域滤波
	if (IrPara.st_rn_para.b_rn_switch)
	{
		SpaceFilter();
		memcpy(IRContent.pst_src_img.pus_data, IRContent.pst_dst_img.pus_data, IRContent.pst_dst_img.n_len * sizeof(short));
	}

	//计算锅盖模板
	if (IrPara.st_dgg_para.b_cgm_switch)
	{
		CalGuoGaiModel();
		IrPara.st_dgg_para.b_cgm_switch = 0;
	}

	//旋转
	if (IrPara.st_rotate_para.b_rotate_switch)
	{
		ImgRotation();
		memcpy(IRContent.pst_src_img.pus_data, IRContent.pst_dst_img.pus_data, IRContent.pst_dst_img.n_len * sizeof(short));
	}

	//镜像
	if (IrPara.st_flip_para.b_flip_switch)
	{
		ImgFlip();
		memcpy(IRContent.pst_src_img.pus_data, IRContent.pst_dst_img.pus_data, IRContent.pst_dst_img.n_len * sizeof(short));
	}

	//是否输出y16数据
	if (b_outdata_y16_switch)
	{
		n_outdata_y16_width = IRContent.pst_src_img.n_width;
		n_outdata_y16_height = IRContent.pst_src_img.n_height;

		memcpy(pus_output_y16, IRContent.pst_src_img.pus_data, IRContent.pst_src_img.n_len * sizeof(short));
	}
	
	if (IrPara.st_drt_para.b_drt_switch)
	{
		//细节增强或调光
		if (IrPara.st_iie_para.b_iie_switch)
		{
			IIE();
			memcpy(IRContent.pst_src_img.puc_data, IRContent.pst_dst_img.puc_data, IRContent.pst_dst_img.n_len * sizeof(unsigned char));
		}
		else
		{
			ModelDRT(); //调光
			memcpy(IRContent.pst_src_img.puc_data, IRContent.pst_dst_img.puc_data, IRContent.pst_dst_img.n_len * sizeof(unsigned char));
		}

		//Gamma校正
		if (IrPara.st_gmc_para.b_gmc_switch)
		{
			ImgGammaCorrect(IRContent.pst_dst_img.puc_data, IRContent.pst_src_img.puc_data, IRContent.pst_src_img.n_len, nGammaTable);
			memcpy(IRContent.pst_src_img.puc_data, IRContent.pst_dst_img.puc_data, IRContent.pst_dst_img.n_len * sizeof(unsigned char));
		}

		////锐化
		if (IrPara.st_sp_para.b_sp_switch)
		{
			LaplaceSharpen();
			memcpy(IRContent.pst_src_img.puc_data, IRContent.pst_dst_img.puc_data, IRContent.pst_dst_img.n_len * sizeof(unsigned char));
		}

		//Y8亮度对比度调节
		if (IrPara.st_y8adjustbc_para.b_adjustbc_switch)
		{
			ImgY8AdjustBC();
			memcpy(IRContent.pst_src_img.puc_data, IRContent.pst_dst_img.puc_data, IRContent.pst_dst_img.n_len * sizeof(unsigned char));
		}

		//是否输出y8数据
		if (b_outdata_y8_switch)
		{
			n_outdata_y8_width = IRContent.pst_dst_img.n_width;
			n_outdata_y8_height = IRContent.pst_dst_img.n_height;

			memcpy(puc_output_y8, IRContent.pst_dst_img.puc_data, IRContent.pst_dst_img.n_len * sizeof(unsigned char));
		}

		//图像放大
		if (IrPara.st_zoom_para.fZoom != 1)
		{
			ImgResize();
			memcpy(IRContent.pst_src_img.puc_data, IRContent.pst_dst_img.puc_data, IRContent.pst_dst_img.n_len * sizeof(unsigned char));
		}

		//伪彩开则输出rgb或者rgba格式图像，伪彩关则输出单通道图像
		if (IrPara.st_psdclr_coef.b_psdclr_switch)
		{
			PseudoColorMap();  //伪彩
		}
	}
}

//非均匀校正
void CInfraredCore::NUCbyTwoPoint()
{
	int i;
	int n_len = IRContent.pst_src_img.n_len;
	int nZoom = 0X2000;

	// NUC校正
	for (i = 0; i < n_len; i++)
	{
		IRContent.pst_dst_img.pus_data[i] =
			short(((IRContent.pus_gain_mat[i] & 0X7FFF) * (IRContent.pus_x16_mat[i] - IRContent.pus_shutter_base[i])) / nZoom);
	}
}

void CInfraredCore::RemoveFrameBadPoint()
{
	int srcWidth = IRContent.pst_src_img.n_width;
	int srcHeight = IRContent.pst_src_img.n_height;
	int srcLength = IRContent.pst_src_img.n_len;
	short* pus_src = IRContent.pst_src_img.pus_data;
	short* pus_dst = IRContent.pst_dst_img.pus_data;
	int n_width_new = srcWidth + 2;
	int n_height_new = srcHeight + 2;
	int len_pad = n_width_new * n_height_new;
	short us_temp = 0;
	int n_win_sz = 9;
	memset(pus_src_pad, 0, sizeof(short) * len_pad);
	// 局部扩展
	PadMatrix(pus_src_pad, pus_src, srcWidth, srcHeight, 3, 3);
	for (int j = 1; j < srcHeight + 1; j++)
	{
		for (int i = 1; i < srcWidth + 1; i++)
		{
			int k = 0;
			short window[9];
			for (int jj = j - 1; jj < j + 2; ++jj)
			{
				for (int ii = i - 1; ii < i + 2; ++ii)
				{
					window[k++] = pus_src_pad[jj * n_width_new + ii];
				}
			}
			for (int m = 0; m < n_win_sz; m++)
			{
				us_temp = window[m];
				for (int n = m + 1; n < n_win_sz; n++)
				{
					if (us_temp < window[n])
					{
						window[m] = window[n];
						window[n] = us_temp;
						us_temp = window[m];
					}
				}
			}
			pus_dst[(j-1) * srcWidth + (i-1)] = window[4];
		}
	}
}

int CInfraredCore::GetBadPointList(unsigned short* nBadPointList, unsigned short* pus_gain_mat, int n_width, int n_height)
{
	int i, j;
	int nBadPointNum = 1;
	if (nBadPointList == NULL || pus_gain_mat == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}

	for (i = 0; i < n_height; i++)
		for (j = 0; j < n_width; j++)
		{
			if ((pus_gain_mat[i * n_width + j] & 0x8000) == 0)  //判断当前点是否为坏点
			{
				continue;
			}
			else
			{
				if (nBadPointNum > nBadPointNumThr)
				{
					nBadPointList[0] = (unsigned short)nBadPointNumThr;
					nBadPointList[1] = (unsigned short)nBadPointNumThr;
					return ITA_BADPOINT_OVERMUCH;
				}
				else
				{
					nBadPointList[2 * nBadPointNum] = (unsigned short)j;
					nBadPointList[2 * nBadPointNum + 1] = (unsigned short)i;
					nBadPointNum++;
				}
			}
		}

	nBadPointNum--;
	nBadPointList[0] = (unsigned short)nBadPointNum;
	nBadPointList[1] = (unsigned short)nBadPointNum;

	return ITA_OK;
}

int CInfraredCore::ReplaceBadPoint(short* pus_src, unsigned short* nBadPointList, unsigned short* pus_gain_mat, int n_width, int n_height)
{
	// 检查输入图像为非空指针
	if (!pus_src || !nBadPointList)
	{
		return ITA_NULL_PTR_ERR;
	}
	unsigned short us_gain_value = 0;
	short us_neighboor_list[8] = { 0 };
	int i, j, num;
	int m, n;
	int n_pos = 0; //当前点的位置
	int n_neighboor_num = 8; //邻域好点数的计数
	int nBadPointNum = nBadPointList[0];

	for (num = 1; num <= nBadPointNum; num++)
	{
		j = nBadPointList[2 * num];
		i = nBadPointList[2 * num + 1];

		// 3*3邻域内好点个数
		memset(us_neighboor_list, 0, 8 * sizeof(short));
		n_neighboor_num = 0;
		for (m = i - 1; m <= i + 1; m++)
		{
			for (n = j - 1; n <= j + 1; n++)
			{
				if (m == i && n == j) //不统计当前坏点
				{
					continue;
				}
				else if (m >= 0 && m < n_height && n >= 0 && n < n_width)  //(可省略？)
				{
					n_pos = m * n_width + n;
					us_gain_value = pus_gain_mat[n_pos];
					if ((us_gain_value & 0x8000) == 0) // 坏点列表未标记，表明非坏点
					{
						if (n_neighboor_num == 0)
						{
							us_neighboor_list[n_neighboor_num] = pus_src[n_pos];
							n_neighboor_num++;
						}
						else
						{
							int nCurIndex;
							for (nCurIndex = 0; nCurIndex < n_neighboor_num; nCurIndex++)
							{
								if (us_neighboor_list[nCurIndex] < pus_src[n_pos])
									continue;
								else
									break;
							}
							if (nCurIndex < n_neighboor_num)
								memcpy(us_neighboor_list + nCurIndex + 1, us_neighboor_list + nCurIndex, (n_neighboor_num - nCurIndex) * sizeof(short));

							us_neighboor_list[nCurIndex] = pus_src[n_pos];
							n_neighboor_num++;
						}
					}
				}
			}
		}

		// 邻域好点排序  (可省略？)
		if (n_neighboor_num > 0)
		{
			// 用好点数组中的中值替换坏点
			if (n_neighboor_num % 2 == 1)
			{
				pus_src[i*n_width + j] = us_neighboor_list[n_neighboor_num / 2];
			}
			else
			{
				pus_src[i*n_width + j] = us_neighboor_list[n_neighboor_num / 2] / 2 + us_neighboor_list[n_neighboor_num / 2 - 1] / 2;
			}
		}
		else
		{
			for (m = i - 2; m <= i + 2; m++)
			{
				for (n = j - 2; n <= j + 2; n++)
				{
					if (m == i && n == j) //不统计当前坏点
					{
						continue;
					}
					else if (m >= 0 && m < n_height && n >= 0 && n < n_width)  //(可省略？)
					{
						n_pos = m * n_width + n;
						us_gain_value = pus_gain_mat[n_pos];
						if ((us_gain_value & 0x8000) == 0) // 坏点列表未标记，表明非坏点
						{
							goto here; //主要是为了跳出两层循环
						}
					}
				}
			}
		here:
			pus_src[i*n_width + j] = pus_src[us_gain_value];
		}
	}
	return ITA_OK;
}

//计算时域滤波权重表
void CInfraredCore::GetTFFGuassWeight()
{
	float f_arg = 0;
	float f_w = 0;
	float f_weight = 0;
	int n_m = 5;
	int i = 0;
	int n_weight_table_len = 128;
	int n_std = IrPara.st_tff_para.n_tff_std;
	for (i = 0; i < n_weight_table_len; i++)  //n_weight_table_len=128
	{
		f_arg = -(i * i * 0.5f) / (n_std * n_std);   //0.5f(fla)   计算权重时用i * i 像素差的平方
		f_weight = (float)exp(f_arg);
		f_w = 2 * f_weight / (1 + f_weight);
		g_nFFTWeightTable[i] = (int)(0x1000 * pow(f_w, n_m));   //0x1000 为了定点化？？？？
	}
}

void CInfraredCore::PadMatrix(short *pus_pad_mat, short *pus_mat, int n_width, int n_height, int n_win_size_width, int n_win_size_height)
{
	int i, j;
	int n_pad_num_hor = n_win_size_width / 2;
	int n_pad_num_ver = n_win_size_height / 2;
	int n_width_new = n_width + 2 * n_pad_num_hor;
	int n_height_new = n_height + 2 * n_pad_num_ver;
	short* pus_mat_tmp = NULL;
	short* pus_pad_mat_tmp = NULL;

	// 对扩展图像进行赋值。padarray
	//初始化赋值
	pus_mat_tmp = pus_mat;
	pus_pad_mat_tmp = pus_pad_mat + n_pad_num_ver*n_width_new + n_pad_num_hor;
	for (i = 0; i < n_height; i++)
	{
		// 复制整行数据
		memcpy(pus_pad_mat_tmp, pus_mat_tmp, n_width * sizeof(short));

		// 复制每行左右两边的数据
		for (j = 1; j <= n_pad_num_hor; j++)
		{
			*(pus_pad_mat_tmp - j) = *pus_pad_mat_tmp;
			*(pus_pad_mat_tmp + n_width - 1 + j) = *(pus_pad_mat_tmp + n_width - 1);
		}

		pus_mat_tmp += n_width;
		pus_pad_mat_tmp += n_width_new;
	}

	// 前几行和最后几行赋值
	pus_mat_tmp = pus_pad_mat + n_width_new * n_pad_num_ver;
	pus_pad_mat_tmp = pus_pad_mat + n_width_new * (n_height_new - n_pad_num_ver - 1);
	for (i = 0; i < n_pad_num_ver; i++)
	{
		memcpy(pus_pad_mat + n_width_new * i, pus_mat_tmp, n_width_new * sizeof(short));
		memcpy(pus_pad_mat + n_width_new * (n_height_new - 1 - i), pus_pad_mat_tmp, n_width_new * sizeof(short));
	}
}


//时域滤波函数
void CInfraredCore::TimeNoiseFliter(short* pus_dst, short* pus_src, int nWidth, int nHeight, int WinSizeW, int WinSizeH)
{
	int n_w = 0;
	int i, j;
	int n_img_size = nWidth * nHeight;
	int sum = 0;
	int nFilterHalfH = WinSizeH >> 1;
	int nFilterHalfW = WinSizeW >> 1;
	int n_height_pad = nHeight + nFilterHalfH * 2;
	int n_width_pad = nWidth + nFilterHalfW * 2;
	int n_len_pad = n_width_pad * n_height_pad;
	int n_weight_table_len = 128;

	memset(diff, 0, sizeof(short) * nWidth * nHeight);
	memset(pus_src_pad, 0, sizeof(short) * n_len_pad);
	memset(pus_last_pad, 0, sizeof(short) * n_len_pad);
	// 局部扩展
	PadMatrix(pus_src_pad, pus_src, nWidth, nHeight, WinSizeW, WinSizeH);

	nFilterFrame++;
	if (nFilterFrame == 1)
	{
		PadMatrix(pus_last_pad, pus_src, nWidth, nHeight, WinSizeW, WinSizeH);
		memcpy(pus_dst, pus_src, sizeof(short) * nWidth * nHeight);
	}
	else
	{
		PadMatrix(pus_last_pad, g_pusTffLastFrame, nWidth, nHeight, WinSizeW, WinSizeH);

		for (i = 0; i < n_len_pad; i++)
		{
			pus_last_pad[i] = (short)abs((short)pus_src_pad[i] - (short)pus_last_pad[i]);
		}

		short *CurRowPtr, *LastRowPtr, *DiffCurPtr;
		for (i = 0; i < nHeight; i++)
		{
			LastRowPtr = (pus_last_pad + (i + nFilterHalfH - 1) * n_width_pad + nFilterHalfW);
			CurRowPtr = LastRowPtr + n_width_pad;
			DiffCurPtr = diff + i * nWidth;
			for (j = 0; j < nWidth; j++)
			{
				sum = *(LastRowPtr - 1) + *(LastRowPtr) + *(LastRowPtr + 1) + *(LastRowPtr + 2)
					+ *(CurRowPtr - 1) + *(CurRowPtr) + *(CurRowPtr + 1) + *(CurRowPtr + 2);
				*DiffCurPtr = short(sum >> 3);
				LastRowPtr++;
				CurRowPtr++;
				DiffCurPtr++;
			}
		}

		for (i = 0; i < n_img_size; i++)
		{
			if (diff[i] >= n_weight_table_len)
			{
				diff[i] = short(n_weight_table_len - 1);
			}
		}

		for (i = 0; i < n_img_size; i++)
		{
			n_w = g_nFFTWeightTable[diff[i]];//mif表中第3个参数对应的表值
			pus_dst[i] = (short)((n_w * g_pusTffLastFrame[i] + (4096 - n_w) * pus_src[i]) >> 12);
		}
	}

	memcpy(g_pusTffLastFrame, pus_dst, sizeof(short) * nWidth * nHeight);
}

void CInfraredCore::TimeFF()
{
	short* pus_src = IRContent.pst_src_img.pus_data;
	short* pus_dst = IRContent.pst_dst_img.pus_data;
	int nWidth = IRContent.pst_src_img.n_width;
	int nHeight = IRContent.pst_src_img.n_height;
	int WinSizeW = IrPara.st_tff_para.n_tff_win_size_w;
	int WinSizeH = IrPara.st_tff_para.n_tff_win_size_h;
	TimeNoiseFliter(pus_dst, pus_src, nWidth, nHeight, WinSizeW, WinSizeH);
}

void CInfraredCore::FixedPoint_GrayFilter_16bit_RVN(short *pus_dst, unsigned short *pus_weight, short *pus_src, int n_width, int n_height, int* gray_weight_table, int n_win_width, int n_win_height)
{
	int i, j;
	int n;
	//int n_mask_area = 0;
	int nZoom = 4096;
	int n_mask_area = nZoom / (n_win_width * n_win_height);
	int n_pad_num_width = (n_win_width - 1) / 2;   //n_win_width滤波器宽度
	int n_pad_num_height = 0;
	int n_width_new = n_width + 2 * n_pad_num_width;
	int n_height_new = n_height + 2 * n_pad_num_height;
	int n_gray_diff = 0;  //gauss灰度强度滤波器索引
	int n_weight = 0;  //卷积窗口内像素的权重

	memset(pus_src_pad, 0, n_width_new * n_height_new * sizeof(short));
	// 扩展图像
	PadMatrix(pus_src_pad, pus_src, n_width, n_height, n_win_width, n_win_height);  //先对行进行扩展 把第一列和最后一列的图像复制给扩展后的图像 再对列进行扩展 把第一行和最后一行复制给扩展后的行

#ifdef AlgOpt
	int n_len = n_width * n_height;
	int n_pad_num_width2 = 2 * n_pad_num_width;
	long int* n_mat_weight_sum = mat_weight_sum;
	long int* n_mat_value_sum = mat_value_sum;
	int* n_gray_weight_table = gray_weight_table;
	memset(n_mat_weight_sum, 0, n_len * sizeof(long int));
	memset(n_mat_value_sum, 0, n_len * sizeof(long int));
	for (i = 0; i < n_height; i++)   //遍历每个像元
	{
		for (j = 0; j < n_width; j++)
		{

			for (n = 0; n <= n_pad_num_width2; n++)  //在滤波器内进行操作
			{
				n_gray_diff = (int)abs(pus_src_pad[i * n_width_new + j + n_pad_num_width] - pus_src_pad[i * n_width_new + j + n]);   //求搜索窗中中心点像素到搜索窗中其他点的差值
				if (n_gray_diff >= GRAY_WEIGHT_TABLE_LEN) // 注意不能超出权重表
				{
					n_gray_diff = GRAY_WEIGHT_TABLE_LEN - 1;
				}

				n_weight = n_gray_weight_table[n_gray_diff];  //查找权重表得到权值
				n_mat_weight_sum[(i - n_pad_num_height) * n_width + j] += n_weight;
				n_mat_value_sum[(i - n_pad_num_height) * n_width + j] += pus_src_pad[i * n_width_new + j + n] * n_weight;  //加权得到滤波器中所有像素的加权和
			}
		}
	}
	//遍历每个像元
	for (i = 0; i < n_len; i++)   
	{
		if (n_mat_weight_sum[i] > 0)
		{
			pus_dst[i] = pus_src[i] - (short)(n_mat_value_sum[i] / n_mat_weight_sum[i]);
		}
		else
		{
			pus_dst[i] = 0;
		}
	}

	for (i = 0; i < n_len; i++)   //遍历每个像元
	{
		pus_weight[i] = (unsigned short)(n_mat_weight_sum[i] * n_mask_area >> 12);//归一化的权值到[0,4096]
	}
	n_mat_weight_sum = NULL;
	n_mat_value_sum = NULL;
	n_gray_weight_table = NULL;
#else
	int n_weight_sum, n_value_sum, m;
	for (i = n_pad_num_height; i < n_height + n_pad_num_height; i++)   //遍历每个像元
	{
		for (j = n_pad_num_width; j < n_width + n_pad_num_width; j++)
		{
			n_weight_sum = 0;
			n_value_sum = 0;
			for (m = i - n_pad_num_height; m <= i + n_pad_num_height; m++)
			{
				for (n = j - n_pad_num_width; n <= j + n_pad_num_width; n++)  //在滤波器内进行操作
				{
					n_gray_diff = (int)abs(pus_src_pad[i * n_width_new + j] - pus_src_pad[m * n_width_new + n]);   //求搜索窗中中心点像素到搜索窗中其他点的差值
					if (n_gray_diff >= GRAY_WEIGHT_TABLE_LEN) // 注意不能超出权重表
					{
						n_gray_diff = GRAY_WEIGHT_TABLE_LEN - 1;
					}

					n_weight = gray_weight_table[n_gray_diff];  //查找权重表得到权值
					n_weight_sum += n_weight;
					n_value_sum += pus_src_pad[m * n_width_new + n] * n_weight;  //加权得到滤波器中所有像素的加权和
				}
			}
			// 计算滤波后当前点的值
			if (n_weight_sum > 0)
			{
				pus_dst[(i - n_pad_num_height) * n_width + (j - n_pad_num_width)] = pus_src[(i - n_pad_num_height) * n_width + (j - n_pad_num_width)] - (short)(n_value_sum / n_weight_sum);
			}
			else
			{
				pus_dst[(i - n_pad_num_height) * n_width + (j - n_pad_num_width)] = 0;
			}

			// 像元权值矩阵
			pus_weight[(i - n_pad_num_height) * n_width + (j - n_pad_num_width)] = (unsigned short)(n_weight_sum * n_mask_area >> 12);//归一化的权值到[0,4096]
		}
	}
#endif
}

//去竖条纹
void CInfraredCore::RemoveVerStripe(short *pus_dst, short *pus_src, int width, int height, int win_width, int* gray_weight_table, int weight_thresh, int detail_thresh)
{
	int i, j;
	int n_width = width;
	int n_height = height;
	int n_filter_width = win_width; //滤波器的窗口宽度
	int n_col_noise_thresh = detail_thresh; //列噪声的取值范围阈值
	int n_weight_thresh = weight_thresh;  //权重阈值最大0.8，最小0.5   * 4096
	int n_len = n_width * n_height;
	int nTemp = 0;
	// 初始化
	unsigned short *n_pus_img_weight = pus_img_weight;
	short* n_img_high = g_img_high;
	short* n_ver_mean = g_ver_mean;
	unsigned short *n_col_num = g_col_num;
	memset(n_pus_img_weight, 0, n_len * sizeof(unsigned short));
	memset(n_img_high, 0, n_len * sizeof(short));
	memset(n_ver_mean, 0, n_width * sizeof(short));
	memset(n_col_num, 0, n_width * sizeof(unsigned short));
	FixedPoint_GrayFilter_16bit_RVN(n_img_high, n_pus_img_weight, pus_src,
		n_width, n_height, gray_weight_table, n_filter_width, 1);   //一维单边灰度滤波

	for (i = 0; i < n_height; i++)
	{
		for (j = 0; j < n_width; j++)
		{
			// 权重均值距离1（4096）最近的段对应的细节均值作为当前列噪声
			if (n_pus_img_weight[i * n_width + j] >= n_weight_thresh)
			{
				n_ver_mean[j] += n_img_high[i * n_width + j];
				n_col_num[j]++;
			}
		}
	}


	for (j = 0; j < n_width; j++)
	{
		nTemp = (n_col_num[j] > 0) ? (n_ver_mean[j] / n_col_num[j]) : 0;
		n_ver_mean[j] = short(min_ir(n_col_noise_thresh, max_ir(-n_col_noise_thresh, nTemp)));
	}

	// 减去当前列的条纹噪声
	for (i = 0; i < n_height; i++)
	{
		for (j = 0; j < n_width; j++)
		{
			pus_dst[i * n_width + j] = pus_src[i * n_width + j] - n_ver_mean[j];
		}
	}
	n_pus_img_weight = NULL;
	n_img_high = NULL;
	n_ver_mean = NULL;
	n_col_num = NULL;
}

void CInfraredCore::RemoveSVN()
{
	short* pus_src = IRContent.pst_src_img.pus_data;
	short* pus_dst = IRContent.pst_dst_img.pus_data;
	int nWidth = IRContent.pst_src_img.n_width;
	int nHeight = IRContent.pst_src_img.n_height;

	int win_width = IrPara.st_rvs_para.n_stripe_win_width;
	int* gray_weight_table_Ver = g_gray_weight_table_Ver;
	int weight_thresh = IrPara.st_rvs_para.n_stripe_weight_thresh;
	int detail_thresh = IrPara.st_rvs_para.n_stripe_detail_thresh;
	RemoveVerStripe(pus_dst, pus_src, nWidth, nHeight, win_width, gray_weight_table_Ver, weight_thresh, detail_thresh);
}


void CInfraredCore::FixedPoint_GrayFilter_16bit_RHN(short *pus_dst, unsigned short *pus_weight, short *pus_src, int n_width, int n_height, int* gray_weight_table, int n_win_width, int n_win_height)
{
	unsigned short n_gray_diff = 0;  //gauss灰度强度滤波器索引
	int i, j, m, n;
	int nZoom = 4096;
	int n_mask_area = nZoom / (n_win_width * n_win_height);
	int n_pad_num_width = (n_win_width - 1) / 2;   //n_win_width滤波器宽度
	int n_pad_num_height = (n_win_height - 1) / 2;
	int n_width_new = n_width + 2 * n_pad_num_width;
	int n_height_new = n_height + 2 * n_pad_num_height;
	int n_weight = 0;  //卷积窗口内像素的权重
	int nCurIndex = 0;
	int nCurDstIndex = 0;
	int nCurWinIndex = 0;
	long long n_weight_sum = 0;  //卷积窗口内所有像素的权重和
	long long n_value_sum = 0;  //卷积和

	memset(pus_src_pad, 0, n_width_new * n_height_new * sizeof(short));
	// 扩展图像
	PadMatrix(pus_src_pad, pus_src, n_width, n_height, n_win_width, n_win_height);  //先对行进行扩展 把第一列和最后一列的图像复制给扩展后的图像 再对列进行扩展 把第一行和最后一行复制给扩展后的行

	for (i = n_pad_num_height; i < n_height + n_pad_num_height; i++)   //遍历每个像元
	{
		nCurIndex = i * n_width_new + n_pad_num_width; //nCurIndex = i * n_width_new + j;
		nCurDstIndex = (i - n_pad_num_height) * n_width; // nCurDstIndex = (i - n_pad_num_height) * n_width + (j - n_pad_num_width);
		for (j = n_pad_num_width; j < n_width + n_pad_num_width; j++)
		{

			n_weight_sum = 0;
			n_value_sum = 0;
			for (m = i - n_pad_num_height; m <= i + n_pad_num_height; m++)
			{
				nCurWinIndex = m * n_width_new + j - n_pad_num_width; // nCurWinIndex = m*n_width_new+n
				for (n = j - n_pad_num_width; n <= j + n_pad_num_width; n++)  //在滤波器内进行操作
				{
					n_gray_diff = (unsigned short)abs(*(pus_src_pad + nCurIndex) - *(pus_src_pad + nCurWinIndex));
					//n_gray_diff = (unsigned short)abs(pus_src_pad[nCurIndex] - pus_src_pad[nCurWinIndex]);   //求搜索窗中中心点像素到搜索窗中其他点的差值
					if (n_gray_diff >= GRAY_WEIGHT_TABLE_LEN) //注意不能超出权重表
					{
						n_gray_diff = GRAY_WEIGHT_TABLE_LEN - 1;
					}

					n_weight = *(gray_weight_table + n_gray_diff);  //查找权重表得到权值
					n_weight_sum += n_weight;
					n_value_sum += *(pus_src_pad + nCurWinIndex) * n_weight;  //加权得到滤波器中所有像素的加权和
					nCurWinIndex++;
				}
			}
			// 计算滤波后当前点的值
			if (n_weight_sum > 0)
			{
				pus_dst[nCurDstIndex] = (short)(n_value_sum / n_weight_sum);
			}
			else
			{
				pus_dst[nCurDstIndex] = *(pus_src_pad + nCurIndex);
			}

			// 像元权值矩阵
			pus_weight[nCurDstIndex] = (unsigned short)(n_weight_sum * n_mask_area >> 12);//归一化的权值到[0,4096]
			nCurIndex++;
			nCurDstIndex++;
		}
	}
}

//去横纹
void CInfraredCore::RemoveHorStripe(short* pus_dst, short *pus_src, int n_width, int n_height, int win_height, int* gray_weight_table, int weight_thresh, int detail_thresh)
{
	int i, j;
	int n_filter_height = win_height;      //滤波器的窗口宽度
	int n_row_noise_thresh = detail_thresh;         //列噪声的取值范围阈值
	int n_weight_thresh = weight_thresh;  //权重阈值最大0.8，最小0.5,×4096
	int n_len = n_width * n_height;
	int n_row_sum_buffer = 0;
	int n_row_num = 0;
	int n_mean = 0;

	// 初始化
	memset(pus_img_low, 0, n_len * sizeof(short));
	memset(pus_img_weight, 0, n_len * sizeof(unsigned short));
	memset(g_img_high, 0, n_len * sizeof(short));
	memset(g_row_mean, 0, n_height * sizeof(short));

	FixedPoint_GrayFilter_16bit_RHN(pus_img_low, pus_img_weight, pus_src, n_width, n_height, gray_weight_table, 1, n_filter_height);

	// 直接求解细节图像 pn_img_high
	for (i = 0; i < n_len; i++)
	{
		g_img_high[i] = pus_src[i] - pus_img_low[i];
		g_img_high[i] = (short)(min_ir(n_row_noise_thresh, max_ir(-n_row_noise_thresh, g_img_high[i])));	 //相当于去噪？？
	}

	//对细节图求行噪声
	for (i = 0; i < n_height; i++)
	{
		n_row_num = 0;
		n_row_sum_buffer = 0;

		// 权重均值距离1（4096）最近的段对应的细节均值作为当前列噪声
		for (j = 0; j < n_width; j++)
		{
			if (pus_img_weight[i * n_width + j] >= n_weight_thresh)   //像元权值的判断
			{
				n_row_sum_buffer += g_img_high[i * n_width + j];
				n_row_num++;
			}
		}
		n_mean = (n_row_num > 0) ? (n_row_sum_buffer / n_row_num) : 0;	  //横条纹噪声的灰度均值

		g_row_mean[i] = short(min_ir(n_row_noise_thresh, max_ir(-n_row_noise_thresh, n_mean)));
	}

	for (i = 0; i < n_height; i++)
	{
		for (j = 0; j < n_width; j++)
		{
			pus_dst[i*n_width + j] = pus_src[i*n_width + j] - g_row_mean[i];  //减去条纹均值实现去横条纹
		}
	}
}

void CInfraredCore::RemoveSHN()
{
	short* pus_src = IRContent.pst_src_img.pus_data;
	short* pus_dst = IRContent.pst_dst_img.pus_data;
	int nWidth = IRContent.pst_src_img.n_width;
	int nHeight = IRContent.pst_src_img.n_height;

	int win_width = IrPara.st_rhs_para.n_stripe_win_width;
	int* gray_weight_table_Hor = g_gray_weight_table_Hor;
	int weight_thresh = IrPara.st_rhs_para.n_stripe_weight_thresh;
	int detail_thresh = IrPara.st_rhs_para.n_stripe_detail_thresh;
	RemoveHorStripe(pus_dst, pus_src, nWidth, nHeight, win_width, gray_weight_table_Hor, weight_thresh, detail_thresh);
}

void CInfraredCore::HSM() {
	int max_deta = -32768;
	int min_deta = 32768;
	int weight = IrPara.st_hsm_para.n_hsm_weight;
	int nGrayMean = 0;
	int nOriMax = 0;
	int nOriMin = 0;
	int nDiscMax = 0;
	int nDiscMin = 0;
	if (gCurFrameNum == 0) {
		memcpy(pus_pre, IRContent.pst_src_img.pus_data, IRContent.pst_src_img.n_width * IRContent.pst_src_img.n_height * sizeof(short));
	}
	else {
		for (int i = 0; i < IRContent.pst_src_img.n_width * IRContent.pst_src_img.n_height; i++) {
			pus_deta[i] = IRContent.pst_src_img.pus_data[i] - pus_pre[i];
		}
		GetHist(hsmAhist, &nOriMax, &nOriMin, &nDiscMax, &nDiscMin, &nGrayMean, pus_deta, IRContent.pst_src_img.n_width, IRContent.pst_src_img.n_height, GRAYLEVEL_16BIT, 1, 1);
		int range = nDiscMax - nDiscMin + 1;
		if (range < IrPara.st_hsm_para.n_hsm_stayThr)
		{
			weight = IrPara.st_hsm_para.n_hsm_stayWeight;
		}
		else {
			if (range < (IrPara.st_hsm_para.n_hsm_stayThr + IrPara.st_hsm_para.n_hsm_interval)) {
				weight = IrPara.st_hsm_para.f_hsm_k * range + IrPara.st_hsm_para.f_hsm_b;
			}
			else {
				weight = IrPara.st_hsm_para.n_hsm_stayWeight;
			}
		}
		if (IrPara.st_hsm_para.b_midfilter_switch)
		{
			meanTHr(pus_deta, pus_pre, IRContent.pst_src_img.n_width, IRContent.pst_src_img.n_height, IrPara.st_hsm_para.n_midfilter_winSize, IrPara.st_hsm_para.n_point_thr, IrPara.st_hsm_para.n_delta_upthr, IrPara.st_hsm_para.n_delta_downthr);
			memcpy(pus_deta, pus_pre, IRContent.pst_src_img.n_width * IRContent.pst_src_img.n_height * sizeof(short));
		}
		//m_logger->output(LOG_INFO, "r:%d,w:%d", range, weight);
		for (int i = 0; i < IRContent.pst_src_img.n_width * IRContent.pst_src_img.n_height; i++) {
			IRContent.pst_dst_img.pus_data[i] = (short)(Clip16(IRContent.pst_src_img.pus_data[i] + weight * pus_deta[i]));
		}
		memcpy(pus_pre, IRContent.pst_src_img.pus_data, IRContent.pst_src_img.n_width * IRContent.pst_src_img.n_height * sizeof(short));
	}
}

void CInfraredCore::meanTHr(short* pus_src, short* pus_dst, int n_width, int n_height, int winSize, int thr, int deltaUpthr, int deltaDownthr) {
	float sum = 0;
	int startIndex = winSize / 2;
	memcpy(pus_dst, pus_src, n_width * n_height * sizeof(short));
	for (int i = startIndex; i < n_height - startIndex; i++) {
		for (int j = startIndex; j < n_width - startIndex; j++) {
			if (abs(pus_src[i * n_width + j]) < thr) {
				for (int i1 = -startIndex; i1 <= startIndex; i1++) {
					for (int j1 = -startIndex; j1 <= startIndex; j1++) {
						sum += abs(pus_src[(i + i1) * n_width + j +j1]);
					}
				}
				sum /= (winSize * winSize);
				if (sum < thr) {
					pus_dst[i * n_width + j] = 0;
					continue;
				}
			}
			pus_dst[i * n_width + j] = pus_src[i * n_width + j] > deltaUpthr ? deltaUpthr : (pus_src[i * n_width + j] < deltaDownthr ? deltaDownthr : pus_src[i * n_width + j]);
		}
	}
}

void CInfraredCore::GetGuassPosWeightTable(int* a_pos_weight_table, int n_win_wid, int n_win_hei, int std)
{
	int i, j;
	int n_half_width = (n_win_wid - 1) / 2;
	int n_half_height = (n_win_hei - 1) / 2;
	int n_dist_x;
	int n_dist_y;
	float f_arg = 0;
	float f_sum = 0;
	float f_tmp;

	for (i = 0; i < n_win_hei; i++)
	{
		for (j = 0; j < n_win_wid; j++)
		{
			n_dist_x = i - n_half_width;
			n_dist_y = j - n_half_height;
			f_arg = -(n_dist_x * n_dist_x + n_dist_y * n_dist_y) / (2.f * std * std / 100 + 0.01f);
			f_tmp = (float)exp(f_arg);
			f_sum += f_tmp;
			a_pos_weight_table[i * n_win_wid + j] = (int)(f_tmp * 4096 + 0.5f);//精度为1/4096
		}
	}

	//归一化位置高斯空间滤波器
	for (i = 0; i < n_win_wid * n_win_hei; i++)
	{
		a_pos_weight_table[i] = (int)(a_pos_weight_table[i] / f_sum);
	}
}

void CInfraredCore::GetGaussGrayWeightTable(int *pa_gray_weight_table, int n_len, int n_gray_std)
{
	int i;
	float arg = 0;

	//得到灰度权值表
	for (i = 0; i < n_len; i++)
	{
		arg = -(i * i) / (2.0f * n_gray_std * n_gray_std);
		pa_gray_weight_table[i] = (int)(exp(arg) * 4096 + 0.5f);
	}
}

void CInfraredCore::GaussianFilter_16bit(short *pus_dst, short *pus_src, int n_width, int n_height, int n_win_wid, int* pos_weight_table)
{
	int i, j;
	n_win_wid = 3;
	//int n_win_half_wid = (n_win_wid - 1) / 2;
	int n_width_new = n_width + n_win_wid - 1;
	int n_height_new = n_height + n_win_wid - 1;
	long n_val_sum = 0;
	memset(pus_src_pad, 0, n_width_new * n_height_new * sizeof(short));
	// 垫衬扩展范围
	PadMatrix(pus_src_pad, pus_src, n_width, n_height, n_win_wid, n_win_wid);

	// 计算邻域内灰度加权和
	short *CurRowPtr, *LastRowPtr, *NextRowPtr;
	short *DstPixelPtr;
	for (i = 0; i < n_height; i++)
	{
		LastRowPtr = pus_src_pad + i * n_width_new + 1;
		CurRowPtr = LastRowPtr + n_width_new;
		NextRowPtr = CurRowPtr + n_width_new;
		DstPixelPtr = pus_dst + i * n_width;
		for (j = 0; j < n_width; j++)
		{
			n_val_sum = (*(LastRowPtr - 1) * pos_weight_table[0] + *(LastRowPtr) * pos_weight_table[1]
				+ *(LastRowPtr + 1) * pos_weight_table[2] + *(CurRowPtr - 1) * pos_weight_table[3]
				+ *(CurRowPtr) * pos_weight_table[4] + *(CurRowPtr + 1) * pos_weight_table[5]
				+ *(NextRowPtr - 1) * pos_weight_table[6] + *(NextRowPtr) * pos_weight_table[7]
				+ *(NextRowPtr + 1) * pos_weight_table[8]);
			*DstPixelPtr = (short)(n_val_sum >> 12);
			LastRowPtr++;
			CurRowPtr++;
			NextRowPtr++;
			DstPixelPtr++;
		}
	}
	LastRowPtr = NULL;
	CurRowPtr = NULL;
	NextRowPtr = NULL;
	DstPixelPtr = NULL;
	DstPixelPtr = NULL;
}

void CInfraredCore::FixedPoint_GrayFilter_16bit(short *pus_dst, short *pus_src, int n_width, int n_height, int* gray_weight_table, int n_win_width, int n_win_height)
{
	int i, j;
	int m, n;
	int n_pad_num_width = (n_win_width - 1) / 2;
	int n_pad_num_height = (n_win_height - 1) / 2;
	int n_width_new = n_width + 2 * n_pad_num_width;
	int n_height_new = n_height + 2 * n_pad_num_height;
	int n_gray_diff = 0;  //gauss灰度强度滤波器索引
	int n_weight = 0;  //卷积窗口内像素的权重
	memset(pus_src_pad, 0, n_width_new * n_height_new * sizeof(short));
	// 扩展图像
	PadMatrix(pus_src_pad, pus_src, n_width, n_height, n_win_width, n_win_height);

#ifdef AlgOpt
{	
	int n_len = n_width*n_height;
	long int* n_mat_value_sum = mat_value_sum;
	long int* n_mat_weight_sum = mat_weight_sum;
	int* n_gray_weight_table = gray_weight_table;
	memcpy(pus_dst, pus_src, n_len*sizeof(short));
	memset(n_mat_value_sum, 0, n_len * sizeof(long int));
	memset(n_mat_weight_sum, 0, n_len * sizeof(long int));
	for (i = 0; i < n_height; i++)//图片像素遍历
	{
		for (j = 0; j < n_width; j++)
		{
			for (m = 0; m <= 2; m++)   //遍历窗口中像素进行加权
			{
				for (n = 0; n <= 2; n++)
				{
					n_gray_diff = (int)abs(pus_src_pad[(i + 1) * n_width_new + j + 1] - pus_src_pad[(i + m) * n_width_new + (j + n)]);
					if (n_gray_diff >= GRAY_WEIGHT_TABLE_LEN) // 注意不能超出权重表
					{
						n_gray_diff = GRAY_WEIGHT_TABLE_LEN - 1;
					}
					n_weight = n_gray_weight_table[n_gray_diff];
					n_mat_weight_sum[i * n_width + j] += n_weight;
					n_mat_value_sum[i * n_width + j] += (pus_src_pad[(i + m) * n_width_new + (j + n)] * n_weight);
				}
			}
		}
	}
	for (i = 0; i < n_len; i++)
	{
		// 计算滤波后当前点的值
		if (n_mat_weight_sum[i] > 0)
		{
			pus_dst[i] = (short)(n_mat_value_sum[i] / n_mat_weight_sum[i]);
		}
	}
	n_mat_value_sum = NULL;
	n_mat_weight_sum = NULL;
	n_gray_weight_table = NULL;
}
#else
{
	int n_weight_sum = 0;
	int n_value_sum = 0;
	for (i = n_pad_num_height; i < n_height + n_pad_num_height; i++)//图片像素遍历
	{
		for (j = n_pad_num_width; j<n_width + n_pad_num_width; j++)
		{
			n_weight_sum = 0;
			n_value_sum = 0;
			for (m = i - n_pad_num_height; m <= i + n_pad_num_height; m++)   //遍历窗口中像素进行加权
			{
				for (n = j - n_pad_num_width; n <= j + n_pad_num_width; n++)
				{
					n_gray_diff = (int)abs(pus_src_pad[i * n_width_new + j] - pus_src_pad[m * n_width_new + n]);
					if (n_gray_diff >= GRAY_WEIGHT_TABLE_LEN) // 注意不能超出权重表
					{
						n_gray_diff = GRAY_WEIGHT_TABLE_LEN - 1;
					}

					n_weight = gray_weight_table[n_gray_diff];
					n_weight_sum += n_weight;
					n_value_sum += pus_src_pad[m * n_width_new + n] * n_weight;
				}
			}

			// 计算滤波后当前点的值
			if (n_weight_sum > 0)
			{
				pus_dst[(i - n_pad_num_height) * n_width + (j - n_pad_num_width)] = (short)(n_value_sum / n_weight_sum);
			}
			else
			{
				pus_dst[(i - n_pad_num_height) * n_width + (j - n_pad_num_width)] = (short)(pus_src_pad[i * n_width_new + j]);
			}
		}
	}
}
#endif // AlgOpt
}

//空域滤波
void CInfraredCore::SpaceFilter()
{
	short *pus_dst = IRContent.pst_dst_img.pus_data;
	short *pus_src = IRContent.pst_src_img.pus_data;
	int n_width = IRContent.pst_src_img.n_width;
	int n_height = IRContent.pst_src_img.n_height;

	if (RN_TYPE_GAUSSIAN == IrPara.st_rn_para.n_rn_arith_type)
	{
		int* pos_weight_table = g_pos_weight_table;
		int n_win_wid = IrPara.st_rn_para.n_win_size;
		GaussianFilter_16bit(pus_dst, pus_src, n_width, n_height, n_win_wid, pos_weight_table);
	}
	else if (RN_TYPE_GREYSCALE == IrPara.st_rn_para.n_rn_arith_type)
	{
		int n_win_width = IrPara.st_rn_para.n_win_size;
		int n_win_height = IrPara.st_rn_para.n_win_size;
		int* gray_weight_table = g_gray_weight_table;
		FixedPoint_GrayFilter_16bit(pus_dst, pus_src, n_width, n_height, gray_weight_table, n_win_width, n_win_height);
	}
}

//图像翻转
void CInfraredCore::ImgFlip()
{
	short *pus_src = IRContent.pst_src_img.pus_data;
	int n_height = IRContent.pst_src_img.n_height;
	int n_width = IRContent.pst_src_img.n_width;
	short *pus_dst = IRContent.pst_dst_img.pus_data;
	int nfliptpye = IrPara.st_flip_para.nFlipType;

	Flip(pus_dst, pus_src, n_width, n_height, nfliptpye);
}

template<class T>
void CInfraredCore::Flip(T pus_dst, T pus_src, int n_width, int n_height, int nfliptype)
{
	int i, j;
	int nrowindex, nrowindexdst;
	if (nfliptype == 0)//水平镜像
	{
		for (i = 0; i < n_height; i++)
		{
			nrowindex = i * n_width;
			for (j = 0; j < n_width; j++)
			{
				pus_dst[nrowindex + j] = pus_src[nrowindex + (n_width - j - 1)];
			}
		}
	}
	else if (nfliptype == 1)//垂直镜像
	{
		for (i = 0; i < n_height; i++)
		{
			nrowindex = i * n_width;
			nrowindexdst = (n_height - i - 1) * n_width;
			for (j = 0; j < n_width; j++)
			{
				pus_dst[nrowindex + j] = pus_src[nrowindexdst + j];
			}
		}
	}
	else if (nfliptype == 2) //水平+垂直镜像
	{
		for (i = 0; i < n_height; i++)
		{
			nrowindex = i * n_width;
			nrowindexdst = (n_height - i - 1) * n_width;
			for (j = 0; j < n_width; j++)
			{
				pus_dst[nrowindex + j] = pus_src[nrowindexdst + (n_width - j - 1)];
			}
		}
	}
}
template void CInfraredCore::Flip(unsigned char* pus_dst, unsigned char* pus_src, int n_width, int n_height, int nfliptype);
template void CInfraredCore::Flip(short* pus_dst, short* pus_src, int n_width, int n_height, int nfliptype);
//图像旋转
void CInfraredCore::ImgRotation()
{
	short *pus_src = IRContent.pst_src_img.pus_data;
	int n_height = IRContent.pst_src_img.n_height;
	int n_width = IRContent.pst_src_img.n_width;
	short *pus_dst = IRContent.pst_dst_img.pus_data;
	int nrotationtype = IrPara.st_rotate_para.nRotateType;

	Rotation(pus_dst, pus_src, n_width, n_height, nrotationtype);
}

template<class T>
void CInfraredCore::Rotation(T pus_dst, T pus_src, int n_width, int n_height, int nrotationtype)
{
	int i, j;
	if (nrotationtype == 0) //旋转90
	{
		for (i = 0; i < n_width; i++)
		{
			for (j = 0; j < n_height; j++)
			{
				pus_dst[i * n_height + j] = pus_src[(n_height - 1 - j) * n_width + i];
			}
		}

		IRContent.pst_src_img.n_width = n_height;
		IRContent.pst_src_img.n_height = n_width;
		IRContent.pst_dst_img.n_width = n_height;
		IRContent.pst_dst_img.n_height = n_width;
	}
	else if (nrotationtype == 1)//旋转180
	{
		int nrowindex, nrowindexdst;
		for (i = 0; i < n_height; i++)
		{
			nrowindex = i * n_width;
			nrowindexdst = (n_height - i - 1) * n_width;
			for (j = 0; j < n_width; j++)
			{
				pus_dst[nrowindex + j] = pus_src[nrowindexdst + (n_width - j - 1)];
			}
		}
	}
	else if (nrotationtype == 2)//旋转270
	{
		for (i = 0; i < n_width; i++)
		{
			for (j = 0; j < n_height; j++)
			{
				pus_dst[i * n_height + j] = pus_src[j * n_width + n_width - 1 - i];
			}
		}

		IRContent.pst_src_img.n_width = n_height;
		IRContent.pst_src_img.n_height = n_width;
		IRContent.pst_dst_img.n_width = n_height;
		IRContent.pst_dst_img.n_height = n_width;
	}

}

template void CInfraredCore::Rotation(unsigned char* pus_dst, unsigned char* pus_src, int n_width, int n_height, int nrotationtype);
template void CInfraredCore::Rotation(short* pus_dst, short* pus_src, int n_width, int n_height, int nrotationtype);

//获取Gamma表
void CInfraredCore::ImgGetGammaTable(unsigned char *GammaTable, float fGamma)
{
	int i = 0;
	memset(GammaTable, 0, 256 * sizeof(unsigned char));
	if (IrPara.st_gmc_para.b_gmc_type == GAMMACOR_TYPE_SINGLE)
	{
		for (i = 0; i < 256; i++)
		{
			GammaTable[i] = (unsigned char)(ClipU8(255 * pow(i / 255.0f, fGamma)));
		}
	}
	else if (IrPara.st_gmc_para.b_gmc_type == GAMMACOR_TYPE_DOUBLE)
	{
		for (i = 0; i < 128; i++)
		{
			GammaTable[i] = (unsigned char)(ClipU8(128 * pow(i / 128.0, fGamma)));
			GammaTable[255 - i] = (unsigned char)(ClipU8(255 - GammaTable[i]));
		}
	}
}

//Gamma校正
void CInfraredCore::ImgGammaCorrect(unsigned char* nPucDst, unsigned char* nPucSrc, int nLen, unsigned char *GammaTable)
{
	int i = 0;
	int nTempValue = 0;

	for (i = 0; i < nLen; i++)
	{
		nTempValue = GammaTable[*(nPucSrc + i)];
		*(nPucDst + i) = (unsigned char)(ClipU8(nTempValue));
	}
}

//拉普拉斯锐化
void CInfraredCore::LaplaceSharpen(unsigned char *pus_dst, unsigned char *pus_src, int n_width, int n_height, float weight)
{
	int i, j;
	int temp = 0;
	int nCurRowIndex = 0;
	int nLastRowIndex = 0;
	int nLastRowIndex0 = 0;
	int nNextRowIndex = 0;
	int nDstCurIndex = 0;
	int nZoom = 1024;
	int nWeightZoom = int(weight * nZoom);
	memcpy(pus_dst, pus_src, n_width * n_height * sizeof(unsigned char));

	for (i = 1; i < n_height - 1; i++)
	{
		nDstCurIndex = i * n_width + 1;
		nLastRowIndex0 = nDstCurIndex - 1 - n_width;
		for (j = 1; j < n_width - 1; j++)
		{
			nLastRowIndex = nLastRowIndex0 + j - 1;
			nCurRowIndex = nLastRowIndex + n_width;
			nNextRowIndex = nCurRowIndex + n_width;
			temp = (int)(((nZoom + nWeightZoom) * pus_src[nCurRowIndex + 1]
				- (nWeightZoom*((pus_src[nLastRowIndex]
					+ pus_src[nLastRowIndex + 1]
					+ pus_src[nLastRowIndex + 2]
					+ pus_src[nNextRowIndex]
					+ pus_src[nNextRowIndex + 1]
					+ pus_src[nNextRowIndex + 2]
					+ pus_src[nCurRowIndex]
					+ pus_src[nCurRowIndex + 2]) >> 3)))) >> 10;
			
			pus_dst[nDstCurIndex] = (unsigned char)(ClipU8(temp));
			nDstCurIndex++;
			nLastRowIndex++;
			/*nCurRowIndex++;
			nNextRowIndex++;*/
		}
	}
}

void CInfraredCore::LaplaceSharpen()
{
	unsigned char *pus_dst = IRContent.pst_dst_img.puc_data;
	unsigned char *pus_src = IRContent.pst_src_img.puc_data;
	int n_width = IRContent.pst_src_img.n_width;
	int n_height = IRContent.pst_src_img.n_height;
	float weight = IrPara.st_sp_para.n_sp_laplace_weight;
	LaplaceSharpen(pus_dst, pus_src, n_width, n_height, weight);
}


void CInfraredCore::GetHist(unsigned short* pHist, int* pnMaxOri, int* pnMinOri, int* pnMaxDisc, int* pnMinDisc, int* pGrayMean, short *pusSrcImg, int n_width,
	int n_height, int nHistSize, int nUpDiscardRatio, int nDownDiscardRatio)
{
	int i;
	int nPixVal;
	int nHistmax = 0;
	int nHistmin = nHistSize;
	int n_len = n_width * n_height;
	int nSigma_1 = (int)(nDownDiscardRatio * n_len / 1000);//下抛点比例
	int nSigma_2 = (int)(nUpDiscardRatio * n_len / 1000); //上抛点比例
	int nCnt = 0;
	int nOrimin = nHistSize;
	int nOrimax = 0;
	int bFindVal[4] = { 0 };
	long long nGraySum = 0;

	for (i = 0; i < n_len; i++)
	{
		nPixVal = (int)(pusSrcImg[i] + g_nOffset);
		pHist[nPixVal]++;
		nGraySum += nPixVal;

		if (nPixVal < nOrimin)
		{
			nOrimin = nPixVal;
		}else if (nPixVal > nOrimax)
		{
			nOrimax = nPixVal;
		}
	}
	*pnMaxOri = (int)(nOrimax - g_nOffset);
	*pnMinOri = (int)(nOrimin - g_nOffset);
	*pGrayMean = (int)(nGraySum / n_len - g_nOffset); // 灰度均值

	for (i = nOrimin; i <= nOrimax; i++)
	{
		nCnt += pHist[i];

		if (nCnt >= nSigma_1 && pHist[i] > 0 && bFindVal[0] == 0)
		{
			nHistmin = i;
			bFindVal[0] = 1;
			break;
		}
	}

	nCnt = 0;
	for (i = nOrimax; i >= nOrimin; i--)
	{
		nCnt += pHist[i];

		if (nCnt >= nSigma_2 && pHist[i] > 0 && bFindVal[1] == 0)
		{
			nHistmax = i;
			bFindVal[1] = 1;
			break;
		}
	}

	*pnMaxDisc = nHistmax - g_nOffset;
	*pnMinDisc = nHistmin - g_nOffset;
}

void CInfraredCore::DRC_LinearAutoBC(unsigned char* puc_dst, short *pus_src, int n_width, int n_height, int nUpDiscardRatio, int nDownDiscardRatio, int nBrightExp, int nContrastExp, int nRestrainRangeThre)
{

	int i = 0;
	int n_len = n_width * n_height;
	int nGrayRange = 0;  //抛点后动态范围    
	int nBright = 0;
	int nContrast = 0;
	int nCompensate = 0;//64;  //动态调整动态范围补偿值，防止动态范围过小时，出现图像对比度过大的现象
	int nGrayMean = 0;
	int nOriMax = 0;
	int nOriMin = 0;
	int nDiscMax = 0;
	int nDiscMin = 0;
	int nZoom = 1024;
	int nHistSize = GRAYLEVEL_16BIT;
	unsigned char pMapTable[GRAYLEVEL_16BIT] = { 0 };
	unsigned short aHist[GRAYLEVEL_16BIT] = { 0 };

	// 统计直方图
	GetHist(aHist, &nOriMax, &nOriMin, &nDiscMax, &nDiscMin, &nGrayMean, pus_src, n_width, n_height, nHistSize, nUpDiscardRatio, nDownDiscardRatio);
	nGrayRange = nDiscMax - nDiscMin + 1;

	if (gCurFrameNum < FRAME_SMOOTH_NUM)
	{
		g_nRangeSmooth[gCurFrameNum] = nGrayRange;
		nGrayRange = 0;
		for (i = 0; i < (gCurFrameNum + 1); i++)
		{
			nGrayRange += g_nRangeSmooth[i];
		}
		nGrayRange /= (gCurFrameNum + 1);
	}
	else
	{
		memcpy(g_nRangeSmooth, g_nRangeSmooth + 1, (FRAME_SMOOTH_NUM - 1) * sizeof(int));
		g_nRangeSmooth[FRAME_SMOOTH_NUM - 1] = nGrayRange;
		nGrayRange = 0;
		for (i = 0; i < FRAME_SMOOTH_NUM; i++)
		{
			nGrayRange += g_nRangeSmooth[i];
		}
		nGrayRange /= FRAME_SMOOTH_NUM;
	}
	gCurFrameNum++;

	//// 自适应计算补偿量
	if (nGrayRange < nRestrainRangeThre)
	{
		nCompensate = nRestrainRangeThre - nGrayRange;
	}
	else
	{
		nCompensate = 0;
	}
	nContrast = nZoom * nContrastExp / (nGrayRange + nCompensate * 4);

	// 最大对比度限制
	if (nContrast > nZoom * g_nMaxContrast)
	{
		nContrast = nZoom * g_nMaxContrast;
	}
	nBright = nBrightExp - (nGrayMean * nContrast) / nZoom;

	int nIndex;
	for (i = nOriMin; i <= nOriMax; i++)
	{
		nIndex = i + g_nOffset;
		pMapTable[nIndex] = (unsigned char)(ClipU8(nContrast * i / nZoom + nBright));
	}

	for (i = 0; i < n_len; i++)
	{
		puc_dst[i] = pMapTable[pus_src[i] + g_nOffset];
	}
}


void CInfraredCore::DRC_ManualTone(unsigned char* puc_dst, short* pus_src, int n_width, int n_height, short nTmaxToY16, short nTminToY16)
{
	int i = 0;
	int nTemp = 0;
	int nLen = n_width * n_height;
	int nY8Min = 0;
	int nY8Max = 255;

	if ((nTmaxToY16 - nTminToY16) < 0)
	{
		memset(puc_dst, 0, nLen * sizeof(unsigned char));
	}
	else if ((nTmaxToY16 - nTminToY16) == 0)
	{
		for (i = 0; i < nLen; i++)
		{
			if (pus_src[i] < nTminToY16)
			{
				nTemp = nY8Min;
			}
			else if (pus_src[i] >= nTminToY16)
			{
				nTemp = nY8Max;
			}
			puc_dst[i] = (unsigned char)(ClipU8(nTemp));
		}
	}
	else
	{
		int nZoom = 9;  //定点化
		int nK = ((nY8Max - nY8Min) << nZoom) / (nTmaxToY16 - nTminToY16);

		for (i = 0; i < nLen; i++)
		{
			if (pus_src[i] < nTminToY16)
			{
				nTemp = nY8Min;
			}
			else if (pus_src[i] > nTmaxToY16)
			{
				nTemp = nY8Max;
			}
			else
			{
				nTemp = ((nK * (pus_src[i] - nTminToY16)) >> nZoom) + nY8Min;
			}
			puc_dst[i] = (unsigned char)(ClipU8(nTemp));
		}
	}
}

void CInfraredCore::DRC_PersonMedical(unsigned char* puc_dst, short *ps_src, int n_width, int n_height, short Y16_Tbase, short Y16_high)
{
	unsigned short pHist[65535] = { 0 };
	float S = 200.0f / 30;
	int i;
	int val = 0;
	int nOffset = 32768;
	int nHistSize = 65535;
	int nHistmin = 65535;
	int nHistmax = 0;
	int n_len = n_width * n_height;
	int nHistCnt = 0;
	int Y16_Tbase_lower = Y16_Tbase - 200;
	int bFindVal[3] = { 0 };
	int nPixCount = 0;
	int nPixVal;
	long long nGraySum = 0;

	if (Y16_high < Y16_Tbase)
	{
		Y16_high = Y16_Tbase + 300;
	}
	Y16_high = 550;
	for (i = 0; i < n_len; i++)
	{
		nPixVal = (int)ps_src[i] + nOffset;
		if (nPixVal < nHistSize && ps_src[i] <= Y16_Tbase_lower)
		{//只统计小于Y16_Tbase_lower的Y16值
			pHist[nPixVal]++;
			nHistCnt++;
			nGraySum += ps_src[i];
			nPixCount++;
		}
	}

	if (nPixCount > 0)
	{// 最低温低于Y16_Tbase_lower，则低于Y16_Tbase_lower的部分进行自动线性调光
		int nSigma_1 = int(nHistCnt * 0.05);
		int nSigma_2 = int(nHistCnt * 0.99);
		int nSum = 0;
		for (i = 0; i < 65535; i++)
		{// 抛点最大最小值
			nSum += pHist[i];
			if (nSum >= nSigma_1 && pHist[i] > 0 && bFindVal[1] == 0)
			{
				nHistmin = i;
				bFindVal[1] = 1;
			}

			if (nSum >= nSigma_2 && pHist[i] > 0 && bFindVal[2] == 0)
			{
				nHistmax = i;
				bFindVal[2] = 1;
			}
		}
		nHistmax = nHistmax - nOffset;
		nHistmin = nHistmin - nOffset;

		float fContrast = (205 - 200.0f / S) / (Y16_Tbase_lower - nHistmin);
		fContrast = float(fmax(0.15, fContrast));

		for (i = 0; i < n_len; i++)
		{
			if (ps_src[i] < Y16_Tbase_lower)
			{// // 低于Tbase的，采用平台直方图映射到0~205 - 200.0 / S
				val = (int)(fContrast * (ps_src[i] - nHistmin)); //自适应线性调光
				val = int(fmax(0, fmin(val, 205 - 200.0 / S)));
			}
			else if (ps_src[i] <= Y16_high)
			{
				val = 205 + int((ps_src[i] - Y16_Tbase_lower - 200) * 1.0 / S);
			}
			else if (ps_src[i] > Y16_high)
			{
				val = 255;
			}
			val = int(fmax(0, fmin(255, val)));

			puc_dst[i] = (unsigned char)val;
		}
	}
	else
	{// 最低温高于Tbase，则直接按线性映射的方式固定到210以上颜色
		for (i = 0; i < n_len; i++)
		{
			val = 0;
			if (ps_src[i] <= Y16_high)
			{
				val = 205 + int((ps_src[i] - Y16_Tbase_lower - 200.0) * 1.0 / S);
			}
			else if (ps_src[i] > Y16_high)
			{
				val = 255;
			}
			val = int(fmax(0, fmin(255, val)));
			puc_dst[i] = (unsigned char)val;
		}
	}
}


void CInfraredCore::DRC_PlatHistEqualize(unsigned char* puc_dst, short *pus_src, int n_width, int n_height, int nPlatThresh, int nUpDiscardRatio, int nDownDiscardRatio, int nEqhistMax, int nMidValue)
{
	unsigned char pMapTable[GRAYLEVEL_16BIT] = { 0 };
	unsigned short pHist[GRAYLEVEL_16BIT] = { 0 };
	int i, nIndex;
	int n_len = n_width * n_height;
	unsigned int pHistCum[GRAYLEVEL_16BIT] = { 0 };
	unsigned int nHistCumMax = 0;
	int nHistSize = GRAYLEVEL_16BIT;
	int nOriMax = 0;
	int nOriMin = 0;
	int nDiscMax = 0;
	int nDiscMin = 0;
	int nGrayMean = 0;
	int nGrayRange = 0;
	int nY8Range = 0;
	int nY8start = 0;
	
	// 统计直方图
	GetHist(pHist, &nOriMax, &nOriMin, &nDiscMax, &nDiscMin, &nGrayMean, pus_src, n_width, n_height, nHistSize, nUpDiscardRatio, nDownDiscardRatio);
	// Y16动态范围			
	nGrayRange = nDiscMax - nDiscMin + 1;   //Y16抛点的动态范围

	if (gCurFrameNum < FRAME_SMOOTH_NUM)
	{
		g_nRangeSmooth[gCurFrameNum] = nGrayRange;
		nGrayRange = 0;
		for (i = 0; i < (gCurFrameNum + 1); i++)
		{
			nGrayRange += g_nRangeSmooth[i];
		}
		nGrayRange /= (gCurFrameNum + 1);
	}
	else
	{
		memcpy(g_nRangeSmooth, g_nRangeSmooth + 1, (FRAME_SMOOTH_NUM - 1) * sizeof(int));
		g_nRangeSmooth[FRAME_SMOOTH_NUM - 1] = nGrayRange;
		nGrayRange = 0;
		for (i = 0; i < FRAME_SMOOTH_NUM; i++)
		{
			nGrayRange += g_nRangeSmooth[i];
		}
		nGrayRange /= FRAME_SMOOTH_NUM;
	}
	gCurFrameNum++;


	// 平台阈值处理并累计
	unsigned int temp = 0;
	for (i = nOriMin; i <= nOriMax; i++)
	{
		nIndex = i + g_nOffset;
		if (pHist[nIndex] > nPlatThresh)   //某一灰度级直方图数量超过平台阈值
		{
			pHist[nIndex] = (unsigned short)nPlatThresh;
		}
		temp += pHist[nIndex];     //直方图前面的累加和   pHist[i]为什么要除以2？？？？？？ 除2有影响？？ 除以2是为了降低均衡化后图像的动态范围？？？
		pHistCum[nIndex] = (unsigned int)temp;
	}
	nHistCumMax = pHistCum[nDiscMax + g_nOffset];
	//新增 当Y16动态范围较小时减小直方图均衡的程度，抑制锅盖
	if (nGrayRange < nEqhistMax)
	{
		nEqhistMax = nGrayRange;
	}

	nY8Range = nEqhistMax;
	nY8start = nMidValue - nY8Range / 2;
	float fTemp = nY8Range * 1.0f / nHistCumMax;//除法预先计算
	for (i = nOriMin; i <= nOriMax; i++)
	{
		nIndex = i + g_nOffset;
		pMapTable[nIndex] = (unsigned char)(ClipU8(pHistCum[nIndex] * fTemp + nY8start));
	}

	for (i = 0; i < n_len; i++)
	{
		nIndex = pus_src[i] + g_nOffset;
		puc_dst[i] = pMapTable[nIndex];
	}
}

void CInfraredCore::DRC_Mix(unsigned char* puc_dst, short *pus_src, int n_width, int n_height, int nMixContrastExp, int nMixBrightExp, int nRestrainRangeThre, int n_mixrange, int nMidValue, int nPlatThresh, int nUpDiscardRatio, int nDownDiscardRatio, int nMixThrHigh, int nMixThrLow)
{
	int i = 0;
	int n_len = n_width * n_height;
	int nGrayRange = 0;  //抛点后动态范围    
	int nBright = 0;
	int nContrast = 0;
	int nCompensate = 0;//64;  //动态调整动态范围补偿值，防止动态范围过小时，出现图像对比度过大的现象
	int nGrayMean = 0;
	int nOriMax = 0;
	int nOriMin = 0;
	int nDiscMax = 0;
	int nDiscMin = 0;
	int nIndex;
	int nY8start;
	int nY8Range;
	int nZoom = 1024;
	int nHistCumMax;
	unsigned short aHist[GRAYLEVEL_16BIT] = { 0 };
	unsigned int pHistCum[GRAYLEVEL_16BIT] = { 0 };
	unsigned char pMapTable[GRAYLEVEL_16BIT] = { 0 };

	// 统计直方图
	GetHist(aHist, &nOriMax, &nOriMin, &nDiscMax, &nDiscMin, &nGrayMean, pus_src, n_width, n_height, GRAYLEVEL_16BIT, nUpDiscardRatio, nDownDiscardRatio);
	nGrayRange = nDiscMax - nDiscMin + 1;

	if (gCurFrameNum < FRAME_SMOOTH_NUM)
	{
		g_nRangeSmooth[gCurFrameNum] = nGrayRange;
		nGrayRange = 0;
		for (i = 0; i < (gCurFrameNum + 1); i++)
		{
			nGrayRange += g_nRangeSmooth[i];
		}
		nGrayRange /= (gCurFrameNum + 1);
	}
	else
	{
		memcpy(g_nRangeSmooth, g_nRangeSmooth + 1, (FRAME_SMOOTH_NUM - 1) * sizeof(int));
		g_nRangeSmooth[FRAME_SMOOTH_NUM - 1] = nGrayRange;
		nGrayRange = 0;
		for (i = 0; i < FRAME_SMOOTH_NUM; i++)
		{
			nGrayRange += g_nRangeSmooth[i];
		}
		nGrayRange /= FRAME_SMOOTH_NUM;
	}
	gCurFrameNum++;


	//// 自适应计算补偿量
	if (nGrayRange < nRestrainRangeThre)
	{
		nCompensate = nRestrainRangeThre - nGrayRange;
	}
	else
	{
		nCompensate = 0;
	}
	nContrast = nZoom * nMixContrastExp / (nGrayRange + nCompensate * 4);

	// 最大对比度限制
	if (nContrast > nZoom * g_nMaxContrast)
	{
		nContrast = nZoom * g_nMaxContrast;
	}
	nBright = nMixBrightExp - (nGrayMean * nContrast) / nZoom;

	// 平台阈值处理并累计
	unsigned int temp = 0;
	for (i = nOriMin; i <= nOriMax; i++)
	{
		nIndex = i + g_nOffset;
		if (aHist[nIndex] > nPlatThresh)   //某一灰度级直方图数量超过平台阈值
		{
			aHist[nIndex] = (unsigned short)nPlatThresh;
		}
		temp += aHist[nIndex];     //直方图前面的累加和
		pHistCum[nIndex] = temp;
	}
	nHistCumMax = (int)(pHistCum[nDiscMax + g_nOffset]);

	//新增 当Y16动态范围较小时减小直方图均衡的程度，抑制锅盖
	if (nGrayRange < n_mixrange)
	{
		n_mixrange = nGrayRange;
	}

	// 自适应权值
	int nAlpha = 0;
	//int nThrHigh = 300;
	//int nThrLow = 100;
	int nThrLowL = int(nMixThrLow * 0.2);
	if (nGrayRange >= nMixThrHigh)
	{
		nAlpha = 204;
	}
	else if (nGrayRange < nMixThrHigh && nGrayRange > nMixThrLow)
	{
		nAlpha = 616 * (nMixThrHigh - nGrayRange) / (nMixThrHigh - nMixThrLow) + 204;
	}
	else if (nGrayRange > nThrLowL && nGrayRange <= nMixThrLow)
	{
		nAlpha = 204 * (nMixThrLow - nGrayRange) / (nMixThrLow - nThrLowL) + 820;
	}
	else if (nGrayRange <= nThrLowL)
	{
		nAlpha = 1024;
	}
	int nBeta = 1024 - nAlpha;

	nY8Range = n_mixrange;
	nY8start = nMidValue - nY8Range / 2;
	short nHistMapValue;
	short nLinearMapValue;
	for (i = nOriMin; i <= nOriMax; i++)
	{
		nIndex = i + g_nOffset;
		nHistMapValue = short(((pHistCum[nIndex] * nY8Range / nHistCumMax)) + nY8start);
		nLinearMapValue = short(((nContrast * i) >> 10) + nBright);
		pMapTable[nIndex] = (unsigned char)(ClipU8((nAlpha * nLinearMapValue + nBeta * nHistMapValue) >> 10));
	}

	for (i = 0; i < n_len; i++)
	{
		puc_dst[i] = pMapTable[pus_src[i] + g_nOffset];
	}
}

//调光
void CInfraredCore::ModelDRT()
{
	//从stIrContent中获取图像内容
	int n_width = IRContent.pst_src_img.n_width;
	int n_height = IRContent.pst_src_img.n_height;
	short *pus_src = IRContent.pst_src_img.pus_data;

	//定义指针指向返回地址
	unsigned char* puc_dst = IRContent.pst_dst_img.puc_data;

	if (DRT_TYPE_LINEAR == IrPara.st_drt_para.n_drt_type)
	{
		//从stIrPara中获取成像参数
		int nUpDiscardRatio = IrPara.st_drt_para.n_discard_upratio;
		int nDownDiscardRatio = IrPara.st_drt_para.n_discard_downratio;
		int nBrightExp = IrPara.st_drt_para.n_linear_brightness;
		int nContrastExp = IrPara.st_drt_para.n_linear_contrast;
		int nRestrainRangeThre = IrPara.st_drt_para.n_linear_restrain_rangethre;
		//线性调光
		DRC_LinearAutoBC(puc_dst, pus_src, n_width, n_height, nUpDiscardRatio, nDownDiscardRatio, nBrightExp, nContrastExp, nRestrainRangeThre);
	}
	else if (DRT_TYPE_PLATHE == IrPara.st_drt_para.n_drt_type)
	{
		int nPlatThresh = IrPara.st_drt_para.n_heq_plat_thresh;
		int nUpDiscardRatio = IrPara.st_drt_para.n_discard_upratio;
		int nDownDiscardRatio = IrPara.st_drt_para.n_discard_downratio;
		int nEqhistMax = IrPara.st_drt_para.n_heq_range_max;
		int nMidValue = IrPara.st_drt_para.n_heq_midvalue;
		DRC_PlatHistEqualize(puc_dst, pus_src, n_width, n_height, nPlatThresh, nUpDiscardRatio, nDownDiscardRatio, nEqhistMax, nMidValue);
	}
	else if (DRT_TYPE_MIX_PHE_LIN == IrPara.st_drt_para.n_drt_type)
	{
		int nMixContrastExp = IrPara.st_drt_para.n_linear_contrast;
		int nMixBrightExp = IrPara.st_drt_para.n_linear_brightness;
		int n_mixrange = IrPara.st_drt_para.n_heq_range_max;
		int nPlatThresh = IrPara.st_drt_para.n_heq_plat_thresh;
		int nUpDiscardRatio = IrPara.st_drt_para.n_discard_upratio;
		int nDownDiscardRatio = IrPara.st_drt_para.n_discard_downratio;
		int nMidValue = IrPara.st_drt_para.n_heq_midvalue;
		int nRestrainRangeThre = IrPara.st_drt_para.n_linear_restrain_rangethre;
		int nMixThrHigh = IrPara.st_drt_para.n_mix_ThrHigh;
		int nMixThrLow = IrPara.st_drt_para.n_mix_ThrLow;;
		DRC_Mix(puc_dst, pus_src, n_width, n_height, nMixContrastExp, nMixBrightExp, nRestrainRangeThre, n_mixrange, nMidValue, nPlatThresh, nUpDiscardRatio, nDownDiscardRatio, nMixThrHigh, nMixThrLow);
	}
	else if (DRT_TYPE_MANUALTONE == IrPara.st_drt_para.n_drt_type)
	{
		short nTmaxToY16 = IrPara.st_drt_para.n_manltone_maxy16;
		short nTminToY16 = IrPara.st_drt_para.n_manltone_miny16;
		DRC_ManualTone(puc_dst, pus_src, n_width, n_height, nTmaxToY16, nTminToY16);
	}
	else if (DRT_TYPE_PERSONMEDICAL == IrPara.st_drt_para.n_drt_type)
	{
		short Y16_Tbase = IrPara.st_drt_para.n_personmedical_y16_base;
		short Y16_high = IrPara.st_drt_para.n_personmedical_y16_high;

		if (IrPara.st_psdclr_coef.t_psdclr_type != PSEUDO_NAME_BODYDRT)
		{
			IrPara.st_psdclr_coef.t_psdclr_type = PSEUDO_NAME_BODYDRT;
		}
		DRC_PersonMedical(puc_dst, pus_src, n_width, n_height, Y16_Tbase, Y16_high);
	}
}

void CInfraredCore::FixedPoint_IIE(unsigned char *puc_dst, short *pus_src, int n_width, int n_height)
{
	unsigned char n_dr = IrPara.st_iie_para.n_iie_detail_thr;
	int i;
	int n_tmp = 0;
	int n_len = n_width * n_height;
	int n_pos_filter_winsize = IrPara.st_iie_para.n_iie_pos_win_size;
	int n_iie_detail_gain = IrPara.st_iie_para.n_iie_enhance_coef;
	int* pos_weight_table = g_IDE_pos_weight_table;
	
	memset(ps_detail_img, 0, n_len * sizeof(short));
	memset(puc_base_img, 0, n_len * sizeof(unsigned char));
	memset(pus_base_img, 0, n_len * sizeof(short));

	// Step_2 Gauss滤波提取细节
	GaussianFilter_16bit(pus_base_img, pus_src, n_width, n_height, n_pos_filter_winsize, pos_weight_table);
	// Step_3 计算细节图像
	for (i = 0; i < n_len; i++)
	{
		ps_detail_img[i] = (short)(pus_src[i] - pus_base_img[i]);
		ps_detail_img[i] = min_ir(n_dr, max_ir(ps_detail_img[i], -n_dr)); 
	}

	if (DRT_TYPE_LINEAR == IrPara.st_drt_para.n_drt_type)
	{
		//从stIrPara中获取成像参数
		int nUpDiscardRatio = IrPara.st_drt_para.n_discard_upratio;
		int nDownDiscardRatio = IrPara.st_drt_para.n_discard_downratio;
		int nBrightExp = IrPara.st_drt_para.n_linear_brightness;
		int nContrastExp = IrPara.st_drt_para.n_linear_contrast;
		int nRestrainRangeThre = IrPara.st_drt_para.n_linear_restrain_rangethre;
		//线性调光
		DRC_LinearAutoBC(puc_base_img, pus_base_img, n_width, n_height, nUpDiscardRatio, nDownDiscardRatio, nBrightExp, nContrastExp, nRestrainRangeThre);
	}
	else if (DRT_TYPE_PLATHE == IrPara.st_drt_para.n_drt_type)
	{
		int nPlatThresh = IrPara.st_drt_para.n_heq_plat_thresh;
		int nUpDiscardRatio = IrPara.st_drt_para.n_discard_upratio;
		int nDownDiscardRatio = IrPara.st_drt_para.n_discard_downratio;
		int nEqhistMax = IrPara.st_drt_para.n_heq_range_max;
		int nMidValue = IrPara.st_drt_para.n_heq_midvalue;
		//平台直方图调光
		DRC_PlatHistEqualize(puc_base_img, pus_base_img, n_width, n_height, nPlatThresh, nUpDiscardRatio, nDownDiscardRatio, nEqhistMax, nMidValue);
	}
	else if (DRT_TYPE_MIX_PHE_LIN == IrPara.st_drt_para.n_drt_type)
	{
		int nMixContrastExp = IrPara.st_drt_para.n_linear_contrast;
		int nMixBrightExp = IrPara.st_drt_para.n_linear_brightness;
		int n_mixrange = IrPara.st_drt_para.n_heq_range_max;
		int nPlatThresh = IrPara.st_drt_para.n_heq_plat_thresh;
		int nUpDiscardRatio = IrPara.st_drt_para.n_discard_upratio;
		int nDownDiscardRatio = IrPara.st_drt_para.n_discard_downratio;
		int nMidValue = IrPara.st_drt_para.n_heq_midvalue;
		int nRestrainRangeThre = IrPara.st_drt_para.n_linear_restrain_rangethre;
		int nMixThrHigh = IrPara.st_drt_para.n_mix_ThrHigh;
		int nMixThrLow = IrPara.st_drt_para.n_mix_ThrLow;
		//混合调光
		DRC_Mix(puc_base_img, pus_base_img, n_width, n_height, nMixContrastExp, nMixBrightExp, nRestrainRangeThre, n_mixrange, nMidValue, nPlatThresh, nUpDiscardRatio, nDownDiscardRatio, nMixThrHigh, nMixThrLow);
	}
	else if (DRT_TYPE_MANUALTONE == IrPara.st_drt_para.n_drt_type)
	{
		short nTmaxToY16 = IrPara.st_drt_para.n_manltone_maxy16;
		short nTminToY16 = IrPara.st_drt_para.n_manltone_miny16;
		DRC_ManualTone(puc_base_img, pus_base_img, n_width, n_height, nTmaxToY16, nTminToY16);
	}
	else if (DRT_TYPE_PERSONMEDICAL == IrPara.st_drt_para.n_drt_type)
	{
		short Y16_Tbase = IrPara.st_drt_para.n_personmedical_y16_base;
		short Y16_high = IrPara.st_drt_para.n_personmedical_y16_high;

		if (IrPara.st_psdclr_coef.t_psdclr_type != PSEUDO_NAME_BODYDRT)
		{
			IrPara.st_psdclr_coef.t_psdclr_type = PSEUDO_NAME_BODYDRT;
		}
		DRC_PersonMedical(puc_base_img, pus_base_img, n_width, n_height, Y16_Tbase, Y16_high);
	}

	// Step_6 叠加细节
	int nZoom = 1024;
	int nCoef = n_iie_detail_gain * (nZoom >> 4);
	for (i = 0; i < n_len; i++)
	{
		n_tmp = ((int)puc_base_img[i] + ((ps_detail_img[i] * nCoef) >> 10));
		puc_dst[i] = (unsigned char)(ClipU8(n_tmp));
	}
}

void CInfraredCore::IIE()
{
	short *pus_src = IRContent.pst_src_img.pus_data;
	int n_width = IRContent.pst_src_img.n_width;
	int n_height = IRContent.pst_src_img.n_height;
	//定义指针指向返回地址
	unsigned char* puc_dst = IRContent.pst_dst_img.puc_data;
	FixedPoint_IIE(puc_dst, pus_src, n_width, n_height);
}

void CInfraredCore::GetTable(int srcH, int srcW, double scaleH, double scaleW, unsigned short* Table)//ix  v iy u
{
	int i, j;
	int dstW = int(srcW * scaleW);
	int dstH = int(srcH * scaleH);
	float InverScaleH = float(1 / scaleH);
	float InverScaleW = float(1 / scaleW);
	double x, y;

	for (j = 0; j < dstW; j++)
	{
		x = j * InverScaleW;  //原图中对应横坐标									   	
		Table[j + dstW + 1] = (unsigned short)((x - (int)x) * 512);
		Table[j] = (unsigned short)x;
		if (Table[j] >= (srcW - 1))
		{
			Table[j] = (unsigned short)(srcW - 1);
			Table[j + dstW + 1] = 0;
		}
			
	}
	Table[dstW + dstW + 1] = 0;
	Table[dstW] = (unsigned short)(srcW - 1);

	for (i = 0; i < dstH; i++)
	{
		//原图中对应纵坐标//u和v为x,y的小数部分,并放大2048倍以规避浮点运算//ix,iy分别存储x,y的整数部分
		y = i * InverScaleH;
		Table[i + dstH + 1 + 2 * (dstW + 1)] = (unsigned short)((y - (int)y) * 512);
		Table[i + 2 * (dstW + 1)] = (unsigned short)y;
		if (Table[i + 2 * (dstW + 1)] >= (srcH - 1))
		{
			Table[i + 2 * (dstW + 1)] = (unsigned short)(srcH - 1);
			Table[i + dstH + 1 + 2 * (dstW + 1)] = 0;
		}	
	}
	Table[dstH + dstH + 1 + 2 * (dstW + 1)] = 0;
	Table[dstH + 2 * (dstW + 1)] = (unsigned short)(dstH - 1);
}

int CInfraredCore::ImgBilinearFast(unsigned char * DstBuf, unsigned char * SrcBuf, int srcH, int srcW, double scaleH, double scaleW, unsigned short* Table)
{
	if ((!Table) || (!SrcBuf) || (!DstBuf))
	{
		return ITA_NULL_PTR_ERR;
	}

	unsigned char * pDstLine = NULL;
	int i, j;
	int zoom = 512;
	int dstW = int(srcW * scaleW);
	int dstH = int(srcH * scaleH);
	if (srcW <= 0 || srcH <= 0 || dstW <= 0 || dstH <= 0)
		return ITA_ILLEGAL_PAPAM_ERR;

	unsigned char* p00;
	unsigned char* p01;
	unsigned char* p10;
	unsigned char* p11;
	unsigned short* iy;
	unsigned short* u;
	unsigned short* ix;
	unsigned short* v;
	if (IrPara.st_rotate_para.b_rotate_switch && ((IrPara.st_rotate_para.nRotateType == 0)||(IrPara.st_rotate_para.nRotateType == 2)))
	{
		iy = Table;
		u = Table + dstH + 1;
		ix = Table + (dstH + 1) * 2;
		v = Table + (dstH + 1) * 2 + dstW + 1;
	}
	else
	{
		ix = Table;
		v = Table + dstW + 1;
		iy = Table + (dstW + 1) * 2;
		u = Table + (dstW + 1) * 2 + dstH + 1;
	}

	int uv = 0;
	int uzoom = 0;
	int vzoom = 0;
	int zoom2 = zoom * zoom;
	int nRowIndex = 0;
	int nCur_u = 0;
	int nCur_v = 0;
	for (i = 0; i < dstH; i++)
	{
		pDstLine = DstBuf + i * dstW;
		nCur_u = *(u + i);
		uzoom = nCur_u << 9;
		nRowIndex = *(iy + i) * srcW;
		
		for (j = 0; j < dstW; j++)
		{
			p00 = SrcBuf + nRowIndex + *(ix + j);
			p01 = p00 + 1;
			p10 = p00 + srcW;
			p11 = p01 + srcW;
			nCur_v = *(v + j);
			uv = nCur_u * nCur_v;
			vzoom = nCur_v << 9;
			*(pDstLine + j) = (unsigned char)(ClipU8(((*p00 * (zoom2 - vzoom - uzoom + uv)
				+ *p01 * (vzoom - uv)
				+ *p10 * (uzoom - uv)
				+ *p11 * uv) >> 18)));
		}
	}
	return ITA_OK;
}

//图像缩放
void CInfraredCore::Resize(unsigned char* puc_dst, unsigned char *puc_src, int n_width, int n_height, float fZoom, int ZoomType)
{
	int i, j;
	int x0, y0;
	int nWidthZoom = int(n_width * fZoom);
	int nHeightZoom = int(n_height * fZoom);
	float fZoomInverse = 1 / fZoom;

	//图像放大后更新宽高
	IRContent.pst_src_img.n_width = nWidthZoom;
	IRContent.pst_src_img.n_height = nHeightZoom;
	IRContent.pst_src_img.n_len = nHeightZoom * nWidthZoom;
	IRContent.pst_dst_img.n_width = nWidthZoom;
	IRContent.pst_dst_img.n_height = nHeightZoom;
	IRContent.pst_dst_img.n_len = nHeightZoom * nWidthZoom;

	if (ZoomType == Near)  //最邻近插值
	{
		for (i = 0; i < nHeightZoom; i++)
		{
			y0 = int(i * fZoomInverse);
			for (j = 0; j < nWidthZoom; j++)
			{
				x0 = int(j * fZoomInverse);
				*(puc_dst + i * nWidthZoom + j) = *(puc_src + y0 * n_width + x0);
			}
		}
	}
	else if (ZoomType == Bilinear)  //双线性插值
	{
		int x1, y1;
		float oh, ow;
		
		for (i = 0; i < nHeightZoom; i++)
		{
			y0 = int(i * fZoomInverse);
			y1 = y0 + 1;
			if (y1 >= n_height)
				y1 = n_height - 1;

			for (j = 0; j < nWidthZoom; j++)
			{
				x0 = int(j * fZoomInverse);
				x1 = x0 + 1;
				if (x1 >= n_width)
					x1 = n_width - 1;

				oh = i * fZoomInverse - y0;
				ow = j * fZoomInverse - x0;
				puc_dst[i * nWidthZoom + j] = (unsigned char)(ClipU8(((1 - ow) * (1 - oh) *puc_src[y0 * n_width + x0] + ow * (1 - oh) *puc_src[y0 * n_width + x1] +
					(1 - ow) * oh *puc_src[y1 * n_width + x0] + ow * oh *puc_src[y1 * n_width + x1])));
			}
		}
	}
	else if (ZoomType == BilinearFast)
	{
		ImgBilinearFast(puc_dst, puc_src, n_height, n_width, fZoom, fZoom, ResizeTable);
	}
}

//图像放大
void CInfraredCore::ImgResize()
{
	int n_width = IRContent.pst_src_img.n_width;
	int n_height = IRContent.pst_src_img.n_height;
	float fZoom = IrPara.st_zoom_para.fZoom;
	int ZoomType = IrPara.st_zoom_para.n_ZoomType;
	unsigned char* puc_dst = IRContent.pst_dst_img.puc_data;
	unsigned char* puc_src = IRContent.pst_src_img.puc_data;

	Resize(puc_dst, puc_src, n_width, n_height, fZoom, ZoomType);
}

void CInfraredCore::PseudoColorMap()
{
	if (IrPara.st_psdclr_coef.b_psdclr_equallinemode == 0)
	{
		unsigned char *puc_src = IRContent.pst_src_img.puc_data;
		int height = IRContent.pst_src_img.n_height;
		int width = IRContent.pst_src_img.n_width;
		unsigned char *puc_dst = IRContent.pst_dst_img.puc_data;
		int pseudo_color_type = IrPara.st_psdclr_coef.t_psdclr_type;
		int colorimg_tpye = IrPara.st_psdclr_coef.n_colorimg_tpye;
		PseudoColor(puc_dst, puc_src, height, width, pseudo_color_type, colorimg_tpye);
	}
	else if (IrPara.st_psdclr_coef.b_psdclr_equallinemode == 1)
	{
		unsigned char *puc_src = IRContent.pst_src_img.puc_data;
		int height = IRContent.pst_src_img.n_height;
		int width = IRContent.pst_src_img.n_width;
		unsigned char *puc_dst = IRContent.pst_dst_img.puc_data;
		int pseudo_color_type = IrPara.st_psdclr_coef.t_psdclr_type;
		int colorimg_tpye = IrPara.st_psdclr_coef.n_colorimg_tpye;
		short* y16Arr = IRContent.pst_src_img.pus_data;
		EqualLineMode* s_equal_line_para = &IrPara.st_psdclr_coef.s_equalline_para;
		EqualLine(puc_dst, puc_src, y16Arr, height, width, pseudo_color_type, colorimg_tpye, s_equal_line_para);
	}
}

void CInfraredCore::DeGuoGai()
{
	int i;
	int nWidth = IRContent.pst_src_img.n_width;
	int nHeight = IRContent.pst_src_img.n_height;
	int nLen = nWidth * nHeight;
	short* nSrc = IRContent.pst_src_img.pus_data;
	short* nDst = IRContent.pst_dst_img.pus_data;
	short* nGuoGaiModel = g_guogai_model;

	if (IrPara.st_dgg_para.n_dgg_arith_type == DGG_TYPE_FIX)
	{
		for (i = 0; i < nLen; i++)
		{
			nDst[i] = nSrc[i] - nGuoGaiModel[i];
		}
	}
	else if (IrPara.st_dgg_para.n_dgg_arith_type == DGG_TYPE_VAR1)
	{
		if (IrPara.st_dgg_para.stDggTPara.fDelta > 0)
		{
			float fCoef = (IrPara.st_dgg_para.stDggTPara.fRealShutterT - IrPara.st_dgg_para.stDggTPara.fStartShutterT) / IrPara.st_dgg_para.stDggTPara.fDelta;
			for (i = 0; i < nLen; i++)
			{
				nDst[i] = nSrc[i] - (short)(fCoef * nGuoGaiModel[i]);
			}
		}
		else
		{
			memcpy(nDst, nSrc, nLen * sizeof(short));
		}
	}
	else if (IrPara.st_dgg_para.n_dgg_arith_type == DGG_TYPE_VAR2)
	{
		float fDeltaT = IrPara.st_dgg_para.stDggTPara.fLastShutterT - IrPara.st_dgg_para.stDggTPara.fLastLastShutterT;
		if (fDeltaT != 0)
		{
			float fCoef = (IrPara.st_dgg_para.stDggTPara.fRealShutterT - IrPara.st_dgg_para.stDggTPara.fLastShutterT) / fDeltaT;
			for (i = 0; i < nLen; i++)
			{
				nDst[i] = nSrc[i] - (short)(fCoef * nGuoGaiModel[i]);
			}
		}
		else
		{
			memcpy(nDst, nSrc, nLen * sizeof(short));
		}

	}

}

void CInfraredCore::CalGuoGaiModel()
{
	int i;
	int nWidth = IRContent.pst_src_img.n_width;
	int nHeight = IRContent.pst_src_img.n_height;
	int nLen = nWidth * nHeight;
	int nWidthHalf = nWidth >> 1;
	int nHeightHalf = nHeight >> 1;
	short* nSrc = IRContent.pst_src_img.pus_data;
	short* nDst = g_guogai_model;
	//5*5的均值滤波
	int x, y, tempx, tempy, nTemp, nSum, nSumnum;

	for (x = 2; x < nHeight - 2; x++)
	{
		for (y = 2; y < nWidth - 2; y++)
		{
			nTemp = (x - 2) * nWidth + (y - 2);
			nSum = nSrc[nTemp] + nSrc[nTemp + 1] + nSrc[nTemp + 2] + nSrc[nTemp + 3] + nSrc[nTemp + 4]
				+ nSrc[nTemp + nWidth] + nSrc[nTemp + nWidth + 1] + nSrc[nTemp + nWidth + 2] + nSrc[nTemp + nWidth + 3] + nSrc[nTemp + nWidth + 4]
				+ nSrc[nTemp + nWidth * 2] + nSrc[nTemp + nWidth * 2 + 1] + nSrc[nTemp + nWidth * 2 + 2] + nSrc[nTemp + nWidth * 2 + 3] + nSrc[nTemp + nWidth * 2 + 4]
				+ nSrc[nTemp + nWidth * 3] + nSrc[nTemp + nWidth * 3 + 1] + nSrc[nTemp + nWidth * 3 + 2] + nSrc[nTemp + nWidth * 3 + 3] + nSrc[nTemp + nWidth * 3 + 4]
				+ nSrc[nTemp + nWidth * 4] + nSrc[nTemp + nWidth * 4 + 1] + nSrc[nTemp + nWidth * 4 + 2] + nSrc[nTemp + nWidth * 4 + 3] + nSrc[nTemp + nWidth * 4 + 4];

			nDst[x * nWidth + y] = short(nSum / 25);
		}
	}


	for (x = 0; x < nHeight; x++)
	{
		for (y = 0; y < 2; y++)
		{
			nSum = 0;
			nSumnum = 0;
			for (tempx = -2; tempx <= 2; tempx++)
			{
				for (tempy = -2; tempy <= 2; tempy++)
				{
					if ((x + tempx) >= 0 && (x + tempx) < nHeight && (y + tempy) >= 0 && (y + tempy) < nWidth)
					{
						nSum = nSum + nSrc[(x + tempx) * nWidth + (y + tempy)];
						nSumnum++;
					}
				}
			}
			nDst[x * nWidth + y] = short(nSum / nSumnum);
		}
	}
	for (x = 0; x < nHeight; x++)
	{
		for (y = nWidth - 2; y < nWidth; y++)
		{
			nSum = 0;
			nSumnum = 0;
			for (tempx = -2; tempx <= 2; tempx++)
			{
				for (tempy = -2; tempy <= 2; tempy++)
				{
					if ((x + tempx) >= 0 && (x + tempx) < nHeight && (y + tempy) >= 0 && (y + tempy) < nWidth)
					{
						nSum = nSum + nSrc[(x + tempx) * nWidth + (y + tempy)];
						nSumnum++;
					}
				}
			}
			nDst[x * nWidth + y] = short(nSum / nSumnum);
		}
	}
	for (x = 0; x < 2; x++)
	{
		for (y = 2; y < nWidth - 2; y++)
		{
			nSum = 0;
			nSumnum = 0;
			for (tempx = -2; tempx <= 2; tempx++)
			{
				for (tempy = -2; tempy <= 2; tempy++)
				{
					if ((x + tempx) >= 0 && (x + tempx) < nHeight && (y + tempy) >= 0 && (y + tempy) < nWidth)
					{
						nSum = nSum + nSrc[(x + tempx) * nWidth + (y + tempy)];
						nSumnum++;
					}
				}
			}
			nDst[x * nWidth + y] = short(nSum / nSumnum);
		}
	}
	for (x = nHeight - 2; x < nHeight; x++)
	{
		for (y = 2; y < nWidth - 2; y++)
		{
			nSum = 0;
			nSumnum = 0;
			for (tempx = -2; tempx <= 2; tempx++)
			{
				for (tempy = -2; tempy <= 2; tempy++)
				{
					if ((x + tempx) >= 0 && (x + tempx) < nHeight && (y + tempy) >= 0 && (y + tempy) < nWidth)
					{
						nSum = nSum + nSrc[(x + tempx) * nWidth + (y + tempy)];
						nSumnum++;
					}
				}
			}
			nDst[x * nWidth + y] = short(nSum / nSumnum);
		}
	}

	short nCenterY16 = (nDst[nHeightHalf * nWidth + nWidthHalf] + nDst[nHeightHalf * nWidth + nWidthHalf + 1]
		+ nDst[(nHeightHalf + 1) * nWidth + nWidthHalf] + nDst[(nHeightHalf + 1) * nWidth + nWidthHalf + 1]) >> 2;
	for (i = 0; i < nLen; i++)
	{
		nDst[i] = short(nDst[i] - nCenterY16);
	}
}

void CInfraredCore::PseudoColor(unsigned char* puc_dst, unsigned char* puc_src, int height, int width, int pseudo_color_type, int colorimg_tpye)
{
	int i, j, temp;
	unsigned char* head = nullptr;
	if (isCustomPalette) {
		head = customPalette->getCustomPaleHead();
	}
	int nLen = height * width;
	if (colorimg_tpye == COLORIMG_TYPE_BGR)
	{
		if (!isCustomPalette) {
			for (i = 0; i < nLen; i++)
			{
				temp = puc_src[i];
				puc_dst[i * 3 + 2] = PseudoColorTable[pseudo_color_type][temp][0];
				puc_dst[i * 3 + 1] = PseudoColorTable[pseudo_color_type][temp][1];
				puc_dst[i * 3 + 0] = PseudoColorTable[pseudo_color_type][temp][2];
			}
			//puc_dst按照bgr的顺序
		}
		else {
			for (i = 0; i < nLen; i++)
			{
				temp = puc_src[i];
				int paletteIndex = pseudo_color_type - PSEUDONUM;
				puc_dst[i * 3 + 2] = head[(paletteIndex * 256 + temp) * 4 + 0];
				puc_dst[i * 3 + 1] = head[(paletteIndex * 256 + temp) * 4 + 1];
				puc_dst[i * 3 + 0] = head[(paletteIndex * 256 + temp) * 4 + 2];
			}
		}
		IRContent.pst_dst_img.n_channels = 3;
	}
	else if (colorimg_tpye == COLORIMG_TYPE_RGBA)
	{
		if (!isCustomPalette) {
			for (i = 0; i < nLen; i++)
			{
				temp = puc_src[i];
				puc_dst[i * 4 + 0] = PseudoColorTable[pseudo_color_type][temp][0];
				puc_dst[i * 4 + 1] = PseudoColorTable[pseudo_color_type][temp][1];
				puc_dst[i * 4 + 2] = PseudoColorTable[pseudo_color_type][temp][2];
				puc_dst[i * 4 + 3] = 255;
			}
		}
		else {
			for (i = 0; i < nLen; i++)
			{
				temp = puc_src[i];
				int paletteIndex = pseudo_color_type - PSEUDONUM;
				puc_dst[i * 4 + 0] = head[(paletteIndex * 256 + temp) * 4 + 0];
				puc_dst[i * 4 + 1] = head[(paletteIndex * 256 + temp) * 4 + 1];
				puc_dst[i * 4 + 2] = head[(paletteIndex * 256 + temp) * 4 + 2];
				puc_dst[i * 4 + 3] = 255;
			}
		}
		IRContent.pst_dst_img.n_channels = 4;
	}
	else if (colorimg_tpye == COLORIMG_TYPE_RGB)
	{
		if (!isCustomPalette) {
			for (i = 0; i < nLen; i++)
			{
				temp = puc_src[i];
				puc_dst[i * 3 + 0] = PseudoColorTable[pseudo_color_type][temp][0];
				puc_dst[i * 3 + 1] = PseudoColorTable[pseudo_color_type][temp][1];
				puc_dst[i * 3 + 2] = PseudoColorTable[pseudo_color_type][temp][2];
			}
		}
		else {
			for (i = 0; i < nLen; i++)
			{
				temp = puc_src[i];
				int paletteIndex = pseudo_color_type - PSEUDONUM;
				puc_dst[i * 3 + 0] = head[(paletteIndex * 256 + temp) * 4 + 0];
				puc_dst[i * 3 + 1] = head[(paletteIndex * 256 + temp) * 4 + 1];
				puc_dst[i * 3 + 2] = head[(paletteIndex * 256 + temp) * 4 + 2];
			}
		}
		IRContent.pst_dst_img.n_channels = 3;
	}
	else if (colorimg_tpye == COLORIMG_TYPE_RGB565)
	{
		unsigned char r, g, b;
		short *dst = (short *)puc_dst;
		if (!isCustomPalette) {
			for (i = 0; i < nLen; i++) {
				temp = puc_src[i];
				r = PseudoColorTable[pseudo_color_type][temp][0];
				g = PseudoColorTable[pseudo_color_type][temp][1];
				b = PseudoColorTable[pseudo_color_type][temp][2];
				//*(dst + i) = ((r & 0x1F) << 11) | ((g & 0x3F) << 5) | (b & 0x1F);
				*(dst + i) = ((r & 0b11111000) << 8) | ((g & 0b11111100) << 3) | (b >> 3);
			}
		}
		else {
			for (i = 0; i < nLen; i++) {
				temp = puc_src[i];
				int paletteIndex = pseudo_color_type - PSEUDONUM;
				r = head[(paletteIndex * 256 + temp) * 4 + 0];
				g = head[(paletteIndex * 256 + temp) * 4 + 1];
				b = head[(paletteIndex * 256 + temp) * 4 + 2];
				//*(dst + i) = ((r & 0x1F) << 11) | ((g & 0x3F) << 5) | (b & 0x1F);
				*(dst + i) = ((r & 0b11111000) << 8) | ((g & 0b11111100) << 3) | (b >> 3);
			}
		}
		IRContent.pst_dst_img.n_channels = 3;
	}
	else if (COLORIMG_TYPE_YUV422YUYV == colorimg_tpye)
	{ // YUV422_YUYV 
		if (!isCustomPalette) {
			for (i = 0; i < nLen; i++) {
				temp = puc_src[i];
				puc_dst[i * 2 + 0] = PseudoColorTableYUV[pseudo_color_type][temp][0];
				if (i % 2 == 0)
				{
					puc_dst[i * 2 + 1] = PseudoColorTableYUV[pseudo_color_type][temp][1];
				}
				else
				{
					puc_dst[i * 2 + 1] = PseudoColorTableYUV[pseudo_color_type][temp][2];
				}
			}
		}
		else {
			for (i = 0; i < nLen; i++) {
				temp = puc_src[i];
				int paletteIndex = pseudo_color_type - PSEUDONUM;
				puc_dst[i * 2 + 0] = head[(paletteIndex * 256 + temp) * 4 + 0];
				if (i % 2 == 0)
				{
					puc_dst[i * 2 + 1] = head[(paletteIndex * 256 + temp) * 4 + 1];
				}
				else
				{
					puc_dst[i * 2 + 1] = head[(paletteIndex * 256 + temp) * 4 + 2];
				}
			}
		}
		IRContent.pst_dst_img.n_channels = 3;
	}
	else if (COLORIMG_TYPE_YUV422UYVY == colorimg_tpye)
	{	// YUV422_UYVY 
		if (!isCustomPalette) {
			for (i = 0; i < nLen; i++) {
				temp = puc_src[i];
				puc_dst[i * 2 + 0] = PseudoColorTableYUV[pseudo_color_type][temp][0];
				if (i % 2 == 0)
				{
					puc_dst[i * 2 + 1] = PseudoColorTableYUV[pseudo_color_type][temp][2];
				}
				else
				{
					puc_dst[i * 2 + 1] = PseudoColorTableYUV[pseudo_color_type][temp][1];
				}
			}
		}
		else {
			for (i = 0; i < nLen; i++) {
				temp = puc_src[i];
				int paletteIndex = pseudo_color_type - PSEUDONUM;
				puc_dst[i * 2 + 0] = head[(paletteIndex * 256 + temp) * 4 + 0];
				if (i % 2 == 0)
				{
					puc_dst[i * 2 + 1] = head[(paletteIndex * 256 + temp) * 4 + 2];
				}
				else
				{
					puc_dst[i * 2 + 1] = head[(paletteIndex * 256 + temp) * 4 + 1];
				}
			}
		}
		IRContent.pst_dst_img.n_channels = 3;
	}
	else if (COLORIMG_TYPE_YUV422Plane == colorimg_tpye)
	{	// YUV422_Plane, 先存Y通道所有的值，再存U通道，再存V通道 
		unsigned char* Y = puc_dst;
		unsigned char* U = puc_dst + nLen;
		unsigned char* V = puc_dst + nLen * 3 / 2;
		if (!isCustomPalette) {
			for (i = 0; i < nLen; i++) {
				temp = puc_src[i];
				Y[i] = PseudoColorTableYUV[pseudo_color_type][temp][0];
				if (i % 2 == 0)
				{
					U[i / 2] = PseudoColorTableYUV[pseudo_color_type][temp][1];
					V[i / 2] = PseudoColorTableYUV[pseudo_color_type][temp][2];
				}
			}
		}
		else {
			for (i = 0; i < nLen; i++) {
				temp = puc_src[i];
				int paletteIndex = pseudo_color_type - PSEUDONUM;
				Y[i] = head[(paletteIndex * 256 + temp) * 4 + 0];
				if (i % 2 == 0)
				{
					U[i / 2] = head[(paletteIndex * 256 + temp) * 4 + 1];
					V[i / 2] = head[(paletteIndex * 256 + temp) * 4 + 2];
				}
			}
		}
		IRContent.pst_dst_img.n_channels = 3;
	}
	else if (COLORIMG_TYPE_YVU422Plane == colorimg_tpye)
	{	// YVU422_Plane, 先存Y通道所有的值，再存V通道，再存U通道 
		unsigned char* Y = puc_dst;
		unsigned char* V = puc_dst + nLen;
		unsigned char* U = puc_dst + nLen * 3 / 2;
		if (!isCustomPalette) {
			for (i = 0; i < nLen; i++) {
				temp = puc_src[i];
				Y[i] = PseudoColorTableYUV[pseudo_color_type][temp][0];
				if (i % 2 == 0)
				{
					U[i / 2] = PseudoColorTableYUV[pseudo_color_type][temp][1];
					V[i / 2] = PseudoColorTableYUV[pseudo_color_type][temp][2];
				}
			}
		}
		else {
			for (i = 0; i < nLen; i++) {
				temp = puc_src[i];
				int paletteIndex = pseudo_color_type - PSEUDONUM;
				Y[i] = head[(paletteIndex * 256 + temp) * 4 + 0];
				if (i % 2 == 0)
				{
					U[i / 2] = head[(paletteIndex * 256 + temp) * 4 + 1];
					V[i / 2] = head[(paletteIndex * 256 + temp) * 4 + 2];
				}
			}
		}
		IRContent.pst_dst_img.n_channels = 3;
	}
	else if (colorimg_tpye == COLORIMG_TYPE_YUV420NV12)
	{
		unsigned char* y = puc_dst;
		unsigned char* uv = puc_dst + height * width;
		int num;
		int widthhalf = width / 2;
		if (!isCustomPalette) {
			for (i = 0; i < height; i++)
			{
				for (j = 0; j < width; j++)
				{
					temp = puc_src[i * width + j];
					y[i * width + j] = PseudoColorTableYUV[pseudo_color_type][temp][0];

					if ((i % 2 == 0) && (j % 2 == 0))
					{
						num = i / 2 * widthhalf + j / 2;
						uv[num * 2 + 0] = PseudoColorTableYUV[pseudo_color_type][temp][1];
						uv[num * 2 + 1] = PseudoColorTableYUV[pseudo_color_type][temp][2];
					}
				}
			}
		}
		else {
			for (i = 0; i < height; i++)
			{
				for (j = 0; j < width; j++)
				{
					temp = puc_src[i * width + j];
					int paletteIndex = pseudo_color_type - PSEUDONUM;
					y[i * width + j] = head[(paletteIndex * 256 + temp) * 4 + 0];

					if ((i % 2 == 0) && (j % 2 == 0))
					{
						num = i / 2 * widthhalf + j / 2;
						uv[num * 2 + 0] = head[(paletteIndex * 256 + temp) * 4 + 1];
						uv[num * 2 + 1] = head[(paletteIndex * 256 + temp) * 4 + 2];
					}
				}
			}
		}
		IRContent.pst_dst_img.n_channels = 3;
	}
	else if (colorimg_tpye == COLORIMG_TYPE_YUV420NV21)
	{
		unsigned char* y = puc_dst;
		unsigned char* uv = puc_dst + height * width;
		int num;
		int widthhalf = width / 2;
		if (!isCustomPalette) {
			for (i = 0; i < height; i++)
			{
				for (j = 0; j < width; j++)
				{
					temp = puc_src[i * width + j];
					y[i * width + j] = PseudoColorTableYUV[pseudo_color_type][temp][0];

					if ((i % 2 == 0) && (j % 2 == 0))
					{
						num = i / 2 * widthhalf + j / 2;
						uv[num * 2 + 1] = PseudoColorTableYUV[pseudo_color_type][temp][1];
						uv[num * 2 + 0] = PseudoColorTableYUV[pseudo_color_type][temp][2];
					}
				}
			}
		}
		else {
			for (i = 0; i < height; i++)
			{
				for (j = 0; j < width; j++)
				{
					temp = puc_src[i * width + j];
					int paletteIndex = pseudo_color_type - PSEUDONUM;
					y[i * width + j] = head[(paletteIndex * 256 + temp) * 4 + 0];

					if ((i % 2 == 0) && (j % 2 == 0))
					{
						num = i / 2 * widthhalf + j / 2;
						uv[num * 2 + 1] = head[(paletteIndex * 256 + temp) * 4 + 1];
						uv[num * 2 + 0] = head[(paletteIndex * 256 + temp) * 4 + 2];
					}
				}
			}
		}
		IRContent.pst_dst_img.n_channels = 3;
	}
	else if (colorimg_tpye == COLORIMG_TYPE_YUV420PLANE)
	{
		unsigned char* y = puc_dst;
		unsigned char* u = puc_dst + height * width;
		unsigned char* v = puc_dst + height * width * 5 / 4;
		int num;
		int widthhalf = width / 2;
		if (!isCustomPalette) {
			for (i = 0; i < height; i++)
			{
				for (j = 0; j < width; j++)
				{
					temp = puc_src[i * width + j];
					y[i * width + j] = PseudoColorTableYUV[pseudo_color_type][temp][0];

					if ((i % 2 == 0) && (j % 2 == 0))
					{
						num = i / 2 * widthhalf + j / 2;
						u[num] = PseudoColorTableYUV[pseudo_color_type][temp][1];
						v[num] = PseudoColorTableYUV[pseudo_color_type][temp][2];
					}
				}
			}
		}
		else {
			for (i = 0; i < height; i++)
			{
				for (j = 0; j < width; j++)
				{
					temp = puc_src[i * width + j];
					int paletteIndex = pseudo_color_type - PSEUDONUM;
					y[i * width + j] = head[(paletteIndex * 256 + temp) * 4 + 0];

					if ((i % 2 == 0) && (j % 2 == 0))
					{
						num = i / 2 * widthhalf + j / 2;
						u[num] = head[(paletteIndex * 256 + temp) * 4 + 1];
						v[num] = head[(paletteIndex * 256 + temp) * 4 + 2];
					}
				}
			}
		}
		IRContent.pst_dst_img.n_channels = 3;
	}
	else if (colorimg_tpye == COLORIMG_TYPE_YVU420PLANE)
	{
		unsigned char* y = puc_dst;
		unsigned char* u = puc_dst + height * width * 5 / 4;
		unsigned char* v = puc_dst + height * width;
		int num;
		int widthhalf = width / 2;
		if (!isCustomPalette) {
			for (i = 0; i < height; i++)
			{
				for (j = 0; j < width; j++)
				{
					temp = puc_src[i * width + j];
					y[i * width + j] = PseudoColorTableYUV[pseudo_color_type][temp][0];

					if ((i % 2 == 0) && (j % 2 == 0))
					{
						num = i / 2 * widthhalf + j / 2;
						u[num] = PseudoColorTableYUV[pseudo_color_type][temp][1];
						v[num] = PseudoColorTableYUV[pseudo_color_type][temp][2];
					}
				}
			}
		}
		else {
			for (i = 0; i < height; i++)
			{
				for (j = 0; j < width; j++)
				{
					temp = puc_src[i * width + j];
					int paletteIndex = pseudo_color_type - PSEUDONUM;
					y[i * width + j] = head[(paletteIndex * 256 + temp) * 4 + 0];

					if ((i % 2 == 0) && (j % 2 == 0))
					{
						num = i / 2 * widthhalf + j / 2;
						u[num] = head[(paletteIndex * 256 + temp) * 4 + 1];
						v[num] = head[(paletteIndex * 256 + temp) * 4 + 2];
					}
				}
			}
		}
		IRContent.pst_dst_img.n_channels = 3;
	}
}

void CInfraredCore::EqualLine(unsigned char *puc_dst, unsigned char *y8Arr, short* y16Arr, int height, int width, int pseudo_color_type, int colorimg_tpye, EqualLineMode* s_equal_line_para)
{
	unsigned char* head = nullptr;
	if (isCustomPalette) {
		head = customPalette->getCustomPaleHead();
	}
	unsigned char color_r, color_g, color_b;
	unsigned char otherColor_r, otherColor_g, otherColor_b;
	float rgbratio = 1.0f;
	int x = 0, y = 0;
	int i = 0, j = 0;
	int r = 0, g = 0, b = 0;
	int index;
	int num;
	int widthhalf = width / 2;
	int nLen = width * height;
	int color = s_equal_line_para->color;
	int otherColor = s_equal_line_para->otherColor;
	int type = s_equal_line_para->type;
	if (type > 3)
		type = 3;

	if (color == 0 || color == -1)
	{
		if (!isCustomPalette)
		{
			color_r = PseudoColorTable[pseudo_color_type][255][0];
			color_g = PseudoColorTable[pseudo_color_type][255][1];
			color_b = PseudoColorTable[pseudo_color_type][255][2];
		}
		else {
			int paletteIndex = pseudo_color_type - PSEUDONUM;
			color_r = head[(paletteIndex * 256 + 255) * 4 + 0];
			color_g = head[(paletteIndex * 256 + 255) * 4 + 1];
			color_b = head[(paletteIndex * 256 + 255) * 4 + 2];
		}
	}
	else
	{
		color_r = (unsigned char)((color >> 16) & 0xFF);
		color_g = (unsigned char)((color >> 8) & 0xFF);
		color_b = (unsigned char)(color & 0xFF);
	}

	float fZoomInverse = 1 / IrPara.st_zoom_para.fZoom;
	int nY16Width = int(width * fZoomInverse);

	//memset(y16ArrZoom, 0, nLen * sizeof(short));
	for (i = 0; i < height; i++)
	{
		y = int(i * fZoomInverse);
		for (j = 0; j < width; j++)
		{
			x = int(j * fZoomInverse);
			*(y16ArrZoom + i * width + j) = *(y16Arr + y * nY16Width + x);
		}
	}

	short maxY16 = -32768;
	short minY16 = 32767;
	for (i = 0; i < nLen; i++)
	{
		if (y16ArrZoom[i] < minY16)
			minY16 = y16ArrZoom[i];
		if (y16ArrZoom[i] > maxY16)
			maxY16 = y16ArrZoom[i];
	}

	if (otherColor == 0 || otherColor == -1)
	{
		if (!isCustomPalette)
		{
			otherColor_r = PseudoColorTable[pseudo_color_type][255][0];
			otherColor_g = PseudoColorTable[pseudo_color_type][255][1];
			otherColor_b = PseudoColorTable[pseudo_color_type][255][2];
		}
		else
		{
			int paletteIndex = pseudo_color_type - PSEUDONUM;
			otherColor_r = head[(paletteIndex * 256 + 255) * 4 + 0];
			otherColor_g = head[(paletteIndex * 256 + 255) * 4 + 1];
			otherColor_b = head[(paletteIndex * 256 + 255) * 4 + 2];
		}
	}
	else
	{
		otherColor_r = (unsigned char)((otherColor >> 16) & 0xFF);
		otherColor_g = (unsigned char)((otherColor >> 8) & 0xFF);
		otherColor_b = (unsigned char)(otherColor & 0xFF);
	}

	// From top to bottom
	for (y = 0; y < height; ++y)
	{
		// From left to right
		for (x = 0; x < width; ++x)
		{
			// Get each pixel by format
			index = y * width + x;
			if (type == EQUAL_LINE_HighLow)
			{
				//此时应该赋颜色为等温线的颜色
				if (y16ArrZoom[index] > s_equal_line_para->highY16)
				{
					//颜色最大值对于的255,最小对于的64，所以算出系数之后 * （255 - 64）/255 + 64/255 = * 0.75 + 0.25
					rgbratio = ((y16ArrZoom[index] - s_equal_line_para->highY16) * 1.0f /
						(maxY16 - s_equal_line_para->highY16)) * 0.75f + 0.25f;
					r = color_r;
					g = color_g;
					b = color_b;
				}
				else if (y16ArrZoom[index] < s_equal_line_para->lowY16)
				{
					rgbratio = (y16ArrZoom[index] - s_equal_line_para->lowY16) * 1.0f /
						(minY16 - s_equal_line_para->lowY16) * 0.75f + 0.25f;  //分母有问题？？
					r = otherColor_r;
					g = otherColor_g;
					b = otherColor_b;
				}
				else
				{
					rgbratio = 1.0f;
					if (!isCustomPalette)
					{
						r = PseudoColorTable[pseudo_color_type][y8Arr[index]][0];
						g = PseudoColorTable[pseudo_color_type][y8Arr[index]][1];
						b = PseudoColorTable[pseudo_color_type][y8Arr[index]][2];
					}
					else {
						int paletteIndex = pseudo_color_type - PSEUDONUM;
						int temp = y8Arr[index];
						r = head[(paletteIndex * 256 + temp) * 4 + 0];
						g = head[(paletteIndex * 256 + temp) * 4 + 1];
						b = head[(paletteIndex * 256 + temp) * 4 + 2];
					}	
				}
			}
			else if (type == EQUAL_LINE_High)
			{
				if (y16ArrZoom[index] > s_equal_line_para->highY16)
				{
					rgbratio = ((y16ArrZoom[index] - s_equal_line_para->highY16) * 1.0f /
						(maxY16 - s_equal_line_para->highY16)) * 0.75f + 0.25f;
					r = color_r;
					g = color_g;
					b = color_b;
				}
				else {
					rgbratio = 1.0f;
					if (!isCustomPalette)
					{
						r = PseudoColorTable[pseudo_color_type][y8Arr[index]][0];
						g = PseudoColorTable[pseudo_color_type][y8Arr[index]][1];
						b = PseudoColorTable[pseudo_color_type][y8Arr[index]][2];
					}
					else {
						int paletteIndex = pseudo_color_type - PSEUDONUM;
						int temp = y8Arr[index];
						r = head[(paletteIndex * 256 + temp) * 4 + 0];
						g = head[(paletteIndex * 256 + temp) * 4 + 1];
						b = head[(paletteIndex * 256 + temp) * 4 + 2];
					}
					
				}

			}
			else if (type == EQUAL_LINE_Low)
			{
				if (y16ArrZoom[index] < s_equal_line_para->lowY16)
				{
					rgbratio = (y16ArrZoom[index] - s_equal_line_para->lowY16) * 1.0f /
						(minY16 - s_equal_line_para->lowY16) * 0.75f + 0.25f;
					r = color_r;
					g = color_g;
					b = color_b;
				}
				else
				{
					rgbratio = 1.0f;
					if (!isCustomPalette)
					{
						r = PseudoColorTable[pseudo_color_type][y8Arr[index]][0];
						g = PseudoColorTable[pseudo_color_type][y8Arr[index]][1];
						b = PseudoColorTable[pseudo_color_type][y8Arr[index]][2];
					}
					else {
						int paletteIndex = pseudo_color_type - PSEUDONUM;
						int temp = y8Arr[index];
						r = head[(paletteIndex * 256 + temp) * 4 + 0];
						g = head[(paletteIndex * 256 + temp) * 4 + 1];
						b = head[(paletteIndex * 256 + temp) * 4 + 2];
					}
				}

			}
			else if (type == EQUAL_LINE_Middle)
			{
				//此时应该赋颜色为等温线的颜色
				if (y16ArrZoom[index] < s_equal_line_para->highY16 &&
					y16ArrZoom[index] > s_equal_line_para->lowY16)
				{
					//颜色最大值对于的255,最小对于的64，所以算出系数之后 * （255 - 64）/255 + 64/255 = * 0.75 + 0.25
					rgbratio = ((y16ArrZoom[index] - s_equal_line_para->lowY16) * 1.0f /
						(s_equal_line_para->highY16 - s_equal_line_para->lowY16)) * 0.75f + 0.25f;
					r = color_r;
					g = color_g;
					b = color_b;
				}
				else
				{
					rgbratio = 1.0f;
					if (!isCustomPalette)
					{
						r = PseudoColorTable[pseudo_color_type][y8Arr[index]][0];
						g = PseudoColorTable[pseudo_color_type][y8Arr[index]][1];
						b = PseudoColorTable[pseudo_color_type][y8Arr[index]][2];
					}
					else {
						int paletteIndex = pseudo_color_type - PSEUDONUM;
						int temp = y8Arr[index];
						r = head[(paletteIndex * 256 + temp) * 4 + 0];
						g = head[(paletteIndex * 256 + temp) * 4 + 1];
						b = head[(paletteIndex * 256 + temp) * 4 + 2];
					}
				}
			}

			r = (unsigned char)(ClipU8(r * rgbratio));
			g = (unsigned char)(ClipU8(g * rgbratio));
			b = (unsigned char)(ClipU8(b * rgbratio));

			if (colorimg_tpye == COLORIMG_TYPE_BGR)
			{
				puc_dst[index * 3 + 2] = (unsigned char)r;
				puc_dst[index * 3 + 1] = (unsigned char)g;
				puc_dst[index * 3 + 0] = (unsigned char)b;
				//puc_dst按照bgr的顺序
			}
			else if (colorimg_tpye == COLORIMG_TYPE_RGBA)
			{
				puc_dst[index * 4 + 0] = (unsigned char)r;
				puc_dst[index * 4 + 1] = (unsigned char)g;
				puc_dst[index * 4 + 2] = (unsigned char)b;
				puc_dst[index * 4 + 3] = (unsigned char)255;
				//puc_dst按照rgba的顺序
			}
			else if (colorimg_tpye == COLORIMG_TYPE_RGB)
			{
				puc_dst[index * 3 + 0] = (unsigned char)r;
				puc_dst[index * 3 + 1] = (unsigned char)g;
				puc_dst[index * 3 + 2] = (unsigned char)b;
				//puc_dst按照rgb的顺序
			}
			else if (colorimg_tpye == COLORIMG_TYPE_RGB565)
			{
				short *dst = (short *)puc_dst;
				*(dst + index) = ((r & 0x1F) << 11) | ((g & 0x3F) << 5) | (b & 0x1F);
			}
			else if (colorimg_tpye == COLORIMG_TYPE_YUV422YUYV)
			{
				puc_dst[index * 2 + 0] = (unsigned char)(ClipU8(RGB2Y(r, g, b)));
				if (index % 2 == 0)
				{
					puc_dst[index * 2 + 1] = (unsigned char)(ClipU8(RGB2U(r, g, b)));
				}
				else
				{
					puc_dst[index * 2 + 1] = (unsigned char)(ClipU8(RGB2V(r, g, b)));
				}
			}
			else if (colorimg_tpye == COLORIMG_TYPE_YUV422UYVY)
			{
				puc_dst[index * 2 + 0] = (unsigned char)(ClipU8(RGB2Y(r, g, b)));
				if (index % 2 == 0)
				{
					puc_dst[index * 2 + 1] = (unsigned char)(ClipU8(RGB2V(r, g, b)));
				}
				else
				{
					puc_dst[index * 2 + 1] = (unsigned char)(ClipU8(RGB2U(r, g, b)));
				}
			}
			else if (colorimg_tpye == COLORIMG_TYPE_YUV422Plane)
			{

				puc_dst[index] = (unsigned char)(ClipU8(RGB2Y(r, g, b)));
				if (i % 2 == 0)
				{
					puc_dst[index / 2 + width * height] = (unsigned char)(ClipU8(RGB2U(r, g, b)));
					puc_dst[index / 2 + width * height + width * height / 2] = (unsigned char)(ClipU8(RGB2V(r, g, b)));
				}
			}
			else if (colorimg_tpye == COLORIMG_TYPE_YVU422Plane)
			{

				puc_dst[index] = (unsigned char)(ClipU8(RGB2Y(r, g, b)));
				if (i % 2 == 0)
				{
					puc_dst[index / 2 + width * height + width * height / 2] = (unsigned char)(ClipU8(RGB2U(r, g, b)));
					puc_dst[index / 2 + width * height] = (unsigned char)(ClipU8(RGB2V(r, g, b)));
				}
			}
			else if (colorimg_tpye == COLORIMG_TYPE_YUV420NV12)
			{
				puc_dst[index] = (unsigned char)(ClipU8(RGB2Y(r, g, b)));

				if ((y % 2 == 0) && (x % 2 == 0))
				{
					num = y / 2 * widthhalf + x / 2;
					puc_dst[num * 2 + height * width + 0] = (unsigned char)(ClipU8(RGB2U(r, g, b)));
					puc_dst[num * 2 + height * width + 1] = (unsigned char)(ClipU8(RGB2V(r, g, b)));
				}
			}
			else if (colorimg_tpye == COLORIMG_TYPE_YUV420NV21)
			{
				puc_dst[index] = (unsigned char)(ClipU8(RGB2Y(r, g, b)));

				if ((y % 2 == 0) && (x % 2 == 0))
				{
					num = y / 2 * widthhalf + x / 2;
					puc_dst[num * 2 + height * width + 0] = (unsigned char)(ClipU8(RGB2V(r, g, b)));
					puc_dst[num * 2 + height * width + 1] = (unsigned char)(ClipU8(RGB2U(r, g, b)));
				}
			}
			else if (colorimg_tpye == COLORIMG_TYPE_YUV420PLANE)
			{
				puc_dst[index] = (unsigned char)(ClipU8(RGB2Y(r, g, b)));

				if ((y % 2 == 0) && (x % 2 == 0))
				{
					num = y / 2 * widthhalf + x / 2;
					puc_dst[num + width * height] = (unsigned char)(ClipU8(RGB2U(r, g, b)));
					puc_dst[num + width * height + width * height / 4] = (unsigned char)(ClipU8(RGB2V(r, g, b)));
				}
			}
			else if (colorimg_tpye == COLORIMG_TYPE_YVU420PLANE)
			{
				puc_dst[index] = (unsigned char)(ClipU8(RGB2Y(r, g, b)));

				if ((y % 2 == 0) && (x % 2 == 0))
				{
					num = y / 2 * widthhalf + x / 2;
					puc_dst[num + width * height] = (unsigned char)(ClipU8(RGB2V(r, g, b)));
					puc_dst[num + width * height + width * height / 4] = (unsigned char)(ClipU8(RGB2U(r, g, b)));
				}
			}
		}
	}

	if (colorimg_tpye == COLORIMG_TYPE_RGBA)
	{
		IRContent.pst_dst_img.n_channels = 4;
	}
	else
	{
		IRContent.pst_dst_img.n_channels = 3;
	}

}

//Y8纠偏
void CInfraredCore::ImgY8AdjustBC()
{
	int n_width = IRContent.pst_src_img.n_width;
	int n_height = IRContent.pst_src_img.n_height;
	unsigned char* puc_dst = IRContent.pst_dst_img.puc_data;
	unsigned char* puc_src = IRContent.pst_src_img.puc_data;
	int nContrastexp = IrPara.st_y8adjustbc_para.n_adjustbc_contrast;
	int nBrightexp = IrPara.st_y8adjustbc_para.n_adjustbc_bright;

	Y8AdjustBC(puc_dst, puc_src, n_width, n_height, nContrastexp, nBrightexp);
}

void CInfraredCore::Y8AdjustBC(unsigned char* puc_dst, unsigned char* puc_src, int n_width, int n_height, int nContrastexp, int nBrightexp)
{
	int i, temp;
	int nLen = n_width * n_height;
	float fY8Sum = 0;
	int nY8Mean;
	for (i = 0; i < nLen; i++)
	{
		fY8Sum += puc_src[i];
	}
	nY8Mean = int(fY8Sum / nLen + 0.5);

	for (i = 0; i < nLen; i++)
	{
		temp = (puc_src[i] - nY8Mean) * nContrastexp / 128 + nBrightexp - 128 + nY8Mean;
		puc_dst[i] = (unsigned char)(max_ir(min_ir(temp, 255), 0));
	}
}

long CInfraredCore::get_nuc_switch(bool* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = IrPara.st_nuc_para.b_nuc_switch;
	return ITA_OK;
}

long CInfraredCore::put_nuc_switch(bool newVal)
{
	if (IrPara.st_nuc_para.b_nuc_switch == newVal)
	{
		return ITA_INACTIVE_CALL;
	}

	IrPara.st_nuc_para.b_nuc_switch = newVal;
	if (newVal == 1)
	{
		if (!IRContent.pus_shutter_base) {
			IRContent.pus_shutter_base = (unsigned short*)porting_calloc_mem(gWidth * gHeight, sizeof(unsigned short), ITA_IMAGE_MODULE);
		}
		if (!IRContent.pus_x16_mat) {
			IRContent.pus_x16_mat = (unsigned short*)porting_calloc_mem(gWidth * gHeight, sizeof(unsigned short), ITA_IMAGE_MODULE);
		}
		if (!IRContent.pus_gain_mat) {
			IRContent.pus_gain_mat = (unsigned short*)porting_calloc_mem(gWidth * gHeight, sizeof(unsigned short), ITA_IMAGE_MODULE);
		}
	}
	return ITA_OK;
}

long CInfraredCore::get_rpbp_updatalist_switch(bool* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = IrPara.st_rpbp_para.b_rpbp_updatalist_switch;
	return ITA_OK;
}

long CInfraredCore::put_rpbp_updatalist_switch(bool newVal)
{
	if (IrPara.st_rpbp_para.b_rpbp_updatalist_switch == newVal)
	{
		return ITA_ILLEGAL_PAPAM_ERR;
	}
	IrPara.st_rpbp_para.b_rpbp_updatalist_switch = newVal;
	return ITA_OK;
}

long CInfraredCore::get_rpbp_switch(bool* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = IrPara.st_rpbp_para.b_rpbp_switch;
	return ITA_OK;
}
long CInfraredCore::put_rpbp_switch(bool newVal)
{
	if (IrPara.st_rpbp_para.b_rpbp_switch == newVal)
	{
		return ITA_INACTIVE_CALL;
	}

	IrPara.st_rpbp_para.b_rpbp_switch = newVal;
	if (newVal == 1)
	{
		if (!IRContent.pus_gain_mat)
		{
			IRContent.pus_gain_mat = (unsigned short*)porting_calloc_mem(gWidth * gHeight, sizeof(unsigned short), ITA_IMAGE_MODULE);
		}
		if (!gBadPointList)
		{
			gBadPointList = (unsigned short*)porting_calloc_mem((nBadPointNumThr + 1) * 2, sizeof(unsigned short), ITA_IMAGE_MODULE);
		}
	}
	return ITA_OK;
}

long CInfraredCore::get_rpbp_auto_switch(unsigned char* value)
{
	if (value == NULL)
	{
		return ITA_ILLEGAL_PAPAM_ERR;
	}
	*value = IrPara.st_rpbp_para.b_rpbp_auto_switch;
	return ITA_OK;
}
long CInfraredCore::put_rpbp_auto_switch(unsigned char value)
{
	if (value == IrPara.st_rpbp_para.b_rpbp_auto_switch)
	{
		return ITA_INACTIVE_CALL;
	}
	IrPara.st_rpbp_para.b_rpbp_auto_switch = value;
	return ITA_OK;
}

long CInfraredCore::get_tff_switch(bool* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = IrPara.st_tff_para.b_tff_switch;
	return ITA_OK;
}

ITA_RESULT CInfraredCore::put_tff_switch(bool newVal)
{
	if(IrPara.st_tff_para.b_tff_switch == newVal)
		return ITA_INACTIVE_CALL;

	IrPara.st_tff_para.b_tff_switch = newVal;
	if (newVal == 1)  //开算法就申请内存
	{
		if (!g_pusTffLastFrame)
		{
			g_pusTffLastFrame = (short*)porting_calloc_mem(gWidth * gHeight, sizeof(short), ITA_IMAGE_MODULE);
		}
		if (!diff)
		{
			diff = (short*)porting_calloc_mem(gWidth * gHeight, sizeof(short), ITA_IMAGE_MODULE);
		}
		if (!pus_last_pad)
		{
			pus_last_pad = (short*)porting_calloc_mem((gWidth + 15) * (gHeight + 15), sizeof(short), ITA_IMAGE_MODULE);
		}
	}
	return ITA_OK;
}

long CInfraredCore::get_tff_std(int* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = IrPara.st_tff_para.n_tff_std;
	return ITA_OK;
}

long CInfraredCore::put_tff_std(int newVal)
{
	IrPara.st_tff_para.n_tff_std = newVal;
	//每次设置标准差后更新时域滤波权重表
	GetTFFGuassWeight();
	return ITA_OK;
}

long CInfraredCore::get_rvs_switch(bool* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = IrPara.st_rvs_para.b_rs_switch;
	return ITA_OK;
}

long CInfraredCore::put_rvs_switch(bool newVal)
{
	if (IrPara.st_rvs_para.b_rs_switch == newVal)
		return ITA_INACTIVE_CALL;

	//如果之前去条纹算法都是关闭，现在需要打开则重新申请内存
	if (newVal == 1)  //开算法就申请内存
	{
		//去条纹算法内存申请
		if (!pus_img_low)
		{
			pus_img_low = (short*)porting_calloc_mem(gWidth * gHeight, sizeof(short), ITA_IMAGE_MODULE);    // 滤波后的低频图像
		}
		if (!pus_img_weight)
		{
			pus_img_weight = (unsigned short*)porting_calloc_mem(gWidth * gHeight, sizeof(unsigned short), ITA_IMAGE_MODULE);
		}
		if (!g_img_high)
		{
			g_img_high = (short*)porting_calloc_mem(gWidth * gHeight, sizeof(short), ITA_IMAGE_MODULE);        // 细节图像 
		}
		if (!g_ver_mean)
		{
			g_ver_mean = (short*)porting_calloc_mem(gWidth, sizeof(short), ITA_IMAGE_MODULE);  // 列均值数组
		}
		if (!g_col_num)
		{
			g_col_num = (unsigned short*)porting_calloc_mem(gWidth, sizeof(unsigned short), ITA_IMAGE_MODULE);
		}
		if (!g_row_mean)
		{
			g_row_mean = (short*)porting_calloc_mem(gHeight, sizeof(short), ITA_IMAGE_MODULE);     // 列均值数组
		}
	}
	IrPara.st_rvs_para.b_rs_switch = newVal;
	return ITA_OK;
}

long CInfraredCore::get_rvs_win_width(int* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = IrPara.st_rvs_para.n_stripe_win_width;
	return ITA_OK;
}
long CInfraredCore::put_rvs_win_width(int newVal)
{
	IrPara.st_rvs_para.n_stripe_win_width = newVal;
	return ITA_OK;
}
long CInfraredCore::get_rvs_weight_thresh(int* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = IrPara.st_rvs_para.n_stripe_weight_thresh;
	return ITA_OK;
}
long CInfraredCore::put_rvs_weight_thresh(int newVal)
{
	IrPara.st_rvs_para.n_stripe_weight_thresh = newVal;
	return ITA_OK;
}

long CInfraredCore::get_rvs_detail_thresh(int* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = IrPara.st_rvs_para.n_stripe_detail_thresh;
	return ITA_OK;
}

long CInfraredCore::put_rvs_detail_thresh(int newVal)
{
	IrPara.st_rvs_para.n_stripe_detail_thresh = newVal;
	return ITA_OK;
}

long CInfraredCore::get_rvs_std(int* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = IrPara.st_rvs_para.n_stripe_std;
	return ITA_OK;
}

long CInfraredCore::put_rvs_std(int newVal)
{
	IrPara.st_rvs_para.n_stripe_std = newVal;
	GetGaussGrayWeightTable(g_gray_weight_table_Ver, GRAY_WEIGHT_TABLE_LEN, IrPara.st_rvs_para.n_stripe_std);
	return ITA_OK;
}


long CInfraredCore::get_rhs_switch(bool* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = IrPara.st_rhs_para.b_rs_switch;
	return ITA_OK;
}

long CInfraredCore::put_rhs_switch(bool newVal)
{
	if (IrPara.st_rhs_para.b_rs_switch == newVal)
		return ITA_INACTIVE_CALL;

	if (newVal == 1)
	{
		//去条纹算法内存申请
		if (!pus_img_low)
		{
			pus_img_low = (short*)porting_calloc_mem(gWidth * gHeight, sizeof(short), ITA_IMAGE_MODULE);    // 滤波后的低频图像
		}
		if (!pus_img_weight)
		{
			pus_img_weight = (unsigned short*)porting_calloc_mem(gWidth * gHeight, sizeof(unsigned short), ITA_IMAGE_MODULE);
		}
		if (!g_img_high)
		{
			g_img_high = (short*)porting_calloc_mem(gWidth * gHeight, sizeof(short), ITA_IMAGE_MODULE);        // 细节图像 
		}
		if (!g_ver_mean)
		{
			g_ver_mean = (short*)porting_calloc_mem(gWidth, sizeof(short), ITA_IMAGE_MODULE);  // 列均值数组
		}
		if (!g_col_num)
		{
			g_col_num = (unsigned short*)porting_calloc_mem(gWidth, sizeof(unsigned short), ITA_IMAGE_MODULE);
		}
		if (!g_row_mean)
		{
			g_row_mean = (short*)porting_calloc_mem(gHeight, sizeof(short), ITA_IMAGE_MODULE);     // 列均值数组
		}
	}
	IrPara.st_rhs_para.b_rs_switch = newVal;

	return ITA_OK;
}

long CInfraredCore::get_rhs_win_width(int* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = IrPara.st_rhs_para.n_stripe_win_width;
	return ITA_OK;
}
long CInfraredCore::put_rhs_win_width(int newVal)
{
	IrPara.st_rhs_para.n_stripe_win_width = newVal;
	return ITA_OK;
}

long CInfraredCore::get_rhs_weight_thresh(int* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = IrPara.st_rhs_para.n_stripe_weight_thresh;
	return ITA_OK;
}
long CInfraredCore::put_rhs_weight_thresh(int newVal)
{
	IrPara.st_rhs_para.n_stripe_weight_thresh = newVal;
	return ITA_OK;
}

long CInfraredCore::get_rhs_detail_thresh(int* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = IrPara.st_rhs_para.n_stripe_detail_thresh;
	return ITA_OK;
}

long CInfraredCore::put_rhs_detail_thresh(int newVal)
{
	IrPara.st_rhs_para.n_stripe_detail_thresh = newVal;
	return ITA_OK;
}
long CInfraredCore::get_rhs_std(int* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = IrPara.st_rhs_para.n_stripe_std;
	return ITA_OK;
}

long CInfraredCore::put_rhs_std(int newVal)
{
	IrPara.st_rhs_para.n_stripe_std = newVal;
	GetGaussGrayWeightTable(g_gray_weight_table_Hor, GRAY_WEIGHT_TABLE_LEN, IrPara.st_rvs_para.n_stripe_std);
	return ITA_OK;
}

//空域滤波
long CInfraredCore::get_rn_switch(bool* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = IrPara.st_rn_para.b_rn_switch;
	return ITA_OK;
}
long CInfraredCore::put_rn_switch(bool newVal)
{
	IrPara.st_rn_para.b_rn_switch = newVal;
	return ITA_OK;
}

long CInfraredCore::get_rn_arith_type(int* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = IrPara.st_rn_para.n_rn_arith_type;
	return ITA_OK;
}
long CInfraredCore::put_rn_arith_type(int newVal)
{
	IrPara.st_rn_para.n_rn_arith_type = newVal;
	return ITA_OK;
}

long CInfraredCore::get_rn_dist_std(int* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = IrPara.st_rn_para.n_dist_std;
	return ITA_OK;
}
long CInfraredCore::put_rn_dist_std(int newVal)
{
	IrPara.st_rn_para.n_dist_std = newVal;
	GetGuassPosWeightTable(g_pos_weight_table, IrPara.st_rn_para.n_win_size, IrPara.st_rn_para.n_win_size, IrPara.st_rn_para.n_dist_std);
	return ITA_OK;
}

long CInfraredCore::get_rn_gray_std(int* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = IrPara.st_rn_para.n_gray_std;
	return ITA_OK;
}

long CInfraredCore::put_rn_gray_std(int newVal)
{
	IrPara.st_rn_para.n_gray_std = newVal;
	GetGaussGrayWeightTable(g_gray_weight_table, GRAY_WEIGHT_TABLE_LEN, IrPara.st_rn_para.n_gray_std);
	return ITA_OK;
}
/**************初始化气体检测*****/
long CInfraredCore::get_hsm_switch(bool* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = IrPara.st_hsm_para.b_hsm_switch;
	return ITA_OK;
}
long CInfraredCore::put_hsm_switch(bool newVal)
{
	if (IrPara.st_hsm_para.b_hsm_switch == newVal)
		return ITA_INACTIVE_CALL;
	if (newVal == 1)
	{
		//气体检测内存申请,内存只申请一次
		if (!pus_pre) {
			pus_pre = (short*)porting_calloc_mem(gWidth * gHeight, sizeof(short), ITA_IMAGE_MODULE);//前一帧Y16数据
		}
		if (!pus_deta) {
			pus_deta = (short*)porting_calloc_mem(gWidth * gHeight, sizeof(short), ITA_IMAGE_MODULE);//Y16差值数据
		}
		if (!hsmAhist) {
			hsmAhist = (unsigned short*)porting_calloc_mem(GRAYLEVEL_16BIT, sizeof(unsigned short), ITA_IMAGE_MODULE);
		}
	}
	//关闭算法后，等ITA实例销毁后再释放内存
	IrPara.st_hsm_para.b_hsm_switch = newVal;

	return ITA_OK;	
}
long CInfraredCore::get_hsm_weight(int* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = IrPara.st_hsm_para.n_hsm_weight;
	return ITA_OK;
}
ITA_RESULT CInfraredCore::put_hsm_weight(int newVal)
{
	IrPara.st_hsm_para.n_hsm_weight = newVal;
	IrPara.st_hsm_para.f_hsm_k = (IrPara.st_hsm_para.n_hsm_weight - IrPara.st_hsm_para.n_hsm_stayWeight) / IrPara.st_hsm_para.n_hsm_interval;
	IrPara.st_hsm_para.f_hsm_b = IrPara.st_hsm_para.n_hsm_stayWeight - IrPara.st_hsm_para.f_hsm_k * IrPara.st_hsm_para.n_hsm_stayThr;
	return ITA_OK;
}

long CInfraredCore::get_hsm_stayThr(int* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = IrPara.st_hsm_para.n_hsm_stayThr;
	return ITA_OK;
}
long CInfraredCore::put_hsm_stayThr(int newVal)
{
	IrPara.st_hsm_para.n_hsm_stayThr = newVal;
	IrPara.st_hsm_para.f_hsm_k = (IrPara.st_hsm_para.n_hsm_weight - IrPara.st_hsm_para.n_hsm_stayWeight) / IrPara.st_hsm_para.n_hsm_interval;
	IrPara.st_hsm_para.f_hsm_b = IrPara.st_hsm_para.n_hsm_stayWeight - IrPara.st_hsm_para.f_hsm_k * IrPara.st_hsm_para.n_hsm_stayThr;
	return ITA_OK;
}

long CInfraredCore::get_hsm_stayWeight(int* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = IrPara.st_hsm_para.n_hsm_stayWeight;
	return ITA_OK;
}
long CInfraredCore::put_hsm_stayWeight(int newVal)
{
	IrPara.st_hsm_para.n_hsm_stayWeight = newVal;
	IrPara.st_hsm_para.f_hsm_k = (IrPara.st_hsm_para.n_hsm_weight - IrPara.st_hsm_para.n_hsm_stayWeight) / IrPara.st_hsm_para.n_hsm_interval;
	IrPara.st_hsm_para.f_hsm_b = IrPara.st_hsm_para.n_hsm_stayWeight - IrPara.st_hsm_para.f_hsm_k * IrPara.st_hsm_para.n_hsm_stayThr;
	return ITA_OK;
}

long CInfraredCore::get_hsm_interval(int* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = IrPara.st_hsm_para.n_hsm_interval;
	return ITA_OK;
}
long CInfraredCore::put_hsm_interval(int newVal)
{
	IrPara.st_hsm_para.n_hsm_interval = newVal;
	IrPara.st_hsm_para.f_hsm_k = (IrPara.st_hsm_para.n_hsm_weight - IrPara.st_hsm_para.n_hsm_stayWeight) / IrPara.st_hsm_para.n_hsm_interval;
	IrPara.st_hsm_para.f_hsm_b = IrPara.st_hsm_para.n_hsm_stayWeight - IrPara.st_hsm_para.f_hsm_k * IrPara.st_hsm_para.n_hsm_stayThr;
	return ITA_OK;
}

long CInfraredCore::get_hsm_midfilterSwitch(bool* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = IrPara.st_hsm_para.b_midfilter_switch;
	return ITA_OK;
}
long CInfraredCore::put_hsm_midfilterSwitch(bool newVal)
{
	if (IrPara.st_hsm_para.b_midfilter_switch == newVal)
		return ITA_INACTIVE_CALL;
	IrPara.st_hsm_para.b_midfilter_switch = newVal;
	return ITA_OK;
}

long CInfraredCore::get_hsm_midfilterWinSize(int* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = IrPara.st_hsm_para.n_midfilter_winSize;
	return ITA_OK;
}

long CInfraredCore::put_hsm_midfilterWinSize(int newVal)
{
	IrPara.st_hsm_para.n_midfilter_winSize = newVal;
	return ITA_OK;
}

long CInfraredCore::get_hsm_delta_upthr(int* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = IrPara.st_hsm_para.n_delta_upthr;
	return ITA_OK;
}

long CInfraredCore::put_hsm_delta_upthr(int newVal)
{
	IrPara.st_hsm_para.n_delta_upthr = newVal;
	return ITA_OK;
}

long CInfraredCore::get_hsm_delta_downthr(int* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = IrPara.st_hsm_para.n_delta_downthr;
	return ITA_OK;
}

long CInfraredCore::put_hsm_delta_downthr(int newVal)
{
	IrPara.st_hsm_para.n_delta_downthr = newVal;
	return ITA_OK;
}

long CInfraredCore::get_hsm_point_thr(int* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = IrPara.st_hsm_para.n_point_thr;
	return ITA_OK;
}

long CInfraredCore::put_hsm_point_thr(int newVal)
{
	IrPara.st_hsm_para.n_point_thr = newVal;
	return ITA_OK;
}
/*************************************/
long CInfraredCore::get_sp_switch(bool* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = IrPara.st_sp_para.b_sp_switch;
	return ITA_OK;
}

long CInfraredCore::put_sp_switch(bool newVal)
{
	if (IrPara.st_sp_para.b_sp_switch == newVal)
	{
		return ITA_INACTIVE_CALL;
	}
	IrPara.st_sp_para.b_sp_switch = newVal;
	return ITA_OK;
}

long CInfraredCore::get_sp_laplace_weight_y16(float* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = IrPara.st_sp_para.n_sp_laplace_weight;
	return ITA_OK;
}

long CInfraredCore::put_sp_laplace_weight_y16(float newVal)
{
	IrPara.st_sp_para.n_sp_laplace_weight = newVal;
	return ITA_OK;
}

long CInfraredCore::get_flip_switch(bool* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = IrPara.st_flip_para.b_flip_switch;
	return ITA_OK;
}
ITA_RESULT CInfraredCore::put_flip_switch(bool newVal)
{
	if (IrPara.st_flip_para.b_flip_switch == newVal)
	{
		return ITA_INACTIVE_CALL;
	}

	IrPara.st_flip_para.b_flip_switch = newVal;
	return ITA_OK;
}

long CInfraredCore::get_flip_type(int* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = IrPara.st_flip_para.nFlipType;
	return ITA_OK;
}

ITA_RESULT CInfraredCore::put_flip_type(int newVal)
{
	//外面已经做过检查，这里不必做。
	/*if (IrPara.st_flip_para.nFlipType == newVal)
	{
		return ITA_INACTIVE_CALL;
	}*/

	IrPara.st_flip_para.nFlipType = newVal;
	return ITA_OK;
}

//图像旋转
long CInfraredCore::get_rotate_switch(bool* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = IrPara.st_rotate_para.b_rotate_switch;
	return ITA_OK;
}
ITA_RESULT CInfraredCore::put_rotate_switch(bool newVal)
{
	if (IrPara.st_rotate_para.b_rotate_switch == newVal)
	{
		return ITA_INACTIVE_CALL;
	}
	IrPara.st_rotate_para.b_rotate_switch = newVal;
	return ITA_OK;
}

long CInfraredCore::get_rotate_type(int* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = IrPara.st_rotate_para.nRotateType;
	return ITA_OK;
}

ITA_RESULT CInfraredCore::put_rotate_type(int newVal)
{
	//外面已经做过检查，这里不必做。nRotateType值默认90°，用户从不旋转到转90°这里返回的值不对。
	/*if (IrPara.st_rotate_para.nRotateType == newVal)
	{
		return ITA_INACTIVE_CALL;
	}*/
	IrPara.st_rotate_para.nRotateType = newVal;
	return ITA_OK;
}

long CInfraredCore::get_drt_switch(bool* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = IrPara.st_drt_para.b_drt_switch;
	return ITA_OK;
}

ITA_RESULT CInfraredCore::put_drt_switch(bool newVal)
{
	if (IrPara.st_drt_para.b_drt_switch == newVal)
	{
		return ITA_INACTIVE_CALL;
	}
	IrPara.st_drt_para.b_drt_switch = newVal;
	return ITA_OK;
}

//调光参数
long CInfraredCore::get_drt_type(int* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = IrPara.st_drt_para.n_drt_type;
	return ITA_OK;
}

ITA_RESULT CInfraredCore::put_drt_type(int newVal)
{
	//ImageProceesor已做无效判断
	/*if (IrPara.st_drt_para.n_drt_type == newVal)
		return ITA_INACTIVE_CALL;*/
	if (newVal == 2)
	{
		if (!puc_dst_lin)
		{
			puc_dst_lin = (unsigned char*)porting_calloc_mem(gWidth * gHeight, sizeof(unsigned char), ITA_IMAGE_MODULE);
		}
		if (!puc_dst_phe)
		{
			puc_dst_phe = (unsigned char*)porting_calloc_mem(gWidth * gHeight, sizeof(unsigned char), ITA_IMAGE_MODULE);
		}
	}
	IrPara.st_drt_para.n_drt_type = newVal;
	return ITA_OK;
}

long CInfraredCore::get_discard_upratio(int* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = IrPara.st_drt_para.n_discard_upratio;
	return ITA_OK;
}

long CInfraredCore::put_discard_upratio(int newVal)
{
	IrPara.st_drt_para.n_discard_upratio = newVal;
	return ITA_OK;
}

long CInfraredCore::get_discard_downratio(int* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = IrPara.st_drt_para.n_discard_downratio;
	return ITA_OK;
}

long CInfraredCore::put_discard_downratio(int newVal)
{
	IrPara.st_drt_para.n_discard_downratio = newVal;
	return ITA_OK;
}

long CInfraredCore::get_linear_brightness(int* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = IrPara.st_drt_para.n_linear_brightness;
	return ITA_OK;
}

long CInfraredCore::put_linear_brightness(int newVal)
{
	IrPara.st_drt_para.n_linear_brightness = newVal;
	return ITA_OK;
}

long CInfraredCore::get_linear_contrast(int* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = IrPara.st_drt_para.n_linear_contrast;
	return ITA_OK;
}

long CInfraredCore::put_linear_contrast(int newVal)
{
	IrPara.st_drt_para.n_linear_contrast = newVal;
	return ITA_OK;
}

long CInfraredCore::get_linear_restrain_rangethre(int* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = IrPara.st_drt_para.n_linear_restrain_rangethre;
	return ITA_OK;
}

long CInfraredCore::put_linear_restrain_rangethre(int newVal)
{
	IrPara.st_drt_para.n_linear_restrain_rangethre = newVal;
	return ITA_OK;
}

long CInfraredCore::get_heq_plat_thresh(int* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = IrPara.st_drt_para.n_heq_plat_thresh;
	return ITA_OK;
}

long CInfraredCore::put_heq_plat_thresh(int newVal)
{
	IrPara.st_drt_para.n_heq_plat_thresh = newVal;
	return ITA_OK;
}

long CInfraredCore::get_heq_range_max(int* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = IrPara.st_drt_para.n_heq_range_max;
	return ITA_OK;
}

long CInfraredCore::put_heq_range_max(int newVal)
{
	IrPara.st_drt_para.n_heq_range_max = newVal;
	return ITA_OK;
}
long CInfraredCore::get_heq_midvalue(int* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = IrPara.st_drt_para.n_heq_midvalue;
	return ITA_OK;
}

long CInfraredCore::put_heq_midvalue(int newVal)
{
	IrPara.st_drt_para.n_heq_midvalue = newVal;
	return ITA_OK;
}


long CInfraredCore::get_manultone_maxy16(int* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = IrPara.st_drt_para.n_manltone_maxy16;
	return ITA_OK;
}

long CInfraredCore::put_manultone_maxy16(int newVal)
{
	IrPara.st_drt_para.n_manltone_maxy16 = (short)newVal;
	return ITA_OK;
}

long CInfraredCore::get_manultone_miny16(int* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = IrPara.st_drt_para.n_manltone_miny16;
	return ITA_OK;
}

long CInfraredCore::put_manultone_miny16(int newVal)
{
	IrPara.st_drt_para.n_manltone_miny16 = (short)newVal;
	return ITA_OK;
}

//人体医疗调光
long CInfraredCore::get_personmedical_y16tbase(int* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = IrPara.st_drt_para.n_personmedical_y16_base;
	return ITA_OK;
}

long CInfraredCore::put_personmedical_y16tbase(int newVal)
{
	IrPara.st_drt_para.n_personmedical_y16_base = (short)newVal;
	return ITA_OK;
}

long CInfraredCore::get_personmedical_y16high(int* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = IrPara.st_drt_para.n_personmedical_y16_high;
	return ITA_OK;
}

long CInfraredCore::put_personmedical_y16high(int newVal)
{
	IrPara.st_drt_para.n_personmedical_y16_high = (short)newVal;
	return ITA_OK;
}

long CInfraredCore::get_mix_ThrHigh(int* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = IrPara.st_drt_para.n_mix_ThrHigh;
	return ITA_OK;
}

long CInfraredCore::put_mix_ThrHigh(int newVal)
{
	IrPara.st_drt_para.n_mix_ThrHigh = newVal;
	return ITA_OK;
}

long CInfraredCore::get_mix_ThrLow(int* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = IrPara.st_drt_para.n_mix_ThrLow;
	return ITA_OK;
}

long CInfraredCore::put_mix_ThrLow(int newVal)
{
	IrPara.st_drt_para.n_mix_ThrLow = newVal;
	return ITA_OK;
}

long CInfraredCore::get_iie_switch(bool* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = IrPara.st_iie_para.b_iie_switch;
	return ITA_OK;
}
long CInfraredCore::put_iie_switch(bool newVal)
{
	if (IrPara.st_iie_para.b_iie_switch == newVal)
		return ITA_INACTIVE_CALL;

	if (newVal == 1)
	{
		//细节增强内存申请
		if (!ps_detail_img)
		{
			ps_detail_img = (short*)porting_calloc_mem(gWidth * gHeight, sizeof(short), ITA_IMAGE_MODULE); // 细锟斤拷图锟斤拷
		}
		if (!puc_base_img)
		{
			puc_base_img = (unsigned char*)porting_calloc_mem(gWidth * gHeight, sizeof(unsigned char), ITA_IMAGE_MODULE); // 8bit锟斤拷频
		}
		if (!pus_base_img)
		{
			pus_base_img = (short*)porting_calloc_mem(gWidth * gHeight, sizeof(short), ITA_IMAGE_MODULE); // 16bit锟斤拷频
		}	
	}
	IrPara.st_iie_para.b_iie_switch = newVal;
	return ITA_OK;
}

long CInfraredCore::get_iie_enhance_coef(int* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = IrPara.st_iie_para.n_iie_enhance_coef;
	return ITA_OK;
}

long CInfraredCore::put_iie_enhance_coef(int newVal)
{
	IrPara.st_iie_para.n_iie_enhance_coef = newVal;
	return ITA_OK;
}

long CInfraredCore::get_iie_pos_std(int* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = IrPara.st_iie_para.n_iie_gauss_std;
	return ITA_OK;
}

long CInfraredCore::put_iie_pos_std(int newVal)
{
	IrPara.st_iie_para.n_iie_gauss_std = newVal;
	GetGuassPosWeightTable(g_IDE_pos_weight_table, IrPara.st_iie_para.n_iie_pos_win_size, IrPara.st_iie_para.n_iie_pos_win_size, IrPara.st_iie_para.n_iie_gauss_std);
	return ITA_OK;
}

long CInfraredCore::get_iie_detail_thr(unsigned char* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = IrPara.st_iie_para.n_iie_detail_thr;
	return ITA_OK;
}

long CInfraredCore::put_iie_detail_thr(unsigned char newVal)
{
	IrPara.st_iie_para.n_iie_detail_thr = newVal;
	return ITA_OK;
}

//Y8纠偏
long CInfraredCore::get_y8adjustbc_switch(bool* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = IrPara.st_y8adjustbc_para.b_adjustbc_switch;
	return ITA_OK;
}

long CInfraredCore::put_y8adjustbc_switch(bool newVal)
{
	if (IrPara.st_y8adjustbc_para.b_adjustbc_switch == newVal)
	{
		return ITA_INACTIVE_CALL;
	}

	IrPara.st_y8adjustbc_para.b_adjustbc_switch = newVal;
	return ITA_OK;
}

long CInfraredCore::get_y8adjustbc_contrast(int* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}

	*pVal = IrPara.st_y8adjustbc_para.n_adjustbc_contrast;
	return ITA_OK;
}

long CInfraredCore::put_y8adjustbc_contrast(int newVal)
{
	IrPara.st_y8adjustbc_para.n_adjustbc_contrast = newVal;
	return ITA_OK;
}

long CInfraredCore::get_y8adjustbc_bright(int* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}

	*pVal = IrPara.st_y8adjustbc_para.n_adjustbc_bright;
	return ITA_OK;
}

long CInfraredCore::put_y8adjustbc_bright(int newVal)
{
	IrPara.st_y8adjustbc_para.n_adjustbc_bright = newVal;
	return ITA_OK;
}

long CInfraredCore::get_ImgZoom(float* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = IrPara.st_zoom_para.fZoom;
	return ITA_OK;
}

ITA_RESULT CInfraredCore::put_ImgZoom(float newVal)
{
	if (newVal < 1)
	{
		return ITA_ILLEGAL_PAPAM_ERR;
	}
	/*如果放大几百倍申请不到内存，那么puc_data为NULL。*/
	if (newVal != IrPara.st_zoom_para.fZoom)
	{
		void *pBuf = porting_calloc_mem((int)(4 * gWidth * gHeight * newVal * newVal), sizeof(unsigned char), ITA_IMAGE_MODULE);
		if (!pBuf)
			return ITA_OUT_OF_MEMORY;
		if (IRContent.pst_src_img.puc_data)
			porting_free_mem(IRContent.pst_src_img.puc_data);
		IRContent.pst_src_img.puc_data = (unsigned char*)pBuf;
		pBuf = porting_calloc_mem((int)(4 * gWidth * gHeight * newVal * newVal), sizeof(unsigned char), ITA_IMAGE_MODULE);
		if (!pBuf)
			return ITA_OUT_OF_MEMORY;
		if (IRContent.pst_dst_img.puc_data)
			porting_free_mem(IRContent.pst_dst_img.puc_data);
		IRContent.pst_dst_img.puc_data = (unsigned char*)pBuf;
	}

	IrPara.st_zoom_para.fZoom = newVal;

	if (IrPara.st_zoom_para.n_ZoomType == BilinearFast)
	{
		int srcH = gHeight;
		int srcW = gWidth;
		double scaleH = IrPara.st_zoom_para.fZoom;
		double scaleW = IrPara.st_zoom_para.fZoom;
		int dstW = int(srcW * scaleW);
		int dstH = int(srcH * scaleH);
		void *pBuf = porting_calloc_mem((dstH + dstW + 2) * 2, sizeof(unsigned short), ITA_IMAGE_MODULE);
		if (!pBuf)
			return ITA_OUT_OF_MEMORY;
		if (ResizeTable)
			porting_free_mem(ResizeTable);
		ResizeTable = (unsigned short*)pBuf;
		GetTable(srcH, srcW, scaleH, scaleW, ResizeTable);
	}

	//等温线功能中y16ArrZoom依赖放大倍数
	if (IrPara.st_psdclr_coef.b_psdclr_equallinemode == 1)
	{
		int y16ArrZoomLen = int(IrPara.st_zoom_para.fZoom * IrPara.st_zoom_para.fZoom * gWidth * gHeight);
		void *pBuf = porting_calloc_mem(y16ArrZoomLen, sizeof(short), ITA_IMAGE_MODULE);
		if (!pBuf)
			return ITA_OUT_OF_MEMORY;
		if (y16ArrZoom)
			porting_free_mem(y16ArrZoom);
		y16ArrZoom = (short*)pBuf;
	}

	return ITA_OK;
}

long CInfraredCore::get_ImgZoomType(int* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = IrPara.st_zoom_para.n_ZoomType;
	return ITA_OK;
}

long CInfraredCore::put_ImgZoomType(int newVal)
{
	if (IrPara.st_zoom_para.n_ZoomType == newVal)
	{
		return ITA_INACTIVE_CALL;
	}

	if (newVal == BilinearFast)
	{
		int srcH = gHeight;
		int srcW = gWidth;
		double scaleH = IrPara.st_zoom_para.fZoom;
		double scaleW = IrPara.st_zoom_para.fZoom;
		int dstW = int(srcW * scaleW);
		int dstH = int(srcH * scaleH);
		ResizeTable = (unsigned short*)porting_calloc_mem((dstH + dstW + 2) * 2, sizeof(unsigned short), ITA_IMAGE_MODULE);
		if (!ResizeTable)
			return ITA_OUT_OF_MEMORY;
		memset(ResizeTable, 0, (dstH + dstW + 2) * 2 * sizeof(unsigned short));
		GetTable(srcH, srcW, scaleH, scaleW, ResizeTable);
	}

	if (((newVal != BilinearFast)) && (IrPara.st_zoom_para.n_ZoomType == BilinearFast))
	{
		if (ResizeTable)
		{
			porting_free_mem(ResizeTable);
			ResizeTable = NULL;
		}
	}

	IrPara.st_zoom_para.n_ZoomType = newVal;
	return ITA_OK;
}

long CInfraredCore::get_psdclr_switch(bool* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = IrPara.st_psdclr_coef.b_psdclr_switch;
	return ITA_OK;
}

long CInfraredCore::put_psdclr_switch(bool newVal)
{
	if (IrPara.st_psdclr_coef.b_psdclr_switch == newVal)
	{
		return ITA_INACTIVE_CALL;
	}

	IrPara.st_psdclr_coef.b_psdclr_switch = newVal;
	return ITA_OK;
}

long CInfraredCore::get_psdclr_type(int* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = IrPara.st_psdclr_coef.t_psdclr_type;
	return ITA_OK;
}

void CInfraredCore::tvRang2FullRange(unsigned char* nDst,const unsigned char* nSrc, int width, int height)
{
	int i, nTemp;
	int nLen = height * width;
	int tMax = 235;
	int tMin = 16;
	int fMax = 255;
	int fMin = 0;
	for (i = 0; i < nLen; i++)
	{
		nTemp = fMin + (nSrc[i] - tMin) * (fMax - fMin) / (tMax - tMin);
		if (nTemp > 255)
			nDst[i] = 255;
		else if (nTemp < 0)
			nDst[i] = 0;
		else
			nDst[i] = (unsigned char)nTemp;
	}
}

long CInfraredCore::put_psdclr_type(int newVal)
{
	int total = getPaletteTotalNum();
	if ((newVal < 0) || (newVal >= total))
	{
		//IrPara.st_psdclr_coef.t_psdclr_type = 0;
		return ITA_ARG_OUT_OF_RANGE;
	}
	IrPara.st_psdclr_coef.t_psdclr_type = newVal;
	isCustomPalette = IrPara.st_psdclr_coef.t_psdclr_type >= PSEUDONUM ? true : false;
	return ITA_OK;
}

int CInfraredCore::getPaletteTotalNum()
{
	if (customPalette) {
		return customPalette->getCustomCurPaleNum() + PSEUDONUM;
	}
	return PSEUDONUM;
}

long CInfraredCore::get_colorimg_tpye(int* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = IrPara.st_psdclr_coef.n_colorimg_tpye;
	return ITA_OK;
}

long CInfraredCore::put_colorimg_tpye(int newVal)
{
	if ((newVal < 0) || (newVal >= COLORIMGTYPENUM))
	{
		//IrPara.st_psdclr_coef.n_colorimg_tpye = 0;
		return ITA_ARG_OUT_OF_RANGE;
	}
	if (IrPara.st_psdclr_coef.n_colorimg_tpye == newVal)
	{
		return ITA_INACTIVE_CALL;
	}
	IrPara.st_psdclr_coef.n_colorimg_tpye = newVal;
	return ITA_OK;
}

long CInfraredCore::get_colorimg_bytenum(int* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	if ((IrPara.st_psdclr_coef.n_colorimg_tpye == COLORIMG_TYPE_BGR) || (IrPara.st_psdclr_coef.n_colorimg_tpye == COLORIMG_TYPE_RGB))
	{
		IrPara.st_psdclr_coef.n_colorimg_len = 3 * IRContent.pst_src_img.n_width * IRContent.pst_src_img.n_height;
	}
	else if (IrPara.st_psdclr_coef.n_colorimg_tpye == COLORIMG_TYPE_RGBA)
	{
		IrPara.st_psdclr_coef.n_colorimg_len = 4 * IRContent.pst_src_img.n_width * IRContent.pst_src_img.n_height;
	}
	else if (IrPara.st_psdclr_coef.n_colorimg_tpye == COLORIMG_TYPE_RGB565 || (IrPara.st_psdclr_coef.n_colorimg_tpye == COLORIMG_TYPE_YUV422YUYV)
		|| (IrPara.st_psdclr_coef.n_colorimg_tpye == COLORIMG_TYPE_YUV422UYVY) || (IrPara.st_psdclr_coef.n_colorimg_tpye == COLORIMG_TYPE_YUV422Plane)
		|| (IrPara.st_psdclr_coef.n_colorimg_tpye == COLORIMG_TYPE_YVU422Plane))
	{
		IrPara.st_psdclr_coef.n_colorimg_len = 2 * IRContent.pst_src_img.n_width * IRContent.pst_src_img.n_height;
	}
	else if ((IrPara.st_psdclr_coef.n_colorimg_tpye == COLORIMG_TYPE_YUV420NV12) || (IrPara.st_psdclr_coef.n_colorimg_tpye == COLORIMG_TYPE_YUV420NV21)
		|| (IrPara.st_psdclr_coef.n_colorimg_tpye == COLORIMG_TYPE_YUV420PLANE) || (IrPara.st_psdclr_coef.n_colorimg_tpye == COLORIMG_TYPE_YVU420PLANE))
	{
		IrPara.st_psdclr_coef.n_colorimg_len = 3 * IRContent.pst_src_img.n_width * IRContent.pst_src_img.n_height / 2;
	}

	*pVal = IrPara.st_psdclr_coef.n_colorimg_len;
	return ITA_OK;
}

long CInfraredCore::get_psdclr_equallinemode(bool* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = IrPara.st_psdclr_coef.b_psdclr_equallinemode;
	return ITA_OK;
}

long CInfraredCore::put_psdclr_equallinemode(bool newVal)
{
	if (IrPara.st_psdclr_coef.b_psdclr_equallinemode == newVal)
	{
		return ITA_INACTIVE_CALL;
	}
	//等温线模式开启，强制打开伪彩开关
	if (newVal == 1)
	{
		put_psdclr_switch(1);
		if (!y16ArrZoom)
		{
			int y16ArrZoomLen = int(IrPara.st_zoom_para.fZoom * IrPara.st_zoom_para.fZoom * gWidth * gHeight);
			y16ArrZoom = (short*)porting_calloc_mem(y16ArrZoomLen, sizeof(short), ITA_IMAGE_MODULE);
		}
	}
	IrPara.st_psdclr_coef.b_psdclr_equallinemode = newVal;
	return ITA_OK;
}

long CInfraredCore::get_psdclr_equallinepara(EqualLineMode* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	memcpy(pVal, &(IrPara.st_psdclr_coef.s_equalline_para), sizeof(EqualLineMode));
	return ITA_OK;
}

long CInfraredCore::put_psdclr_equallinepara(EqualLineMode newVal)
{
	memcpy(&(IrPara.st_psdclr_coef.s_equalline_para), &newVal, sizeof(EqualLineMode));
	return ITA_OK;
}

ITA_RESULT CInfraredCore::putGuoGaiArithType(int newVal)
{
	if (IrPara.st_dgg_para.n_dgg_arith_type == newVal)
	{
		return ITA_ILLEGAL_PAPAM_ERR;
	}
	IrPara.st_dgg_para.n_dgg_arith_type = newVal;
	return ITA_OK;
}

ITA_RESULT CInfraredCore::getGuoGaiArithType(int* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = IrPara.st_dgg_para.n_dgg_arith_type;
	return ITA_OK;
}

ITA_RESULT CInfraredCore::putGuoGaiTPara(stDGGTPara* sDggTPara)
{
	if (sDggTPara == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}

	if (IrPara.st_dgg_para.n_dgg_arith_type == DGG_TYPE_VAR1)
	{
		IrPara.st_dgg_para.stDggTPara.fDelta = sDggTPara->fDelta;
		IrPara.st_dgg_para.stDggTPara.fStartShutterT = sDggTPara->fStartShutterT;
		IrPara.st_dgg_para.stDggTPara.fRealShutterT = sDggTPara->fRealShutterT;
	}
	else if (IrPara.st_dgg_para.n_dgg_arith_type == DGG_TYPE_VAR2)
	{
		IrPara.st_dgg_para.stDggTPara.fLastLastShutterT = sDggTPara->fLastLastShutterT;
		IrPara.st_dgg_para.stDggTPara.fLastShutterT = sDggTPara->fLastShutterT;
		IrPara.st_dgg_para.stDggTPara.fRealShutterT = sDggTPara->fRealShutterT;
	}
	return ITA_OK;
}

ITA_RESULT CInfraredCore::getGuoGaiTPara(stDGGTPara* sDggTPara)
{
	if (sDggTPara == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	if (IrPara.st_dgg_para.n_dgg_arith_type == DGG_TYPE_VAR1)
	{
		sDggTPara->fDelta = IrPara.st_dgg_para.stDggTPara.fDelta;
		sDggTPara->fStartShutterT = IrPara.st_dgg_para.stDggTPara.fStartShutterT;
		sDggTPara->fRealShutterT = IrPara.st_dgg_para.stDggTPara.fRealShutterT;
	}
	else if (IrPara.st_dgg_para.n_dgg_arith_type == DGG_TYPE_VAR2)
	{
		sDggTPara->fLastLastShutterT = IrPara.st_dgg_para.stDggTPara.fLastLastShutterT;
		sDggTPara->fLastShutterT = IrPara.st_dgg_para.stDggTPara.fLastShutterT;
		sDggTPara->fRealShutterT = IrPara.st_dgg_para.stDggTPara.fRealShutterT;
	}

	return ITA_OK;
}

ITA_RESULT CInfraredCore::getGuoGaiSwitch(bool* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}

	*pVal = IrPara.st_dgg_para.b_dgg_switch;
	return ITA_OK;
}

ITA_RESULT CInfraredCore::putGuoGaiSwitch(bool newVal)
{
	if (IrPara.st_dgg_para.b_dgg_switch == newVal)
	{
		return ITA_ILLEGAL_PAPAM_ERR;
	}
	IrPara.st_dgg_para.b_dgg_switch = newVal;
	return ITA_OK;
}

ITA_RESULT CInfraredCore::putCalGuoGaiSwitch(bool newVal)
{
	if (IrPara.st_dgg_para.b_cgm_switch == newVal)
	{
		return ITA_ILLEGAL_PAPAM_ERR;
	}
	IrPara.st_dgg_para.b_cgm_switch = newVal;

	//计算锅盖模板需关闭去锅盖算法
	if (IrPara.st_dgg_para.b_cgm_switch == 1)
	{
		IrPara.st_dgg_para.b_dgg_switch = 0;
	}
	return ITA_OK;
}

ITA_RESULT CInfraredCore::getCalGuoGaiSwitch(bool* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = IrPara.st_dgg_para.b_cgm_switch;
	return ITA_OK;
}

ITA_RESULT CInfraredCore::getGuoGaiModel(short* nModel, int nWidth, int nHeight)
{
	if (nModel == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}

	if ((nWidth != gWidth) || (nHeight != gHeight))
	{
		return ITA_ERROR;
	}

	memcpy(nModel, g_guogai_model, gWidth * gHeight * sizeof(short));

	return ITA_OK;
}

ITA_RESULT CInfraredCore::putGuoGaiModel(short* nNewModel, int nWidth, int nHeight)
{
	if (nNewModel == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	if ((nWidth != gWidth) || (nHeight != gHeight))
	{
		return ITA_ERROR;
	}

	memcpy(g_guogai_model, nNewModel, gWidth * gHeight * sizeof(short));

	return ITA_OK;
}

long CInfraredCore::get_output_y8_switch(bool* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = b_outdata_y8_switch;
	return ITA_OK;
}

long CInfraredCore::put_output_y8_switch(bool newVal, unsigned char *y8Data)
{
	if(b_outdata_y8_switch != newVal)
		b_outdata_y8_switch = newVal;

	if (newVal == 1)
	{
		//puc_output_y8 = (unsigned char*)porting_calloc_mem(gWidth * gHeight, sizeof(unsigned char), ITA_IMAGE_MODULE);
		//不用再申请内存，使用外面传入的内存，减少一次拷贝和节省内存。
		puc_output_y8 = y8Data;
	}
	else if (newVal == 0)
	{
		//porting_free_mem(puc_output_y8);
		puc_output_y8 = NULL;
	}
	return ITA_OK;
}

long CInfraredCore::get_output_y16_switch(bool* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = b_outdata_y16_switch;
	return ITA_OK;
}

long CInfraredCore::put_output_y16_switch(bool newVal, short *y16Data)
{
	if (b_outdata_y16_switch != newVal)
		b_outdata_y16_switch = newVal;

	if (newVal == 1)
	{
		//pus_output_y16 = (short*)porting_calloc_mem(gWidth * gHeight, sizeof(short), ITA_IMAGE_MODULE);
		//不用再申请内存，使用外面传入的内存，减少一次拷贝和节省内存。
		pus_output_y16 = y16Data;
	}
	else if (newVal == 0)
	{
		//porting_free_mem(pus_output_y16);
		pus_output_y16 = NULL;
	}
	return ITA_OK;
}
//双三次线性插值
void CInfraredCore::imgBigger(short * src, int srcWidth, int srcHeight, short * dst, int dstWidth, int dstHeight)
{
	//矩阵扩展
	//不能频繁申请内存，接口压力测试死机。申请内存时Android上返回不为空的指针，但实际不可用。
	if (m_paddingLen < (srcHeight + 6) * (srcWidth + 6))
	{
		if (m_paddingSrc)
		{
			porting_free_mem(m_paddingSrc);
			m_paddingSrc = NULL;
			m_paddingLen = 0;
		}
	}
	if (!m_paddingSrc)
	{
		m_paddingSrc = (short *)porting_calloc_mem((srcHeight + 6) * (srcWidth + 6), sizeof(short), ITA_IMAGE_MODULE);
		m_paddingLen = (srcHeight + 6) * (srcWidth + 6);
	}
	memset(m_paddingSrc, 0, (srcHeight + 6) * (srcWidth + 6) * sizeof(short));
	padMatrixChar(m_paddingSrc, src, srcWidth, srcHeight, 7, 7);
	memset(dst, 0, dstWidth * dstHeight * sizeof(short));
	//进行双三次线性插值
	for (int hnum = 0; hnum < dstHeight; hnum++)
	{
		for (int wnum = 0; wnum < dstWidth; wnum++)
		{
			double dOriHeight = hnum * srcHeight / dstHeight;
			double dOriWidth = wnum * srcWidth / dstWidth;
			int iOriHeight = (int)dOriHeight + 3;
			int iOriWidth = (int)dOriWidth + 3;
			//计算权重系数
			double wx[4], wy[4];
			getWx(wx, dOriHeight);
			getWy(wy, dOriWidth);
			int DstPoint = hnum * dstWidth + wnum;
			for (int i = 0; i < 4; i++)
			{
				for (int j = 0; j < 4; j++)
				{
					int SrcPoint = (iOriHeight + i - 1) * (srcWidth + 6) + iOriWidth + j - 1;
					dst[DstPoint] += (short)(m_paddingSrc[SrcPoint] * wx[i] * wy[j]);
				}
			}
		}
	}
}

void CInfraredCore::imgSmaller(short* src, int srcWidth, int srcHeight, short* dst, int dstWidth, int dstHeight)
{
	float x_k = float(dstHeight) / float(srcHeight);
	float y_k = float(dstWidth) / float(srcWidth);

	float delta_x = float(1 / x_k);
	float delta_y = float(1 / y_k);
	int i_start = 0;
	int i_end = 0;
	int sum = 0;
	int j_start = 0;
	int j_end = 0;
	int count = 0;
	int i = 0;
	int j = 0;
	int jj = 0;
	int ii = 0;
	for (i = 0; i < dstHeight; i++) {
		i_start = (int)round(i * delta_x);
		if (i_start < 0)
			i_start = 0;
		if (i_start > srcHeight - 1)
			i_start = srcHeight - 1;
		i_end = (int)round((i + 1) * delta_x - 1);
		if (i_end > srcHeight - 1)
			i_end = srcHeight - 1;
		for (j = 0; j < dstWidth; j++) {
			sum = 0;
			j_start = (int)round(j * delta_y);
			if (j_start < 0)
				j_start = 0;
			if (j_start > srcWidth - 1)
				j_start = srcWidth - 1;
			j_end = (int)round((j + 1) * delta_y - 1);
			if (j_end > srcWidth - 1)
				j_end = srcWidth - 1;
			count = (j_end - j_start + 1) * (i_end - i_start + 1);
			for (ii = i_start; ii <= i_end; ii++)
				for (jj = j_start; jj <= j_end; jj++) {
					sum += src[ii * srcWidth + jj];
				}
			dst[i * dstWidth + j] = sum / count;
		}
	}

}

void CInfraredCore::getWx(double wx[4], double x)
{
	//BiCubic基函数
	double a = -0.5;
	//取整
	int X = (int)x;
	double stempx[4];
	stempx[0] = 1 + (x - X);
	stempx[1] = x - X;
	stempx[2] = 1 - (x - X);
	stempx[3] = 2 - (x - X);
	wx[0] = a * fabs(stempx[0] * stempx[0] * stempx[0]) - 5 * a * stempx[0] * stempx[0] + 8 * a * fabs(stempx[0]) - 4 * a;
	wx[1] = (a + 2) * fabs(stempx[1] * stempx[1] * stempx[1]) - (a + 3) * stempx[1] * stempx[1] + 1;
	wx[2] = (a + 2) * fabs(stempx[2] * stempx[2] * stempx[2]) - (a + 3) * stempx[2] * stempx[2] + 1;
	wx[3] = a * fabs(stempx[3] * stempx[3] * stempx[3]) - 5 * a * stempx[3] * stempx[3] + 8 * a * fabs(stempx[3]) - 4 * a;
}

void CInfraredCore::getWy(double wy[4], double y)
{
	//BiCubic基函数
	double a = -0.5;
	//取整
	int Y = (int)y;
	double stempy[4];
	stempy[0] = 1 + (y - Y);
	stempy[1] = y - Y;
	stempy[2] = 1 - (y - Y);
	stempy[3] = 2 - (y - Y);
	wy[0] = a * fabs(stempy[0] * stempy[0] * stempy[0]) - 5 * a * stempy[0] * stempy[0] + 8 * a * fabs(stempy[0]) - 4 * a;
	wy[1] = (a + 2) * fabs(stempy[1] * stempy[1] * stempy[1]) - (a + 3) * stempy[1] * stempy[1] + 1;
	wy[2] = (a + 2) * fabs(stempy[2] * stempy[2] * stempy[2]) - (a + 3) * stempy[2] * stempy[2] + 1;
	wy[3] = a * fabs(stempy[3] * stempy[3] * stempy[3]) - 5 * a * stempy[3] * stempy[3] + 8 * a * fabs(stempy[3]) - 4 * a;
}

void CInfraredCore::padMatrixChar(short * pus_pad_mat, short * pus_mat, int n_width, int n_height, int n_win_size_width, int n_win_size_height)
{
	int i, j;
	int n_pad_num_hor = (n_win_size_width - 1) / 2;
	int n_pad_num_ver = (n_win_size_height - 1) / 2;
	int n_width_new = n_width + 2 * n_pad_num_hor;
	int n_height_new = n_height + 2 * n_pad_num_ver;
	short *pus_mat_tmp = NULL;
	short *pus_pad_mat_tmp = NULL;

	// 对扩展图像进行赋值。padarray
	//初始化赋值
	pus_mat_tmp = pus_mat;
	pus_pad_mat_tmp = pus_pad_mat + n_pad_num_ver*n_width_new + n_pad_num_hor;
	for (i = 0; i<n_height; i++)
	{
		// 复制整行数据
		memcpy(pus_pad_mat_tmp, pus_mat_tmp, n_width * sizeof(short));

		// 复制每行左右两边的数据
		for (j = 1; j <= n_pad_num_hor; j++)
		{
			*(pus_pad_mat_tmp - j) = *pus_pad_mat_tmp;
			*(pus_pad_mat_tmp + n_width - 1 + j) = *(pus_pad_mat_tmp + n_width - 1);
		}

		pus_mat_tmp += n_width;
		pus_pad_mat_tmp += n_width_new;
	}

	// 前几行和最后几行赋值
	pus_mat_tmp = pus_pad_mat + n_width_new * n_pad_num_ver;
	pus_pad_mat_tmp = pus_pad_mat + n_width_new * (n_height_new - n_pad_num_ver - 1);
	for (i = 0; i < n_pad_num_ver; i++)
	{
		memcpy(pus_pad_mat + n_width_new * i, pus_mat_tmp, n_width_new * sizeof(short));
		memcpy(pus_pad_mat + n_width_new * (n_height_new - 1 - i), pus_pad_mat_tmp, n_width_new * sizeof(short));
	}
}

bool CInfraredCore::getGainMat(unsigned short* pus_high_base, unsigned short* pus_low_base, unsigned short* pn_gain_mat,
	int n_width, int n_height)
{
	int i;
	int n_sum_high = 0;
	int n_sum_low = 0;
	int n_size = n_width * n_height;
	unsigned short us_avg_high = 0;
	unsigned short us_avg_low = 0;
	int n_avg_diff = 0;
	int nZoom = 8192;//8192

	// 计算高温本底和低温本底的平均响应值
	for (i = 0; i < n_size; i++)
	{
		n_sum_high += pus_high_base[i];
		n_sum_low += pus_low_base[i];
	}
	us_avg_high = (unsigned short)(n_sum_high / n_size);
	us_avg_low = (unsigned short)(n_sum_low / n_size);
	n_avg_diff = us_avg_high - us_avg_low;

	// 计算增益系数矩阵
	for (i = 0; i < n_size; i++)
	{
		if (pus_high_base[i] > pus_low_base[i]) //注意分母不能为0
		{
			pn_gain_mat[i] = (n_avg_diff * nZoom / (pus_high_base[i] - pus_low_base[i]));
		}
		else
		{
			pn_gain_mat[i] = nZoom;
		}
	}

	//将K矩阵中心点强制转换为8192，其余按相同规则转换
	int iCussor = pn_gain_mat[n_height * n_width / 2 + n_width / 2];

	double dRatio = 1.0;

	if (iCussor == 0)
	{
		return true;
	}
	else
	{
		dRatio = 8192 * 1.0 / iCussor;
		for (i = 0; i < n_size; i++)
		{
			pn_gain_mat[i] = (unsigned short)(pn_gain_mat[i] * dRatio + 0.5);
		}
	}
	return true;
}

void CInfraredCore::setLogger(GuideLog* logger)
{
	m_logger = logger;
}

//gamma校正参数
long CInfraredCore::get_gmc_switch(bool* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = IrPara.st_gmc_para.b_gmc_switch;
	return ITA_OK;
}

long CInfraredCore::put_gmc_switch(bool newVal)
{
	if(IrPara.st_gmc_para.b_gmc_switch == newVal)
		return ITA_INACTIVE_CALL;

	if (newVal == 1 && IrPara.st_gmc_para.b_gmc_switch == 0)
	{
		ImgGetGammaTable(nGammaTable, IrPara.st_gmc_para.f_gmc_gamma);
	}
	IrPara.st_gmc_para.b_gmc_switch = newVal;

	return ITA_OK;
}

long CInfraredCore::get_gmc_gamma(float* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}

	*pVal = IrPara.st_gmc_para.f_gmc_gamma;
	return ITA_OK;
}

long CInfraredCore::put_gmc_gamma(float newVal)
{
	IrPara.st_gmc_para.f_gmc_gamma = newVal;
	ImgGetGammaTable(nGammaTable, IrPara.st_gmc_para.f_gmc_gamma);

	return ITA_OK;
}

long CInfraredCore::get_gmc_gammatype(int* pVal)
{
	if (pVal == NULL)
	{
		return ITA_NULL_PTR_ERR;
	}
	*pVal = IrPara.st_gmc_para.b_gmc_type;
	return ITA_OK;
}

long CInfraredCore::put_gmc_gammatype(int newVal)
{
	if (IrPara.st_gmc_para.b_gmc_type == newVal)
		return ITA_ILLEGAL_PAPAM_ERR;

	IrPara.st_gmc_para.b_gmc_type = newVal;
	//调光类型改变重新生成Gamma表
	ImgGetGammaTable(nGammaTable, IrPara.st_gmc_para.f_gmc_gamma);
	return ITA_OK;
}
