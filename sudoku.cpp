#include "sudoku.h"

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

template <typename T>
class Iter {
public:
    virtual ~Iter() {}
    virtual bool hasNext() const = 0;
    virtual T next()             = 0;
};

class RowCellIter : public Iter<RC> {
    const int r;
    int c = 0;

public:
    RowCellIter(int r_) : r(r_) {}
    bool hasNext() const { return c < 9; };
    RC next() { return {r, c++}; }
};

class ColumnCellIter : public Iter<RC> {
    const int c;
    int r = 0;

public:
    ColumnCellIter(int c_) : c(c_) {}
    bool hasNext() const { return r < 9; };
    RC next() { return {r++, c}; }
};

class BlockCellIter : public Iter<RC> {
    const int br;
    const int bc;
    int n = 0;

public:
    BlockCellIter(int b_) : br(b_ / 3 * 3), bc(b_ % 3 * 3) {}
    bool hasNext() const { return n < 9; };
    RC next() {
        RC rc = {br + n / 3, bc + n % 3};
        n++;
        return rc;
    }
};

class SiblingIter : public Iter<RC> {
    const RC rc;
    RC nextrc;
    int iteridx = 0;
    std::unique_ptr<Iter<RC>> iter;

    RC findnextrc() {
        if (iter && iter->hasNext()) {
            RC nextrc = iter->next();
            if (nextrc.r != rc.r || nextrc.c != rc.c) return nextrc;
            return findnextrc();
        } else if (iteridx < 3) {
            iteridx++;
            if (iteridx == 1) iter = std::make_unique<RowCellIter>(rc.r);
            if (iteridx == 2) iter = std::make_unique<ColumnCellIter>(rc.c);
            if (iteridx == 3) iter = std::make_unique<BlockCellIter>(rc.r / 3 * 3 + rc.c / 3);
            return findnextrc();
        } else {
            return RCNil;
        }
    }

public:
    SiblingIter(RC rc_) : rc(rc_) { nextrc = findnextrc(); }
    bool hasNext() const { return nextrc.r != RCNil.r; };
    RC next() {
        RC rc  = nextrc;
        nextrc = findnextrc();
        return rc;
    }
};

class AllCellIter : public Iter<RC> {
    int iteridx = 0;
    std::unique_ptr<RowCellIter> iter;

public:
    bool hasNext() const { return iteridx == 0 || iter->hasNext(); };
    RC next() {
        if (iteridx == 0) iter = std::make_unique<RowCellIter>(iteridx++);
        RC rc = iter->next();
        if (!iter->hasNext() && iteridx < 9) iter = std::make_unique<RowCellIter>(iteridx++);
        return rc;
    }
};

class IterIter : public Iter<std::unique_ptr<Iter<RC>>> {
    int iteridx = 0;

public:
    bool hasNext() const { return iteridx < 27; };
    std::unique_ptr<Iter<RC>> next() {
        if (iteridx < 9) return std::make_unique<RowCellIter>(iteridx++);
        if (iteridx < 18) return std::make_unique<ColumnCellIter>((iteridx++) - 9);
        return std::make_unique<BlockCellIter>((iteridx++) - 18);
    }
};

inline static int rc2b(RC rc) { return rc.r / 3 * 3 + rc.c / 3; }

bool Sudoku::calc_only_one_candidate_in_cell(Sudoku &dst) const {
    AllCellIter allCellIter;
    while (allCellIter.hasNext()) {
        RC rc      = allCellIter.next();
        Cell &cell = dst.cellref(rc);
        if (cell.done()) continue;
        if (!cell.onlyOneCandidate()) continue;

        bool anychange = false;
        SiblingIter siblings(rc);
        while (siblings.hasNext()) {
            RC sibling = siblings.next();
            if (!dst.cellref(sibling).intersect_c(cell)) continue;

            anychange = true;
            dst.cellref(sibling).exclude_c(cell);
        }

        cell.markDone();

        if (anychange) {
            dst.reasons[0] = rc;
            return true;
        }
    }

    return false;
}

bool Sudoku::calc_only_one_candidate_in_row_or_column_or_block(Sudoku &dst) const {
    for (int n = 1; n <= 9; n++) {
        IterIter iterIter;
        while (iterIter.hasNext()) {
            auto iter = iterIter.next();

            int cnt   = 0;
            RC lastrc = RCNil;

            while (iter->hasNext()) {
                RC rc = iter->next();

                if (dst.cellref(rc).intersect_n(n)) {
                    cnt++;
                    lastrc = rc;
                }

                if (cnt >= 2) break;
            }

            if (cnt == 1 && !dst.cellref(lastrc).onlyOneCandidate()) {
                dst.cellref(lastrc).includeOnly_n(n);
                dst.reasons[0] = lastrc;
                return true;
            }
        }
    }

    return false;
}

bool Sudoku::calc_block_candidates_in_same_row_or_column(Sudoku &dst) const {
    for (int b = 0; b < 9; b++) {
        for (int n = 1; n <= 9; n++) {
            std::vector<RC> rcs;
            BlockCellIter iter(b);
            while (iter.hasNext()) {
                RC rc = iter.next();
                if (!dst.cellref(rc).intersect_n(n)) continue;
                rcs.push_back(rc);
            }

            if (rcs.size() <= 1) continue;

            int r = rcs[0].r;
            if (std::all_of(rcs.begin() + 1, rcs.end(), [r](RC rc) { return rc.r == r; })) {
                bool anychange = false;
                RowCellIter rowiter(r);
                while (rowiter.hasNext()) {
                    RC rcrc = rowiter.next();
                    if (rc2b(rcrc) != b && dst.cellref(rcrc).intersect_n(n)) {
                        dst.cellref(rcrc).exclude_n(n);
                        anychange = true;
                    }
                }
                if (anychange) {
                    for (int i = 0; i < rcs.size() && i < 4; i++) dst.reasons[i] = rcs[i];
                    return true;
                }
            }

            int c = rcs[0].c;
            if (std::all_of(rcs.begin() + 1, rcs.end(), [c](RC rc) { return rc.c == c; })) {
                bool anychange = false;
                ColumnCellIter coliter(c);
                while (coliter.hasNext()) {
                    RC rcrc = coliter.next();
                    if (rc2b(rcrc) != b && dst.cellref(rcrc).intersect_n(n)) {
                        dst.cellref(rcrc).exclude_n(n);
                        anychange = true;
                    }
                }
                if (anychange) {
                    for (int i = 0; i < rcs.size() && i < 4; i++) dst.reasons[i] = rcs[i];
                    return true;
                }
            }
        }
    }

    return false;
}

bool Sudoku::calc_row_candidates_in_same_block(Sudoku &dst) const {
    for (int r = 0; r < 9; r++) {
        for (int n = 1; n <= 9; n++) {
            int b = -1;
            RowCellIter rowiter(r);
            while (rowiter.hasNext()) {
                RC rc = rowiter.next();
                if (dst.cellref(rc).intersect_n(n)) {
                    if (b == -1) b = rc2b(rc);
                    if (b != rc2b(rc)) b = -2;
                }
            }
            if (0 <= b && b < 9) {
                BlockCellIter blockiter(b);
                while (blockiter.hasNext()) {
                    RC rcrc = blockiter.next();
                    if (rcrc.r != r && dst.cellref(rcrc).intersect_n(n)) {
                        dst.cellref(rcrc).exclude_n(n);
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

bool Sudoku::calc_column_candidates_in_same_block(Sudoku &dst) const {
    for (int c = 0; c < 9; c++) {
        for (int n = 1; n <= 9; n++) {
            int b = -1;
            ColumnCellIter coliter(c);
            while (coliter.hasNext()) {
                RC rc = coliter.next();
                if (dst.cellref(rc).intersect_n(n)) {
                    if (b == -1) b = rc2b(rc);
                    if (b != rc2b(rc)) b = -2;
                }
            }
            if (0 <= b && b < 9) {
                BlockCellIter blockiter(b);
                while (blockiter.hasNext()) {
                    RC rcrc = blockiter.next();
                    if (rcrc.c != c && dst.cellref(rcrc).intersect_n(n)) {
                        dst.cellref(rcrc).exclude_n(n);
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

bool Sudoku::calc_isolate2(Sudoku &dst) const {
    IterIter iteriter;
    IterIter iteriter2;
    IterIter iteriter3;
    while (iteriter.hasNext()) {
        auto iter  = iteriter.next();
        auto iter2 = iteriter2.next();
        auto iter3 = iteriter3.next();

        while (iter->hasNext()) {
            RC rc = iter->next();

            unsigned short candidates1 = dst.cellref(rc).candidates();

            if (Cell::cntCandidates(candidates1) == 1) continue;
            if (Cell::cntCandidates(candidates1) > 2) continue;

            while (iter2->hasNext()) {
                RC rc2 = iter2->next();
                if (RC::cmp(rc, rc2) >= 0) continue;
                unsigned short candidates2 = candidates1 | dst.cellref(rc2).candidates();
                if (Cell::cntCandidates(candidates2) == 1) continue;
                if (Cell::cntCandidates(candidates2) > 2) continue;

                while (iter3->hasNext()) {
                    RC rc3 = iter3->next();
                    if (RC::cmp(rc, rc3) >= 0) continue;
                    if (RC::cmp(rc2, rc3) >= 0) continue;
                    if (dst.cellref(rc3).intersect_v(candidates2)) {
                        dst.cellref(rc3).exclude_v(candidates2);
                        dst.reasons[0] = rc;
                        dst.reasons[1] = rc2;
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

bool Sudoku::calc_isolate3(Sudoku &dst) const {
    IterIter iteriter;
    IterIter iteriter2;
    IterIter iteriter3;
    IterIter iteriter4;
    while (iteriter.hasNext()) {
        auto iter  = iteriter.next();
        auto iter2 = iteriter2.next();
        auto iter3 = iteriter3.next();
        auto iter4 = iteriter4.next();

        while (iter->hasNext()) {
            RC rc = iter->next();

            unsigned short candidates1 = dst.cellref(rc).candidates();
            if (Cell::cntCandidates(candidates1) == 1) continue;
            if (Cell::cntCandidates(candidates1) > 3) continue;

            while (iter2->hasNext()) {
                RC rc2 = iter2->next();
                if (RC::cmp(rc, rc2) >= 0) continue;
                if (dst.cellref(rc2).onlyOneCandidate()) continue;

                unsigned short candidates2 = candidates1 | dst.cellref(rc2).candidates();
                if (Cell::cntCandidates(candidates2) == 1) continue;
                if (Cell::cntCandidates(candidates2) > 3) continue;

                while (iter3->hasNext()) {
                    RC rc3 = iter3->next();
                    if (RC::cmp(rc, rc3) >= 0) continue;
                    if (RC::cmp(rc2, rc3) >= 0) continue;
                    if (dst.cellref(rc3).onlyOneCandidate()) continue;

                    unsigned short candidates3 = candidates2 | dst.cellref(rc3).candidates();
                    if (Cell::cntCandidates(candidates3) == 1) continue;
                    if (Cell::cntCandidates(candidates3) > 3) continue;

                    while (iter4->hasNext()) {
                        RC rc4 = iter4->next();
                        if (RC::cmp(rc, rc4) >= 0) continue;
                        if (RC::cmp(rc2, rc4) >= 0) continue;
                        if (RC::cmp(rc3, rc4) >= 0) continue;
                        if (dst.cellref(rc4).onlyOneCandidate()) continue;
                        if (dst.cellref(rc4).intersect_v(candidates3)) {
                            dst.cellref(rc4).exclude_v(candidates3);
                            dst.reasons[0] = rc;
                            dst.reasons[1] = rc2;
                            dst.reasons[2] = rc3;
                            return true;
                        }
                    }
                }
            }
        }
    }

    return false;
}

bool Sudoku::calcOneStep(Sudoku &dst) const {
    dst.clear_reasons();

    if (calc_only_one_candidate_in_cell(dst)) return true;

    if (calc_only_one_candidate_in_row_or_column_or_block(dst)) return true;

    if (calc_block_candidates_in_same_row_or_column(dst)) return true;

    if (calc_row_candidates_in_same_block(dst)) return true;

    if (calc_column_candidates_in_same_block(dst)) return true;

    if (calc_isolate2(dst)) return true;

    if (calc_isolate3(dst)) return true;

    return false;
}