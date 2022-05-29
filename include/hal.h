#pragma once

#include <stdbool.h>
#include <stdint.h>

#define COM1 0
#define COM2 1

typedef struct {
  void *base_addr;
} uart;

extern uart pc;

bool uart_init(uart *u, int channel);
bool uart_can_read(uart *u);
bool uart_can_write(uart *u);
void uart_put_char(uart *u, uint8_t c);
uint8_t uart_get_char(uart *u);

void uart_put_str_block(uart *u, char *s);
void panic(char *s);
void enable_cache();
