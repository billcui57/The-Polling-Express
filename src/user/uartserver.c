#include "uartserver.h"

#include <hal.h>
#include <syscall.h>

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

void unpark_com2_tx_notifier(task_tid notifier_tid) {
  uartserver_response res;
  memset(&res, 0, sizeof(res));
  res.data = 0;
  res.type = GOOD;
  Reply(notifier_tid, (char *)&res, sizeof(uartserver_response));
}

void uart_com2_tx_server() {
  RegisterAs("uart2txserver");

  Create(10, "UartCom2TxNotifier", uart_com2_tx_notifier);
  task_tid notifier_parking = -1;

  uartserver_request req;
  uartserver_response res;
  task_tid client;
  memset(&res, 0, sizeof(res));

  int MAX_CAPACITY = 100;
  task_tid lock_backing[MAX_CAPACITY];
  circular_buffer lock_cb;
  cb_init(&lock_cb, (void *)lock_backing, MAX_CAPACITY, sizeof(task_tid));

  char char_buffer[MAX_NUM_TASKS];

  memset(char_buffer, 0, sizeof(char) * MAX_CAPACITY);

  task_tid owned_by = -1;

  for (;;) {
    Receive(&client, (char *)&req, sizeof(uartserver_request));
    // bw_uart_put_char(COM2, 'R');
    if (req.type == NOTIFIER_TX_GOOD) {

      notifier_parking = client;

      // printf(BW_COM2, "Notifier Good\r\n");

      if ((owned_by != -1) && (char_buffer[owned_by] != 0)) {

        // printf(BW_COM2, "Client returned\r\n");
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
          // printf(BW_COM2, "Client can send\r\n");
          uart_put_char(COM2, send_char);
          res.data = 0;
          res.type = GOOD;
          Reply(owned_by, (char *)&res, sizeof(uartserver_response));
        } else {
          // printf(BW_COM2, "Client cannot send yet\r\n");

          char_buffer[owned_by] = send_char;

          if (notifier_parking != -1) {
            unpark_com2_tx_notifier(notifier_parking);
            notifier_parking = -1;
          }
        }
      } else {
        cb_push_back(&lock_cb, (void *)&client, false);
        char_buffer[client] = send_char;
      }

    } else if (req.type == RELEASE_LOCK) {
      // printf(BW_COM2, "Release lock\r\n");
      KASSERT(owned_by == client, "Uart 2 You must own the lock to uart");

      int status = cb_pop_front(&lock_cb, (void *)&owned_by);

      if (status == CB_EMPTY) {
        owned_by = -1;
      }

      if (notifier_parking != -1) {
        unpark_com2_tx_notifier(notifier_parking);
        notifier_parking = -1;
      }

      res.data = 0;
      res.type = GOOD;
      Reply(client, (char *)&res, sizeof(uartserver_response));
    }
  }
}

// NOTIFIER SEMANTICS:
// Notify when server can take action

void uart_com1_tx_notifier() {
  int parent = MyParentTid();
  int junk = 0;
  uartserver_request req;
  memset(&req, 0, sizeof(req));
  req.type = NOTIFIER_TX_GOOD;
  req.data = 0;
  while (true) {
    Send(parent, (char *)&req, sizeof(uartserver_request), (char *)&junk, 0);
    AwaitEvent(UART1_TX_INTR);
  }
}

void uart_com1_rx_notifier() {
  int parent = MyParentTid();
  int junk = 0;
  uartserver_request req;
  memset(&req, 0, sizeof(req));
  req.type = NOTIFIER_RX_GOOD;
  req.data = 0;

  while (true) {
    Send(parent, (char *)&req, sizeof(uartserver_request), (char *)&junk, 0);
    AwaitEvent(UART1_RX_INTR);
    req.data = uart_get_char(COM1);
  }
}

void uart_com1_server() {
  RegisterAs("uart1");
  uart_init(COM1);
  task_tid tx = Create(10, "UartCom1TxNotifier", uart_com1_tx_notifier);
  task_tid rx = Create(10, "UartCom1RxNotifier", uart_com1_rx_notifier);

  uartserver_request req;
  uartserver_response res;
  memset(&res, 0, sizeof(res));
  task_tid client;

  task_tid lock_backing[MAX_NUM_TASKS];
  circular_buffer lock_cb;
  cb_init(&lock_cb, (void *)lock_backing, MAX_NUM_TASKS, sizeof(task_tid));

  task_tid owned_by = -1;

  bool can_send = true;
  bool want_send = false;
  char send_buffer[MAX_NUM_TASKS];

  while (uart_can_read(COM1))
    uart_get_char(COM1);

  while (true) {
    Receive(&client, (char *)&req, sizeof(uartserver_request));
    if (req.type == NOTIFIER_TX_GOOD) {
      if (want_send) {
        want_send = false;
        res.data = 0;
        res.type = GOOD;
        uart_put_char(COM1, send_buffer[owned_by]);
        Reply(client, (char *)&res, sizeof(uartserver_response));
        Reply(owned_by, (char *)&res, sizeof(uartserver_response));
      } else {
        can_send = true;
      }
    } else if (req.type == NOTIFIER_RX_GOOD && owned_by != -1) {
      res.data = req.data;
      res.type = GOOD;
      Reply(owned_by, (char *)&res, sizeof(uartserver_response));
    } else if (req.type == SEND_CHAR) {
      if (owned_by == -1) {
        owned_by = client;
      }
      if (owned_by == client) {
        if (can_send) {
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
        cb_push_back(&lock_cb, (void *)&client, false);
        send_buffer[client] = req.data;
      }
    } else if (req.type == GET_CHAR) {
      if (uart_can_read(COM1)) {
        res.data = uart_get_char(COM1);
        res.type = GOOD;
        Reply(owned_by, (char *)&res, sizeof(uartserver_response));
      } else {
        res.data = 0;
        res.type = GOOD;
        Reply(rx, (char *)&res, sizeof(uartserver_response));
      }
    } else if (req.type == RELEASE_LOCK) {

      KASSERT(owned_by == client, "Uart 1 You must own the lock to uart");

      int status = cb_pop_front(&lock_cb, (void *)&owned_by);

      if (status == CB_EMPTY) {
        owned_by = -1;
      } else {
        if (can_send) {
          can_send = false;
          res.data = 0;
          res.type = GOOD;
          uart_put_char(COM1, send_buffer[owned_by]);
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
void uart_com2_rx_notifier() {
  int parent = MyParentTid();
  int junk = 0;
  uartserver_request req;
  memset(&req, 0, sizeof(req));
  req.type = NOTIFIER_RX_GOOD;
  req.data = 0;

  for (;;) {
    // bw_uart_put_char(COM2, 'A');
    Send(parent, (char *)&req, sizeof(uartserver_request), (char *)&junk, 0);
    // bw_uart_put_char(COM2, 'B');
    AwaitEvent(UART2_RX_INCOMING);
    // bw_uart_put_char(COM2, 'C');
  }
}

void uart_com2_rx_server() {
  RegisterAs("uart2rxserver");
  task_tid notifier_tid =
      Create(10, "UartCom2RxNotifier", uart_com2_rx_notifier);

  uartserver_request req;
  uartserver_response res;
  task_tid client;
  memset(&res, 0, sizeof(res));

  task_tid waiting = -1;

  for (;;) {

    Receive(&client, (char *)&req, sizeof(uartserver_request));
    // bw_uart_put_char(COM2, 'H');
    // bw_uart_put_char(COM2, 'R');
    if (req.type == NOTIFIER_RX_GOOD) {
      // bw_uart_put_char(COM2, 'F');
      if (waiting != -1) {
        // bw_uart_put_char(COM2, 'G');
        char c = uart_get_char(COM2);
        res.data = (int)c;
        res.type = GOOD;

        task_tid reply = waiting;
        waiting = -1;
        Reply(reply, (char *)&res, sizeof(uartserver_response));
      }

    } else if (req.type == GET_CHAR) {

      if (uart_can_read(COM2)) {
        char c = uart_get_char(COM2);
        res.data = (int)c;
        res.type = GOOD;
        Reply(client, (char *)&res, sizeof(uartserver_response));
      } else {
        waiting = client;
        res.data = 0;
        res.type = GOOD;
        Reply(notifier_tid, (char *)&res, sizeof(uartserver_response));
      }

    } else {
      // printf(BW_COM2, "\r\n%d\r\n", req.type);
      KASSERT(0, "Invalid RX server request type");
    }
  }
}
