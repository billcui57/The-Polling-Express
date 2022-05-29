#include <context.h>
#include <my_assert.h>
#include <syscall.h>

registers kernel_reg;
registers *user_reg;

void init_user_task(user_task *t, void (*func)()) {
  t->stack_check = t;
  t->reg.r0 = 0;
  t->reg.r1 = 0;
  t->reg.r2 = 0;
  t->reg.r3 = 0;
  t->reg.r4 = 0;
  t->reg.r5 = 0;
  t->reg.r6 = 0;
  t->reg.r7 = 0;
  t->reg.r8 = 0;
  t->reg.r9 = 0;
  t->reg.r10 = 0;
  t->reg.r11 = 0;
  t->reg.r12 = 0;
  t->reg.r13 = &t->stack[STACK_SIZE - 1];
  t->reg.r14 = &Exit;
  t->reg.r15 = func;
  t->reg.psr = (1 << 7) | (0b10000);
}

int run_user(user_task *t) {
  KASSERT(t->stack_check == t, "STACK PREBROKEN");
  user_reg = &t->reg;
  switch_user();
  user_reg = 0;
  KASSERT(t->stack_check == t, "STACK POSTBROKEN");
  if (t->reg.r15 & 1) {
    t->reg.r15--;
    return SYSCALL_IRQ;
  }
  return (*(uint32_t *)(t->reg.r15 - 4)) & 0xFFFFFF;
}

int get_data(user_task *t) { return t->reg.r0; }
void set_return(user_task *t, int data) { t->reg.r0 = data; }
