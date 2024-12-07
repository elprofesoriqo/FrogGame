#ifndef structures_h
#define structures_h
#include <time.h>
#include <ncurses.h>

// Game Constants
#define WINDOW_HEIGHT 20
#define PLAY_WIDTH 40
#define STATS_WIDTH 30
#define MIN_WIDTH 50
#define MIN_HEIGHT 25
#define MAX_CARS 30
#define CAR_SPAWN_CHANCE 60

#define COLOR_FROG 1
#define COLOR_BONUS_CAR 2
#define COLOR_NORMAL_CAR_1 3
#define COLOR_NORMAL_CAR_2 4
#define COLOR_NORMAL_CAR_3 5
#define COLOR_FRIENDLY_CAR 6  // Add this to structures.h color definitions


#define MAX_TREES 20
#define COLOR_TREE COLOR_GREEN
#define BASE_TIME 40
#define EXTRA_TIME 30
#define BASE_LEVEL_CARS 20
#define CARS_PER_LEVEL 10
#define EXTRA_TIME 30

#define CAR_NORMAL 0
#define CAR_BONUS 1
#define CAR_FRIENDLY 2

typedef struct {
    int width;
    int height;
    int startx;
    int starty;
    int color;
    WINDOW *win;
} Window;

typedef struct {
    char symbol;
    int x, y;
    int speed;
    int active;
    int color;
    int type;
} Car;

typedef struct {
    int x, y;
    char symbol;
    int active;
    int color;
} Tree;

typedef struct {
    int x, y;
    char symbol;
    int color;
} Frog;


typedef struct {
    int level;
    int total_points;
    int bonus_car_spawned;
    int bonus_time;
    time_t start_time;
} GameState;



typedef struct {
    int frog_color;
    int car_colors[3];
    int bonus_car_color;
    int tree_color;
} ColorConfig;


typedef struct {
    int points;
    int level;
    char date[20];
    char time[20];
} LeaderboardEntry;
#endif
