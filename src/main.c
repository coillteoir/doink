#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>

typedef struct{
    size_t x;
    size_t y;
} Cursor;

int main(int argc, char **argv)
{
    initscr();
    printw("Hello world");
    refresh();
    getch();
    endwin();
}
