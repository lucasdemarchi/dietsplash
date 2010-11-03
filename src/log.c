#include "log.h"
#include "util.h"

#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>

static const char *_log_level_names[] = {
    "CRITICAL",
    "ERROR",
    "WARNING",
    "INFO",
    "DEBUG"
};

static int _log_level = LOGLEVEL;
static const char *_prog;
static pid_t _pid;

static void _logv(int level, const char *file, int line, const char *func, const char *fmt, va_list ap)
{
    const char *level_name;

    if (level > _log_level)
        return;

    if (level >= 0 && (size_t)level < ARRAY_SIZE(_log_level_names))
        level_name = _log_level_names[level];
    else
        level_name = NULL;

#ifdef DEBUG
    if (level_name)
        fprintf(stderr, "%s[%d] %s: %s:%d %s() ",
                _prog, _pid, level_name, file, line, func);
    else
        fprintf(stderr, "%s[%d] %03d: %s:%d %s() ",
                _prog, _pid, level, file, line, func);
#else
    if (level_name)
        fprintf(stderr, "%s[%d] %s: ", _prog, _pid, level_name);
    else
        fprintf(stderr, "%s[%d] %03d ", _prog, _pid, level);
    (void)file;
    (void)line;
    (void)func;
#endif

    vfprintf(stderr, fmt, ap);
    putc('\n', stderr);
}

int ds_log_get_current_level(void)
{
    return _log_level;
}

void ds_log(int level, const char *file, int line, const char *func, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    _logv(level, file, line, func, fmt, ap);
    va_end(ap);
}

void ds_log_init(const char *argv0)
{
    _prog = basename(argv0);
    _pid = getpid();
    inf("paints on");
}

void ds_log_shutdown(void)
{
    inf("paints off");
    _pid = 0;
    _prog = NULL;
}
