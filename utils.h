#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdbool.h>
#include <stdlib.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h> // for STDIN_FILENO


void my_assert(bool r, const char *fcname, int line, const char *fname);

void call_termios(int reset);
void* my_alloc(size_t size);

void info(const char *str);
void debug(const char *str);
void error(const char *str);
void warn(const char *str);

#endif