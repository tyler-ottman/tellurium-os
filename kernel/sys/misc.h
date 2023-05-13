#ifndef MISC_H
#define MISC_H

#include <stdbool.h>

// #define DEBUG

#define ERR_NO_MEM                                  1

#define ASSERT(cond, err, msg)                      \
{                                                   \
    if (!(cond)) {                                  \
        kerror(msg, err);                           \
    }                                               \
}

#define ASSERT_RET(cond, err)                       \
{                                                   \
    if (!(cond)) {                                  \
        return err;                                 \
    }                                               \
}

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#endif // MISC_H