#include <memory.h>

void *memcpy(void *__restrict dest, const void *__restrict src, size_t n) {
  char *csrc = (char *)src;
  char *cdest = (char *)dest;

  for (unsigned int i = 0; i < n; i++) {
    cdest[i] = csrc[i];
  }
  return dest;
}

void *memset(void *ptr, int val, size_t n) {
  char *cptr = (char *)ptr;
  char set = val;
  for (unsigned int i = 0; i < n; i++) {
    cptr[i] = set;
  }
  return ptr;
}
