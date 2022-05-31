void task_k1init() {
  int ret;
  ret = Create(-10, task_k1test);
  printf(&pc, "Created: %d\r\n", ret);
  ret = Create(-10, task_k1test);
  printf(&pc, "Created: %d\r\n", ret);
  ret = Create(10, task_k1test);
  printf(&pc, "Created: %d\r\n", ret);
  ret = Create(10, task_k1test);
  printf(&pc, "Created: %d\r\n", ret);
  printf(&pc, "FirstUserTask: exiting\r\n");
  Exit();
}

void task_k1test() {
  int me = MyTid();
  int parent = MyParentTid();
  printf(&pc, "Me: %d Parent: %d \r\n", me, parent);
  Yield();
  printf(&pc, "Me: %d Parent: %d \r\n", me, parent);
  Exit();
}
