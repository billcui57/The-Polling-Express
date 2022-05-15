#include <hal.h>

void uart_put_str_block(uart *u, char *s) {
  while (*s) {
    while (!uart_can_write(u))
      ;
    uart_put_char(u, *s);
    s++;
  }
}
