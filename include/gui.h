#pragma once

#include <stdbool.h>
#include <X11/Xlib.h>

typedef struct {
    Display *display;
    Window window;
    GC gc;
    int screen;
} Win;

int init_gui_window(int stdio_fd);
void draw_pixel(Win window, int x, int y, bool white);
void write_str(Win win, const char *s);
