#ifndef __DIETSPLASH_LOG_H_
#define __DIETSPLASH_LOG_H_

#include <stdarg.h>

void ds_log_init(const char *argv0);
void ds_log_shutdown(void);
void ds_log(int level, const char *file, int line, const char *func, const char *fmt, ...);
int ds_log_get_current_level(void);

#define LOG_CRITICAL  0
#define LOG_ERROR     1
#define LOG_WARNING   2
#define LOG_INFO      3
#define LOG_DEBUG     4

#define crit(fmt, ...) ds_log(LOG_CRITICAL, __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)
#define err(fmt, ...) ds_log(LOG_ERROR, __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)
#define wrn(fmt, ...) ds_log(LOG_WARNING, __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)
#define inf(fmt, ...) ds_log(LOG_INFO, __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)
#define dbg(fmt, ...) ds_log(LOG_DEBUG, __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)

#endif
