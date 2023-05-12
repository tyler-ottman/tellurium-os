#ifndef MISC_H
#define MISC_H

#include <stdbool.h>

#define HLT
#undef RET

#define ERR_NO_MEM                                  1

#define ASSERT(cond, err, msg)                      \
{                                                   \
    if (!(cond)) {                                  \
        kerror(msg);                                \
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