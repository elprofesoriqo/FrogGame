#include <stdio.h>
#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>


// Game Constants
#define WINDOW_HEIGHT 20
#define PLAY_WIDTH 40
#define STATS_WIDTH 30
#define MIN_WIDTH 50
#define MIN_HEIGHT 25
#define MAX_CARS 20  // Increased max cars
#define CAR_SPAWN_CHANCE 30  // Increased spawn chance

// Color Pair Definitions
#define COLOR_FROG 1
#define COLOR_BONUS_CAR 2
#define COLOR_NORMAL_CAR_1 3
#define COLOR_NORMAL_CAR_2 4
#define COLOR_NORMAL_CAR_3 5

#define MAX_TREES 10  // Add this with color pair definition
#define COLOR_TREE COLOR_GREEN  // Define tree color
// Game Mechanics
#define BASE_TIME 40
#define EXTRA_TIME 30
#define BASE_LEVEL_CARS 20
#define CARS_PER_LEVEL 10


typedef struct {
    int frog_color;
    int car_colors[3];
    int bonus_car_color;
    int tree_color;
} ColorConfig;

int readColorConfig(ColorConfig* config);
int getColorFromName(const char* colorName);


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




int getColorFromName(const char* colorName) {
    if (strcmp(colorName, "black") == 0) return COLOR_BLACK;
    if (strcmp(colorName, "red") == 0) return COLOR_RED;
    if (strcmp(colorName, "green") == 0) return COLOR_GREEN;
    if (strcmp(colorName, "yellow") == 0) return COLOR_YELLOW;
    if (strcmp(colorName, "blue") == 0) return COLOR_BLUE;
    if (strcmp(colorName, "magenta") == 0) return COLOR_MAGENTA;
    if (strcmp(colorName, "cyan") == 0) return COLOR_CYAN;
    if (strcmp(colorName, "white") == 0) return COLOR_WHITE;
    
    // Default to black if color not recognized
    return COLOR_BLACK;
}

int readColorConfig(ColorConfig* config) {
    FILE* file = fopen("input.txt", "r");
    if (file == NULL) {
        // If file can't be opened, return default configuration
        config->frog_color = COLOR_GREEN;
        config->car_colors[0] = COLOR_RED;
        config->car_colors[1] = COLOR_BLUE;
        config->car_colors[2] = COLOR_RED;
        config->bonus_car_color = COLOR_YELLOW;
        config->tree_color = COLOR_GREEN;
        return 0;
    }

    char line[100];
    char colorName[20];

    // Read frog color
    if (fgets(line, sizeof(line), file)) {
        if (sscanf(line, "frog %s", colorName) == 1) {
            config->frog_color = getColorFromName(colorName);
        }
    }

    // Read car colors
    if (fgets(line, sizeof(line), file)) {
        // Default to 3 colors
        config->car_colors[0] = COLOR_RED;
        config->car_colors[1] = COLOR_BLUE;
        config->car_colors[2] = COLOR_RED;

        char* token = strtok(line, " \n");
        if (token && strcmp(token, "car") == 0) {
            int colorCount = 0;
            while ((token = strtok(NULL, " \n")) && colorCount < 3) {
                config->car_colors[colorCount++] = getColorFromName(token);
            }
        }
    }

    // Read bonus car color
    if (fgets(line, sizeof(line), file)) {
        if (sscanf(line, "bonus_car %s", colorName) == 1) {
            config->bonus_car_color = getColorFromName(colorName);
        }
    }

    // Read tree color
    if (fgets(line, sizeof(line), file)) {
        if (sscanf(line, "tree %s", colorName) == 1) {
            config->tree_color = getColorFromName(colorName);
        }
    }

    fclose(file);
    return 1;
}

// Full function prototypes
WINDOW* Start();
Window* Init(WINDOW* parent, int y, int x, int width, int color, int bo, int delay);
void CleanWin(Window* W, int bo);
void initializeCars(Car* cars, int playHeight, int currentLevel);
// Replace the existing prototype with:
void spawnCars(Car* cars, int playHeight, int level, int* bonus_car_spawned);
void moveCars(Car* cars, Window* playwin);
void drawCars(Car* cars, Window* playwin);
void initFrog(Frog* frog, int play_width);
void moveFrog(Frog* frog, int ch, Tree* trees);

void drawFrog(Frog* frog, Window* playwin);
void initializeTrees(Tree* trees, int playHeight);
int checkCollision(Frog* frog, Car* cars);
int randomSpeed(int level);
int randomSpeedBonusCar();
void stats(int position_x, int position_y, int remaining_time);
int timer(time_t start_time, int current_level, int middle_lane_time);



void initializeGameState(GameState* game_state, int initial_level);
int runGame(WINDOW* mainwin, GameState* game_state);
void handleBonusCar(Frog* frog, Car* cars, int middle_lane);
int processPlayerInput(Frog* frog, Tree* trees, int ch, int* key_state);
void updateGameLevel(GameState* game_state, Frog* frog, Car* cars, Tree* trees, int remaining_time);
void displayGameOverScreen(int game_over_reason, GameState* game_state);



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

int randomSpeed(int level) {
    int base_speed = (level * 2);  // Increased base speed scaling
    int max_speed = base_speed + 3;
    return base_speed + rand() % (max_speed - base_speed + 1);
}

int randomSpeedBonusCar() {
    int base_speed = 1;  // Increased base speed scaling
    int max_speed = 2;
    return base_speed + rand() % (max_speed - base_speed + 1);
}


void initializeTrees(Tree* trees, int playHeight) {
    srand(time(NULL));
    for (int i = 0; i < MAX_TREES; i++) {
        trees[i].active = 0;  // Reset all trees
    }
    
    int trees_to_place = 3 + rand() % 3;  // 3-5 trees
    for (int i = 0; i < trees_to_place; i++) {
        int index = rand() % MAX_TREES;
        trees[index].x = 1 + rand() % (PLAY_WIDTH - 3);
        trees[index].y = 1 + rand() % (playHeight - 2);
        trees[index].symbol = 'T';
        trees[index].active = 1;
        trees[index].color = COLOR_TREE;
    }
}

void drawTrees(Tree* trees, Window* playwin) {
    for (int i = 0; i < MAX_TREES; i++) {
        if (trees[i].active) {
            wattron(playwin->win, COLOR_PAIR(trees[i].color));
            mvwprintw(playwin->win, trees[i].y, trees[i].x, "%c", trees[i].symbol);
            wattroff(playwin->win, COLOR_PAIR(trees[i].color));
        }
    }
}


void stats(int position_x, int position_y, int remaining_time) {
    int row = position_y;
    mvprintw(row--, position_x, "Level: %d", level);
    mvprintw(row--, position_x, "Cars: %d", BASE_LEVEL_CARS + (level - 1) * CARS_PER_LEVEL);
    mvprintw(row--, position_x, "Time Left: %d sec", remaining_time);
}

int timer(time_t start_time, int current_level, int middle_lane_extra_time) {
    time_t current_time = time(NULL);
    int base_time = BASE_TIME + (current_level - 1) * 5;  // More time per level
    int elapsed_time = (int)difftime(current_time, start_time);
    int remaining_time = base_time + middle_lane_extra_time - elapsed_time;
    
    return (remaining_time > 0) ? remaining_time : 0;
}


void initializeCars(Car* cars, int playHeight, int currentLevel) {
    srand(time(NULL));
    int total_cars = BASE_LEVEL_CARS + (currentLevel - 1) * CARS_PER_LEVEL;
    
    for (int i = 0; i < total_cars; i++) {
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

void spawnCars(Car* cars, int playHeight, int level, int* bonus_car_spawned) {
    int total_cars = BASE_LEVEL_CARS + (level - 1) * CARS_PER_LEVEL;
    int middle_lane = playHeight / 2;
    time_t current_time = time(NULL);
    int elapsed_time = (int)difftime(current_time, start_time);

    // Regular car spawning
    if (rand() % 100 < CAR_SPAWN_CHANCE) {
        for (int i = 0; i < total_cars; i++) {
            if (!cars[i].active) {
                cars[i].x = PLAY_WIDTH - 2;
                cars[i].y = 1 + rand() % (playHeight - 2);

                // Ensure the car does not spawn in the middle lane
                if (cars[i].y != middle_lane) {
                    cars[i].speed = randomSpeed(level);
                    cars[i].active = 1;
                    break;
                }
            }
        }
    }

    // Bonus car spawning (one per level, after 7 seconds)
    if (!*bonus_car_spawned && elapsed_time >= 7) {
        for (int i = 0; i < total_cars; i++) {
            if (!cars[i].active) {
                cars[i].x = PLAY_WIDTH - 2;
                cars[i].y = middle_lane; // Middle lane only
                cars[i].speed = randomSpeedBonusCar();
                cars[i].color = COLOR_BONUS_CAR;
                cars[i].active = 1;
                *bonus_car_spawned = 1; // Mark bonus car as spawned
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

void initFrog(Frog* frog, int play_width) {
    frog->x = play_width / 2;
    frog->y = WINDOW_HEIGHT - 2;  // Last line
    frog->symbol = '^';
    frog->color = COLOR_FROG;
}
void moveFrog(Frog* frog, int ch, Tree* trees) {
    int new_x = frog->x, new_y = frog->y;

    switch(ch) {
        case 'w': 
            new_y = (frog->y > 1) ? frog->y - 1 : frog->y; 
            break;
        case 's': 
            new_y = (frog->y < WINDOW_HEIGHT - 2) ? frog->y + 1 : frog->y; 
            break;
        case 'a': 
            new_x = (frog->x > 1) ? frog->x - 1 : frog->x; 
            break;
        case 'd': 
            new_x = (frog->x < PLAY_WIDTH - 2) ? frog->x + 1 : frog->x; 
            break;
    }

    // Check for tree collision
    int can_move = 1;
    for (int i = 0; i < MAX_TREES; i++) {
        if (trees[i].active && trees[i].x == new_x && trees[i].y == new_y) {
            can_move = 0;
            break;
        }
    }

    if (can_move) {
        frog->x = new_x;
        frog->y = new_y;
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
int countActiveTrees(Tree* trees) {
    int count = 0;
    for (int i = 0; i < MAX_TREES; i++) {
        if (trees[i].active) {
            count++;
        }
    }
    return count;
}

void initializeGameState(GameState* game_state, int initial_level) {
    game_state->level = initial_level;
    game_state->total_points = 0;
    game_state->bonus_car_spawned = 0;
    game_state->bonus_time = 0;
    game_state->start_time = time(NULL);
}

int calculateLevelPoints(int remaining_time) {
    return remaining_time > 0 ? remaining_time : 0;
}

void updateGameLevel(GameState* game_state, Frog* frog, Car* cars, Tree* trees, int remaining_time) {
    // Calculate and add points for the level
    int level_points = calculateLevelPoints(remaining_time);
    game_state->total_points += level_points;

    game_state->level++;
    initFrog(frog, PLAY_WIDTH);
    initializeCars(cars, WINDOW_HEIGHT, game_state->level);
    initializeTrees(trees, WINDOW_HEIGHT);
    game_state->start_time = time(NULL);
    game_state->bonus_car_spawned = 0;
    game_state->bonus_time = 0;  
    
    clear();
    mvprintw(LINES/2, COLS/2 - 10, "Next Level: %d!", game_state->level);
    mvprintw(LINES/2 + 1, COLS/2 - 10, "Points: +%d", level_points);
    refresh();
    napms(500);
    clear();
    refresh();
}

void handleBonusCar(Frog* frog, Car* cars, int middle_lane) {
    for (int i = 0; i < MAX_CARS; i++) {
        if (cars[i].active &&
            cars[i].y == middle_lane &&
            cars[i].x == frog->x &&
            frog->y == middle_lane) {

            // Move frog up by 3 positions if possible
            if (frog->y > 3) {
                frog->y -= 3;
            } else {
                frog->y = 1;  // If not enough space, move to top
            }

            // Deactivate the bonus car
            cars[i].active = 0;
            break;
        }
    }
    return;
}

int processPlayerInput(Frog* frog, Tree* trees, int ch, int* key_state) {
    if (ch == 'q' || ch == 'Q') {
        return 0;  // Signal to exit game
    }

    if (ch != ERR && key_state[ch] == 0) {
        switch(ch) {
            case 'w':
            case 's':
            case 'a':
            case 'd':
                moveFrog(frog, ch, trees);
                key_state[ch] = 1;  // Mark key as pressed
                break;
        }
    }
    return 1;  // Continue game
}

int runGame(WINDOW* mainwin, GameState* game_state) {
    Window* playwin = Init(mainwin, 0, 0, PLAY_WIDTH, 1, 1, 1);
    Window* statwin = Init(mainwin, 0, PLAY_WIDTH + 2, STATS_WIDTH, 1, 1, 0);

    // Prevent key repeat and buffer
    nodelay(playwin->win, TRUE);
    timeout(100);  // Wait for input for 100ms
    
    // Initialize game components
    Car cars[MAX_CARS];
    Frog frog;
    Tree trees[MAX_TREES] = {0};
    
    initializeCars(cars, WINDOW_HEIGHT, game_state->level);
    initializeTrees(trees, WINDOW_HEIGHT);
    initFrog(&frog, PLAY_WIDTH);

    int game_running = 1;
    int key_state[256] = {0};  // Track state of each key
    int game_over_reason = 0;  // 0: playing, 1: car collision, 2: time out

    while (game_running) {
        // Update remaining time
        int remaining_time = timer(game_state->start_time, game_state->level, game_state->bonus_time);
        
        // Clear interior of windows without redrawing box
        werase(playwin->win);
        werase(statwin->win);
        box(playwin->win, 0, 0);
        box(statwin->win, 0, 0);

        // Update stats window
        mvwprintw(statwin->win, 2, 2, "Level: %d", game_state->level);
        mvwprintw(statwin->win, 3, 2, "Cars: %d", BASE_LEVEL_CARS + (game_state->level - 1) * CARS_PER_LEVEL);
        mvwprintw(statwin->win, 4, 2, "Trees: %d", countActiveTrees(trees));
        mvwprintw(statwin->win, 5, 2, "Time Left: %d sec", remaining_time);
        mvwprintw(statwin->win, 6, 2, "Points: %d", game_state->total_points);
        
        wrefresh(statwin->win);

        // Game mechanics
        spawnCars(cars, WINDOW_HEIGHT, game_state->level, &game_state->bonus_car_spawned);        
        moveCars(cars, playwin);
        drawCars(cars, playwin);
        drawTrees(trees, playwin);
        drawFrog(&frog, playwin);

        // Player input handling
        int ch = wgetch(playwin->win);
        
        // Handle player input
        if (!processPlayerInput(&frog, trees, ch, key_state)) {
            game_running = 0;
            break;
        }

        // Reset key states when no key is pressed
        if (ch == ERR) {
            memset(key_state, 0, sizeof(key_state));
        }

        // Check for bonus car interaction
        int middle_lane = WINDOW_HEIGHT / 2;
        handleBonusCar(&frog, cars, middle_lane);

        // Add time for bonus car
        if (handleBonusCar) {
            game_state->bonus_time += EXTRA_TIME;
        }

        // Check for collision with cars
        if (checkCollision(&frog, cars)) {
            game_running = 0;
            game_over_reason = 1;  // Car collision
            break;
        }

        // Level progression
        if (frog.y == 1) {
            updateGameLevel(game_state, &frog, cars, trees, remaining_time);
        }

        // Refresh windows and add delay
        wrefresh(playwin->win);
        wrefresh(statwin->win);
        napms(100);

        // Time-out condition
        if (remaining_time <= 0) {
            game_running = 0;
            game_over_reason = 2;  // Time out
        }
    }

    // Clean up windows
    delwin(playwin->win);
    delwin(statwin->win);
    free(playwin);
    free(statwin);

    // Display game over screen
    displayGameOverScreen(game_over_reason, game_state);

    return game_running;
}

void displayGameOverScreen(int game_over_reason, GameState* game_state) {
    clear();
    switch(game_over_reason) {
        case 1:  // Car collision
            mvprintw(LINES/2 - 2, COLS/2 - 15, "GAME OVER: Car Collision!");
            break;
        case 2:  // Time out
            mvprintw(LINES/2 - 2, COLS/2 - 15, "GAME OVER: Time Ran Out!");
            break;
        default: // Quit or other reasons
            mvprintw(LINES/2 - 2, COLS/2 - 15, "GAME OVER!");
            break;
    }

    mvprintw(LINES/2, COLS/2 - 10, "Final Level: %d", game_state->level);
    mvprintw(LINES/2 + 1, COLS/2 - 10, "Total Points: %d", game_state->total_points);
    
    // Display countdown
    for (int i = 5; i > 0; i--) {
        mvprintw(LINES/2 + 2, COLS/2 - 10, "Closing in %d seconds...", i);
        refresh();
        napms(1000);  // 1 second delay
    }
}



int main() {
    WINDOW* mainwin = Start();
    
    GameState game_state;
    initializeGameState(&game_state, 1);  // Start at level 1
    
    runGame(mainwin, &game_state);

    endwin();
    return 0;
}