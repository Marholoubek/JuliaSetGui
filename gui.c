#include <SDL.h>

#include "xwin_sdl.h"
#include "utils.h"
#include "computation.h"
#include "event_queue.h"
#include "gui.h"

#define SDL_EVENT_WAIT_MS 100

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

void gui_buffer_cleanup(void){
    if (gui.img){
        free(gui.img);
        gui.img = my_calloc(gui.w * gui.h * 3);
        gui_refresh();
    }
}


void gui_refresh(void){
    if (gui.img){
        update_image(gui.w, gui.h, gui.img);
        xwin_redraw(gui.w, gui.h, gui.img);
    }
}



void *gui_win_thread(void* arg){
    bool quit = false;
    SDL_Event sdl_event;
    event ev;
    while (!quit){
        ev.type = EV_TYPE_NUM;
        if (SDL_PollEvent(&sdl_event)){
            if (sdl_event.type == SDL_KEYDOWN){
                switch (sdl_event.key.keysym.sym) {
                    case SDLK_q:
                        ev.type = EV_QUIT; // TODO remove reading pipe
                        break;
                    case SDLK_s:
                        ev.type = EV_SET_COMPUTE;
                        break;
                    case SDLK_a:
                        ev.type = EV_ABORT;
                        break;
                    case SDLK_c:
                        ev.type = EV_COMPUTE;
                        break;

                }
            } else if (sdl_event.type == SDL_KEYUP){

            } else{

            }

        }
        if (ev.type != EV_TYPE_NUM){
            queue_push(ev);
        }
        SDL_Delay(SDL_EVENT_WAIT_MS);


        quit = is_quit();
    }
    return NULL;
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