#include <context.h>
#include <syscall.h>
void init_user_task(user_task *t, void (*func)()) {
  t->reg.r0 = 0;
  t->reg.r1 = 0;
  t->reg.r2 = 0;
  t->reg.r3 = 0;
  t->reg.r4 = 0;
  t->reg.r5 = 0;
  t->reg.r6 = 0;
  t->reg.r7 = 0;
  t->reg.r8 = 0;
  t->reg.r9 = &t->reg;
  t->reg.r10 = 0;
  t->reg.r11 = 0;
  t->reg.r12 = 0;
  t->reg.r13 = &t->stack[STACK_SIZE - 1];
  t->reg.r14 = &Exit;
  t->reg.r15 = func;
  t->reg.psr = (1 << 7) | (0b10000);
}
