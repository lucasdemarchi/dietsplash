#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <sys/types.h>

#include "util.h"
#include "log.h"

void _die(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);

    exit(1);
}

static int _devtmpfs_mounted;

/**
 * Check if device is accessible and try mounting devtmpfs on /dev otherwise.
 * This is needed in case the kernel was compiled without DEVTMPFS_MOUNT symbol.
 *
 * @param dev device to check for. Must be a file under /dev.
 *
 * @return 0 in case no mount is needed, 1 in case a posterior unmount should
 * take place or -1 on errors.
 */
int ds_fs_setup(const char *dev)
{
    int ret;
    if (!access(dev, R_OK | W_OK))
        return 0;

    if (_devtmpfs_mounted)
        return -1;

    ret =  mount("devtmpfs", "/dev", "devtmpfs", MS_NOSUID, "mode=755");
    if (ret) {
        err("Failed work-around by mounting /dev -- %m");
        return -1;
    }

    _devtmpfs_mounted++;
    if (!access(dev, R_OK | W_OK))
        return 1;

    err("Failed to find a device %s -- %m", dev);
    return -1;
}

int ds_fs_shutdown(void)
{
#ifdef DEVTMPFS_UMOUNT_REQUIRED
    if (--_devtmpfs_mounted)
        return 0;

    if (umount("/dev")) {
        err("Failed to umount /dev - %m");
        return -1;
    }
#endif

    return 0;
}
