/*
 * File name: prga-hw09-main.c
 * Date:      2017/04/14 18:51
 * Author:    Jan Faigl
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <pthread.h>

#include "xwin_sdl.h"

#include "prg_io_nonblock.h"
#include "messages.h"
#include "event_queue.h"
#include "utils.h"
#include "computation.h"
#include "gui.h"

#define READ_TIMEOUT_MS 10 // Timeout for io_getc_timeout function


typedef struct { // Structure definition for carrying all the important variables
    int in_pipe;
    int out_pipe;
} data_t;






void* input_thread_kb(void*);
void* input_thread_pipe(void*);
void* main_thread(void *arg);
// TODO: add supporting threads functions

void process_pipe_message(event * const ev);

// - main ---------------------------------------------------------------------
int main(int argc, char *argv[]) {
    data_t data = { // Initialization of structure with stat values
            .in_pipe = -1,
            .out_pipe = -1,
    };

    const char *in = argc > 1 ? argv[1] : "/tmp/computational_module.out"; // If pipes are not specified in arguments, program uses the default pipes
    const char *out = argc > 2 ? argv[2] : "/tmp/computational_module.in";

    data.in_pipe = io_open_read(in);
    data.out_pipe = io_open_write(out);

    if (data.in_pipe == -1) { // Pipes opening validation
        fprintf(stderr, "Cannot open named pipe port %s\n", in); // TODO VALIDATE by MyAssert
        exit(100);
    }
    if (data.out_pipe == -1) {
        fprintf(stderr, "Cannot open named pipe port %s\n", out);
        exit(100);
    }


	call_termios(0);





    enum { INPUT_KB, INPUT_PIPE, MAIN_THREAD, WIN_THREAD, NUM_THREADS }; // Creating an array of threads
    const char *thread_names[] = { "keyboard_thread", "pipe_input_thread", "main_thread", "gui_win_thread"};
    void* (*thr_functions[])(void*) = { input_thread_kb, input_thread_pipe, main_thread, gui_win_thread };
    pthread_t threads[NUM_THREADS];



    for(int i = 0; i < NUM_THREADS; ++i) { // Creating them and checking for error
        int r = pthread_create(&threads[i], NULL, thr_functions[i], &data);
        printf("Create thread '%s' %s\r\n", thread_names[i], (r == 0 ? "OK" : "FAIL"));
    }


    int *ex; // For saving returned values
    for(int i = 0; i < NUM_THREADS; ++i){ // Running the threads
        printf("Call join to thread '%s'\r\n", thread_names[i]);
        int r = pthread_join(threads[i], (void*)&ex);
        printf("Joining thread '%s' has been %s -- return values %d\r\n", thread_names[i], (r == 0 ? "OK" : "FAIL"), *ex);
    }


    io_close(data.in_pipe); // Closing the pipes properly
    io_close(data.out_pipe);





	queue_cleanup();
	call_termios(1); // restore terminal settings

	return EXIT_SUCCESS;
}



// - thread -----------------------------------------------------------------
void* input_thread_kb(void *arg){ // Thread for reading an input from user keyboard
    int c;
    static int ret = 0;
    // data_t *data = (data_t*) arg;
    event ev;

    bool q = false;

    while (!q && (c = getchar()) != EOF && c != 'q') {
        ev.type = EV_TYPE_NUM;
        switch(c) {
            case 'g':
                ev.type = EV_GET_VERSION;
                break;
            case 'a':
                ev.type = EV_ABORT;
                break;
            case 's':
                ev.type = EV_SET_COMPUTE;
                break;
            case '1':
                ev.type = EV_COMPUTE;
                break;
            case 'r':
                ev.type = EV_RESET_CHUNK;
                break;
            case 'c':
                ev.type = EV_COMPUTE_CPU;
                break;
            case 'p':
                ev.type = EV_REFRESH;
                break;
            case 'l':
                ev.type = EV_CLEAR_BUFFER;
                break;
            case 'm':
                ev.type = EV_SET_MANDELBROT;
                break;
            case 'o':
                ev.type = EV_SET_ORIGINAL;
                break;
            default:
                info("This keyboard command is not specified");
                break;
        }
        if (ev.type != EV_TYPE_NUM){
            queue_push(ev);
        }
        q = is_quit();
    }
    set_quit();
    ev.type = EV_QUIT;
    queue_push(ev);
    return &ret;
}

// - thread -----------------------------------------------------------------
void* input_thread_pipe(void *arg){ // Thread for reading an input from pipe
    data_t *data = (data_t*) arg;
    static int ret = 0;
    unsigned char c;
    uint8_t msg_buf[sizeof(message)];
    int i = 0;
    int len = 0;

    bool q = is_quit();

    while ((io_getc_timeout(data->in_pipe, READ_TIMEOUT_MS, &c)) > 0) {};

    while (!q){ // While user don't want to quit
        int r = io_getc_timeout(data->in_pipe, READ_TIMEOUT_MS, &c); // Reading the pipe input
        if (r > 0) { // If there was an input
            if (i == 0){
                len = 0;
                if (get_message_size(c, &len)) {
                    msg_buf[i++] = c;
                } else {
                    error("Unknown message received from module");
                }

            } else { // read remaning bites
                msg_buf[i++] = c;
            }
            if (len > 0 && i == len){
                message *msg = my_alloc(sizeof(message));
                if (parse_message_buf(msg_buf, len, msg)){
                    event ev = { .type = EV_PIPE_IN_MESSAGE };
                    ev.data.msg = msg;
                    queue_push(ev);
                } else {
                    error("Message from pipe couldn't be parsed");
                    free(msg);
                }
                i = len = 0;
            }
        } else if (r == -1) {
            //error - quit
            error("Reading from pipe was unsuccessful");
            set_quit();
            event ev = { .type = EV_QUIT };
            queue_push(ev);
        }
        q = is_quit();
    }
    return &ret;
}

void* main_thread(void *arg) { // Thread for reading an input from user keyboard
     // int c;
    static int ret = 0;
    data_t *data = (data_t *) arg;
    message msg;
    uint8_t msg_buf[sizeof(message)];
    int msg_len;
    bool q = false;


    computation_init();
    gui_init();

    while (!q) {

        event ev = queue_pop();
        msg.type = MSG_NBR;

        switch (ev.type) {
            case EV_GET_VERSION:
                msg.type = MSG_GET_VERSION;
                break;
            case EV_SET_COMPUTE:
                msg.type = MSG_SET_COMPUTE;
                info( set_compute(&msg) ? "set compute" : "fail set compute");
                break;
            case EV_COMPUTE:
                if (is_set()){
                    enable_comp();
                    msg.type = MSG_COMPUTE;
                    info( compute(&msg) ? "compute" : "fail compute");
                } else {
                    warn("The computation isn't set yet. Hint: press 's' to set the computations parameters");
                }

                break;
            case EV_ABORT:
                msg.type = MSG_ABORT;
                break;
            case EV_QUIT:
                debug("Quit received");
                set_quit();
                break;
            case EV_PIPE_IN_MESSAGE:
                process_pipe_message(&ev);
                break;

            case EV_RESET_CHUNK:
                if (!reset_chunk()){
                    info("Chunk was resetted");
                } else {
                    info("Cant be reset while it's computing");
                }
                break;
            case EV_CLEAR_BUFFER:
                buffer_cleanup();
                gui_refresh();
                info("Buffer was cleandup");
                break;
            case EV_REFRESH:
                gui_refresh();
                info("Gui refreshed");
                break;
            case EV_COMPUTE_CPU:
                my_compute();
                gui_refresh();
                info("Computed on PC");
                break;
            case EV_SET_MANDELBROT:
                if (!is_computing()){
                    set_parameters(0, 0, -2, -2, 2, 2);
                } else warn("You can't set new parameters while computing");
                break;
            case EV_SET_ORIGINAL:
                if (!is_computing()){
                    set_parameters(-0.4, 0.6, -1.6, -1.1, 1.6, 1.1);
                } else warn("You can't set new parameters while computing");
                break;
            default:
                debug("Unknown message");
                break;
        }



        if (msg.type != MSG_NBR){
            // Do in my assert
            fill_message_buf(&msg, msg_buf, sizeof(message), &msg_len);
            if (write(data->out_pipe, msg_buf, msg_len) == msg_len) {
                debug("Sent message to pipe");
            } else {
                info("send message fail"); // TODO eror form it
            }
        }
        q = is_quit();
    }


    gui_cleanup();
    computation_cleanup();
    return &ret;
}


void process_pipe_message(event * const ev){
    // do my Assert
    ev->type = EV_TYPE_NUM;
    const message *msg = ev->data.msg;
    switch (msg->type) {
        case MSG_STARTUP:
            fprintf(stderr, "Startup message: %s\n",msg->data.startup.message);
            break;
        case MSG_OK:
            info("OK message from pipe");
            break;
        case MSG_VERSION:
            fprintf(stderr, "Modelue wersion %d.%d-p%d\n", msg->data.version.major, msg->data.version.minor, msg->data.version.patch);
            break;
        case MSG_DONE:
            info("Message done");
            gui_refresh();
            if (is_done()){
                info("Computation done");
            } else {
                event event = { .type = EV_COMPUTE };
                info("Moving to another chunk");
                queue_push(event);
            }
            break;
        case MSG_ABORT:
            info("Computation aborted");
            abort_comp();
            break;
        case MSG_COMPUTE_DATA:
            if (!is_abort()) {
                update_data(&(msg->data.compute_data));
            }
            break;
        default:
            fprintf(stderr, "unknown message type in process pipi func\n");;
            break;
        
    }
    free(ev->data.msg);
    ev->data.msg = NULL;


}


/* end of prga-hw09-main.c */
