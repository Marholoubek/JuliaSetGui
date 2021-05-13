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

void gui_init(void){ // Allocating memory for image
    get_grid_size(&gui.w, &gui.h);
    gui.img = my_alloc(gui.w * gui.h * 3);
    my_assert(xwin_init(gui.w, gui.h) == 0, __func__, __LINE__, __FILE__);
}

void gui_cleanup(void){ // Cleaning the allocated memory
    if (gui.img){
        free(gui.img);
        gui.img = NULL;
    }
    xwin_close(); // Closing the window
}

void gui_refresh(void){
    if (gui.img){
        redraw(gui.w, gui.h, gui.img);
        xwin_redraw(gui.w, gui.h, gui.img);
    }
}

/* Reading user keyboard input from window, and sending the events to main thread */
void *gui_win_thread(void* arg){
    bool quit = false;
    SDL_Event sdl_event;
    event ev;
    while (!quit){
        ev.type = EV_TYPE_NUM;
        if (SDL_PollEvent(&sdl_event)){
            if (sdl_event.type == SDL_KEYDOWN){
                switch (sdl_event.key.keysym.sym) {
                    case SDLK_s:
                        ev.type = EV_SET_COMPUTE;
                        break;
                    case SDLK_a:
                        ev.type = EV_ABORT;
                        break;
                    case SDLK_c:
                        ev.type = EV_COMPUTE_CPU;
                        break;
                    case SDLK_1:
                        ev.type = EV_COMPUTE;
                        break;
                    case SDLK_g:
                        ev.type = EV_GET_VERSION;
                        break;
                    case SDLK_r:
                        ev.type = EV_RESET_CHUNK;
                        break;
                    case SDLK_p:
                        ev.type = EV_REFRESH;
                        break;
                    case SDLK_l:
                        ev.type = EV_CLEAR_BUFFER;
                        break;
                    case SDLK_z:
                        ev.type = EV_ZOOM;
                        break;
                    case SDLK_h:
                        ev.type = EV_DECREASE_ZOOM;
                        break;
                    case SDLK_d:
                        ev.type = EV_MOVE_D;
                        break;
                    case SDLK_n:
                        ev.type = EV_MOVE_L;
                        break;
                    case SDLK_m:
                        ev.type = EV_MOVE_R;
                        break;
                    case SDLK_u:
                        ev.type = EV_MOVE_U;
                        break;
                    case SDLK_o:
                        ev.type = EV_MOOD_O;
                        break;
                    case SDLK_2:
                        ev.type = EV_MOOD_2;
                        break;
                    case SDLK_3:
                        ev.type = EV_MOOD_3;
                        break;
                    case SDLK_4:
                        ev.type = EV_MOOD_4;
                        break;
                    case SDLK_5:
                        ev.type = EV_MOOD_5;
                        break;
                    case SDLK_6:
                        ev.type = EV_MOOD_6;
                        break;
                }
            } else if (sdl_event.type == SDL_KEYUP){} else {}
        }
        if (ev.type != EV_TYPE_NUM) queue_push(ev);
        SDL_Delay(SDL_EVENT_WAIT_MS);
        quit = is_quit();
    }
    return NULL;
}



