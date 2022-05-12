#include <task.h>

TCB* free_tcb;

void memory_pool_init(size_t capacity, TCB *blocks)
{
  for (unsigned int i = 0; i < capacity-1; i++)
  {
    blocks[i].next = &blocks[i+1];
  }
  blocks[capacity-1].next = NULL;
  free_tcb = blocks;
}

TCB* alloc_task()
{
  if (!free_tcb) return NULL;
  TCB *ret = free_tcb;
  free_tcb = ret->next;
  return ret;
}

void free_task(TCB *task_ptr)
{
  task_ptr->next = free_tcb;
  free_tcb = task_ptr;
}
