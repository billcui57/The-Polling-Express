#include <syscall.h>
int Create(int priority, void (*function)()) {
  volatile create_args a;
  a.priority = priority;
  a.function = function;
  int ret;
  __asm__ volatile("mov r0, %[arg] \n\t"
                   "swi %[syscall] \n\t"
                   "mov %[ret], r0"
                   : [ret] "=r"(ret)
                   : [syscall] "i"(SYSCALL_CREATE), [arg] "r"(&a)
                   : "r0");
  return ret;
}

int MyTid() {
  int ret;
  __asm__ volatile("swi %[syscall] \n\t"
                   "mov %[ret], r0"
                   : [ret] "=r"(ret)
                   : [syscall] "i"(SYSCALL_MYTID)
                   : "r0");
  return ret;
}
int MyParentTid() {
  int ret;
  __asm__ volatile("swi %[syscall] \n\t"
                   "mov %[ret], r0"
                   : [ret] "=r"(ret)
                   : [syscall] "i"(SYSCALL_MYPARENTTID)
                   : "r0");
  return ret;
}

void Yield() {
  __asm__ volatile("swi %[syscall]" ::[syscall] "i"(SYSCALL_YIELD));
}

void Exit() {
  __asm__ volatile("swi %[syscall]" ::[syscall] "i"(SYSCALL_EXIT));
}

int Send(int tid, const char *msg, int msglen, char *reply, int rplen) {
  volatile send_args a;
  a.tid = tid;
  a.msg = msg;
  a.msglen = msglen;
  a.reply = reply;
  a.rplen = rplen;
  int ret;
  __asm__ volatile("mov r0, %[arg] \n\t"
                   "swi %[syscall] \n\t"
                   "mov %[ret], r0"
                   : [ret] "=r"(ret)
                   : [syscall] "i"(SYSCALL_SEND), [arg] "r"(&a)
                   : "r0");
  return ret;
}

int Receive(int *tid, char *msg, int msglen) {
  volatile receive_args a;
  a.tid = tid;
  a.msg = msg;
  a.msglen = msglen;
  int ret;
  __asm__ volatile("mov r0, %[arg] \n\t"
                   "swi %[syscall] \n\t"
                   "mov %[ret], r0"
                   : [ret] "=r"(ret)
                   : [syscall] "i"(SYSCALL_RECEIVE), [arg] "r"(&a)
                   : "r0");
  return ret;
}

int Reply(int tid, const char *reply, int rplen) {
  volatile reply_args a;
  a.tid = tid;
  a.reply = reply;
  a.rplen = rplen;
  int ret;
  __asm__ volatile("mov r0, %[arg] \n\t"
                   "swi %[syscall] \n\t"
                   "mov %[ret], r0"
                   : [ret] "=r"(ret)
                   : [syscall] "i"(SYSCALL_REPLY), [arg] "r"(&a)
                   : "r0");
  return ret;
}
