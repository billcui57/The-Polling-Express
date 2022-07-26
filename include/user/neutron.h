#pragma once
#include "virtual.h"
#include <stdint.h>

typedef struct {
  uint32_t speed;    // train speed
  uint32_t distance; // total distance
  uint32_t time_a;   // accel done
  uint32_t dist_a;
  uint32_t time_b; // stop start
  uint32_t dist_b;
  uint32_t time_c; // stop done
  uint32_t accel;
  uint32_t deaccel;
  uint32_t steady;
} neutron;

void create_neutron(neutron *n, int train, uint32_t distance, bool force_slow);
uint32_t find_time(const neutron *n, uint32_t distance);
void adjust_offset(neutron *n, int train, int32_t offset);
