#include <libc/ctype.h>

int __is_digit(char c) {
    return ((c >= '0') && (c <= '9'));
}