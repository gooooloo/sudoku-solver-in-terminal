#ifndef SCREEN_H
#define SCREEN_H

#include <ncurses.h>

#define BG_COLOR_DEFAULT COLOR_BLACK
#define BG_COLOR_SELECTED COLOR_YELLOW
#define BG_COLOR_CANDIDATE COLOR_BLUE
#define BG_COLOR_TARGET COLOR_GREEN

#define COLOR_INDEX_DEFAULT_COLOR 0
#define COLOR_INDEX_RED 1
#define COLOR_INDEX_DEFAULT_HIGHLIGHT 2
#define COLOR_INDEX_YELLOW 3
#define COLOR_INDEX_DEFAULT_COLOR_CANDIDATE 4
#define COLOR_INDEX_RED_HIGHLIGHT 5
#define COLOR_INDEX_DEFAULT_COLOR_TARGET 6
#define COLOR_INDEX_YELLOW_HIGHLIGHT 7

#define NO_SCREEN_PERF_CACHE
class ScreenPerfCache {
    const static int ROW_CNT = 128;  // maybe this number is too small
    const static int COL_CNT = 256;  // maybe this number is too small
    char str[ROW_CNT][COL_CNT];
    int colorindex[ROW_CNT][COL_CNT];

public:
    ScreenPerfCache() {
        for (int r = 0; r < ROW_CNT; r++) {
            for (int c = 0; c < COL_CNT; c++) {
                str[r][c]        = 0;
                colorindex[r][c] = -1;
            }
        }
    }

    bool isCached(int r, int c, const char *p, int ci) const {
#ifdef NO_SCREEN_PERF_CACHE
        return false;
#endif
        for (; 0 <= r && r < ROW_CNT && 0 <= c && c < COL_CNT && *p; p++, c++)
            if (*p != str[r][c] || ci != colorindex[r][c]) return false;

        // check for the terminal '\0' in str
        if (0 <= r && r < ROW_CNT && 0 <= c && c < COL_CNT && *p != str[r][c]) return false;

        return true;
    }
    void setCached(int r, int c, const char *p, int ci) {
#ifdef NO_SCREEN_PERF_CACHE
        return;
#endif
        for (; *p && 0 <= r && r < ROW_CNT && 0 <= c && c < COL_CNT; p++, c++) {
            str[r][c]        = *p;
            colorindex[r][c] = ci;
        }
        // set the terminal '\0' in str
        if (0 <= r && r < ROW_CNT && 0 <= c && c < COL_CNT) str[r][c] = 0;
    }
    void clearCache() {
#ifdef NO_SCREEN_PERF_CACHE
        return;
#endif
        for (int r = 0; r < ROW_CNT; r++) clearRowCache(r);
    }
    void clearRowCache(int r) {
#ifdef NO_SCREEN_PERF_CACHE
        return;
#endif
        if (0 <= r && r < ROW_CNT)
            for (int c = 0; c < COL_CNT; c++) {
                str[r][c]        = 0;
                colorindex[r][c] = -1;
            }
    }
};

class Screen {
    ScreenPerfCache perf_cache;

public:
    Screen() { init(); }
    ~Screen() { destroy(); }

    void init();
    void destroy();
    void draw(int r, int c, const char *str, int color_index);
    int get_color_index(bool is_red, int bg_color);
    void clear_line(int r);
    void clear_screen();
    int width();
};

#endif  // SCREEN_H