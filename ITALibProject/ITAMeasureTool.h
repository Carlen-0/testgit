#ifndef ITA_MEASURETOOL_H
#define ITA_MEASURETOOL_H

#ifdef __cplusplus
#    define ITA_MEASURETOOL_START  extern "C" {
#    define ITA_MEASURETOOL_END    };
#else
#    define ITA_TOOLBOX_START
#    define ITA_TOOLBOX_END
#endif

ITA_MEASURETOOL_START

#include "ERROR.h"
#include "ITADTD.h"

/**
* @brief:	测温工具类模块初始化。
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
* @note:
**/
ITA_API HANDLE_T ITA_MeasureToolInit();

/**
* @brief:	释放一个测温工具（ITA）库实例资源。
* @param:	HANDLE_T instance	ITA实例。
* @return:	无。
**/
ITA_API void ITA_MeasureToolUnInit(HANDLE_T instance);

/**
* @brief:	配置文件内容导出,用户自定义保存文件。
* @param:	HANDLE_T instance									ITA工具箱实例。
* @param:	ITAConfig* config									配置文件所需的参数结构体。
* @param:	char* outputEncryStr								输出参数，生成加密的配置文件字符串。内存由用户申请。
* @param:	int encryStrLen										用户申请的生成加密的配置文件字符串的长度。
* @param:	int* length											输出参数，生成加密的配置文件字符长度。
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_ConfigFileExport(HANDLE_T instance, ITAConfig* config,char* outputEncryStr,int encryStrLen,int* length);


/**
* @brief:	配置文件内容导入
* @param:	HANDLE_T instance									ITA工具箱实例。
* @param:	char* inputStr										输入参数，配置文件的字符串内容。
* @param:	int length											输入参数，配置文件的字符串长度。
* @param:	ITAConfig* config								    输出参数，ITAConfig结构体。
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_ConfigFileImport(HANDLE_T instance, char* inputStr, int length, ITAConfig* config);
ITA_MEASURETOOL_END

#endif // !ITA_MEASURETOOL_H
