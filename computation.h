

#ifndef __COMPUTATION_H__
#define __COMPUTATION_H__

#include <stdbool.h>
#include <stdio.h>
#include <complex.h>

#include "messages.h"


void computation_init(void);
void computation_cleanup(void);

void get_grid_size(int *w, int *h);

bool is_computing(void);
bool is_done(void);

void abort_comp(void);
void enable_comp(void);
bool is_abort(void);
bool is_set(void);

bool set_compute(message *msg);

bool compute(message *msg);

void update_image(int w, int h, unsigned char *img);

void update_data(const msg_compute_data *compute_data);

void move_chunk_back(void);

bool reset_chunk(void);

void buffer_cleanup(void);

void my_compute(void);

void redraw(int w, int h, unsigned char *img);

void set_parameters(double c_re, double c_im, double r_re_min, double r_im_min, double r_re_max, double r_im_max);




#endif