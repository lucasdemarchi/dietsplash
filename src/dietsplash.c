#include "fb.h"
#include "log.h"

#include <stdbool.h>

struct ds_info {
    struct ds_fb fb;
    bool testing;
};
static struct ds_info ds_info;


int main(int argc, char *argv[])
{
    ds_log_init(argv[0]);

    if (ds_fb_init(&ds_info.fb))
        return 1;

    ds_fb_draw_logo(&ds_info.fb);

    ds_fb_shutdown(&ds_info.fb);
    ds_log_shutdown();

    return 0;
}
