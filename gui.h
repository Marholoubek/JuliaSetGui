

#ifndef __GUI_H__
#define __GUI_H__

void gui_init(void);

void gui_cleanup(void);

void gui_refresh(void);

// assigns color to calculated fractals
void redraw(int w, int h, uint8_t *grid, uint8_t threshold, unsigned char *out);








#endif