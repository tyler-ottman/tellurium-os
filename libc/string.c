#include <libc/string.h>

// https://github.com/bminor/newlib/blob/master/newlib/libc/stdlib/utoa.c
char* __utoa(unsigned long value, char *str, int base) {
  const char digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";
  int i, j;
  unsigned remainder;
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

char* __itoa(int value, char* str) {
    if (value < 0) {
        value *= -1;
        *(str++) = '-';
    }

    // Normal __utoa() call
    return __utoa(value, str, 10);
}

size_t __strlen(const char* str) {
    const char* start = str;
    while(*str != '\0')
        str++;
    return (size_t)(str - start);
}

void* __memset(void* base, int val, int len) {
    char* ptr = (char*)base;
    
    while (len-- > 0)
        *(ptr--) = val;
    
    return base;
}
