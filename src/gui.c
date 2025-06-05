/* Handles the graphical window which displays the terminal's text.
 * Copyright (C) 2025 Jake Steinburger (UnmappedStack) under the Mozilla Public License 2.0.
 * See the LICENSE file in the Git repository's root for mroe information. */

#include <font.h>
#include <gui.h>
#include <dev.h>

#include <X11/Xlib.h>
#include <stdio.h>
#include <unistd.h>

typedef struct {
    int x, y;
} TTY;

TTY tty = {0};

void draw_pixel(Win window, int x, int y, bool white) {
    if (white)
        XSetForeground(window.display, window.gc, WhitePixel(window.display, window.screen));
    else
        XSetForeground(window.display, window.gc, BlackPixel(window.display, window.screen));
    XDrawPoint(window.display, window.window, window.gc, x, y);
}

void draw_char(Win win, char ch, int x_coord, int y_coord) {
    int first_byte_idx = ch * 16;
    for (size_t y = 0; y < 16; y++) {
        for (size_t x = 0; x < 8; x++) {
            if ((font[first_byte_idx + y] >> (7 - x)) & 1)
                draw_pixel(win, x_coord + x, y_coord + y, false);
            else
                draw_pixel(win, x_coord + x, y_coord + y, true);
        }
    }
}

void write_char(Win win, char ch) {
    draw_char(win, ch, tty.x, tty.y);
    tty.x += 8;
}

void write_str(Win win, const char *s) {
    while (*s)
        write_char(win, *(s++));
}

int init_gui_window(int stdio_fd) {
    Display *display;
    Window window;
    int screen;
    GC gc;
    unsigned long black, white;
    if (!(display=XOpenDisplay(NULL))) {
        fprintf(stderr, "XOpenDisplay returned error");
        return -1;
    }
    screen = DefaultScreen(display);
    black = BlackPixel(display, screen);
    white = WhitePixel(display, screen);
    window = XCreateSimpleWindow(display, RootWindow(display, screen),
            100, 100, 400, 300, 1, black, white);
    XSelectInput(display, window, ExposureMask | KeyPressMask);
    XMapWindow(display, window);
    gc = XCreateGC(display, window, 0, NULL);
    XSetForeground(display, gc, black);
    XEvent e;
    Win win = (Win) {
        .display=display,
        .window=window,
        .gc=gc,
        .screen=screen,
    };
    for (;;) {
        XNextEvent(display, &e);
        if (wait_for_vtty_write_iteration(stdio_fd, win) < 0) return -1;
        if (e.type == KeyPress) break;
    }
    XFreeGC(display, gc);
    XDestroyWindow(display, window);
    XCloseDisplay(display);
    return 0;
}
