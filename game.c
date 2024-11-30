#include <stdio.h>
#include <ncurses.h>
#include <stdlib.h>

// Definicje stałych
#define WINDOW_HEIGHT 20  // Minimalna wysokość okna
#define PLAY_WIDTH 40         // Szerokość okna gry
#define STATS_WIDTH 30        // Szerokość okna statusu
#define MIN_WIDTH 50          // Minimalna szerokość ekranu
#define MIN_HEIGHT 25         // Minimalna wysokość ekranu

// Struktura przechowująca informacje o oknie
typedef struct {
    int width;
    int height;
    int startx;
    int starty;
    int color;
    WINDOW *win;
} Window;

// Prototyp funkcji CleanWin
void CleanWin(Window* W, int bo);

// Funkcja inicjalizująca nowe okno
Window* Init(WINDOW* parent, int y, int x, int width, int color, int bo, int delay) {
    Window* W = (Window*)malloc(sizeof(Window));
    W->startx = x;
    W->starty = y;
    W->width = width;  // Ustalamy szerokość w zależności od okna
    W->height = WINDOW_HEIGHT;  // Ustawiamy wysokość okna
    W->color = color;
    W->win = subwin(parent, W->height, W->width, W->starty, W->startx);
    CleanWin(W, bo);
    if (delay) {
        nodelay(W->win, TRUE);
    }
    wrefresh(W->win);
    return W;
}

// Funkcja do czyszczenia okna i rysowania obramowania
void CleanWin(Window* W, int bo) {
    wclear(W->win);
    if (bo) {
        wattron(W->win, COLOR_PAIR(W->color));
        box(W->win, 0, 0);
        wattroff(W->win, COLOR_PAIR(W->color));
    }
    wrefresh(W->win);
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


    // Inicjalizacja ncurses
    WINDOW* mainwin = Start();
    
    // Inicjalizacja okna gry i okna statusu obok siebie
    Window* playwin = Init(mainwin, 0, 0, PLAY_WIDTH,1, 1, 1);  // Okno gry na początek (0,0)
    Window* statwin = Init(mainwin, 0, PLAY_WIDTH + 2, STATS_WIDTH,1, 1, 0);  // Okno statusu obok z przesunięciem o PLAY_WIDTH + 2

    // Program może działać dalej, dodaj logikę gry
    
    // Zakończenie ncurses
    getch();
    endwin();
    return 0;
}
