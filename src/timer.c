#include "timer.h"

int timer_init(timer *t, int timer_num) {
  switch (timer_num) {
  case TIMER3:
    t->control = (int *)(TIMER3_BASE + CRTL_OFFSET);
    t->load = (int *)(TIMER3_BASE + LDR_OFFSET);
    t->value = (int *)(TIMER3_BASE + VAL_OFFSET);

    // FASTCLOCK
    // PMODE
    *(t->control) = *(t->control) | CLKSEL_MASK | MODE_MASK;
    *(t->load) = *(t->load) | TIMER3INITVAL;
    t->clock_rate = FASTCLOCKRATE;
    break;
  default:
    return ETDNE;
    break;
  }
  t->is_enabled = false;
  t->num = timer_num;

  return 0;
}

int stop_timer(timer *t) {
  t->is_enabled = false;
  *(t->control) = *(t->control) & ~ENABLE_MASK;

  return 0;
}

int start_timer(timer *t) {

  t->is_enabled = true;
  *(t->control) = *(t->control) | ENABLE_MASK;

  return 0;
}

int read_timer(timer *t, unsigned int *val) {
  *val = *(t->value);
  return 0;
}

int wait(timer *t, unsigned int ticks) {
  unsigned int start_time = 0;
  int status = read_timer(t, &start_time);

  unsigned int end_time = start_time;
  while (start_time - end_time < ticks) {
    status = read_timer(t, &end_time);
  }
}

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
  int status = read_timer(t, &curr_time);
  return start_time - curr_time > ticks;
}