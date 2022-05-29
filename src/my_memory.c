#include <my_memory.h>

void memcpy(void *dest, void *src, size_t n) {
  char *csrc = (char *)src;
  char *cdest = (char *)dest;

  for (unsigned int i = 0; i < n; i++) {
    cdest[i] = csrc[i];
  }
}

void memset(void *ptr, size_t n, char set) {
  char *cptr = (char *)ptr;
  for (unsigned int i = 0; i < n; i++) {
    cptr[i] = set;
  }
}