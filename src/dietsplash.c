#include "fb.h"
#include "log.h"

#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#ifndef REAL_INIT
# define REAL_INIT "/bin/systemd"
#endif

struct ds_info {
    struct ds_fb fb;
    bool testing;
};
static struct ds_info ds_info;


int main(int argc, char *argv[])
{
    pid_t pid;
    ds_log_init(argv[0]);

    ds_info.testing = (getpid() != 1);

    inf("running mode -- %s", ds_info.testing ? "testing":"system");

    if (!ds_info.testing) {
        pid = fork();
        if (pid)
            execv(REAL_INIT, argv);
    }

    if (ds_fb_init(&ds_info.fb))
        return 1;

    ds_fb_draw_logo(&ds_info.fb);

    ds_fb_shutdown(&ds_info.fb);
    ds_log_shutdown();

    return 0;
}
