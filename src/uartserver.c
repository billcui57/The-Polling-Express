#include "uartserver.h"

void uart_com2_tx_notifier() {
  int parent = MyParentTid();
  int junk = 0;
  uartserver_request req;
  req.type = NOTIFIER_TX_GOOD;
  req.data = 0;
  enable_interrupt(UART2TXINTR);
  for (;;) {
    AwaitEvent(UART2_TX_HALF_EMPTY);
    Send(parent, (char *)&req, sizeof(uartserver_request), (char *)&junk, 0);
  }
}

void uart_com2_tx_server() {
  RegisterAs("uart2txserver");
  Create(10, uart_com2_tx_notifier);

  int state = NO_SEND;

  uartserver_request req;
  uartserver_response res;
  task_tid client;

  int MAX_CAPACITY = 1000;
  void *backing[MAX_CAPACITY];
  circular_buffer cb;
  cb_init(&cb, backing, MAX_CAPACITY);

  for (;;) {
    Receive(&client, (char *)&req, sizeof(uartserver_request));
    if (req.type == NOTIFIER_TX_GOOD) {
      state = CAN_SEND;
      res.data = 0;
      res.type = GOOD;

      Reply(client, (char *)&res, sizeof(uartserver_response));

      void *waiting_void;
      int status = cb_pop_front(&cb, &waiting_void);

      if (status == 0) {
        task_tid waiting = (task_tid)waiting_void;
        res.data = 0;
        res.type = GOOD;
        Reply(waiting, (char *)&res, sizeof(uartserver_response));
      }

    } else if (req.type == SEND_CHAR) {
      char send_char = (char)req.data;
      enable_interrupt(UART2TXINTR);
      if (state == CAN_SEND) {
        uart_put_char(COM2, send_char);
        state = NO_SEND;
        res.data = 0;
        res.type = GOOD;
        Reply(client, (char *)&res, sizeof(uartserver_response));
      } else if (state == NO_SEND) {
        cb_push_back(&cb, (void *)client);
      }
    }
  }
}
