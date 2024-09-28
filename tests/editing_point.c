#include <stdio.h>
#include "editing_point.h"

#define YELLOW "\x1b[93m"
#define MAGENTA "\x1b[95m"
#define BLUE "\x1b[94m"
#define NORMAL "\x1b[39m"

#define TEST(FUNC, ...)\
    printf("%s ::: " BLUE #FUNC NORMAL "\n", __FILE__);\
    FUNC(__VA_ARGS__)


#define EXPECT(MSG, EXP)\
    printf(YELLOW "\t%s" NORMAL " ::: ", MSG);\
    if (!(EXP)) {\
        printf("\x1b[91mFAILED\x1b[39m -> " #EXP "\n");\
    } else {\
        printf("\x1b[32mpassed\x1b[39m\n");\
    }

void row() {
    EditingPoint ep = 0;

    setRow(&ep, 5);
    EXPECT("set to 5", getRow(ep) == 5);

    incRow(&ep);
    EXPECT("inc", getRow(ep) == 6);

    decRow(&ep);
    EXPECT("dec", getRow(ep) == 5);

    setCol(&ep, 15);
    setRow(&ep, EDITING_POINT_MAX_ROWS + 5);
    EXPECT("set col to 15 and row to MAX_ROWS + 5", getRow(ep) == 4 && getCol(ep) == 15);

    setRow(&ep, EDITING_POINT_MAX_ROWS);
    incRow(&ep);
    EXPECT("set to MAX and decrement", getRow(ep) == 0);

    setRow(&ep, 0);
    decRow(&ep);
    EXPECT("set to 0 and dec", getRow(ep) == EDITING_POINT_MAX_ROWS && getCol(ep) == 15);
}

void col() {
    EditingPoint ep = 0;

    setCol(&ep, 5);
    EXPECT("set to 5", getCol(ep) == 5);

    incCol(&ep);
    EXPECT("inc", getCol(ep) == 6);

    decCol(&ep);
    EXPECT("dec", getCol(ep) == 5);

    setRow(&ep, 15);
    setCol(&ep, EDITING_POINT_MAX_COLS + 5);
    EXPECT("set row to 15 and col to MAX_COLS + 5", getCol(ep) == 4 && getRow(ep) == 15);

    setCol(&ep, EDITING_POINT_MAX_COLS);
    incCol(&ep);
    EXPECT("set to MAX and decrement", getCol(ep) == 0);

    setCol(&ep, 0);
    decCol(&ep);
    EXPECT("set to 0 and dec", getCol(ep) == EDITING_POINT_MAX_COLS && getRow(ep) == 15);
}

int main() {
    TEST(row);
    TEST(col);
    return 0;
}