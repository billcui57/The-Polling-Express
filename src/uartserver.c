#include "uartserver.h"

#include <syscall.h>
#include <hal.h>

void uart_com2_tx_notifier() {
  int parent = MyParentTid();
  int junk = 0;
  uartserver_request req;
  memset(&req, 0, sizeof(req));
  req.type = NOTIFIER_TX_GOOD;
  req.data = 0;
  for (;;) {
    // bw_uart_put_char(COM2, 'W');
    Send(parent, (char *)&req, sizeof(uartserver_request), (char *)&junk, 0);
    // bw_uart_put_char(COM2, 'A');
    AwaitEvent(UART2_TX_HALF_EMPTY);
    // bw_uart_put_char(COM2, 'B');
    // bw_uart_put_char(COM2, 'S');
  }
}

void uart_com2_tx_server() {
  RegisterAs("uart2txserver");
  task_tid notifier_tid = Create(10, uart_com2_tx_notifier);

  uartserver_request req;
  uartserver_response res;
  task_tid client;
  memset(&res, 0, sizeof(res));

  int MAX_CAPACITY = 100;
  void *lock_backing[MAX_CAPACITY];
  circular_buffer lock_cb;
  cb_init(&lock_cb, lock_backing, MAX_CAPACITY);

  char char_buffer[MAX_NUM_TASKS];

  memset(char_buffer, 0, sizeof(char) * MAX_CAPACITY);

  task_tid owned_by = -1;

  for (;;) {
    Receive(&client, (char *)&req, sizeof(uartserver_request));
    // bw_uart_put_char(COM2, 'R');
    if (req.type == NOTIFIER_TX_GOOD) {

      // bw_uart_put_char(COM2, 'N');

      if ((owned_by != -1) && (char_buffer[owned_by] != 0)) {
        // bw_uart_put_char(COM2, 'J');
        uart_put_char(COM2, char_buffer[owned_by]);
        char_buffer[owned_by] = 0;
        res.data = 0;
        res.type = GOOD;
        Reply(owned_by, (char *)&res, sizeof(uartserver_response));
      }

    } else if (req.type == SEND_CHAR) {
      char send_char = (char)req.data;
      // bw_uart_put_char(COM2, 'G');
      if ((owned_by == client) || (owned_by == -1)) {

        if (owned_by == -1) {
          owned_by = client;
        }

        if (uart_can_write(COM2)) {
          // bw_uart_put_char(COM2, '8');
          uart_put_char(COM2, send_char);
          res.data = 0;
          res.type = GOOD;
          Reply(owned_by, (char *)&res, sizeof(uartserver_response));
        } else {
          // bw_uart_put_char(COM2, '3');
          char_buffer[owned_by] = send_char;
          res.data = 0;
          res.type = GOOD;
          Reply(notifier_tid, (char *)&res, sizeof(uartserver_response));
        }
      } else {
        cb_push_back(&lock_cb, (void *)client);
        char_buffer[client] = send_char;
      }

    } else if (req.type == RELEASE_LOCK) {

      KASSERT(owned_by == client, "You must own the lock to uart");

      void *next_void;
      // bw_uart_put_char(COM2, 'E');
      int status = cb_pop_front(&lock_cb, &next_void);

      if (status == CB_EMPTY) {
        owned_by = -1;
      } else {
        owned_by = (task_tid)next_void;
      }

      res.data = 0;
      res.type = GOOD;
      Reply(notifier_tid, (char *)&res, sizeof(uartserver_response));

      res.data = 0;
      res.type = GOOD;
      Reply(client, (char *)&res, sizeof(uartserver_response));
    }
  }
}

// NOTIFIER SEMANTICS:
// Notify when server can take action


void uart_com1_tx_notifier(){
  int parent = MyParentTid();
  int junk = 0;
  uartserver_request req;
  req.type = NOTIFIER_TX_GOOD;
  req.data = 0;
  volatile int *uart1_ctrl = (int *)(get_base_addr(COM1) + UART_CTLR_OFFSET);
  volatile int *uart1_intr = (int *)(get_base_addr(COM1) + UART_INTR_OFFSET);
  volatile int *uart1_mdmsts = (int *)(get_base_addr(COM1) + UART_MDMSTS_OFFSET);
  while (true) {
    Send(parent, (char *)&req, sizeof(uartserver_request), (char *)&junk, 0);
    *uart1_mdmsts;
    *uart1_ctrl = *uart1_ctrl | TIEN_MASK | MSIEN_MASK;
    int tx_ready = 0;
    int cts_ready = 0;

    while(true){
        AwaitEvent(UART1_INTR);
        int status = *uart1_intr;
        if (status & TIS_MASK) {
          tx_ready = 1;
          *uart1_ctrl = *uart1_ctrl & ~TIEN_MASK;
        }
        if ((status & MIS_MASK) && (*uart1_mdmsts & MSR_DCTS)) {
          *uart1_intr = 0;
          cts_ready++;
          if (cts_ready == 2) {
            *uart1_ctrl = *uart1_ctrl & ~MSIEN_MASK;
          }
        }
        if (tx_ready == 1 && cts_ready == 2){
          break;
        }
    }
  }
}

void uart_com1_rx_notifier(){
  int parent = MyParentTid();
  int junk = 0;
  uartserver_request req;
  req.type = NOTIFIER_RX_GOOD;
  req.data = 0;
  volatile int *uart1_ctrl = (int *)(get_base_addr(COM1) + UART_CTLR_OFFSET);

  while (true) {
    Send(parent, (char *)&req, sizeof(uartserver_request), (char *)&junk, 0);
    *uart1_ctrl = *uart1_ctrl | RIEN_MASK;
    AwaitEvent(UART1_RX_INTR);
    req.data = uart_get_char(COM1);
  }
}

void uart_com1_server(){
  RegisterAs("uart1");

  task_tid tx = Create(10, uart_com1_tx_notifier);
  task_tid rx = Create(10, uart_com1_rx_notifier);

  uartserver_request req;
  uartserver_response res;
  task_tid client;

  void *lock_backing[MAX_NUM_TASKS];
  circular_buffer lock_cb;
  cb_init(&lock_cb, lock_backing, MAX_NUM_TASKS);

  task_tid owned_by = -1;

  bool can_send = true;
  bool want_send = false;
  char send_buffer[MAX_NUM_TASKS];

  bool can_get = false;
  bool want_get = false;
  char get_buffer = '\x00';

  while(true){
    Receive(&client, (char *)&req, sizeof(uartserver_request));
    if (req.type == NOTIFIER_TX_GOOD){
      if (want_send){
        want_send = false;
        res.data = 0;
        res.type = GOOD;
        uart_put_char(COM1, send_buffer[owned_by]);
        Reply(client, (char *)&res, sizeof(uartserver_response));
        Reply(owned_by, (char *)&res, sizeof(uartserver_response));
      } else {
        can_send = true;
      }
    } else if (req.type == NOTIFIER_RX_GOOD && owned_by != -1){
      res.data = req.data;
      res.type = GOOD;
      Reply(owned_by, (char *)&res, sizeof(uartserver_response));
    } else if (req.type == SEND_CHAR) {
      if (owned_by == -1){
        owned_by = client;
      }
      if(owned_by == client){
        if (can_send){
          can_send = false;
          res.data = 0;
          res.type = GOOD;
          uart_put_char(COM1, req.data);
          Reply(tx, (char *)&res, sizeof(uartserver_response));
          Reply(client, (char *)&res, sizeof(uartserver_response));
        } else {
          want_send = true;
          send_buffer[owned_by] = req.data;
        }
      } else {
        cb_push_back(&lock_cb, (void *)client);
        send_buffer[client] = req.data;
      }
    } else if (req.type == GET_CHAR){
      if (uart_can_read(COM1)){
        res.data = uart_get_char(COM1);
        res.type = GOOD;
        Reply(owned_by, (char *)&res, sizeof(uartserver_response));
      } else {
        res.data = 0;
        res.type = GOOD;
        Reply(rx, (char *)&res, sizeof(uartserver_response));
      }
    } else if (req.type == RELEASE_LOCK) {

      KASSERT(owned_by == client, "You must own the lock to uart");

      void *next_void;

      int status = cb_pop_front(&lock_cb, &next_void);

      if (status == CB_EMPTY) {
        owned_by = -1;
      } else {
        owned_by = (task_tid)next_void;
        if (can_send){
          can_send = false;
          res.data = 0;
          res.type = GOOD;
          uart_put_char(COM1, req.data);
          Reply(tx, (char *)&res, sizeof(uartserver_response));
          Reply(owned_by, (char *)&res, sizeof(uartserver_response));
        } else {
          want_send = true;
        }
      }

      res.data = 0;
      res.type = GOOD;
      Reply(client, (char *)&res, sizeof(uartserver_response));
    }
  }
}
