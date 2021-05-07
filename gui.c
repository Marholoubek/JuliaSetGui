

#include "xwin_sdl.h"
#include "utils.h"
#include "computation.h"
#include "gui.h"

static struct {
    int w;
    int h;
    unsigned char *img;
} gui = {.img = NULL};

void gui_init(void){
    get_grid_size(&gui.w, &gui.h);
    gui.img = my_alloc(gui.w * gui.h * 3);
    my_assert(xwin_init(gui.w, gui.h) == 0, __func__, __LINE__, __FILE__);
}

void gui_cleanup(void){
    if (gui.img){
        free(gui.img);
        gui.img = NULL;
    }
    xwin_close();
}

void gui_refresh(void){
    if (gui.img){
        update_image(gui.w, gui.h, gui.img);
        xwin_redraw(gui.w, gui.h, gui.img);
    }

}


// - function -----------------------------------------------------------------
void redraw(int w, int h, uint8_t *grid, uint8_t threshold, unsigned char *out)
{
    int nsize = w * h;
    unsigned char *cur = out;
    for (int i = 0; i < nsize; ++i)
    {
        const int n = *(grid++);
        const double t = 1. * n / threshold;
        if (t < threshold)
        {
            *(cur++) = (int)(9 * (1 - t) * t * t * t * 255);
            *(cur++) = (int)(15 * (1 - t) * (1 - t) * t * t * 255);
            *(cur++) = (int)(8.5 * (1 - t) * (1 - t) * (1 - t) * t * 255);
        }
        else
        {
            for (int j = 0; j < 3; ++j)
            {
                *(cur++) = 0;
            }
        }
    }
}