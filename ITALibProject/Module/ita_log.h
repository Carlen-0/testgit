/*************************************************************
Copyright (C), 2021--2022, Wuhan Guide Sensmart Tech Co., Ltd
File name   : PortingAPI.h
Author      : Guide Sensmart RD. wangyan
Version     : 1.0
Date        : 2021/11/9
Description : Porting API definition.
*************************************************************/

#ifndef ITA_ITA_LOG_H
#define ITA_ITA_LOG_H

#include <string.h>

//
// Example Usage: 
// #include "ita_log.h"
// int main()
// {
//     ita_logger_t *logger = ita_logger_create();
//     // 1: ITA_LOG_LEVEL_VERBOSE
//     // 2: ITA_LOG_LEVEL_DEBUG
//     // 3: ITA_LOG_LEVEL_INFO
//     // 4: ITA_LOG_LEVEL_WARN
//     // 5: ITA_LOG_LEVEL_ERROR
//     // 6: ITA_LOG_LEVEL_FATAL
//     ita_logger_set_level(logger, 0);
//     ita_logger_set_max_filecount(logger, 10); // max 10 log files
//     ita_logger_set_max_filesize(logger, (1<<24)); // max size of a log file is 16MB
//     ita_logger_set_file(logger, "example.log");
//
//     LOGV(logger, "this is log line: %s", "a string");
//     LOGD(logger, "this is log line: %s", "DEBUG");
//     LOGI(logger, "this is log line: %s", "INFO");
//     LOGW(logger, "this is log line: %s", "WARN");
//     LOGE(logger, "this is log line: %s", "ERROR");
//     LOGF(logger, "this is log line: %s", "FATAL");
//
//     LOGV_IF(logger, true, "conditional log line: %s", "a string");
//
//     ita_logger_destroy(logger);
//     return 0;
// }
//


#ifdef _WIN32
#define DIR_SEPARATOR     '\\'
#define DIR_SEPARATOR_STR "\\"
#else
#define DIR_SEPARATOR      '/'
#define DIR_SEPARATOR_STR  "/"
#endif


#ifndef __FILENAME__
#define __FILENAME__ (strrchr(DIR_SEPARATOR_STR __FILE__, DIR_SEPARATOR) + 1)
#endif

#ifdef __cplusplus
extern "C" {
#endif


#ifdef ITA_USE_DBUG
#define DEBUG_STR "DBUG"
#else
#define DEBUG_STR "DEBUG"
#endif

#define LOG_LEVEL_MAP(XXX) \
    XXX(LOG_LEVEL_VERBOSE,  "VERBOSE",   "V")     \
    XXX(LOG_LEVEL_DEBUG,    DEBUG_STR,   "D")     \
    XXX(LOG_LEVEL_INFO,     "INFO",      "I")     \
    XXX(LOG_LEVEL_WARN,     "WARN",      "W")     \
    XXX(LOG_LEVEL_ERROR,    "ERROR",     "E")     \
    XXX(LOG_LEVEL_FATAL,    "FATAL",     "F")


typedef enum { // NOLINT(modernize-use-using)
    // ITA_LOG_LEVEL_TRACE = 0,
#define XXX(id, str, c) ITA_##id,
    LOG_LEVEL_MAP(XXX)
#undef XXX
    ITA_LOG_LEVEL_SILENT

} ITA_LOG_LEVEL_E;


#define DEFAULT_LOG_FILE   "ita"

#ifndef DEFAULT_LOG_LEVEL
#define DEFAULT_LOG_LEVEL  ITA_LOG_LEVEL_INFO
#endif

#ifndef DEFAULT_LOG_MIN_BUFSIZE
#define DEFAULT_LOG_MIN_BUFSIZE  (1<<4) // 16 
#endif

#ifndef DEFAULT_LOG_MAX_BUFSIZE
#define DEFAULT_LOG_MAX_BUFSIZE  (1<<9) // 512
#endif

#ifndef DEFAULT_LOG_MIN_FILESIZE
#define DEFAULT_LOG_MIN_FILESIZE (1<<10) // 1K(1024)
#endif

#ifndef DEFAULT_LOG_MAX_FILESIZE
#define DEFAULT_LOG_MAX_FILESIZE (1<<24) //  16M(16777216)
#endif

#ifndef DEFAULT_LOG_MIN_FILECOUNT
#define DEFAULT_LOG_MIN_FILECOUNT (1)
#endif

#ifndef DEFAULT_LOG_MAX_FILECOUNT
#define DEFAULT_LOG_MAX_FILECOUNT (5)
#endif

typedef void (*ita_logger_handler_fn)(int loglvl, const char *buf, int len);

typedef struct ita_logger_s ita_logger_t;
// logger
ita_logger_t *ita_logger_create();
void ita_logger_destroy(ita_logger_t *logger);
void ita_logger_set_handler(ita_logger_t *logger, ita_logger_handler_fn fn);
void ita_logger_set_level(ita_logger_t *logger, int level);
void ita_logger_set_max_bufsize(ita_logger_t *logger, unsigned int bufsize);
int ita_logger_print(ita_logger_t *logger, int level, const char *fmt, ...);

// sink
void ita_logger_set_file(ita_logger_t *logger, const char *filepath);
void ita_logger_set_max_filesize(ita_logger_t *logger, size_t filesize);
void ita_logger_set_max_filecount(ita_logger_t *logger, unsigned int filecount);
// level
const char *ita_logger_levelstr(int level);
void ita_logger_enable_flush(ita_logger_t *logger, int on);

// #define LOG_VERBOSE(logger, fmt, ...) ita_logger_print((logger), ITA_LOG_LEVEL_VERBOSE, fmt, ##__VA_ARGS__)
// #define LOG_DEBUG(logger, fmt, ...)   ita_logger_print((logger), ITA_LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__)
// #define LOG_INFO(logger, fmt, ...)    ita_logger_print((logger), ITA_LOG_LEVEL_INFO, fmt, ##__VA_ARGS__)
// #define LOG_WARN(logger, fmt, ...)    ita_logger_print((logger), ITA_LOG_LEVEL_WARN, fmt, ##__VA_ARGS__)
// #define LOG_ERROR(logger, fmt, ...)   ita_logger_print((logger), ITA_LOG_LEVEL_ERROR, fmt, ##__VA_ARGS__)
// #define LOG_FATAL(logger, fmt, ...)   ita_logger_print((logger), ITA_LOG_LEVEL_FATAL, fmt, ##__VA_ARGS__)

// #define LOGV LOG_VERBOSE
// #define LOGD LOG_DEBUG
// #define LOGI LOG_INFO
// #define LOGW LOG_WARN
// #define LOGE LOG_ERROR
// #define LOGF LOG_FATAL

// #define LOGV(logger, fmt, ...) ita_logger_print((logger), ITA_LOG_LEVEL_VERBOSE, fmt, ##__VA_ARGS__)
// #define LOGD(logger, fmt, ...) ita_logger_print((logger), ITA_LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__)
// #define LOGI(logger, fmt, ...) ita_logger_print((logger), ITA_LOG_LEVEL_INFO, fmt, ##__VA_ARGS__)
// #define LOGW(logger, fmt, ...) ita_logger_print((logger), ITA_LOG_LEVEL_WARN, fmt, ##__VA_ARGS__)
// #define LOGE(logger, fmt, ...) ita_logger_print((logger), ITA_LOG_LEVEL_ERROR, fmt, ##__VA_ARGS__)
// #define LOGF(logger, fmt, ...) ita_logger_print((logger), ITA_LOG_LEVEL_FATAL, fmt, ##__VA_ARGS__)

// #define LOGV_IF(logger, bool_expression, fmt, ...) \
//     if (!(bool_expression)) {} else ita_logger_print((logger), ITA_LOG_LEVEL_VERBOSE, fmt, ##__VA_ARGS__)
// 
// #define LOGD_IF(logger, bool_expression, fmt, ...) \
//     if (!(bool_expression)) {} else ita_logger_print((logger), ITA_LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__)
// 
// #define LOGI_IF(logger, bool_expression, fmt, ...) \
//     if (!(bool_expression)) {} else ita_logger_print((logger), ITA_LOG_LEVEL_INFO, fmt, ##__VA_ARGS__)
// 
// #define LOGW_IF(logger, bool_expression, fmt, ...) \
//     if (!(bool_expression)) {} else ita_logger_print((logger), ITA_LOG_LEVEL_WARN, fmt, ##__VA_ARGS__)
// 
// #define LOGE_IF(logger, bool_expression, fmt, ...) \
//     if (!(bool_expression)) {} else ita_logger_print((logger), ITA_LOG_LEVEL_ERROR, fmt, ##__VA_ARGS__)
// 
// #define LOGF_IF(logger, bool_expression, fmt, ...) \
//     if (!(bool_expression)) {} else ita_logger_print((logger), ITA_LOG_LEVEL_FATAL, fmt, ##__VA_ARGS__)





#ifdef __cplusplus
}
#endif



#endif // ITA_ITA_LOG_H
