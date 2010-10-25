#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <linux/kd.h>
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

static struct termios term_attributes_orig;
static int            term_attributes_saved;
static struct termios locked_term_attributes_orig;
static int            locked_term_attributes_saved;

static int _console_input_unbuffered_set(int console_dev)
{
    struct termios term_attributes;
    struct termios locked_term_attributes;

    tcgetattr(console_dev, &term_attributes);
    term_attributes_orig = term_attributes;

    cfmakeraw(&term_attributes);
    /* Make return output new line like canonical mode */
    term_attributes.c_iflag |= ICRNL;
    /* Make \n return go to the beginning of the next line */
    term_attributes.c_oflag |= ONLCR | OPOST;

    if (tcsetattr (console_dev, TCSANOW, &term_attributes) != 0)
        return -1;

    term_attributes_saved = 1;

    if (ioctl(console_dev,
              TIOCGLCKTRMIOS, &locked_term_attributes) == 0) {
        locked_term_attributes_orig = locked_term_attributes;

        memset (&locked_term_attributes, 0xff, sizeof (locked_term_attributes));
        if (ioctl(console_dev,
                  TIOCSLCKTRMIOS, &locked_term_attributes) == 0)
            locked_term_attributes_saved = 1;
        else
            err("Failed to save terminal settings -- %m");
    }

    return 0;
}

static int _console_input_restore(int console_dev)
{
    struct termios term_attributes;

    if (!term_attributes_saved)
        return -1;

    if (locked_term_attributes_saved) {
        if (ioctl(console_dev,
                  TIOCSLCKTRMIOS, &locked_term_attributes_orig) == -1)
            err("Failed to unlock terminal settings -- %m");

        locked_term_attributes_saved = 0;
    }

    if (!(term_attributes_orig.c_lflag & ICANON)) {
        tcgetattr(console_dev, &term_attributes);
        term_attributes.c_iflag |= BRKINT | IGNPAR | ISTRIP | ICRNL | IXON;
        term_attributes.c_oflag |= OPOST;
        term_attributes.c_lflag |= ECHO | ICANON | ISIG | IEXTEN;
    }
    else {
        term_attributes = term_attributes_orig;
    }

    if (tcsetattr(console_dev, TCSAFLUSH, &term_attributes) == -1) {
        err("Failed to restore console attributes");
        return -1;
    }

    term_attributes_saved = 0;

    return 0;
}

static const char *tty_path = "/dev/tty0";

/**
 * Let console in graphics mode, effectivelly disabling fbcon. This is
 * accomplished by calling ioctl(fd, KDSETMODE, KD_GRAPHICS)
 *
 * @param ds_fb the framebuffer identifier in dietsplash
 */
int ds_console_setup(void)
{
    int console_dev;

    console_dev = open(tty_path, O_WRONLY);
    if (console_dev < 0)
        goto return_on_err;

    if (_console_input_unbuffered_set(console_dev) < 0)
        goto close_on_err;

    if (ioctl(console_dev, KDSETMODE, KD_GRAPHICS) == -1)
        goto close_on_err;

    close(console_dev);
    return 0;

close_on_err:
    close(console_dev);

return_on_err:
    err("Failed to go in graphics mode -- %m");
    return -1;
}

int ds_console_restore(void)
{
    int console_dev;

    console_dev = open(tty_path, O_WRONLY);
    if (console_dev < 0)
        goto return_on_err;

    if (ioctl(console_dev, KDSETMODE, KD_TEXT) == -1)
        goto close_on_err;

    if (_console_input_restore(console_dev))
        goto close_on_err;

    close(console_dev);
    return 0;

close_on_err:
    close(console_dev);

return_on_err:
    err("Failed to return to text mode -- %m");
    return -1;
}
