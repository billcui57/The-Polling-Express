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

uint8_t bw_uart_get_char(uart *u) {
  while (!uart_can_read(u))
    ;
  return uart_get_char(u);
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

// must invalide caches before enabling
/*
 * Need to clean and invalidate D cache index by index
 * Need to invalidate I cache
 * Finally turn D and I cache on
 */
void enable_cache() {
  for (int seg = 0; seg < 7; seg++) {
    for (int idx = 0; idx < 63; idx++) {
      int val = idx << 26 | seg << 5;
      __asm__ volatile("MCR p15,0,%[val],c7,c14,2" ::[val] "r"(val));
    }
  }
  __asm__ volatile("MCR p15,0,%[zero],c7,c5,0" ::[zero] "r"(0));
  int reg;
  __asm__ volatile("MRC p15,0,%[reg],c1,c0,0" : [reg] "=r"(reg));
  reg = reg | 1 << 12 | 1 << 2;
  __asm__ volatile("MCR p15,0,%[reg],c1,c0,0" ::[reg] "r"(reg));
}

void enable_irq() {

  *(int *)(VIC1_BASE + INT_ENABLE_OFFSET) = 0;
  *(int *)(VIC2_BASE + INT_ENABLE_OFFSET) = 0;

  // enable TC3 interrupt
  int *enable = (int *)(VIC1_BASE + INT_ENABLE_OFFSET);
  int *select = (int *)(VIC1_BASE + INT_SELECT_OFFSET);
  *select = *select & ~VIC_TIMER1_MASK; // use IRQ
  *enable = *enable | VIC_TIMER1_MASK;
}
