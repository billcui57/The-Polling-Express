#pragma once

#include <stdint.h>

#define STACK_SIZE 2048

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
  uint32_t stack[STACK_SIZE];
  registers reg;
} user_task;

void init_user_task(user_task *t, void (*func)());

void switch_user(registers *r);
extern void return_irq();
extern void return_swi();
extern registers kernel_reg;
