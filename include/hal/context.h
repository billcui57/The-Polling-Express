#pragma once

#include <memory.h>
#include <stdint.h>

#define STACK_SIZE 8192

typedef struct registers {
  uint32_t r0;
  uint32_t r1;
  uint32_t r2;
  uint32_t r3;
  uint32_t r4; //+16
  uint32_t r5;
  uint32_t r6;
  uint32_t r7;
  uint32_t r8; //+32
  uint32_t r9;
  uint32_t r10;
  uint32_t r11; // framepointer
  uint32_t r12;
  uint32_t r15; // user pc +52
  uint32_t r13; // user sp +56
  uint32_t r14; // user lr
  uint32_t psr; // +64
} registers;

typedef struct user_task {
  int stack_check_a;
  uint32_t stack[STACK_SIZE];
  int stack_check_b;
  registers reg;
} user_task;

void init_user_task(user_task *t, void (*func)());
int run_user(user_task *t);
int get_data(user_task *t);
void set_return(user_task *t, int data);
void switch_user();
void return_irq();
void return_swi();

extern registers kernel_reg;
extern registers *user_reg;
extern uint32_t irq_stack[16];
