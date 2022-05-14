#include "my_assert.h"
#include <task.h>

TCB *free_tcb;
size_t capacity;

TCB **ready_queue;
unsigned int count;

void memory_pool_init(size_t cap, TCB *blocks) {
  for (unsigned int i = 0; i < cap - 1; i++) {
    blocks[i].next = &blocks[i + 1];
  }
  blocks[cap - 1].next = NULL;
  free_tcb = blocks;
  capacity = cap;
}

void scheduler_init(TCB **_heap) {
  ready_queue = _heap;
  count = 0;
}

void swap(unsigned int a, unsigned int b) {
  TCB *temp = ready_queue[a];
  ready_queue[a] = ready_queue[b];
  ready_queue[b] = temp;
}

void bottom_up_heapify(unsigned int i) {

  KASSERT((i >= 0) && (i < count), 12);

  if (i == 0) {
    return;
  }

  unsigned int parent = (i - 1) / 2;

  if (ready_queue[i]->priority > ready_queue[parent]->priority) {
    swap(i, parent);
    // Recursively heapify the parent node
    bottom_up_heapify(parent);
  }
}

void top_down_heapify(unsigned int i) {
  unsigned int largest = i;   // Initialize largest as root
  unsigned int l = 2 * i + 1; // left = 2*i + 1
  unsigned int r = 2 * i + 2; // right = 2*i + 2

  // If left child is larger than root
  if (l < count && ready_queue[l] > ready_queue[largest])
    largest = l;

  // If right child is larger than largest so far
  if (r < count && ready_queue[r] > ready_queue[largest])
    largest = r;

  // If largest is not root
  if (largest != i) {
    swap(i, largest);

    // Recursively heapify the affected sub-tree
    top_down_heapify(largest);
  }
}

void heap_pop() {}

TCB *alloc_task(unsigned int priority, char name) {
  if (!free_tcb)
    return NULL;
  TCB *ret = free_tcb;
  free_tcb = ret->next;
  ret->next = NULL;
  ret->name = name;
  ret->priority = priority;

  KASSERT(count < capacity, 23);

  // priority queue
  ready_queue[count] = ret;
  count++;
  bottom_up_heapify(count - 1);

  return ret;
}

void remove_from_ready_queue(TCB *t) {
  KASSERT(t != NULL, 1);
  KASSERT(count > 0, 2);
  t->priority = 100;

  for (unsigned int i = 0; i < capacity; i++) {
    if (ready_queue[i] == t) {
      bottom_up_heapify(i);
    }
  }

  ready_queue[0] = ready_queue[count - 1];

  count--;

  top_down_heapify(0);
}

void free_task(TCB *task_ptr) {
  task_ptr->next = free_tcb;
  free_tcb = task_ptr;
}
