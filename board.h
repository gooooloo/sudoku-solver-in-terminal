#ifndef BOARD_H
#define BOARD_H

#include "screen.h"
#include "sudoku.h"

class Board {
public:
    Board(Screen &screen_) : screen(screen_) {}
    void draw(const Sudoku &model);

private:
    Screen &screen;
};

#endif  // BOARD_H
