#ifndef __DIETSPLASH_UTIL_H
#define __DIETSPLASH_UTIL_H

#define DIE_PREFIX "[" PACKAGE_NAME "] ERR: "
#define LOG_SUFFIX "\n"

void _die(const char *fmt, ...)  __attribute__((noreturn)) __attribute__((format (printf, 1, 2)));
int ds_fs_setup(const char *dev);
int ds_fs_shutdown(void);

#define die(x, ...) \
    _die(DIE_PREFIX x LOG_SUFFIX, ## __VA_ARGS__)

#endif
