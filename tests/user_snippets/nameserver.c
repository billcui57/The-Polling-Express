#include <user.h>

void task_k1init() {
  int ret;
  ret = Create(-10, nameserver);
  Create(-10, task1);
  Create(-10, task2);
}

void task1() {

  char abc[] = "abc";

  while (RegisterAs(abc) != 0)
    ;

  for (;;) {

    task_tid tid = WhoIs(abc);

    if (tid == -1) {
      printf(COM2, "ERROR\r\n");
    } else {

      if (tid == 3) {
        while (RegisterAs(abc) != 0)
          ;
      }
      printf(COM2, "%d\r\n", tid);
    }
  }
}

void task2() {
  char abc[] = "abc";

  while (RegisterAs(abc) != 0)
    ;
  for (;;) {

    task_tid tid = WhoIs(abc);

    if (tid == -1) {
      printf(COM2, "ERROR\r\n");
    } else {

      if (tid == 2) {
        while (RegisterAs(abc) != 0)
          ;
      }

      printf(COM2, "%d\r\n", tid);
    }
  }
}
