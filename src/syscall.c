#include <syscall.h>

void Yield() { __asm__ volatile("swi %0" ::"i"(SYSCALL_YIELD)); }

void Exit() { __asm__ volatile("swi %0" ::"i"(SYSCALL_EXIT)); }
