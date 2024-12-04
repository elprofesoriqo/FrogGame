#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Definicja struktury WIN
typedef struct {
    int x, y, rows, cols, color;
    WINDOW *window;
} WIN;

// Funkcja do czyszczenia okna i rysowania obramowania
void CleanWin(WIN* W, int bo) {
    wclear(W->window);
    if (bo) {
        wattron(W->window, COLOR_PAIR(W->color));
        box(W->window, 0, 0);
        wattroff(W->window, COLOR_PAIR(W->color));
    }
    wrefresh(W->window);
}

// Funkcja do inicjalizacji okna
WIN* Init(WINDOW* parent, int rows, int cols, int y, int x, int color, int bo, int delay) {
    WIN* W = (WIN*)malloc(sizeof(WIN));
    W->x = x;
    W->y = y;
    W->rows = rows;
    W->cols = cols;
    W->color = color;
    W->window = subwin(parent, rows, cols, y, x);
    CleanWin(W, bo);
    if (delay) {
        nodelay(W->window, TRUE);
    }
    wrefresh(W->window);
    return W;
}

// Funkcja startowa do inicjalizacji ncurses
WINDOW* Start() {
    initscr();
    noecho();
    curs_set(0);
    start_color();
    init_pair(1, COLOR_WHITE, COLOR_BLACK); // Biały tekst na czarnym tle
    return stdscr;
}

int main() {
    WINDOW *mainwin = Start();

    // Rozmiary i offsety
    int ROWS = 20, COLS = 40, OFFY = 2, OFFX = 2;

    // Tworzenie okna gry i statusu
    WIN* playwin = Init(mainwin, ROWS, COLS, OFFY, OFFX, 1, 1, 1);
    WIN* statwin = Init(mainwin, 3, COLS, ROWS + OFFY, OFFX, 1, 1, 0);

    // Wyświetlenie przykładowych danych
    mvwprintw(playwin->window, 1, 1, "Welcome to the game!");
    mvwprintw(statwin->window, 1, 1, "Status window");

    wrefresh(playwin->window);
    wrefresh(statwin->window);

    // Czekanie na klawisz
    getch();

    // Czyszczenie pamięci
    delwin(playwin->window);
    delwin(statwin->window);
    delwin(mainwin);
    endwin();
    return EXIT_SUCCESS;
}
