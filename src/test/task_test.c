#include "task.h"

int main(int argc, char *argv[])
{
  size_t capacity = 10;
  mp_node memory_pool_blocks[capacity];

  for (unsigned int i = 0; i < capacity; i++)
  {
    TCB t;
    t.name = 'a' + i;
    t.val = i;

    TCB *t_ptr;

    int status = alloc_task(t, t_ptr);
  }
}