#ifndef SUDOKU_H
#define SUDOKU_H

#include <memory>
#include <string>

struct RC {
    int r;
    int c;

    inline static int cmp(RC rc1, RC rc2) { return (rc1.r != rc2.r) ? rc1.r - rc2.r : rc1.c - rc2.c; }
};

static constexpr RC RCNil{-1, -1};

class Cell {
private:
    constexpr static unsigned short V_MASK        = (1 << 9) - 1;
    constexpr static unsigned short DONE_MASK     = 1 << 15;
    constexpr static unsigned short ORIGINAL_MASK = 1 << 14;

    unsigned short v = V_MASK;

    inline static unsigned short n2v(int n) { return 1 << (n - 1); }

public:
    inline bool done() const { return (v & DONE_MASK) == DONE_MASK; }
    inline void markDone() { v |= DONE_MASK; }

    inline bool original() const { return (v & ORIGINAL_MASK) == ORIGINAL_MASK; }
    inline void markOriginal() { v |= ORIGINAL_MASK; }

    inline unsigned short candidates() const { return v & V_MASK; }

    inline bool intersect_n(unsigned short n) const { return (candidates() & n2v(n)) != 0; }
    inline bool intersect_v(unsigned short v) const { return (candidates() & v) != 0; }
    inline bool intersect_c(const Cell &c) const { return (candidates() & c.candidates()) != 0; }

    inline static int cntCandidates(unsigned int candidates) {
        int cnt = 0;
        for (unsigned short c = candidates; c; cnt++) c &= c - 1;
        return cnt;
    }
    inline bool onlyOneCandidate() const { return cntCandidates(candidates()) == 1; }

    inline void exclude_n(int n) { v &= ~n2v(n); }
    inline void exclude_v(int vv) { v &= ~vv; }
    inline void exclude_c(const Cell &c) { v &= ~c.candidates(); }
    inline void includeOnly_n(int n) { v = n2v(n); }
};

class Sudoku {
    Cell cells[9][9];
    RC reasons[4];

    inline Cell &cellref(const RC &rc) { return cells[rc.r][rc.c]; }

    bool calc_only_one_candidate_in_cell(Sudoku &dst) const;
    bool calc_only_one_candidate_in_row_or_column_or_block(Sudoku &dst) const;

    bool calc_block_candidates_in_same_row_or_column(Sudoku &dst) const;
    bool calc_row_candidates_in_same_block(Sudoku &dst) const;
    bool calc_column_candidates_in_same_block(Sudoku &dst) const;

    bool calc_isolate2(Sudoku &dst) const;
    bool calc_isolate3(Sudoku &dst) const;

    inline void clear_reasons() {
        for (int i = 0; i < 4; i++) reasons[i] = RCNil;
    }

public:
    inline Sudoku(const Sudoku &src) {
        memcpy(&this->cells[0][0], &src.cells[0][0], sizeof(Cell) * 81);
        memcpy(&this->reasons[0], &src.reasons[0], sizeof(RC) * 4);
    }

    inline Sudoku(const char *str) {
        int len = strlen(str);
        for (int i = 0; i < len && i < 81; i++)
            if ('1' <= str[i] && str[i] <= '9') {
                cells[i / 9][i % 9].includeOnly_n(str[i] - '0');
                cells[i / 9][i % 9].markOriginal();
            }

        clear_reasons();
    }

    inline bool possible(int r, int c, int n) const { return cells[r][c].intersect_n(n); }
    inline bool onlyOneCandidate(int r, int c) const { return cells[r][c].onlyOneCandidate(); }
    inline bool original(int r, int c) const { return cells[r][c].original(); }
    inline bool isreason(int r, int c) const {
        RC rc{r, c};
        for (int i = 0; i < 4; i++)
            if (RC::cmp(reasons[i], rc) == 0) return true;
        return false;
    }

    bool calcOneStep(Sudoku &dst) const;
};

#endif  // SUDOKU_H