#ifndef TIMER_H
#define TIMER_H

#include "ts7200.h"
#include <stdbool.h>

#define PMODE 1     // periodic mode
#define FMODE 0     // freerunning mode
#define SLOWCLOCK 0 // 2kHz
#define FASTCLOCK 1 // 508kHz

#define FASTCLOCKRATE 508000 // 508000 times/sec

#define TIMER3LOAD 4294967295
#define TIMER1LOAD 508 * 10 // tick

void timer_init(int timer_num);
void start_timer(int timer_num);
void stop_timer(int timer_num);
void read_timer(int timer_num, unsigned int *val);

void get_formatted_curr_time(unsigned int curr_time, int *result,
                             int tick_speed);

unsigned int ticks_to_ms(unsigned int ticks, unsigned int clock_rate);

#endif // TIMER_H
