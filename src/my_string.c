#include "my_string.h"

void strncpy(char *dest, const char *src, size_t n) {

  while (*src && n >= 0) {
    *dest = *src;
    dest++;
    src++;
    n--;
  }
  *dest = '\0';
}

unsigned int strlen(const char *str) {
  unsigned int count = 0;
  while (*str != '\000') {
    count++;
    str++;
  }
  return count;
}

int strncmp(const char *s1, const char *s2, size_t n) {

  while (n >= 0 && *s1 && *s2) {

    if (*s1 > *s2) {
      return 1;
    } else if (*s1 < *s2) {
      return -1;
    }

    s1++;
    s2++;
  }

  if (n == 0) {
    return 0;
  }

  if (*s1) {
    return 1;
  }

  if (*s2) {
    return -1;
  }

  return 0;
}