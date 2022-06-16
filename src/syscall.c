#include <syscall.h>
int Create(int priority, void (*function)()) {
  volatile create_args a;
  a.priority = priority;
  a.function = function;
  int ret;
  __asm__ volatile("mov r0, %[arg] \n\t"
                   "swi %[syscall] \n\t"
                   "mov %[ret], r0"
                   : [ ret ] "=r"(ret)
                   : [ syscall ] "i"(SYSCALL_CREATE), [ arg ] "r"(&a)
                   : "r0");
  return ret;
}

int MyTid() {
  int ret;
  __asm__ volatile("swi %[syscall] \n\t"
                   "mov %[ret], r0"
                   : [ ret ] "=r"(ret)
                   : [ syscall ] "i"(SYSCALL_MYTID)
                   : "r0");
  return ret;
}
int MyParentTid() {
  int ret;
  __asm__ volatile("swi %[syscall] \n\t"
                   "mov %[ret], r0"
                   : [ ret ] "=r"(ret)
                   : [ syscall ] "i"(SYSCALL_MYPARENTTID)
                   : "r0");
  return ret;
}

void Yield() {
  __asm__ volatile("swi %[syscall]" ::[syscall] "i"(SYSCALL_YIELD));
}

void Exit() {
  __asm__ volatile("swi %[syscall]" ::[syscall] "i"(SYSCALL_EXIT));
}

void Shutdown() {
  __asm__ volatile("swi %[syscall]" ::[syscall] "i"(SYSCALL_SHUTDOWN));
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
                   : [ ret ] "=r"(ret)
                   : [ syscall ] "i"(SYSCALL_SEND), [ arg ] "r"(&a)
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
                   : [ ret ] "=r"(ret)
                   : [ syscall ] "i"(SYSCALL_RECEIVE), [ arg ] "r"(&a)
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
                   : [ ret ] "=r"(ret)
                   : [ syscall ] "i"(SYSCALL_REPLY), [ arg ] "r"(&a)
                   : "r0");
  return ret;
}

int WhoIs(const char *name) {
  nameserver_request rq;
  nameserver_request_init(&rq, REQUEST_WHO_IS, name, strlen(name) + 1);
  char *request_buffer = (char *)&rq; // Serialize

  char response_buffer[sizeof(nameserver_response)];

  int status = Send(nameserver_tid, request_buffer, sizeof(nameserver_request),
                    response_buffer, sizeof(nameserver_response));

  // nameserver isn't up
  if (status == -1) {
    return -1;
  }

  nameserver_response *response =
      (nameserver_response *)response_buffer; // Deserialize

  if (response->type == RESPONSE_GOOD) {
    return get_tid_from_nameserver_response(response);
  } else if (response->type == RESPONSE_NAME_DNE) {
    return -2;
  } else {
    return -3;
  }
}

int WhoIsBlock(const char *name) {
  task_tid i = -1;
  while (i < 0)
    i = WhoIs(name);
  return i;
}
int RegisterAs(const char *name) {
  nameserver_request rq;
  nameserver_request_init(&rq, REQUEST_REGISTER_AS, name, strlen(name) + 1);
  char *request_buffer = (char *)&rq; // Serialize

  char response_buffer[sizeof(nameserver_response)];

  int status = Send(nameserver_tid, request_buffer, sizeof(nameserver_request),
                    response_buffer, sizeof(nameserver_response));

  // nameserver isn't up
  if (status == -1) {
    return -1;
  }

  nameserver_response *response =
      (nameserver_response *)response_buffer; // Deserialize

  return 0;
}

int AwaitEvent(int eventid) {
  int ret;
  __asm__ volatile("mov r0, %[arg] \n\t"
                   "swi %[syscall] \n\t"
                   "mov %[ret], r0"
                   : [ ret ] "=r"(ret)
                   : [ syscall ] "i"(SYSCALL_AWAITEVENT), [ arg ] "r"(eventid)
                   : "r0");
  return ret;
}

int Putc(int tid, int uart, char ch) {
  uartserver_request req;
  memset(&req, 0, sizeof(req));
  req.data = ch;
  req.type = SEND_CHAR;
  uartserver_response res;

  int status = Send(tid, (char *)&req, sizeof(uartserver_request), (char *)&res,
                    sizeof(uartserver_response));

  return 0;
}

int Getc(int tid, int uart) {
  uartserver_request req;
  memset(&req, 0, sizeof(req));
  req.data = 0;
  req.type = GET_CHAR;
  uartserver_response res;

  int status = Send(tid, (char *)&req, sizeof(uartserver_request), (char *)&res,
                    sizeof(uartserver_response));

  return res.data;
}

int ReleaseUartLock(int tid) {
  uartserver_request req;
  memset(&req, 0, sizeof(req));
  req.data = 0;
  req.type = RELEASE_LOCK;
  uartserver_response res;

  int status = Send(tid, (char *)&req, sizeof(uartserver_request), (char *)&res,
                    sizeof(uartserver_response));

  return 0;
}
