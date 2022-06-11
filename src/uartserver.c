#include "uartserver.h"

#include <syscall.h>

void uart_com2_tx_notifier() {
  int parent = MyParentTid();
  int junk = 0;
  uartserver_request req;
  req.type = NOTIFIER_TX_GOOD;
  req.data = 0;
  for (;;) {
    // bw_uart_put_char(COM2, 'W');
    AwaitEvent(UART2_TX_HALF_EMPTY);
    // bw_uart_put_char(COM2, 'S');
    Send(parent, (char *)&req, sizeof(uartserver_request), (char *)&junk, 0);
  }
}

void uart_com2_tx_server() {
  RegisterAs("uart2txserver");
  Create(10, uart_com2_tx_notifier);

  uartserver_request req;
  uartserver_response res;
  task_tid client;

  int MAX_CAPACITY = 1000;
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
      res.data = 0;
      res.type = GOOD;

      // bw_uart_put_char(COM2, 'N');
      Reply(client, (char *)&res, sizeof(uartserver_response));

      if ((owned_by != -1) && (char_buffer[owned_by] != 0)) {
        uart_put_char(COM2, char_buffer[owned_by]);
        char_buffer[owned_by] = 0;
        res.data = 0;
        res.type = GOOD;
        Reply(owned_by, (char *)&res, sizeof(uartserver_response));
      }

    } else if (req.type == SEND_CHAR) {
      char send_char = (char)req.data;

      if ((owned_by == client) || (owned_by == -1)) {

        if (owned_by == -1) {
          owned_by = client;
        }

        if (uart_can_write(COM2)) {
          uart_put_char(COM2, send_char);
          res.data = 0;
          res.type = GOOD;
          Reply(owned_by, (char *)&res, sizeof(uartserver_response));
        } else {
          char_buffer[owned_by] = send_char;
          enable_interrupt(UART2TXINTR);
        }
      } else {
        cb_push_back(&lock_cb, (void *)client);
        char_buffer[client] = send_char;
      }

    } else if (req.type == RELEASE_LOCK) {

      KASSERT(owned_by == client, "You must own the lock to uart");

      void *next_void;

      int status = cb_pop_front(&lock_cb, &next_void);

      if (status == CB_EMPTY) {
        owned_by = -1;
      } else {
        owned_by = (task_tid)next_void;
      }
      enable_interrupt(UART2TXINTR);

      res.data = 0;
      res.type = GOOD;
      Reply(client, (char *)&res, sizeof(uartserver_response));
    }
  }
}
