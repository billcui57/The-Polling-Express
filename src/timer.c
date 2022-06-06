#include "timer.h"

#include <stdint.h>

void start_timer(timer *t);

void timer_init(timer *t, int timer_num) {
  switch (timer_num) {
  case TIMER3:
    t->control = (unsigned int *)(TIMER3_BASE + CRTL_OFFSET);
    t->load = (unsigned int *)(TIMER3_BASE + LDR_OFFSET);
    t->value = (unsigned int *)(TIMER3_BASE + VAL_OFFSET);

    *(t->load) = 508 * 10;
    *(t->control) = CLKSEL_MASK | MODE_MASK; // FASTCLOCK, PMODE
    t->clock_rate = FASTCLOCKRATE;
    break;
  default:

    break;
  }
  t->is_enabled = false;
  t->num = timer_num;
  start_timer(t);
}

void stop_timer(timer *t) {
  t->is_enabled = false;
  *(t->control) = *(t->control) & ~ENABLE_MASK;
}

void start_timer(timer *t) {

  t->is_enabled = true;
  *(t->control) = *(t->control) | ENABLE_MASK;
}

void read_timer(timer *t, unsigned int *val) { *val = *(t->value); }

int ticks_to_mins(unsigned int time_passed, unsigned int clock_rate) {
  return time_passed / (FASTCLOCKRATE * 60);
}

int ticks_to_secs(unsigned int time_passed, unsigned int clock_rate) {
  return (time_passed / FASTCLOCKRATE) % 60;
}

int ticks_to_tens_secs(unsigned int time_passed, unsigned int clock_rate) {
  return (time_passed / (FASTCLOCKRATE / 10)) % 10;
}

void get_formatted_curr_time(timer *t, int *result) {
  unsigned int curr_time = 0;
  read_timer(t, &curr_time);

  unsigned int mins, secs, tens;

  if (t->num == TIMER3) {
    mins = ticks_to_mins(TIMER3INITVAL - curr_time, FASTCLOCKRATE);
    secs = ticks_to_secs(TIMER3INITVAL - curr_time, FASTCLOCKRATE);
    tens = ticks_to_tens_secs(TIMER3INITVAL - curr_time, FASTCLOCKRATE);
  }

  result[0] = mins;
  result[1] = secs;
  result[2] = tens;
}

int tens_secs_to_ticks(unsigned int tens_secs, unsigned int clock_rate) {
  return (tens_secs * clock_rate) / 10;
}

int ticks_to_micro_secs_full(unsigned int ticks, unsigned int clock_rate) {
  return (ticks * 1000000) / clock_rate;
}

bool has_time_elapsed(timer *t, unsigned int start_time, unsigned int ticks) {
  unsigned int curr_time = 0;
  read_timer(t, &curr_time);
  return start_time - curr_time > ticks;
}

void timer4_init() { *(unsigned int *)(TIMER4_BASE + 4) = 1 << 8; }

unsigned int timer4_read() {
  uint64_t a = *(volatile unsigned int *)TIMER4_BASE;
  uint64_t b = *(volatile unsigned int *)(TIMER4_BASE + 4);
  return ((b & 0xff) << 32 | a) / 983;
}
