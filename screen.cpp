#include "screen.h"

#include <locale.h>

void Screen::init() {
    setlocale(LC_ALL, "");
    initscr();
    noecho();
    curs_set(0);
    start_color();
    init_pair(COLOR_INDEX_DEFAULT_HIGHLIGHT, COLOR_WHITE, BG_COLOR_CANDIDATE);

    init_pair(COLOR_INDEX_YELLOW, COLOR_YELLOW, BG_COLOR_DEFAULT);
    init_pair(COLOR_INDEX_YELLOW_HIGHLIGHT, COLOR_YELLOW, COLOR_BLUE);

    init_pair(COLOR_INDEX_RED, COLOR_RED, BG_COLOR_DEFAULT);
    init_pair(COLOR_INDEX_RED_HIGHLIGHT, COLOR_RED, BG_COLOR_CANDIDATE);

    init_pair(COLOR_INDEX_DEFAULT_COLOR_CANDIDATE, COLOR_WHITE, BG_COLOR_CANDIDATE);
    init_pair(COLOR_INDEX_DEFAULT_COLOR_TARGET, COLOR_WHITE, BG_COLOR_TARGET);
}

void Screen::destroy() { endwin(); }

void Screen::draw(int r, int c, const char *str, int color_index) {
    if (!perf_cache.isCached(r, c, str, color_index)) {
        if (color_index != COLOR_INDEX_DEFAULT_COLOR) attron(COLOR_PAIR(color_index));
        mvprintw(r, c, str);
        if (color_index != COLOR_INDEX_DEFAULT_COLOR) attroff(COLOR_PAIR(color_index));

        perf_cache.setCached(r, c, str, color_index);
    }
}

int Screen::get_color_index(bool is_red, int bg_color) {
    if (is_red) {
        if (bg_color == BG_COLOR_SELECTED) return COLOR_INDEX_YELLOW;
        if (bg_color == BG_COLOR_CANDIDATE) return COLOR_INDEX_RED_HIGHLIGHT;
        if (bg_color == BG_COLOR_TARGET) return COLOR_INDEX_YELLOW_HIGHLIGHT;
        return COLOR_INDEX_RED;
    } else {
        if (bg_color == BG_COLOR_SELECTED) return COLOR_INDEX_DEFAULT_HIGHLIGHT;
        if (bg_color == BG_COLOR_CANDIDATE) return COLOR_INDEX_DEFAULT_COLOR_CANDIDATE;
        if (bg_color == BG_COLOR_TARGET) return COLOR_INDEX_DEFAULT_COLOR_TARGET;
        return COLOR_INDEX_DEFAULT_COLOR;
    }
}

void Screen::clear_line(int r) {
    move(r, 0);
    clrtoeol();

    perf_cache.clearRowCache(r);
}

void Screen::clear_screen() {
    clear();
    perf_cache.clearCache();
}

int Screen::width() { return COLS; }