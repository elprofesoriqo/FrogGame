#include <stdio.h>
#include <ncurses.h>
#include <stdlib.h>
#include <time.h>

// Game Constants
#define WINDOW_HEIGHT 20
#define PLAY_WIDTH 40
#define STATS_WIDTH 30
#define MIN_WIDTH 50
#define MIN_HEIGHT 25
#define MAX_CARS 10
#define CAR_SPAWN_CHANCE 20

// Color Pair Definitions
#define COLOR_FROG 1
#define COLOR_BONUS_CAR 2
#define COLOR_NORMAL_CAR_1 3
#define COLOR_NORMAL_CAR_2 4
#define COLOR_NORMAL_CAR_3 5

// Game Mechanics
#define BASE_TIME 40
#define EXTRA_TIME 30

// Structs
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
} Car;

typedef struct {
    int x, y;
    char symbol;
    int color;
} Frog;

// Global Variables
int level = 1;
int stats_x, stats_y;
time_t start_time;

// Color Map
const char* color_map[] = {"R", "B", "G", "Y", "C", "M"};

// Function Prototypes
WINDOW* Start();
Window* Init(WINDOW* parent, int y, int x, int width, int color, int bo, int delay);
void CleanWin(Window* W, int bo);

// Car Functions
void initializeCars(Car* cars, int playHeight);
void spawnCars(Car* cars, int playHeight, int level);
void moveCars(Car* cars, Window* playwin);
void drawCars(Car* cars, Window* playwin);

// Frog Functions
void initFrog(Frog* frog, int play_width);
void moveFrog(Frog* frog, int ch);
void drawFrog(Frog* frog, Window* playwin);
int checkCollision(Frog* frog, Car* cars);

// Game Mechanics Functions
int randomSpeed(int level) {
    int base_speed = (level - 1) * 2 + 1;
    int max_speed = base_speed + 2;
    return base_speed + rand() % (max_speed - base_speed + 1);
}

void stats(int position_x, int position_y, int remaining_time) {
    int row = position_y;
    mvprintw(row--, position_x, "Level: %d", level);
    mvprintw(row--, position_x, "Cars: %d", MAX_CARS);
    mvprintw(row--, position_x, "Time Left: %d sec", remaining_time);
}

int timer(time_t start_time, int current_level, int middle_lane_time) {
    time_t current_time = time(NULL);
    int base_time = BASE_TIME;
    int elapsed_time = (int)difftime(current_time, start_time);
    int remaining_time = base_time - elapsed_time - middle_lane_time;
    
    return (remaining_time > 0) ? remaining_time : 0;
}

// Initialization Functions
WINDOW* Start() {
    initscr();
    noecho();
    curs_set(0);
    start_color();
    
    // Color Pair Initialization
    init_pair(COLOR_FROG, COLOR_GREEN, COLOR_BLACK);
    init_pair(COLOR_BONUS_CAR, COLOR_YELLOW, COLOR_BLACK);
    init_pair(COLOR_NORMAL_CAR_1, COLOR_RED, COLOR_BLACK);
    init_pair(COLOR_NORMAL_CAR_2, COLOR_BLUE, COLOR_BLACK);
    init_pair(COLOR_NORMAL_CAR_3, COLOR_CYAN, COLOR_BLACK);
    
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

// Frog Functions
void initFrog(Frog* frog, int play_width) {
    frog->x = play_width / 2;
    frog->y = WINDOW_HEIGHT - 2;  // Last line
    frog->symbol = 'F';
    frog->color = COLOR_FROG;
}

void moveFrog(Frog* frog, int ch) {
    switch(ch) {
        case 'w': frog->y = (frog->y > 1) ? frog->y - 1 : frog->y; break;
        case 's': frog->y = (frog->y < WINDOW_HEIGHT - 2) ? frog->y + 1 : frog->y; break;
        case 'a': frog->x = (frog->x > 1) ? frog->x - 1 : frog->x; break;
        case 'd': frog->x = (frog->x < PLAY_WIDTH - 2) ? frog->x + 1 : frog->x; break;
    }
}

void drawFrog(Frog* frog, Window* playwin) {
    wattron(playwin->win, COLOR_PAIR(frog->color));
    mvwaddch(playwin->win, frog->y, frog->x, frog->symbol);
    wattroff(playwin->win, COLOR_PAIR(frog->color));
}

int checkCollision(Frog* frog, Car* cars) {
    for (int i = 0; i < MAX_CARS; i++) {
        if (cars[i].active && cars[i].x == frog->x && cars[i].y == frog->y) {
            return 1;
        }
    }
    return 0;
}

// Car Functions
void initializeCars(Car* cars, int playHeight) {
    srand(time(NULL));
    for (int i = 0; i < MAX_CARS; i++) {
        cars[i].active = 0;
        cars[i].symbol = '-';
        
        // Assign random colors
        switch(rand() % 3) {
            case 0: cars[i].color = COLOR_NORMAL_CAR_1; break;
            case 1: cars[i].color = COLOR_NORMAL_CAR_2; break;
            case 2: cars[i].color = COLOR_NORMAL_CAR_3; break;
        }
    }
}

void spawnCars(Car* cars, int playHeight, int level) {
    // Spawn regular cars
    if (rand() % 100 < CAR_SPAWN_CHANCE) {
        for (int i = 0; i < MAX_CARS; i++) {
            if (!cars[i].active) {
                cars[i].x = PLAY_WIDTH - 2;
                cars[i].y = 1 + rand() % (playHeight - 2);
                cars[i].speed = randomSpeed(level);
                cars[i].active = 1;
                break;
            }
        }
    }
    
    // Spawn bonus car in the middle lane
    int middle_lane = playHeight / 2;
    int bonus_car_exists = 0;
    for (int i = 0; i < MAX_CARS; i++) {
        if (cars[i].active && cars[i].y == middle_lane) {
            bonus_car_exists = 1;
            break;
        }
    }
    
    if (!bonus_car_exists) {
        for (int i = 0; i < MAX_CARS; i++) {
            if (!cars[i].active) {
                cars[i].x = PLAY_WIDTH - 2;
                cars[i].y = middle_lane;
                cars[i].speed = randomSpeed(level);
                cars[i].color = COLOR_BONUS_CAR;
                cars[i].active = 1;
                break;
            }
        }
    }
}

void moveCars(Car* cars, Window* playwin) {
    for (int i = 0; i < MAX_CARS; i++) {
        if (cars[i].active) {
            cars[i].x -= cars[i].speed;
            
            if (cars[i].x <= 1) {
                cars[i].active = 0;
            }
        }
    }
}

void drawCars(Car* cars, Window* playwin) {
    for (int i = 0; i < MAX_CARS; i++) {
        if (cars[i].active) {
            wattron(playwin->win, COLOR_PAIR(cars[i].color));
            mvwaddch(playwin->win, cars[i].y, cars[i].x, cars[i].symbol);
            wattroff(playwin->win, COLOR_PAIR(cars[i].color));
        }
    }
}

int main() {
    WINDOW* mainwin = Start();
    
    Window* playwin = Init(mainwin, 0, 0, PLAY_WIDTH, 1, 1, 1);
    Window* statwin = Init(mainwin, 0, PLAY_WIDTH + 2, STATS_WIDTH, 1, 1, 0);

    // Initialize game components
    Car cars[MAX_CARS];
    Frog frog;
    
    initializeCars(cars, WINDOW_HEIGHT);
    initFrog(&frog, PLAY_WIDTH);
    
    start_time = time(NULL);
    int middle_lane_time = 0;
    int game_running = 1;

    while (game_running) {
        // Clear play window
        CleanWin(playwin, 1);

        // Spawn cars
        spawnCars(cars, WINDOW_HEIGHT, level);

        // Move cars
        moveCars(cars, playwin);

        // Draw cars
        drawCars(cars, playwin);

        // Draw frog
        drawFrog(&frog, playwin);

        // Game logic
        int ch = wgetch(playwin->win);
        moveFrog(&frog, ch);

        // Check for collision with middle lane bonus car
        int middle_lane = WINDOW_HEIGHT / 2;
        for (int i = 0; i < MAX_CARS; i++) {
            if (cars[i].active && cars[i].y == middle_lane && 
                cars[i].x == frog.x && frog.y == middle_lane) {
                middle_lane_time += EXTRA_TIME;
                cars[i].active = 0;
                break;
            }
        }

        // Check for collision with other cars
        if (checkCollision(&frog, cars)) {
            game_running = 0;
        }

        // Update timer and stats
        int remaining_time = timer(start_time, level, middle_lane_time);
        
        if (remaining_time <= 0) {
            game_running = 0;
        }

        // Display stats
        stats_x = PLAY_WIDTH + 5;
        stats_y = 5;
        stats(stats_x, stats_y, remaining_time);

        // Refresh windows
        wrefresh(playwin->win);
        wrefresh(statwin->win);

        // Control game speed
        napms(100);

        // Quit condition
        if (ch == 'q' || ch == 'Q') {
            game_running = 0;
        }
    }

    // Game over screen
    clear();
    mvprintw(LINES/2, COLS/2 - 10, "Game Over! Level: %d", level);
    refresh();
    getch();

    endwin();
    return 0;
}