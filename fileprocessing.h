#ifndef fileprocessing_h
#define fileprocessing_h
#include <stdio.h>
#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "cars_trees.h"
#include "structures.h"
#include "window.h"

#define MAX_LEADERBOARD_ENTRIES 10
#define LEADERBOARD_FILE "leaderboard.txt"


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


// Function to save score to leaderboard
void saveToLeaderboard(int points, int level) {
    FILE* file = fopen(LEADERBOARD_FILE, "a+");
    if (file == NULL) {
        perror("Error opening leaderboard file");
        return;
    }

    // Get current time
    time_t current_time;
    struct tm* time_info;
    time(&current_time);
    time_info = localtime(&current_time);

    // Format: points level timestamp
    fprintf(file, "%d %d %04d-%02d-%02d %02d:%02d:%02d\n", 
        points, 
        level,
        time_info->tm_year + 1900, 
        time_info->tm_mon + 1, 
        time_info->tm_mday,
        time_info->tm_hour, 
        time_info->tm_min, 
        time_info->tm_sec);

    fclose(file);
}

// Function to read leaderboard entries
int readLeaderboardEntries(LeaderboardEntry* entries) {
    FILE* file = fopen(LEADERBOARD_FILE, "r");
    if (file == NULL) {
        return 0;
    }

    int entry_count = 0;
    while (fscanf(file, "%d %d %s %s", 
        &entries[entry_count].points, 
        &entries[entry_count].level,
        entries[entry_count].date,
        entries[entry_count].time) == 4 && 
        entry_count < MAX_LEADERBOARD_ENTRIES) {
        entry_count++;
    }

    fclose(file);
    return entry_count;
}


#endif
