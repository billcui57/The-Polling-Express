#include <hal.h>
#include <ts7200.h>

int get_base_addr(int channel) {
  switch (channel) {
  case COM1:
    return UART1_BASE;
    break;
  case COM2:
    return UART2_BASE;
    break;
  }
}

void uart_init(int channel) {
  int *low = get_base_addr(channel) + UART_LCRL_OFFSET;
  int *med = get_base_addr(channel) + UART_LCRM_OFFSET;
  int *high = get_base_addr(channel) + UART_LCRH_OFFSET;

  switch (channel) {
  case COM1: // 2400 (track)
    *low = 0xBF;
    *med = 0x0;
    *high = WLEN_MASK & ~FEN_MASK | STP2_MASK & ~PEN_MASK; // FIFO disabled
    break;
  case COM2: // 115200 (terminal)
    *low = 0x3;
    *med = 0x0;
    *high = WLEN_MASK | FEN_MASK & ~STP2_MASK & ~PEN_MASK; // FIFO enabled
    break;
  }
}

bool uart_can_read(int channel) {
  return !(*(int *)(get_base_addr(channel) + UART_FLAG_OFFSET) & RXFE_MASK);
}
bool uart_can_write(int channel) {
  return !(*(int *)(get_base_addr(channel) + UART_FLAG_OFFSET) & TXFF_MASK);
}
void uart_put_char(int channel, uint8_t c) {
  *(char *)(get_base_addr(channel) + UART_DATA_OFFSET) = c;
}
uint8_t uart_get_char(int channel) {
  return *(char *)(get_base_addr(channel) + UART_DATA_OFFSET);
}

uint8_t bw_uart_get_char(int channel) {
  while (!uart_can_read(channel))
    ;
  return uart_get_char(channel);
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
  __asm__ volatile("MRC p15,0,%[reg],c1,c0,0" : [ reg ] "=r"(reg));
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
