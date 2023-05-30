#include <arch/cpu.h>
#include <libc/string.h>
#include <stdbool.h>

// https://github.com/bminor/newlib/blob/master/newlib/libc/stdlib/utoa.c
char *__utoa(uint64_t value, char *str, int base) {
  const char digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";
  int i, j;
  uint64_t remainder;
  char c;

  /* Check base is supported. */
  if ((base < 2) || (base > 36)) {
      str[0] = '\0';
      return NULL;
  }

  /* Convert to string. Digits are in reverse order.  */
  i = 0;
  do {
      remainder = value % base;
      str[i++] = digits[remainder];
      value = value / base;
  } while (value != 0);
  str[i] = '\0';

  /* Reverse string.  */
  for (j = 0, i--; j < i; j++, i--) {
      c = str[j];
      str[j] = str[i];
      str[i] = c;
  }
  return str;
}

char *__itoa(int64_t value, char *str) {
    if (value < 0) {
        value *= -1;
        *(str++) = '-';
    }

    // Normal __utoa() call
    return __utoa(value, str, 10);
}

char *__strcat(char *dest, char *src) {
    char *base = dest + __strlen(dest);
    while (*src) {
        *(base++) = *(src++);
    }
    *(base++) = '\0';
    return dest;
}

const char *__strchr(const char *str, int c) {
    while (*str && *str != c) {
        str++;
    }

    return (*str == c) ? str : NULL;
}

const char *__strncpy(char *dest, const char *src, size_t n) {
    if (!dest) {
        return NULL;
    }

    char *start = dest;
    while (*src && n--) {
        *dest++ = *src++;
    }

    *dest = '\0';
    return start;
}

size_t __strcspn(const char *str1, const char *str2) {
    size_t len = 0;
    if (!str1 || !str2) {
        return len;
    }

    while (*str1) {
        if (__strchr(str2, *str1)) {
            return len;
        } else {
            len++;
            str1++;
        }
    }

    return len;
}

size_t __strlen(const char *str) {
    const char *start = str;
    while(*str != '\0')
        str++;
    return (size_t)(str - start);
}

size_t __strncmp(const char *str1, const char *str2, size_t n) {
    if (n == 0) {
        return 0;
    }

    for (; n != 0; n--) {
        if (*str1++ != *str2++) {
            return (*--str1 - *--str2);
        }
    }
    return 0;
}

size_t __strspn(const char *str1, const char *str2) {
    size_t len = 0;
    if (!str1 || !str2) {
        return len;
    }

    while (*str1) {
        if (!__strchr(str2, *str1++)) {
            break;
        }

        len++;
    }

    return len;
}

char *__strtok(char *str, const char *del) {
    static char *old_str;
    char *token;

    if (str) {
        old_str = str;
    }

    if (!old_str) {
        return NULL;
    }

    old_str += __strspn(old_str, del);
    if (*old_str == '\0') {
        return old_str = NULL;
    }

    token = old_str;
    old_str += __strcspn(old_str, del);
    if (*old_str == '\0') {
        old_str = NULL;
    } else {
        *old_str++ = '\0';
    }
    return token;
}

// https://codebrowser.dev/glibc/glibc/string/strtok_r.c.html
char *__strtok_r(char *str, const char *del, char **old_ptr) {
    char *end;
    if (str == NULL) {
        str = *old_ptr;
    }
  
    if (*str == '\0') {
        *old_ptr = str;
        return NULL;
    }

    str += __strspn(str, del);
    if (*str == '\0') {
        *old_ptr = str;
        return NULL;
    }

    end = str + __strcspn(str, del);
    if (*end == '\0') {
        *old_ptr = end;
        return str;
    }
  
    *end = '\0';
    *old_ptr = end + 1;
  
    return str;
}

void *__memset(void *base, unsigned char val, size_t len) {
    char *ptr = (char *)base;
    
    while (len-- > 0) {
        *(ptr++) = val;
    }

    return base;
}

void *__memcpy(void *dest, const void *src, size_t n) {
    char *dest_ptr = (char *)dest;
    const char *src_ptr = (const char *)src;
    while (n-- > 0) {
        *(dest_ptr++) = *(src_ptr++);
    }
    return dest;
}

int __memcmp(const void *str1, const void *str2, size_t n) {
    const char *src1 = (const char *)str1;
    const char *src2 = (const char *)str2;
    while (n-- > 0) {
        if (*src1++ != *src2++) {
            return (*(src1-1) < *(src2-1)) ? -1 : 1;
        }
    }
    return 0;
}