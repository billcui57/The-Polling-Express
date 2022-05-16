#include <hal.h>
#include <ts7200.h>

bool uart_init(uart *u, int channel) {
  switch (channel) {
  case COM1:
    u->base_addr = UART1_BASE;
    break;
  case COM2:
    u->base_addr = UART2_BASE;
    break;
  default:
    return true;
    break;
  }
  int *line, *high, *low, stp;
  line = u->base_addr + UART_LCRH_OFFSET;
  high = u->base_addr + UART_LCRM_OFFSET;
  low = u->base_addr + UART_LCRL_OFFSET;
  switch (channel) {
  case COM1: // 2400
    *low = 0xBF;
    *high = 0x0;
    stp = STP2_MASK;
    break;
  case COM2: // 115200
    *low = 0x3;
    *high = 0x0;
    stp = 0;
    break;
  }
  *line = WLEN_MASK | FEN_MASK | stp;
  return false;
}

bool uart_can_read(uart *u) {
  return !(*(int *)(u->base_addr + UART_FLAG_OFFSET) & RXFE_MASK);
}
bool uart_can_write(uart *u) {
  return !(*(int *)(u->base_addr + UART_FLAG_OFFSET) & TXFF_MASK);
}
void uart_put_char(uart *u, uint8_t c) {
  *(char *)(u->base_addr + UART_DATA_OFFSET) = c;
}
uint8_t uart_get_char(uart *u) {
  return *(char *)(u->base_addr + UART_DATA_OFFSET);
}

void panic(char *s) {
  uart u;
  u.base_addr = UART2_BASE;
  uart_put_str_block(&u, s);
  while (1) {
    int i = 0;
    i++;
  }
}
