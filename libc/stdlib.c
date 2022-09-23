#include <libc/stdlib.h>

int __atoi(const char** str) {
    int sign = (**str == '-') ? -1 : 1;
    if (**str == '-') (*str)++;

    int num = 0;
    while (**str && (**str >= '0' && **str <= '9')) {
        num = num * 10 + **str - '0';
        (*str)++;
    }
    return sign * num;
}