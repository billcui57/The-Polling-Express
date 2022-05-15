#include <syscall.h>
int Create(int priority, void (*function)()) {
  create_args a;
  a.priority = priority;
  a.function = function;
  int ret;
  __asm__ volatile("mov r0, %[arg] \n\t"
                   "swi %[syscall] \n\t"
                   "mov %[ret], r0"
                   : [ret] "=r"(ret)
                   : [syscall] "i"(SYSCALL_CREATE), [arg] "r"(&a));
  return ret;
}

int MyTid() {
  int ret;
  __asm__ volatile("swi %[syscall] \n\t"
                   "mov %[ret], r0"
                   : [ret] "=r"(ret)
                   : [syscall] "i"(SYSCALL_MYTID));
  return ret;
}
int MyParentTid() {
  int ret;
  __asm__ volatile("swi %[syscall] \n\t"
                   "mov %[ret], r0"
                   : [ret] "=r"(ret)
                   : [syscall] "i"(SYSCALL_MYPARENTTID));
  return ret;
}

void Yield() {
  __asm__ volatile("swi %[syscall]" ::[syscall] "i"(SYSCALL_YIELD));
}

void Exit() {
  __asm__ volatile("swi %[syscall]" ::[syscall] "i"(SYSCALL_EXIT));
}
