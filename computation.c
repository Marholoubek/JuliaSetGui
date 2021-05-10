
#include "utils.h"
#include "computation.h"

static struct {
    double c_re;
    double c_im;
    int n;

    double range_re_min;
    double range_re_max;
    double range_im_min;
    double range_im_max;

    int grid_w;
    int grid_h;

    int cur_x;
    int cur_y;

    double d_re;
    double d_im;

    int nbr_chunks;
    int cid;
    double chunk_re;
    double chunk_im;

    uint8_t chunk_n_re;
    uint8_t chunk_n_im;

    uint8_t *grid;
    bool computing;
    bool done;
    bool abort;
    bool is_set;


} comp = {
        .c_re = -0.4,
        .c_im = 0.6,
        .n = 60,
        .range_re_min = -1.6,
        .range_re_max = 1.6,
        .range_im_min = -1.1,
        .range_im_max = 1.1,

        .grid_w = 640,
        .grid_h = 480,


        .chunk_n_re = 64,
        .chunk_n_im = 48,

        .grid = NULL,
        .computing = false,
        .done = false,
        .abort = false,
        .is_set = false,

};

void computation_init(void){

    comp.grid = my_alloc(comp.grid_w * comp.grid_h);
    // comp.grid = calloc(comp.grid_w * comp.grid_h, comp.grid_w * comp.grid_h); // TODO Validate
    comp.d_re = (comp.range_re_max - comp.range_re_min) / (1. * comp.grid_w);
    comp.d_im = -(comp.range_re_max - comp.range_re_min) / (1. * comp.grid_h);
    comp.nbr_chunks = (comp.grid_w * comp.grid_h) /  (comp.chunk_n_re * comp.chunk_n_im);

}
void computation_cleanup(void){
    if (comp.grid) free(comp.grid);
    comp.grid = NULL;
}

bool reset_chunk(void){
    if (!is_abort()) {
        comp.cid = 0;
        comp.computing = true;
        comp.cur_x = comp.cur_y = 0;
        comp.chunk_re = comp.range_re_min;
        comp.chunk_im = comp.range_im_max;
    }
    return is_abort();
}

void move_back_one_chunk(void){
    comp.cid--;
}


bool is_computing(void) {return comp.computing;}
bool is_done(void) {return comp.done;}
bool is_abort(void) {return comp.abort;}
bool is_set(void){ return comp.is_set;}

void abort_comp(void){comp.abort = true;}
void enable_comp(void){comp.abort = false;}

bool set_compute(message *msg){
    my_assert(msg != NULL, __func__ ,__LINE__, __FILE__);
    bool ret = !is_computing();

    if (ret){
        msg->type = MSG_SET_COMPUTE;
        msg->data.set_compute.c_re = comp.c_re;
        msg->data.set_compute.c_im = comp.c_im;
        msg->data.set_compute.d_re = comp.d_re;
        msg->data.set_compute.d_im = comp.d_im;
        msg->data.set_compute.n = comp.n;
        comp.done = false;
        comp.is_set = true;
        printf("Computations parameters: w:%d, h:%d, chunks: %d\n", comp.grid_w, comp.grid_h, comp.nbr_chunks);

    }

    return ret;
}


bool compute(message *msg){
    my_assert(msg != NULL, __func__ ,__LINE__, __FILE__);

    if (!is_computing()) {
        comp.cid = 0;
        comp.computing = true;
        comp.done = false;
        comp.cur_x = comp.cur_y = 0;
        comp.chunk_re = comp.range_re_min;
        comp.chunk_im = comp.range_im_max;
        msg->type = MSG_COMPUTE;
    } else {
        comp.cid += 1;
        if (comp.cid < comp.nbr_chunks){
            comp.cur_x += comp.chunk_n_re;
            comp.chunk_re += comp.chunk_n_re * comp.d_re;
            if (comp.cur_x >= comp.grid_w) {
                comp.cur_x = 0;
                comp.cur_y += comp.chunk_n_im;
                comp.chunk_re = comp.range_re_min;
                comp.chunk_im += comp.chunk_n_im * comp.d_im;

            }
            msg->type = MSG_COMPUTE;
        } else {
            comp.computing = false;
            comp.done = true;
            comp.abort = false;
            debug("Yes byby I was here");
        }
    }

    if (comp.computing && msg->type == MSG_COMPUTE){
        msg->data.compute.re = comp.chunk_re;
        msg->data.compute.im = comp.chunk_im;
        msg->data.compute.cid = comp.cid;
        msg->data.compute.n_re = comp.chunk_n_re;
        msg->data.compute.n_im = comp.chunk_n_im;
    }

    return is_computing();
}

void update_data(const msg_compute_data *compute_data){
    // my assert
    if (compute_data->cid == comp.cid){
        const int idx = comp.cur_x + compute_data->i_re + (comp.cur_y + compute_data->i_im) * comp.grid_w;
        if (idx >= 0 && idx < (comp.grid_w * comp.grid_h)){
            comp.grid[idx] = compute_data->iter;
        }
        if ((comp.cid + 1) >= comp.nbr_chunks && (compute_data->i_re + 1) == comp.chunk_n_re && (compute_data->i_im + 1) == comp.chunk_n_im){
            comp.done = true;
            comp.computing = false;
            debug("I was here to set computing flase");
        }
    } else {
        error("Received chunk with unexpected chunk id");
    }
}


void get_grid_size(int *w, int *h){
    *w = comp.grid_w;
    *h = comp.grid_h;
}

void update_image(int w, int h, unsigned char *img) {
    my_assert(img && comp.grid && w == comp.grid_w && h == comp.grid_h, __func__, __LINE__, __FILE__);
    for (int i = 0; i < w * h; ++i) {
        const double t = 1. * comp.grid[i] / (comp.n + 1.0);
//        *(img++) = 9 * (1-t)*t*t*t * 255;
//        *(img++) = 15 * (1-t)*(1-t)*t*t*255;
//        *(img++) = 8.5 *(1-t)*(1-t)*(1-t)*t*255;
        if (t < comp.n) {
            *(img++) = (int) (9 * (1 - t) * t * t * t * 255);
            *(img++) = (int) (15 * (1 - t) * (1 - t) * t * t * 255);
            *(img++) = (int) (8.5 * (1 - t) * (1 - t) * (1 - t) * t * 255);
        } else {
            for (int j = 0; j < 3; ++j) {
                *(img++) = 0;
            }
        }
    }
}
// - function -----------------------------------------------------------------
void redraw(int w, int h, unsigned char *img)
{
    uint8_t *grid = comp.grid;
    int threshold = comp.n;
    int nsize = w * h;
    unsigned char *cur = img;
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

void buffer_cleanup(void){
    computation_cleanup();
    computation_init();
    int w = comp.grid_w;
    int h = comp.grid_h;
    int i = 0;
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            comp.grid[i++] = 0;
        }
    }
}

void my_compute(void){
    /*complex double z, c;
    int x, y, i;
    int w = comp.grid_w;
    int h = comp.grid_h;
    int n = comp.n;
    c = comp.c_re + comp.c_im * I;
    int re_range = (-1 * comp.range_re_min) + comp.range_re_max;
    int im_range = (-1 * comp.range_im_min) + comp.range_im_max;


    for (y = 0; y < h; ++y) {
        for (x = 0; x < w; ++x) {
            z = (comp.range_re_min + x * (re_range / w)) + (comp.range_im_min + y * (im_range / h)) * I;
            i = 0;
            while (cabs(z) < 2 && ++i < n)
                z = z * z + c;
            comp.grid[y * h + x] = i;
        }
    }*/


    complex double z, c;
    int x, y, i;
    int w = comp.grid_w;
    int h = comp.grid_h;
    int n = comp.n;
    c = comp.c_re + comp.c_im * I;
    double range_re_min = comp.range_re_min;
    double range_im_min = comp.range_im_min;
    double range_re = -range_re_min + comp.range_re_max;
    double range_im = -range_im_min + comp.range_im_max;

    for (y = 0; y < h; ++y) {
        for (x = 0; x < w; ++x) {
            z = (range_re_min + x * (range_re / w)) + (range_im_min + y * (range_im / h)) * I;
            i = 0;
            while (cabs(z) < 2 && ++i < n)
                z = z * z + c;
            comp.grid[((h - 1 - y) * w) + x] = i;
        }
    }
}



