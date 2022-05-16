#ifndef TIMER_H
#define TIMER_H

#include "ts7200.h"
#include <stdbool.h>

#define NOTDEFINED -99999

#define PMODE 1     // periodic mode
#define FMODE 0     // freerunning mode
#define SLOWCLOCK 0 // 2kHz
#define FASTCLOCK 1 // 508kHz

#define FASTCLOCKRATE 508000 // 508000 times/sec

#define TIMER3INITVAL 4294967295

#define ETDNE 1 // Timer num does not exist

typedef struct timer
{
  int num;
  bool is_enabled;
  int *control; // actual register for changing
  int *load;
  int *value;
  int clock_rate;
} timer;

int timer_init(timer *t, int timer_num);
int stop_timer(timer *t);
int start_timer(timer *t);
int read_timer(timer *t, unsigned int *val);
int wait(timer *t, unsigned int ticks);
bool has_time_elapsed(timer *t, unsigned int start_time, unsigned int ticks);
void get_formatted_curr_time(timer *t, int *result);

int ticks_to_micro_secs_full(unsigned int ticks, unsigned int clock_rate);
int tens_secs_to_ticks(unsigned int tens_secs, unsigned int clock_rate);

#endif // TIMER_H