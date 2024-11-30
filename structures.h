#ifndef structures_h
#define structures_h


typedef struct {
    char color[1];
    int x, y;
} Frog;



//auto
typedef struct {
    char color[1];
    int x, y; //pozycje
    int speed;
} Car;


// Tablica kolorów
const char* color_map[] = {"R", "B", "G", "Y", "C", "M"};  // Kolory: R=Red, B=Blue, G=Green, Y=Yellow, C=Cyan, M=Magenta

#endif
