#include "ita_log.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//#include "ita_types.h"
#ifndef HAVE_PTHREAD_H
#define HAVE_PTHREAD_H 1
#endif

#ifndef HAVE_PTHREAD_MUTEX_INIT
#define HAVE_PTHREAD_MUTEX_INIT 1
#endif

#ifdef _WIN32
#pragma warning (disable: 4244)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN
#include <Windows.h>
#define ita_mutex_t            CRITICAL_SECTION
#define ita_mutex_init         InitializeCriticalSection
#define ita_mutex_destroy      DeleteCriticalSection
#define ita_mutex_lock         EnterCriticalSection
#define ita_mutex_unlock       LeaveCriticalSection
#define ita_msleep(ms)         Sleep((ms))
#define HAVE_THREAD_MUTEX      1

#else // _WIN32

#if HAVE_PTHREAD_H && HAVE_PTHREAD_MUTEX_INIT
#include <pthread.h>
#define ita_mutex_t            pthread_mutex_t
#define ita_mutex_init(pmtx)   pthread_mutex_init((pmtx), NULL)
#define ita_mutex_destroy      pthread_mutex_destroy
#define ita_mutex_lock         pthread_mutex_lock
#define ita_mutex_unlock       pthread_mutex_unlock
#define HAVE_THREAD_MUTEX      1
#else
#define HAVE_THREAD_MUTEX      0
#endif  // HAVE_PTHREAD && HAVE_PTHREAD_MUTEX_INIT

#include <unistd.h>     // for usleep
// TODO: check exist
#include <sys/time.h>   // for gettimeoffday
#include <sys/stat.h>   // for stat

#define ita_msleep(ms)         usleep(((ms)*1000))

#endif // _WIN32

#ifndef MAX_PATH
#define MAX_PATH 256
#endif

#ifndef ita_log_mem_alloc
#define ita_log_mem_alloc(size) malloc((size))
#endif

#ifndef ita_log_mem_realloc
#define ita_log_mem_realloc(ptr, size) realloc((ptr), (size))
#endif

#ifndef ita_log_mem_free
#define ita_log_mem_free(ptr) free((ptr))
#endif

#ifndef ita_log_memzero
#define ita_log_memzero(ptr, size) memset((ptr), 0, (size))
#endif


#define LOG_FILE_OPEN_MODE          "a"
#define LOG_FILE_OPEN_MODE_TRUNCATE "w"

#define ita_log_file_open     fopen
#define ita_log_file_close    fclose
#define ita_log_file_read     fread
#define ita_log_file_write    fwrite
#define ita_log_file_seek     fseek
#define ita_log_file_tell     ftell
#define ita_log_file_flush    fflush

// #define ita_log_file_remove
// #define ita_log_file_exist

static int ita_log_file_exist(const char *file)
{
#ifdef _WIN32
    return (GetFileAttributesA(file) != INVALID_FILE_ATTRIBUTES) ? 1 : 0;
#else
    struct stat st;
    return (stat(file, &st) == 0) ? 1 : 0;
#endif
}

static size_t ita_log_file_size(FILE *fp)
{
    if (NULL == fp)
    {
        return 0;
    }

    long int cur = ita_log_file_tell(fp);

    ita_log_file_seek(fp, 0, SEEK_END);

    size_t size = ita_log_file_tell(fp);

    ita_log_file_seek(fp, cur, SEEK_SET);

    return size;
}

typedef struct ita_logger_sink_s ita_logger_sink_t;
struct ita_logger_sink_s 
{
    // char     filepath[MAX_PATH];
    char     cur_logfile[MAX_PATH];
    size_t   max_filesize;
    size_t   cur_filesize;
    unsigned max_filecount;
    unsigned cur_filecount;
    unsigned enable_fsync;
    FILE     *fp;
};


struct ita_logger_s {
#if HAVE_THREAD_MUTEX
    ita_mutex_t           mutex;
#endif
    ita_logger_handler_fn handler;
    unsigned int          bufsize;
    char                 *buf;
    int                   level;
    // file sink
    ita_logger_sink_t    *sink;
};

static int ita_logger_sink_init(ita_logger_sink_t *sink)
{
    char filepath[MAX_PATH] = { 0 };
    strncpy(filepath, DEFAULT_LOG_FILE, sizeof(filepath) -1);
    char *suffix = strrchr(filepath, '.');
    if (suffix && strcmp(suffix, ".log") == 0)
    {
        *suffix = '\0';
    }

    snprintf(sink->cur_logfile, sizeof(sink->cur_logfile) - 5, "%s.log", filepath);

    sink->max_filesize  = DEFAULT_LOG_MAX_FILESIZE;
    sink->cur_filesize  = 0;

    sink->max_filecount = DEFAULT_LOG_MAX_FILECOUNT;
    sink->cur_filecount = 0;

    sink->enable_fsync = 0;

    sink->fp = NULL;
    return 0;
}

static void ita_logger_sink_set_file(ita_logger_sink_t *sink, const char *filepath)
{
    if (NULL == sink || NULL == filepath)
    {
        return ;
    }
    if (strlen(filepath) >= MAX_PATH)
    {
        return ;
    }

    char sink_filepath[MAX_PATH] = { 0 };
    strncpy(sink_filepath, filepath, sizeof(sink_filepath) - 1);

    char *suffix = strrchr(sink_filepath, '.');
    if (suffix && strcmp(suffix, ".log") == 0)
    {
        *suffix = '\0';
    }

    // True duplicate file
    if (strncmp(sink_filepath, sink->cur_logfile, strlen(sink_filepath)) == 0)
    {
        return ;
    }

    // release owner, close file
    if (sink->fp)
    {
        ita_log_file_flush(sink->fp);

        ita_log_file_close(sink->fp);
        sink->fp = NULL;
    }

    snprintf(sink->cur_logfile, sizeof(sink->cur_logfile) - 5, "%s.log", sink_filepath);
}

static void ita_logger_sink_set_max_filesize(ita_logger_sink_t *sink, size_t filesize)
{
    if (filesize > DEFAULT_LOG_MIN_FILESIZE)
    {
        sink->max_filesize = filesize;
    }
}

static void ita_logger_sink_set_max_filecount(ita_logger_sink_t *sink, unsigned filecount)
{
    if (filecount > DEFAULT_LOG_MIN_FILECOUNT && filecount <= 999)
    {
        sink->max_filecount = filecount;
    }
}

static void ita_logger_sink_enable_fsync(ita_logger_sink_t *sink, int on)
{
    sink->enable_fsync = on ? 1 : 0;
}

static void ita_logger_sink_destroy(ita_logger_sink_t *sink)
{
    if (sink->fp)
    {
        ita_log_file_close(sink->fp);
        sink->fp = NULL;
    }
}


static ita_logger_sink_t *ita_logger_sink_create()
{
    ita_logger_sink_t *sink = (ita_logger_sink_t *)ita_log_mem_alloc(sizeof(ita_logger_sink_t));
    if (NULL == sink)
    {
        return NULL;
    }

    ita_log_memzero(sink, sizeof(ita_logger_sink_t));

    if (0 != ita_logger_sink_init(sink))
    {
        ita_logger_sink_destroy(sink);

        return NULL;
    }

    return sink;
}


static int log_filename(char out[MAX_PATH], const char in[MAX_PATH], size_t index)
{

    if (0 == index)
    {
        strncpy(out, in, MAX_PATH);

        return 0;
    }

    char tmp[MAX_PATH] = { 0 };
    strncpy(tmp, in, MAX_PATH);

    char * suffix = strrchr(tmp, '.');
    // remove log suffix
    // ita.1.log -> ita.1
    if (suffix && strcmp(suffix, ".log") == 0)
    {
        *suffix = '\0';
    }

    // remove log index
    // ita.10.log -> ita
    char *pos = strrchr(tmp, '.');
    if (pos)
    {
        char *pos1 = ++pos;
        if (pos1 && *pos1 >= '0' && *pos1 <= '9')
        {
            *pos = '\0';
        }
    }

    // 9 == '.' + '999' + '.log' 
    snprintf(out, MAX_PATH - 9, "%s.%ld.log", tmp, index);

    return 0;
}

static int log_move(const char *source, const char *target)
{
    remove(target);

    return rename(source, target) == 0 ? 0 : -1;
}

// Rotate files:
// ita.log   -> ita.1.log
// ita.1.log -> ita.2.log
// ita.2.log -> ita.3.log
// ita.3.log -> delete
//
static int ita_logger_sink_rotate(ita_logger_sink_t *sink)
{
    // release owner, close file
    ita_log_file_close(sink->fp);
    sink->fp = NULL;

    for (size_t i = sink->max_filecount; i > 0; --i)
    {
        char source[MAX_PATH] = { 0 };
        log_filename(source, sink->cur_logfile, i - 1);

        if (!ita_log_file_exist(source))
        {
            continue;
        }

        char target[MAX_PATH] = { 0 };
        log_filename(target, sink->cur_logfile, i);

        if (0 != log_move(source, target))
        {
            ita_msleep(50);

            if (0 != log_move(source, target))
            {
                // truncate, prevent it to grow beyond limit
                FILE *tmp = ita_log_file_open(source, LOG_FILE_OPEN_MODE_TRUNCATE);
                if (tmp)
                {
                    ita_log_file_close(tmp);
                }
                sink->cur_filesize = 0;
            }
        }
    }

    // open log file
    FILE *tmp = ita_log_file_open(sink->cur_logfile, LOG_FILE_OPEN_MODE);
    if (NULL == tmp)
    {
        return -1;
    }

    sink->fp = tmp;
    sink->cur_filesize = ita_log_file_size(sink->fp);

    return 0;
}


static int ita_logger_sink_it(ita_logger_sink_t *sink, const char *buf, unsigned int len)
{
    // first write file
    if (NULL == sink->fp)
    {
        char logfile[MAX_PATH] = { 0 };
        log_filename(logfile, sink->cur_logfile, 0);
        sink->fp = ita_log_file_open(logfile, LOG_FILE_OPEN_MODE);
        // @IMPROVE: to do retry open file
        if (NULL == sink->fp)
        {
            return -1;
        }

        sink->cur_filesize = ita_log_file_size(sink->fp);
    }

    size_t n = len + sink->cur_filesize;

    if (n > sink->max_filesize)
    {
        ita_log_file_flush(sink->fp);
        if (ita_log_file_size(sink->fp) > 0)
        {
            // rotate files
            // ita.log -> ita.1.log
            ita_logger_sink_rotate(sink);
        }
    }

    if (NULL == sink->fp)
    {
        return -2;   
    }

    ita_log_file_write(buf, len, 1, sink->fp);
    sink->cur_filesize += len;

    if (sink->enable_fsync)
    {
        ita_log_file_flush(sink->fp);
    }

    return 0;
}


////////////////////////////////////////////////////////////////////////////////

static int ita_logger_sink(ita_logger_t *logger, const char *buf, unsigned int len)
{
    return ita_logger_sink_it(logger->sink, buf, len);
}


static int ita_logger_init(ita_logger_t *logger)
{
#if HAVE_THREAD_MUTEX
    ita_mutex_init(&logger->mutex);
#endif
    logger->handler = NULL;
    logger->bufsize = DEFAULT_LOG_MAX_BUFSIZE;
    logger->buf     = (char *)ita_log_mem_alloc(logger->bufsize);
    if (NULL == logger->buf)
    {
        return -1;
    }
    ita_log_memzero(logger->buf, logger->bufsize);

    logger->level   = DEFAULT_LOG_LEVEL;

    logger->sink = ita_logger_sink_create();

    if (NULL == logger->sink)
    {
        return -2;
    }

    return 0;
}

ita_logger_t *ita_logger_create()
{
    ita_logger_t *logger = (ita_logger_t *)ita_log_mem_alloc(sizeof(ita_logger_t));
    if (NULL == logger)
    {
        return NULL;
    }
    
    ita_log_memzero(logger, sizeof(ita_logger_t));

    if (0 != ita_logger_init(logger))
    {
        ita_logger_destroy(logger);

        return NULL;
    }

    return logger;
}

void ita_logger_destroy(ita_logger_t *logger)
{
    if (logger)
    {
        if (logger->sink)
        {
            ita_logger_sink_destroy(logger->sink);
            logger->sink = NULL;
        }

        if (logger->buf)
        {
            ita_log_mem_free(logger->buf);
            logger->buf = NULL;
        }
#if HAVE_THREAD_MUTEX
        ita_mutex_destroy(&logger->mutex);
#endif
        ita_log_mem_free(logger);
        logger = NULL;
    }
}

void ita_logger_set_handler(ita_logger_t *logger, ita_logger_handler_fn fn)
{
    if (logger)
    {
        logger->handler = fn;
    }
}


void ita_logger_set_level(ita_logger_t *logger, int level)
{
    if (logger)
    {
        logger->level = level;
    }
}


int ita_logger_print(ita_logger_t *logger, int level, const char *fmt, ...)
{
    if (!logger)
    {
        return -2;
    }

    if (level < logger->level)
    {
        return -1;
    }

    int year, month, day, hour, min, sec, us;
#ifdef _WIN32
    SYSTEMTIME tm;
    GetLocalTime(&tm);
    year     = tm.wYear;
    month    = tm.wMonth;
    day      = tm.wDay;
    hour     = tm.wHour;
    min      = tm.wMinute;
    sec      = tm.wSecond;
    us       = tm.wMilliseconds * 1000;
#else
    struct timeval tv;
    struct tm* tm = NULL;
    gettimeofday(&tv, NULL);
    time_t tt = tv.tv_sec;
    tm = localtime(&tt);
    year     = tm->tm_year + 1900;
    month    = tm->tm_mon  + 1;
    day      = tm->tm_mday;
    hour     = tm->tm_hour;
    min      = tm->tm_min;
    sec      = tm->tm_sec;
    us       = tv.tv_usec;
#endif

//     const char *levelstr= "";
// #define XXX(id, str, s) \
//     case ITA_##id: levelstr = str; break;
//
//     switch (level)
//     {
//         LOG_LEVEL_MAP(XXX)
//     }
// #undef XXX

    const char *levelstr = ita_logger_levelstr(level);

#if HAVE_THREAD_MUTEX
    ita_mutex_lock(&logger->mutex);
#endif
    char *buf            = logger->buf;
    unsigned int bufsize = logger->bufsize;
    int len              = 0;

    // memset(buf, 0, bufsize);

    len += snprintf(buf + len, bufsize - len, 
        "%04d-%02d-%02d %02d:%02d:%02d.%03d %s ",
        year, month, day, hour, min, sec, us/1000,
        levelstr);

    va_list ap;
    va_start(ap, fmt);
    len += vsnprintf(buf + len, bufsize - len, fmt, ap);
    va_end(ap);


    // *(buf + len) = '\n';
    // ++len;

    if (logger->handler)
    {
        logger->handler(level, buf, len);
    }
    else
    {
        ita_logger_sink(logger, buf, len);
    }
#if HAVE_THREAD_MUTEX
    ita_mutex_unlock(&logger->mutex);
#endif
    return len;
}


void ita_logger_set_max_bufsize(ita_logger_t *logger, unsigned int bufsize)
{
    if (bufsize > DEFAULT_LOG_MIN_BUFSIZE)
    {
        if (logger)
        {
#if HAVE_THREAD_MUTEX
            ita_mutex_lock(&logger->mutex);
#endif

            logger->bufsize = bufsize;
            logger->buf     = (char *)ita_log_mem_realloc(logger->buf, bufsize);

#if HAVE_THREAD_MUTEX
            ita_mutex_unlock(&logger->mutex);
#endif
        }
    }
}


void ita_logger_set_file(ita_logger_t *logger, const char *filepath)
{
    if (logger)
    {
#if HAVE_THREAD_MUTEX
        ita_mutex_lock(&logger->mutex);
#endif

        ita_logger_sink_set_file(logger->sink, filepath);

#if HAVE_THREAD_MUTEX
        ita_mutex_unlock(&logger->mutex);
#endif
    }
    
}

void ita_logger_set_max_filesize(ita_logger_t *logger, size_t filesize)
{
    if (logger)
    {
        ita_logger_sink_set_max_filesize(logger->sink, filesize);
    }
}


void ita_logger_set_max_filecount(ita_logger_t *logger, unsigned int filecount)
{
    if (logger)
    {
        ita_logger_sink_set_max_filecount(logger->sink, filecount);
    }
}

const char *ita_logger_levelstr(int level)
{
    const char *levelstr= "";
#define XXX(id, str, s) \
    case ITA_##id: levelstr = s; break;

    switch (level)
    {
        LOG_LEVEL_MAP(XXX)
    }
#undef XXX

    return levelstr;
}

void ita_logger_enable_flush(ita_logger_t *logger, int on)
{
    if (logger)
    {
        ita_logger_sink_enable_fsync(logger->sink, on);
    }
}
