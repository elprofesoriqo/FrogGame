//using from demo game window structure & function 

#include <stdio.h>
#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>


//pliki
#include "structures.h"
#include "window.h"
#include "fileprocessing.h"
#include "cars_trees.h"




// Prototypy funkcji
WINDOW* Start();
void displayGameOverScreen(int game_over_reason, GameState* game_state);
int processPlayerInput(Frog* frog, Tree* trees, int ch, int* key_state);
void moveFrog(Frog* frog, int ch, Tree* trees);
void displayGameOverScreenStats(GameState* game_state);
int processGameIteration(Window* playwin, Window* statwin, Car cars[], Frog* frog, 
                         Tree trees[], int key_state[], GameState* game_state, int* game_running);
int initializeAndRunGameLoop(WINDOW* mainwin, GameState* game_state);

//Inicjalizacja pierwszej żaby
void initFrog(Frog* frog, int play_width) {
    frog->x = play_width / 2;
    frog->y = WINDOW_HEIGHT - 2;
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

    //Sprawdzanie potencjalnej kolizji z drzewem
    int can_move = 1;
    for (int i = 0; i < MAX_TREES; i++) {
        if (trees[i].active && trees[i].x == new_x && trees[i].y == new_y) {
            can_move = 0;
            break;
        }
    }

    //Przesunięcie żaby
    if (can_move) {
        frog->x = new_x;
        frog->y = new_y;
    }
}



//Aktualizacja pozycji żaby
void drawFrog(Frog* frog, Window* playwin) {
    wattron(playwin->win, COLOR_PAIR(frog->color));
    mvwaddch(playwin->win, frog->y, frog->x, frog->symbol);
    wattroff(playwin->win, COLOR_PAIR(frog->color));
}




//Sprawdzane kolizji
int checkCollision(Frog* frog, Car* cars) {
    for (int i = 0; i < MAX_CARS; i++) {
        if (cars[i].active && 
            cars[i].y == frog->y && 
            (cars[i].x == frog->x || cars[i].x == frog->x + 1 || cars[i].x == frog->x - 1) &&
            //ignorujemy bonus & friendly
            cars[i].type != CAR_BONUS &&
            cars[i].type != CAR_FRIENDLY) {
            return 1;
        }
    }
    return 0;
}


 // Sortowanie listy leaderboard po punktach
void sortLeaderboardEntries(LeaderboardEntry entries[], int entry_count) {
    for (int i = 0; i < entry_count - 1; i++) {
        for (int j = 0; j < entry_count - i - 1; j++) {
            if (entries[j].points < entries[j+1].points) {
                LeaderboardEntry temp = entries[j];
                entries[j] = entries[j+1];
                entries[j+1] = temp;
            }
        }
    }
}


void displayLeaderboard() {
    LeaderboardEntry entries[MAX_LEADERBOARD_ENTRIES];
    int entry_count = readLeaderboardEntries(entries); // ilość pozycji w pliku
    sortLeaderboardEntries(entries, entry_count);

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

    if (entry_count == 0) {
        mvprintw(LINES/2, COLS/2 - 10, "No scores yet!");
    }

    mvprintw(LINES - 3, COLS/2 - 15, "Press 'ENTER' to continue...");
    refresh();
    
    // Czekamy na klawisz aby wznowić grę
    nodelay(stdscr, FALSE);
    int ch;
    while ((ch = getch()) != '\n' && ch != '\r') {
        // klawisz
    }

    clear();
    refresh();
    nodelay(stdscr, TRUE);
}



void stats(Window * statwin, int remaining_time, GameState * game_state) {
    mvwprintw(statwin->win, 2, 2, "Level: %d", game_state->level);
    mvwprintw(statwin->win, 3, 2, "Max Cars: %d", BASE_LEVEL_CARS + (game_state->level - 1) * CARS_PER_LEVEL);
    mvwprintw(statwin->win, 5, 2, "Time Left: %d sec", remaining_time);
    mvwprintw(statwin->win, 6, 2, "Points: %d", game_state->total_points);
    mvwprintw(statwin->win, 7, 2, "Igor Jankowski s203396");
}


int timer(time_t start_time, int current_level) {
    time_t current_time = time(NULL);
    //podstawowy czas dla poziomu (bazowy czas + 10 sekund na każdy poziom powyżej pierwszego)
    int base_time = BASE_TIME + (current_level - 1) * 10; 

    //  czas, który upłynął od statru poziomu & pozostały
    int elapsed_time = (int)difftime(current_time, start_time);
    int remaining_time = base_time - elapsed_time;

    return (remaining_time > 0) ? remaining_time : 0;
}


//użycie structa od gry i jej start
void initializeGameState(GameState* game_state, int initial_level) {
    game_state->level = initial_level;
    game_state->total_points = 0;
    game_state->bonus_car_spawned = 0;
    game_state->bonus_time = 0;
    game_state->start_time = time(NULL);
}

// punkty są ilościa czasu pozostałego po każdym lvl 
int calculateLevelPoints(int remaining_time) {
    return remaining_time > 0 ? remaining_time : 0;
}



void updateGameLevel(GameState* game_state, Frog* frog, Car* cars, Tree* trees, int remaining_time) {
    int level_points = calculateLevelPoints(remaining_time);
    game_state->total_points += level_points; // Dodanie punktów do całkowitego wyniku gracza

    game_state->level++;

    // Inicjalizacja żaby & aut & drzew w nowym lvl
    initFrog(frog, PLAY_WIDTH);
    initializeCars(cars, WINDOW_HEIGHT, game_state->level);
    initializeTrees(trees, WINDOW_HEIGHT);

    // Reset czasu
    game_state->start_time = time(NULL);

    // Resetowanie stanu bonusów na nowym poziomie
    game_state->bonus_car_spawned = 0;
    game_state->bonus_time = 0;

    // Wyświetlenie komunikatu o przejściu na nowy poziom
    clear();
    mvprintw(LINES/2, COLS/2 - 10, "Next Level: %d!", game_state->level);
    mvprintw(LINES/2 + 1, COLS/2 - 10, "Points: +%d", level_points);
    refresh();
    
    napms(500);
    clear();
    refresh();
}



// resetowanie okien
void resetGameWindows(Window* playwin, Window* statwin) {
    werase(playwin->win);
    werase(statwin->win);
    box(playwin->win, 0, 0);
    box(statwin->win, 0, 0);
}

// specjalne klawisze (q,l)
int handleSpecialKeys(int ch, Window* playwin, Window* statwin) {
    switch(ch) {
        case 'q':
        case 'Q':
            return 0;
        case 'l':
        case 'L':
            displayLeaderboard();
            resetGameWindows(playwin, statwin);
            break;
    }
    return 1;
}

// mechanika gry
void updateGameMechanics(Car* cars, Frog* frog, Tree* trees, Window* playwin, GameState* game_state, int remaining_time) {
    // Spawning i poruszanie samochodów
    int friendly_car_spawned = 0;
    spawnCars(cars, WINDOW_HEIGHT, game_state->level, 
              &game_state->bonus_car_spawned, 
              game_state->start_time, 
              &friendly_car_spawned);
    
    moveCars(cars, frog, playwin);
    
    // Rysowanie obiektów
    drawCars(cars, playwin);
    drawTrees(trees, playwin);
    drawFrog(frog, playwin);
}


// warunki do zakończenia 
int checkGameEndConditions(Frog* frog, Car* cars, Tree* trees, GameState* game_state, int remaining_time) {
    //kolizja
    if (checkCollision(frog, cars)) {
        return 1;
    }

    // lvl up
    if (frog->y == 1) {
        updateGameLevel(game_state, frog, cars, trees, remaining_time);
    }

    //  czas
    if (remaining_time <= 0) {
        return 2;
    }

    return 0;  // Gra trwa dalej
}

//input
int processPlayerInput(Frog* frog, Tree* trees, int ch, int* key_state) {
    if (ch == 'q' || ch == 'Q') return 0;

    if (ch != ERR && key_state[ch] == 0) {
        switch(ch) {
            case 'w':
            case 's':
            case 'a':
            case 'd':
                moveFrog(frog, ch, trees);
                key_state[ch] = 1;
                break;
        }
    }
    return 1;
}

// Ekran końcowy
void displayGameOverScreen(int game_over_reason, GameState* game_state) {
    clear();
    switch(game_over_reason) {
        case 1:  // Kolizja samochodu
            mvprintw(LINES/2 - 2, COLS/2 - 15, "GAME OVER: Car Collision!");
            break;
        case 2:  // Koniec czasu
            mvprintw(LINES/2 - 2, COLS/2 - 15, "GAME OVER: Time Ran Out!");
            break;
        default: // Wyjście lub inne powody
            mvprintw(LINES/2 - 2, COLS/2 - 15, "GAME OVER!");
            break;
    }
    
    displayGameOverScreenStats(game_state);
}

// Wyświetlenie statystyk
void displayGameOverScreenStats(GameState* game_state){
    mvprintw(LINES/2, COLS/2 - 10, "Final Level: %d", game_state->level);
    mvprintw(LINES/2 + 1, COLS/2 - 10, "Total Points: %d", game_state->total_points);
    
    // Zapis wyniku
    saveToLeaderboard(game_state->total_points, game_state->level);
    
    // Odliczanie
    mvprintw(LINES/2 + 2, COLS/2 - 10, "Score saved to Leaderboard!");
    for (int i = 5; i > 0; i--) {
        mvprintw(LINES/2 + 3, COLS/2 - 10, "Closing in %d seconds...", i);
        refresh();
        napms(1000);
    }
}



// Inicjalizacja gry
void initializeGameResources(Window** playwin, Window** statwin, WINDOW* mainwin, 
                              Car* cars, Frog* frog, Tree* trees, GameState* game_state) {
    *playwin = Init(mainwin, 0, 0, PLAY_WIDTH, 1, 1, 1);
    *statwin = Init(mainwin, 0, PLAY_WIDTH + 2, STATS_WIDTH, 1, 1, 0);
    
    nodelay((*playwin)->win, TRUE);
    timeout(100);
    
    initializeCars(cars, WINDOW_HEIGHT, game_state->level);
    initializeTrees(trees, WINDOW_HEIGHT);
    initFrog(frog, PLAY_WIDTH);
}

// Czyszczenie zasobów gry
void cleanupGameResources(Window* playwin, Window* statwin) {
    if (playwin) {
        delwin(playwin->win);
        free(playwin);
    }
    if (statwin) {
        delwin(statwin->win);
        free(statwin);
    }
}

// Główna pętla gry
int initializeAndRunGameLoop(WINDOW* mainwin, GameState* game_state) {
    Window* playwin = NULL;
    Window* statwin = NULL;
    Car cars[MAX_CARS];
    Frog frog;
    Tree trees[MAX_TREES] = {0};
    int game_running = 1;
    int key_state[256] = {0};
    int game_over_reason = 0;

    initializeGameResources(&playwin, &statwin, mainwin, 
                            cars, &frog, trees, game_state);

    while (game_running) {
        game_over_reason = processGameIteration(playwin, statwin, cars, &frog, trees, 
                                                key_state, game_state, &game_running);
    }

    cleanupGameResources(playwin, statwin);
    return game_over_reason;
}

int processGameIteration(Window* playwin, Window* statwin, Car cars[], Frog* frog, 
                         Tree trees[], int key_state[], GameState* game_state, int* game_running) {
    int remaining_time = timer(game_state->start_time, game_state->level);
    resetGameWindows(playwin, statwin);

    stats(statwin, remaining_time, game_state);
    wrefresh(statwin->win);
    updateGameMechanics(cars, frog, trees, playwin, game_state, remaining_time);

    int ch = wgetch(playwin->win);

    // Obsługa specjalnych klawiszy
    if (!handleSpecialKeys(ch, playwin, statwin)) {
        *game_running = 0;
        return 0;
    }

    // Przetwarzanie ruchu gracza
    if (!processPlayerInput(frog, trees, ch, key_state)) {
        *game_running = 0;
        return 0;
    }

    if (ch == ERR) {
        memset(key_state, 0, sizeof(int) * 256);
    }

    // Obsługa bonusowego samochodu
    int middle_lane = WINDOW_HEIGHT / 2;
    handleBonusCar(frog, cars, middle_lane);
    // Sprawdzenie warunków zakończenia gry
    int game_over_reason = checkGameEndConditions(frog, cars, trees, game_state, remaining_time);
    if (game_over_reason) {
        *game_running = 0;
        return game_over_reason;
    }

    wrefresh(playwin->win);
    wrefresh(statwin->win);
    napms(100);

    return 0;
}



// uruchomienie gry
int runGame(WINDOW* mainwin, GameState* game_state) {
    int game_over_reason = initializeAndRunGameLoop(mainwin, game_state);
    displayGameOverScreen(game_over_reason, game_state);
    return 1;
}



int main() {
    WINDOW* mainwin = Start();
    
    GameState game_state;
    initializeGameState(&game_state, 1);
    
    runGame(mainwin, &game_state);

    endwin();
    return 0;
}
