#include "timer.h"

#include <stdint.h>

unsigned int *get_control(int timer_num) {
  switch (timer_num) {
  case TIMER3:
    return (unsigned int *)(TIMER3_BASE + CRTL_OFFSET);
  case TIMER1:
    // used for tick interrupt
    return (unsigned int *)(TIMER1_BASE + CRTL_OFFSET);
  default:
    break;
  }
}

unsigned int *get_load(int timer_num) {
  switch (timer_num) {
  case TIMER3:
    return (unsigned int *)(TIMER3_BASE + LDR_OFFSET);
  case TIMER1:
    // used for tick interrupt
    return (unsigned int *)(TIMER1_BASE + LDR_OFFSET);
  default:
    break;
  }
}

unsigned int *get_value(int timer_num) {
  switch (timer_num) {
  case TIMER3:
    return (unsigned int *)(TIMER3_BASE + VAL_OFFSET);
  case TIMER1:
    // used for tick interrupt
    return (unsigned int *)(TIMER1_BASE + VAL_OFFSET);
  default:
    break;
  }
}

void timer_init(int timer_num) {
  unsigned int *control = get_control(timer_num);
  unsigned int *load = get_load(timer_num);
  switch (timer_num) {
  case TIMER1:
    // FASTCLOCK
    // PMODE
    // load value is tick
    *control = *control | CLKSEL_MASK | MODE_MASK;
    *load = TIMER1LOAD;
    break;
  case TIMER3:

    // FASTCLOCK
    // PMODE
    *control = *control | CLKSEL_MASK | MODE_MASK;
    *load = TIMER3LOAD;
    break;
  default:

    break;
  }
}

void start_timer(int timer_num) {

  unsigned int *control = get_control(timer_num);
  *control = *control | ENABLE_MASK;
}

void stop_timer(int timer_num) {
  unsigned int *control = get_control(timer_num);
  *control = *control & ~ENABLE_MASK;
}

void read_timer(int timer_num, unsigned int *val) {
  unsigned int *value = get_value(timer_num);
  *val = *value;
}

int ticks_to_mins(unsigned int time_passed, unsigned int clock_rate) {
  return time_passed / (clock_rate * 60);
}

int ticks_to_secs(unsigned int time_passed, unsigned int clock_rate) {
  return (time_passed / clock_rate) % 60;
}

int ticks_to_tens_secs(unsigned int time_passed, unsigned int clock_rate) {
  return (time_passed / (clock_rate / 10)) % 10;
}

void get_formatted_curr_time(unsigned int curr_time, int *result,
                             int tick_speed) {

  unsigned int mins, secs, tens;

  mins = ticks_to_mins(curr_time, tick_speed);
  secs = ticks_to_secs(curr_time, tick_speed);
  tens = ticks_to_tens_secs(curr_time, tick_speed);

  result[0] = mins;
  result[1] = secs;
  result[2] = tens;
}

unsigned int ticks_to_ms(unsigned int ticks, unsigned int clock_rate) {
  return (
      unsigned int)((((unsigned long long)ticks * (unsigned long long)1000)) /
                    (unsigned long long)clock_rate);
}
