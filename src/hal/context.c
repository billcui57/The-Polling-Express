#include <context.h>
#include <my_assert.h>
#include <syscall.h>

registers kernel_reg;
registers *user_reg;

uint32_t irq_stack[16];

void init_user_task(user_task *t, void (*func)()) {
  t->stack_check_a = ((int)t) ^ 93;
  t->stack_check_b = ((int)t) ^ 72;
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
  t->reg.r13 = (uint32_t)&t->stack[STACK_SIZE - 1];
  t->reg.r14 = (uint32_t)&Exit;
  t->reg.r15 = (uint32_t)func;
  t->reg.psr = (0b10000);
}

int run_user(user_task *t) {
  KASSERT(t->stack_check_a == (((int)t) ^ 93), "STACK PREBROKEN_A");
  KASSERT(t->stack_check_b == (((int)t) ^ 72), "STACK PREBROKEN_B");
  user_reg = &t->reg;
  switch_user();
  user_reg = 0;
  KASSERT(t->stack_check_a == (((int)t) ^ 93), "STACK POSTBROKEN_A");
  KASSERT(t->stack_check_b == (((int)t) ^ 72), "STACK POSTBROKEN_B");
  if (t->reg.r15 & 1) {
    t->reg.r15--;
    return SYSCALL_IRQ;
  }
  return (*(uint32_t *)(t->reg.r15 - 4)) & 0xFFFFFF;
}

int get_data(user_task *t) { return t->reg.r0; }
void set_return(user_task *t, int data) { t->reg.r0 = data; }
