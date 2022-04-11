#include <vector>
#include "board.h"
#include "screen.h"
#include "sudoku.h"

int main(int argc, char *argv[]) {
    Screen screen;

    Sudoku sudoku(
        " 5  8 91 "
        "  2    6 "
        "   5 13  "
        "   9     "
        "   7   8 "
        "7 5  6  4"
        "         "
        "  9  5176"
        "3 4  2   ");

    Board board(screen);
    board.draw(sudoku);

    std::vector<Sudoku> v{sudoku};
    while (true) {
        char ch = (char)getch();

        if (ch == 'h' && v.size() > 1) {
            v.pop_back();
            board.draw(*v.rbegin());
        } else if (ch == 'l') {
            Sudoku next(*v.rbegin());
            if (!sudoku.calcOneStep(next)) continue;
            v.emplace_back(std::move(next));
            board.draw(*v.rbegin());
        } else if (ch == 'a') {
            while (true) {
                Sudoku next(*v.rbegin());
                if (!sudoku.calcOneStep(next)) break;
                v.emplace_back(std::move(next));
            }
            board.draw(*v.rbegin());
        }
    }

    return 0;
}
