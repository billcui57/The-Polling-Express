void task_k1init() {
  int ret;
  ret = Create(-10, task_k1test);
  printf(COM2, "Created: %d\r\n", ret);
  ret = Create(-10, task_k1test);
  printf(COM2, "Created: %d\r\n", ret);
  ret = Create(10, task_k1test);
  printf(COM2, "Created: %d\r\n", ret);
  ret = Create(10, task_k1test);
  printf(COM2, "Created: %d\r\n", ret);
  printf(COM2, "FirstUserTask: exiting\r\n");
  Exit();
}

void task_k1test() {
  int me = MyTid();
  int parent = MyParentTid();
  printf(COM2, "Me: %d Parent: %d \r\n", me, parent);
  Yield();
  printf(COM2, "Me: %d Parent: %d \r\n", me, parent);
  Exit();
}
