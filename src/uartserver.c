#include "uartserver.h"

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
  void *backing[MAX_CAPACITY];
  circular_buffer cb;
  cb_init(&cb, backing, MAX_CAPACITY);

  char *buffered_characters[MAX_NUM_TASKS];
  memset(buffered_characters, 0, sizeof(char) * MAX_NUM_TASKS);

  for (;;) {
    Receive(&client, (char *)&req, sizeof(uartserver_request));
    // bw_uart_put_char(COM2, 'R');
    if (req.type == NOTIFIER_TX_GOOD) {
      res.data = 0;
      res.type = GOOD;

      // bw_uart_put_char(COM2, 'N');
      Reply(client, (char *)&res, sizeof(uartserver_response));

      while (uart_can_write(COM2)) {
        void *waiting_void;
        int status = cb_pop_front(&cb, &waiting_void);

        if (status == CB_EMPTY) {
          break;
        }

        if (status == 0) {

          task_tid waiting = (task_tid)waiting_void;
          char send_char = buffered_characters[waiting];
          buffered_characters[waiting] = 0;
          uart_put_char(COM2, send_char);
          res.data = 0;
          res.type = GOOD;
          Reply(waiting, (char *)&res, sizeof(uartserver_response));
        }
      }

    } else if (req.type == SEND_CHAR) {
      char send_char = (char)req.data;

      if (uart_can_write(COM2)) {
        // bw_uart_put_char(COM2, 'H');
        uart_put_char(COM2, send_char);
        res.data = 0;
        res.type = GOOD;
        Reply(client, (char *)&res, sizeof(uartserver_response));
      } else {
        // bw_uart_put_char(COM2, 'K');
        cb_push_back(&cb, (void *)client);
        buffered_characters[client] = send_char;
        enable_interrupt(UART2TXINTR);
      }
    }
  }
}
