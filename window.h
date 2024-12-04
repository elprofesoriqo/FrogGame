#ifndef window_h
#define window_h
#include <stdio.h>
#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "cars_trees.h"
#include "structures.h"
#include "fileprocessing.h"


Window* Init(WINDOW* parent, int y, int x, int width, int color, int bo, int delay);
void CleanWin(Window* W, int bo);
// Function Implementations
WINDOW* Start() {
    initscr();
    noecho();
    curs_set(0);
    start_color();
    
    // Read color configuration
    ColorConfig colorConfig;
    readColorConfig(&colorConfig);
    
    // Color Pair Initialization
    init_pair(COLOR_FROG, colorConfig.frog_color, COLOR_BLACK);
    init_pair(COLOR_BONUS_CAR, colorConfig.bonus_car_color, COLOR_BLACK);
    init_pair(COLOR_NORMAL_CAR_1, colorConfig.car_colors[0], COLOR_BLACK);
    init_pair(COLOR_NORMAL_CAR_2, colorConfig.car_colors[1], COLOR_BLACK);
    init_pair(COLOR_NORMAL_CAR_3, colorConfig.car_colors[2], COLOR_BLACK);
    init_pair(COLOR_FRIENDLY_CAR, colorConfig.frog_color, COLOR_BLACK);
    
    cbreak();
    keypad(stdscr, TRUE);
    return stdscr;
}


Window* Init(WINDOW* parent, int y, int x, int width, int color, int bo, int delay) {
    Window* W = (Window*)malloc(sizeof(Window));
    W->startx = x;
    W->starty = y;
    W->width = width;
    W->height = WINDOW_HEIGHT;
    W->color = color;
    W->win = subwin(parent, W->height, W->width, W->starty, W->startx);
    CleanWin(W, bo);
    if (delay) {
        nodelay(W->win, TRUE);
    }
    wrefresh(W->win);
    return W;
}

void CleanWin(Window* W, int bo) {
    wclear(W->win);
    if (bo) {
        wattron(W->win, COLOR_PAIR(W->color));
        box(W->win, 0, 0);
        wattroff(W->win, COLOR_PAIR(W->color));
    }
    wrefresh(W->win);
}

#endif