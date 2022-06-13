#pragma once

#include <stdbool.h>
#include <stdint.h>

#define COM1 0
#define COM2 1
#define BW_COM2 2

void uart_init(int channel);
bool uart_can_read(int channel);
bool uart_can_write(int channel);
void uart_put_char(int channel, uint8_t c);
uint8_t uart_get_char(int channel);
uint8_t bw_uart_get_char(int channel);
void bw_uart_put_char(int channel, uint8_t c);

// KERNEL ONLY
void enable_cache();
void enable_irq();
