
#include "utils.h"


void my_assert(bool r, const char *fcname, int line, const char *fname){
    if (!r) {
        fprintf(stderr, "ERROR: my_assert FAIL: %s() line %d in %s\n", fcname, line, fname);
        exit(105);
    }
}

void* my_alloc(size_t size){
    void *ret = malloc(size); // TODO validate

    return ret;
}

void* my_calloc(size_t size){
    void *ret = calloc(size, size); // TODO validate

    return ret;
}

// - function -----------------------------------------------------------------
void call_termios(int reset)
{
    static struct termios tio, tioOld;
    tcgetattr(STDIN_FILENO, &tio);
    if (reset)
    {
        tcsetattr(STDIN_FILENO, TCSANOW, &tioOld);
    }
    else
    {
        tioOld = tio; //backup
        cfmakeraw(&tio);
        tio.c_oflag |= OPOST;
        tcsetattr(STDIN_FILENO, TCSANOW, &tio);
    }
}


void info(const char *str){
    fprintf(stderr, "INFO %s\n", str);
}
void debug(const char *str){
    fprintf(stderr, "DEBUG %s\n", str);
}

void error(const char *str){
    fprintf(stderr, "ERROR %s\n", str);
}
void warn(const char *str){
    fprintf(stderr, "WARN %s\n", str);
}

