#include <stdio.h>
#include <string.h>
#include <stdlib.h>


unsigned int saturating_sub(unsigned int n1, unsigned int n2) {
    if (n2 > n1) return 0;
    return n1 -n2;
}

unsigned int umin(unsigned int a, unsigned int b) {
    return a < b ? a : b;
}

unsigned int umax(unsigned int a, unsigned int b) {
    return a > b ? a : b;
}
