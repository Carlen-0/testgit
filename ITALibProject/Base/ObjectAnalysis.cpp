#include "../ITA.h"
#include "Config.h"
#include "ITAWrapper.h"
#include "../MT/PolygonAnalysiser.h"

/**
* @brief:    增加一个分析对象。
* @param:    HANDLE_T handle             ITA实例。
* @param:    ITAShape* shape             一个数组，包含一个到多个多边形对象。
* @param:    int size                    数组的长度。
* @return:   返回值类型：ITA_RESULT。成功，返回ITA_OK。失败，其他
**/
ITA_API ITA_RESULT ITA_AnalysisAdd(HANDLE_T handle, ITAShape *shape, int size)
{
#ifdef ULTIMATE_EDITION
    CHECK_NULL_POINTER(handle);
    CHECK_NULL_POINTER(shape);

    ITAWrapper *p = (ITAWrapper *)handle;

    return p->AnalysisAdd(shape, size);
#else
    return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_AnalysisGetRangeMask(HANDLE_T instance, ITARangeMask* rangeMask, int size)
{
#ifdef ULTIMATE_EDITION
    CHECK_NULL_POINTER(instance);
    CHECK_NULL_POINTER(rangeMask);
    ITAWrapper* p = (ITAWrapper*)instance;

    return p->AnalysisGetRangeMask(rangeMask,size);
#else
    return ITA_UNSUPPORT_OPERATION;
#endif
}


/**
* @brief:    清空分析对象。
* @param:    OA_HANDLE_T handle            ITA实例。
* @return:   返回值类型：ITA_RESULT。成功，返回ITA_OK。失败，其他
**/
ITA_API ITA_RESULT ITA_AnalysisClear(HANDLE_T handle)
{
#ifdef ULTIMATE_EDITION
    CHECK_NULL_POINTER(handle);
    ITAWrapper *p = (ITAWrapper *)handle;

    return p->AnalysisClear();
#else
    return ITA_UNSUPPORT_OPERATION;
#endif
}


/**
* @brief:    对分析对象内部区域的温度做分类处理,获取分析对象的温度信息(最大值,最小值,平均值)。
* @param:    HANDLE_T instance                 ITA实例。
* @param:    short* y16Data                    Y16数据
* @param:	 int width  int height			   Y16图像分辨率。
* @param:	 ITAAnalysisTemp* analysisTemp	   输出参数:分析对象的温度数组，内存由用户申请和释放
* @param:	 int size						   分析对象的温度数组(analysisTemp)的大小(size个数必须与添加的分析对象个数相同)
* @return:   返回值类型：ITA_RESULT。成功，返回ITA_OK。失败，其他
**/
ITA_API ITA_RESULT ITA_AnalysisTemp(HANDLE_T instance, short* y16Data, int width, int height, ITAAnalysisTemp* analysisTemp, int size)
{
#ifdef ULTIMATE_EDITION
    CHECK_NULL_POINTER(instance);
    CHECK_NULL_POINTER(y16Data);
    CHECK_NULL_POINTER(analysisTemp);

    ITAWrapper* p = (ITAWrapper*)instance;

    return p->AnalysisTemp(y16Data, width, height, analysisTemp, size);
#else
    return ITA_UNSUPPORT_OPERATION;
#endif
}

/**
* @brief:    对分析对象内部区域的Y16做分类处理,获取分析对象的Y16信息(最大值,最小值,平均值)。
* @param:    HANDLE_T instance							ITA实例。
* @param:    short* y16Data								Y16数据。
* @param:	 int width  int height						Y16图像分辨率。
* @param:    ITAAnalysisY16Info* analysisY16Info        输出参数:处理后的分析对象Y16数组,包含每个分析对象内最大Y16,最小Y16以及平均Y16,内存有用户申请释放
* @param:	 int size									分析对象的Y16信息数组(analysisY16Info)的大小(size个数必须与添加的分析对象个数相同)
* @return:   返回值类型：ITA_RESULT。成功，返回ITA_OK。失败，其他
**/
ITA_API ITA_RESULT ITA_AnalysisY16Info(HANDLE_T instance, short* y16Data, int width, int height, ITAAnalysisY16Info* analysisY16Info, int size)
{
#ifdef ULTIMATE_EDITION
    CHECK_NULL_POINTER(instance);
    CHECK_NULL_POINTER(y16Data);
    CHECK_NULL_POINTER(analysisY16Info);

    ITAWrapper* p = (ITAWrapper*)instance;

    return p->AnalysisY16(y16Data, width, height, analysisY16Info, size);
#else
    return ITA_UNSUPPORT_OPERATION;
#endif
}

/**
* @brief:    判断一个点是否在一个分析对象内部。
* @param:    ITAShape* shape             一个数组，包含一个多边形对象的多个顶点。
* @param:    ITAPoint point              一个点坐标。
* @param:    int *flag              	 0：表示点不在分析对象内。  1：表示点在分析对象内。
* @return:   返回值类型：ITA_RESULT。成功，返回ITA_OK。失败，其他
**/
ITA_API ITA_RESULT ITA_PointInPolygon(ITAShape* shape, ITAPoint point, int* flag)
{
#ifdef ULTIMATE_EDITION
    CHECK_NULL_POINTER(shape);
    CHECK_NULL_POINTER(flag);


    ITA_RESULT ret = ITA_OK;

    PolygonAnalysiser::PolygonEdges item = PolygonAnalysiser::convert(shape);
    *flag = 0;
    *flag = ITA_PointInPolygon_Impl(item, point);

    return ITA_OK;
#else
    return ITA_UNSUPPORT_OPERATION;
#endif
}

