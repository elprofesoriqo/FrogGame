#ifndef cars_trees_h
#define cars_trees_h
#include <stdio.h>
#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>


#include "structures.h"
#include "window.h"
#include "fileprocessing.h"


void spawnCars(Car* cars, int playHeight, int level, int* bonus_car_spawned, time_t start_time, int* friendly_car_spawned);

// cars_trees.h
int randomSpeed(int level);          // Add this prototype
int randomSpeedBonusCar();      // Add this prototype



void initializeCars(Car* cars, int playHeight, int currentLevel) {
    srand(time(NULL));
    int total_cars = BASE_LEVEL_CARS + (currentLevel - 1) * CARS_PER_LEVEL;
    
    for (int i = 0; i < total_cars; i++) {
        cars[i].active = 0;
        cars[i].symbol = '-';
        cars[i].type = CAR_NORMAL;  // Default type
        
        // Assign random colors
        switch(rand() % 3) {
            case 0: cars[i].color = COLOR_NORMAL_CAR_1; break;
            case 1: cars[i].color = COLOR_NORMAL_CAR_2; break;
            case 2: cars[i].color = COLOR_NORMAL_CAR_3; break;
        }
    }
}

void spawnCars(Car* cars, int playHeight, int level, int* bonus_car_spawned, time_t start_time, int* friendly_car_spawned) {
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
                cars[i].type = CAR_BONUS;  // Set type to bonus
                cars[i].active = 1;
                *bonus_car_spawned = 1; // Mark bonus car as spawned
                break;
            }
        }
    }

    // Friendly car spawning (one per level on levels > 2)
    if (level > 2 && !*friendly_car_spawned && rand() % 100 < 20) {
        for (int i = 0; i < total_cars; i++) {
            if (!cars[i].active) {
                cars[i].x = PLAY_WIDTH - 2;
                cars[i].y = 1 + rand() % (playHeight - 2);
                cars[i].speed = 1;  // Very slow
                cars[i].color = COLOR_FRIENDLY_CAR;
                cars[i].type = CAR_FRIENDLY;
                cars[i].active = 1;
                *friendly_car_spawned = 1; // Mark friendly car as spawned
                break;
            }
        }
    }
}


void moveCars(Car* cars, Frog* frog, Window* playwin) {
    for (int i = 0; i < MAX_CARS; i++) {
        if (cars[i].active) {
            // Friendly car logic
            if (cars[i].type == CAR_FRIENDLY) {
                // Check if frog is in the same lane
                if (cars[i].y == frog->y) {
                    // Stop if too close to frog
                    if (abs(cars[i].x - frog->x) > 3) {
                        cars[i].x -= cars[i].speed;
                    }
                } else {
                    cars[i].x -= cars[i].speed;
                }
            } else {
                // Normal car movement
                cars[i].x -= cars[i].speed;
            }
            
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



//trees


int randomSpeed(int level) {
    int base_speed = (level + 1);  // Reduced base speed scaling
    int max_speed = base_speed + 2;  // Reduced max speed variance
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
        trees[index].symbol = 'T';  // More tree-like symbol
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
#endif