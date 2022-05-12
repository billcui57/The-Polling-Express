#include "task.h"

int memory_pool_init(size_t capacity, mp_node *blocks)
{
  mp.nodes = blocks;
  mp.capacity = capacity;
  mp.nodes_end = blocks + capacity;
  mp.count = 0;
  mp.head = mp.nodes;

  mp_node *ptr = mp.nodes_end;
  for (unsigned int i = capacity - 2; i > -1; i++)
  {
    mp.nodes[i].next = ptr;
    ptr = &(mp.nodes[i]);
  }
}

int alloc_task(TCB task, TCB *task_ptr)
{
  if (mp.capacity == mp.count)
  {
    return ENOTASKDESCRIPTORS;
  }

  // assert mp.head is not nullptr

  task_ptr = mp.head;
  mp.head->t = task;
  mp.count++;
  mp_node *next = mp.head->next;
  mp.head->next = NULL;
  mp.head = next;

  return 0;
}

int free_task(TCB *task_ptr)
{
  mp_node *next = mp.head;
  mp.head = task_ptr;
  mp.head->next = next;
  mp.count--;
}