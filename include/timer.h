#ifndef TIMER_H
#define TIMER_H

#include "ts7200.h"
#include <stdbool.h>

#define PMODE 1     // periodic mode
#define FMODE 0     // freerunning mode
#define SLOWCLOCK 0 // 2kHz
#define FASTCLOCK 1 // 508kHz

#define FASTCLOCKRATE 508000 // 508000 times/sec

#define TIMER3INITVAL 4294967295

typedef struct timer {
  int num;
  bool is_enabled;
  unsigned int *control; // actual register for changing
  unsigned int *load;
  unsigned int *value;
  int clock_rate;
} timer;

void timer_init(timer *t, int timer_num);
void stop_timer(timer *t);
// void start_timer(timer *t); INTERNAL USE ONLY! HAS SPECIAL REQUIRMENTS
void read_timer(timer *t, unsigned int *val);
bool has_time_elapsed(timer *t, unsigned int start_time, unsigned int ticks);
void get_formatted_curr_time(timer *t, int *result);

int ticks_to_micro_secs_full(unsigned int ticks, unsigned int clock_rate);
int tens_secs_to_ticks(unsigned int tens_secs, unsigned int clock_rate);

#endif // TIMER_H
