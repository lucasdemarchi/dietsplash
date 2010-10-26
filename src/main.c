#include "fb.h"
#include "log.h"
#include "util.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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
        if (pid) {
            char *argv0 = argv[0];
            /* first, change our argv[0], then exec */
            argv[0] = basename(REAL_INIT);
            execv(REAL_INIT, argv);

            argv[0] = argv0;
            err("Failed to exec %s", REAL_INIT);
            exit(1);
        }
    }

    if (ds_fb_init(&ds_info.fb))
        return 1;

    ds_console_setup();
    ds_fb_draw_logo(&ds_info.fb);

    if (ds_info.testing) {
        sleep(5);
        ds_console_restore();
    }

    ds_fb_shutdown(&ds_info.fb);
    ds_log_shutdown();

    return 0;
}
