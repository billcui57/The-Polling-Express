#include "my_assert.h"
#include <syscall.h>
#include <task.h>

TCB *free_tcb;
size_t capacity;

TCB **_ready_queue;
unsigned int count;

int cmp_priority(unsigned int a, unsigned int b);

void scheduler_init(size_t cap, TCB *blocks, TCB **ready_queue) {
  for (unsigned int i = 0; i < cap; i++) {
    blocks[i].state = ZOMBIE;
    blocks[i].tid = i;
    blocks[i].want_send = NULL;
    blocks[i].want_send_end = NULL;
  }
  for (unsigned int i = 0; i < cap - 1; i++) {
    blocks[i].next = &blocks[i + 1];
  }
  blocks[cap - 1].next = NULL;
  free_tcb = blocks;
  capacity = cap;
  _ready_queue = ready_queue;
  count = 0;
}

void swap(unsigned int a, unsigned int b) {
  TCB *temp = _ready_queue[a];
  _ready_queue[a] = _ready_queue[b];
  _ready_queue[b] = temp;
}

void top_down_heapify(unsigned int i) {
  unsigned int largest = i;   // Initialize largest as root
  unsigned int l = 2 * i + 1; // left = 2*i + 1
  unsigned int r = 2 * i + 2; // right = 2*i + 2

  // If left child is larger than root
  if (l < count && cmp_priority(l, largest) == 1)
    largest = l;

  // If right child is larger than largest so far
  if (r < count && cmp_priority(r, largest) == 1)
    largest = r;

  // If largest is not root
  if (largest != i) {
    swap(i, largest);

    // Recursively heapify the affected sub-tree
    top_down_heapify(largest);
  }
}

int scheduler_add(int priority, void (*func)(), int parentTid) {
  if (!free_tcb)
    return ENOTASKDESCRIPTORS;
  TCB *ret = free_tcb;
  free_tcb = ret->next;
  ret->next = NULL;
  ret->parentTid = parentTid;
  ret->priority = priority;
  ret->state = READY;

  init_user_task(&ret->context, func);
  add_to_ready_queue(ret);
  return ret->tid;
}

int cmp_priority(unsigned int a, unsigned int b) {
  if (_ready_queue[a]->priority > _ready_queue[b]->priority) {
    return 1;
  }

  if (_ready_queue[a]->priority < _ready_queue[b]->priority) {
    return -1;
  }

  if (_ready_queue[a]->added_time > _ready_queue[b]->added_time) {
    return 1;
  }

  return -1;
}

void add_to_ready_queue(TCB *t) {
  KASSERT(t != NULL, "TCB should not be NULL");
  KASSERT(count < capacity, "Ready queue should have space to add");

  read_timer(TIMER3, &(t->added_time));

  count++;

  unsigned int i = count - 1;
  unsigned int parent = (i - 1) / 2;
  _ready_queue[i] = t;

  while (i != 0 && cmp_priority(i, parent) == 1) {
    swap(i, parent);
    i = parent;
    parent = (i - 1) / 2;
  }
}

TCB *pop_ready_queue() {

  KASSERT(count > 0, "Ready queue should contain TCB's to pop");

  TCB *ret = _ready_queue[0];

  if (count == 1) {
    count--;
    return ret;
  }

  _ready_queue[0] = _ready_queue[count - 1];

  count--;

  top_down_heapify(0);

  return ret;
}

void free_task(TCB *task_ptr) {
  task_ptr->next = free_tcb;
  free_tcb = task_ptr;
}

bool scheduler_empty() { return count == 0; }
