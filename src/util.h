#define DIE_PREFIX "[" PACKAGE_NAME "] ERR: "
#define LOG_SUFFIX "\n"

void _die(const char *fmt, ...)  __attribute__((noreturn)) __attribute__((format (printf, 1, 2)));

#define die(x, ...) \
    _die(DIE_PREFIX x LOG_SUFFIX, ## __VA_ARGS__)
