#include "heap.h"

int parent(int i) { return (i - 1) / 2; }
int left(int i) { return 2 * i + 1; }

int right(int i) { return 2 * i + 2; }

void heap_init(heap *h, void **backing) {
  h->items = backing;
  h->len = 0;
}
void heap_add(heap *h, void *item) {
  h->items[h->len] = item;
  int i = h->len;
  h->len++;
  while (i) {
    int a = *(int *)h->items[i];
    int b = *(int *)h->items[parent(i)];
    if (a < b) {
      void *tmp = h->items[i];
      h->items[i] = h->items[parent(i)];
      h->items[parent(i)] = tmp;
      i = parent(i);
    } else {
      break;
    }
  }
}
void *heap_peek(heap *h) {
  if (h->len) {
    return h->items[0];
  }
  return 0;
}
void heap_pop(heap *h) {
  if (h->len < 2) {
    h->len = 0;
    return;
  }
  h->items[0] = h->items[h->len - 1];
  h->len--;
  int i = 0;
  while (i < h->len) {
    int a = left(i) < h->len ? *(int *)h->items[left(i)] : 0x00FFFFFF;
    int b =
        right(i) < h->len ? *(int *)h->items[right(i)] : 0x00FFFFFF;
    int c = *(int *)h->items[i];
    if (c < a && c < b) {
      break;
    }
    int j = a < b ? left(i) : right(i);
    void *tmp = h->items[i];
    h->items[i] = h->items[j];
    h->items[j] = tmp;
    i = j;
  }
}
