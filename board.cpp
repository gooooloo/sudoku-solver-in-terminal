#include "board.h"

#include <locale.h>
#include <string.h>
#include <unistd.h>

#include <functional>
#include <vector>

#include "screen.h"

// https://unicode-table.com/en/blocks/box-drawing/
const static char *HORIZENTAL         = "\u2500";
const static char *HORIZENTAL_THICK   = "\u2550";
const static char *VIRTICAL           = "\u2502";
const static char *VIRTICAL_THICK     = "\u2551";
const static char *LEFT_UP_THICK      = "\u2554";
const static char *RIGHT_UP_THICK     = "\u2513";
const static char *LEFT_BOTTOM_THICK  = "\u2517";
const static char *RIGHT_BOTTOM_THICK = "\u251B";
const static char *UP                 = "\u252C";
const static char *UP_THICK           = "\u2566";
const static char *BOTTOM             = "\u2534";
const static char *BOTTOM_THICK       = "\u2569";
const static char *LEFT               = "\u251C";
const static char *LEFT_THICK         = "\u2560";
const static char *RIGHT              = "\u2524";
const static char *RIGHT_THICK        = "\u2563";
const static char *CROSS              = "\u253C";
const static char *CROSS_THICK        = "\u256C";
const static char *BLANK              = " ";

void Board::draw(const Sudoku &sudoku) {
    const int ROW_N0 = 3;
    const int ROW_N1 = ROW_N0 + 1;
    const int ROW_N2 = 3 * ROW_N1;
    const int ROW_N3 = 3 * ROW_N2;

    const int COL_N0 = 5;
    const int COL_N1 = COL_N0 + 1;
    const int COL_N2 = 3 * COL_N1;
    const int COL_N3 = 3 * COL_N2;

    for (int r = 0; r <= ROW_N3; r++) {
        for (int c = 0; c <= COL_N3; c++) {
            if (false)
                ; /* empty to align codes */

            else if (r == 0 && c == 0)
                screen.draw(r, c, LEFT_UP_THICK, BG_COLOR_DEFAULT);
            else if (r == 0 && c == COL_N3)
                screen.draw(r, c, RIGHT_UP_THICK, BG_COLOR_DEFAULT);
            else if (r == 0 && (c % COL_N2) == 0)
                screen.draw(r, c, UP_THICK, BG_COLOR_DEFAULT);
            else if (r == 0 && (c % COL_N1) == 0)
                screen.draw(r, c, UP_THICK, BG_COLOR_DEFAULT);
            else if (r == 0)
                screen.draw(r, c, HORIZENTAL_THICK, BG_COLOR_DEFAULT);

            else if (r == ROW_N3 && c == 0)
                screen.draw(r, c, LEFT_BOTTOM_THICK, BG_COLOR_DEFAULT);
            else if (r == ROW_N3 && c == COL_N3)
                screen.draw(r, c, RIGHT_BOTTOM_THICK, BG_COLOR_DEFAULT);
            else if (r == ROW_N3 && (c % COL_N2) == 0)
                screen.draw(r, c, BOTTOM_THICK, BG_COLOR_DEFAULT);
            else if (r == ROW_N3 && (c % COL_N1) == 0)
                screen.draw(r, c, BOTTOM_THICK, BG_COLOR_DEFAULT);
            else if (r == ROW_N3)
                screen.draw(r, c, HORIZENTAL_THICK, BG_COLOR_DEFAULT);

            else if ((r % ROW_N2) == 0 && c == 0)
                screen.draw(r, c, LEFT_THICK, BG_COLOR_DEFAULT);
            else if ((r % ROW_N2) == 0 && c == COL_N3)
                screen.draw(r, c, RIGHT_THICK, BG_COLOR_DEFAULT);
            else if ((r % ROW_N2) == 0 && (c % COL_N2) == 0)
                screen.draw(r, c, CROSS_THICK, BG_COLOR_DEFAULT);
            else if ((r % ROW_N2) == 0 && (c % COL_N1) == 0)
                screen.draw(r, c, CROSS_THICK, BG_COLOR_DEFAULT);
            else if ((r % ROW_N2) == 0)
                screen.draw(r, c, HORIZENTAL_THICK, BG_COLOR_DEFAULT);

            else if ((r % ROW_N1) == 0 && c == 0)
                screen.draw(r, c, LEFT_THICK, BG_COLOR_DEFAULT);
            else if ((r % ROW_N1) == 0 && c == COL_N3)
                screen.draw(r, c, RIGHT_THICK, BG_COLOR_DEFAULT);
            else if ((r % ROW_N1) == 0 && (c % COL_N2) == 0)
                screen.draw(r, c, CROSS_THICK, BG_COLOR_DEFAULT);
            else if ((r % ROW_N1) == 0 && (c % COL_N1) == 0)
                screen.draw(r, c, CROSS, BG_COLOR_DEFAULT);
            else if ((r % ROW_N1) == 0)
                screen.draw(r, c, HORIZENTAL, BG_COLOR_DEFAULT);

            else if ((c % COL_N2) == 0)
                screen.draw(r, c, VIRTICAL_THICK, BG_COLOR_DEFAULT);
            else if ((c % COL_N1) == 0)
                screen.draw(r, c, VIRTICAL, BG_COLOR_DEFAULT);

            else {
                int rr        = r / ROW_N1;
                int cc        = c / COL_N1;
                bool done     = sudoku.onlyOneCandidate(rr, cc);
                bool original = sudoku.original(rr, cc);

                auto color_index = [&](int n) {
                    bool isreason = sudoku.isreason(rr, cc, n);

                    if (original && !isreason)
                        return COLOR_INDEX_RED;
                    else if (original && isreason)
                        return COLOR_INDEX_RED_HIGHLIGHT;
                    else if (done && !isreason)
                        return COLOR_INDEX_YELLOW;
                    else if (done && isreason)
                        return COLOR_INDEX_YELLOW_HIGHLIGHT;
                    else if (isreason)
                        return COLOR_INDEX_DEFAULT_HIGHLIGHT;
                    else
                        return COLOR_INDEX_DEFAULT_COLOR;
                };

                if (false)
                    ; /* empty to align codes */

                else if (r % ROW_N1 == 1 && c % COL_N1 == 1 && sudoku.possible(r / ROW_N1, c / COL_N1, 1))
                    screen.draw(r, c, "1", color_index(1));
                else if (r % ROW_N1 == 1 && c % COL_N1 == 3 && sudoku.possible(r / ROW_N1, c / COL_N1, 2))
                    screen.draw(r, c, "2", color_index(2));
                else if (r % ROW_N1 == 1 && c % COL_N1 == 5 && sudoku.possible(r / ROW_N1, c / COL_N1, 3))
                    screen.draw(r, c, "3", color_index(3));
                else if (r % ROW_N1 == 2 && c % COL_N1 == 1 && sudoku.possible(r / ROW_N1, c / COL_N1, 4))
                    screen.draw(r, c, "4", color_index(4));
                else if (r % ROW_N1 == 2 && c % COL_N1 == 3 && sudoku.possible(r / ROW_N1, c / COL_N1, 5))
                    screen.draw(r, c, "5", color_index(5));
                else if (r % ROW_N1 == 2 && c % COL_N1 == 5 && sudoku.possible(r / ROW_N1, c / COL_N1, 6))
                    screen.draw(r, c, "6", color_index(6));
                else if (r % ROW_N1 == 3 && c % COL_N1 == 1 && sudoku.possible(r / ROW_N1, c / COL_N1, 7))
                    screen.draw(r, c, "7", color_index(7));
                else if (r % ROW_N1 == 3 && c % COL_N1 == 3 && sudoku.possible(r / ROW_N1, c / COL_N1, 8))
                    screen.draw(r, c, "8", color_index(8));
                else if (r % ROW_N1 == 3 && c % COL_N1 == 5 && sudoku.possible(r / ROW_N1, c / COL_N1, 9))
                    screen.draw(r, c, "9", color_index(9));

                else
                    screen.draw(r, c, BLANK, BG_COLOR_DEFAULT);
            }
        }
    }
}
