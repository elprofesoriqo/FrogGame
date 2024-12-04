#include <stdio.h>
#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "cars_trees.h"
#include "structures.h"
#include "window.h"
#include "fileprocessing.h"



// Full function prototypes
WINDOW* Start();
void displayGameOverScreen(int game_over_reason, GameState* game_state);


void initFrog(Frog* frog, int play_width) {
    frog->x = play_width / 2;
    frog->y = WINDOW_HEIGHT - 2;  // Last line
    frog->symbol = '^';
    frog->color = COLOR_FROG;
}

void moveFrog(Frog* frog, int ch, Tree* trees) {
    int new_x = frog->x, new_y = frog->y;

    switch(ch) {
        case 'w': new_y = (frog->y > 1) ? frog->y - 1 : frog->y; break;
        case 's': new_y = (frog->y < WINDOW_HEIGHT - 2) ? frog->y + 1 : frog->y; break;
        case 'a': new_x = (frog->x > 1) ? frog->x - 1 : frog->x; break;
        case 'd': new_x = (frog->x < PLAY_WIDTH - 2) ? frog->x + 1 : frog->x; break;
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

void handleBonusCar(Frog* frog, Car* cars, int middle_lane) {
    for (int i = 0; i < MAX_CARS; i++) {
        if (cars[i].active &&
            cars[i].y == middle_lane &&
            cars[i].type == CAR_BONUS && // Ensure it's a bonus car
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
}

int checkCollision(Frog* frog, Car* cars) {
    for (int i = 0; i < MAX_CARS; i++) {
        if (cars[i].active && 
            cars[i].y == frog->y && 
            (cars[i].x == frog->x || cars[i].x == frog->x + 1 || cars[i].x == frog->x - 1) &&
            cars[i].type != CAR_BONUS && // Ignore bonus cars
            cars[i].type != CAR_FRIENDLY) { // Ignore friendly cars
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

void displayLeaderboard() {
    LeaderboardEntry entries[MAX_LEADERBOARD_ENTRIES];
    int entry_count = readLeaderboardEntries(entries);

    // Sort entries by points (bubble sort)
    for (int i = 0; i < entry_count - 1; i++) {
        for (int j = 0; j < entry_count - i - 1; j++) {
            if (entries[j].points < entries[j+1].points) {
                LeaderboardEntry temp = entries[j];
                entries[j] = entries[j+1];
                entries[j+1] = temp;
            }
        }
    }

    // Clear screen for leaderboard
    clear();
    mvprintw(2, COLS/2 - 10, "LEADERBOARD");
    mvprintw(3, COLS/2 - 15, "----------------------------");

    // Display entries
    for (int i = 0; i < entry_count && i < 10; i++) {
        mvprintw(5 + i, COLS/2 - 15, "%d. Points: %d | Level: %d | %s %s", 
                 i+1, 
                 entries[i].points, 
                 entries[i].level, 
                 entries[i].date, 
                 entries[i].time);
    }

    // If no entries
    if (entry_count == 0) {
        mvprintw(LINES/2, COLS/2 - 10, "No scores yet!");
    }

    mvprintw(LINES - 3, COLS/2 - 15, "Press 'ENTER' to continue...");
    refresh();
    
    // Wait specifically for ENTER key
    nodelay(stdscr, FALSE);
    int ch;
    while ((ch = getch()) != '\n' && ch != '\r') {
        // Wait for ENTER key
    }
    
    // Clear screen completely
    clear();
    refresh();
    
    // Return to non-blocking input
    nodelay(stdscr, TRUE);
}


void stats(Window * statwin, int remaining_time, GameState * game_state) {
    mvwprintw(statwin->win, 2, 2, "Level: %d", game_state->level);
    mvwprintw(statwin->win, 3, 2, "Cars: %d", BASE_LEVEL_CARS + (game_state->level - 1) * CARS_PER_LEVEL);
    mvwprintw(statwin->win, 5, 2, "Time Left: %d sec", remaining_time);
    mvwprintw(statwin->win, 6, 2, "Points: %d", game_state->total_points);
    mvwprintw(statwin->win, 7, 2, "Igor Jankowski s203396");
}


int timer(time_t start_time, int current_level) {
    time_t current_time = time(NULL);
    int base_time = BASE_TIME + (current_level - 1) * 10;  // 10 seconds per level
    int elapsed_time = (int)difftime(current_time, start_time);
    int remaining_time = base_time - elapsed_time;
    
    return (remaining_time > 0) ? remaining_time : 0;
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
    int friendly_car_spawned = 0;
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
        int remaining_time = timer(game_state->start_time, game_state->level);
        
        // Clear interior of windows without redrawing box
        werase(playwin->win);
        werase(statwin->win);
        box(playwin->win, 0, 0);
        box(statwin->win, 0, 0);

        // Update stats window
        stats(statwin, remaining_time, game_state);
    

        wrefresh(statwin->win);

        // Game mechanics
        spawnCars(cars, WINDOW_HEIGHT, game_state->level, &game_state->bonus_car_spawned, game_state->start_time, &friendly_car_spawned);
        moveCars(cars, &frog , playwin);
        drawCars(cars, playwin);
        drawTrees(trees, playwin);
        drawFrog(&frog, playwin);

        // Player input handling
        int ch = wgetch(playwin->win);
        switch(ch) {
            case 'q':
            case 'Q':
                game_running = 0;
                break;
            case 'l':
            case 'L':
                // Display leaderboard
                displayLeaderboard();
                // Redraw game screen after leaderboard
                werase(playwin->win);
                werase(statwin->win);
                box(playwin->win, 0, 0);
                box(statwin->win, 0, 0);
                break;
            default:
        // Handle player input
        if (!processPlayerInput(&frog, trees, ch, key_state)) {
            game_running = 0;
            break;
        }
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
    
    // Save score to leaderboard
    saveToLeaderboard(game_state->total_points, game_state->level);
    
    // Display countdown
    mvprintw(LINES/2 + 2, COLS/2 - 10, "Score saved to Leaderboard!");
    for (int i = 5; i > 0; i--) {
        mvprintw(LINES/2 + 3, COLS/2 - 10, "Closing in %d seconds...", i);
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