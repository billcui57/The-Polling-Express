#include <kprintf.h>
#include <my_assert.h>
#include <stdint.h>

#include "neutron.h"

uint32_t SquareRoot(uint32_t a_nInput);

// steady_tail(kmm),steady_vel (kmm/tick),
// steady_start(tick), steady_stop(tick),
// accel(kmm/tick^2)*1000, deaccel(kmm/tick^2)*1000,
// magic1, magic2

// x*1000/magic1 = t **2 for x< steady_tail
// magic2 * t / 1000 = stop
const uint32_t constants[][8] = {
    {293005, 1526, 205, 179, 7448, 8520, 6979, 874},   // 1 7
    {1927195, 4707, 496, 322, 9483, 14595, 7822, 650}, // 1 12
    {510000, 1000, 1000, 20, 1000, 50000, 510, 20},    // 2 DUMMY
    {510000, 1000, 1000, 20, 1000, 50000, 510, 20},
    {359022, 1582, 252, 202, 6288, 7821, 5671, 804}, // 24
    {1687417, 4523, 384, 363, 11792, 12474, 11470, 945},
    {300653, 1445, 215, 201, 6707, 7200, 6478, 932}, // 58
    {1722952, 4338, 444, 350, 9771, 12380, 8741, 789},
    {1108617, 3198, 421, 272, 7591, 11755, 6247, 646}, // 74
    {2447698, 5761, 586, 264, 9833, 21835, 7130, 450},
    {333698, 1246, 343, 193, 3632, 6469, 2836, 562}, // 78
    {1755431, 3726, 626, 317, 5956, 11765, 4486, 506},
};

// priority
// speed 14 steady
// speed 7 steady
// speed 7 accel

// 0-[a]->steady-[b]->deaccel-[c]->stop
void create_neutron(neutron *n, int train, uint32_t distance, bool force_slow) {
  const uint32_t *vals;
  int offset = 0;
  n->distance = distance;
  if (v_p_train_num(train) == 1) {
    offset = 0;
  } else if (v_p_train_num(train) == 2) {
    offset = 2;
  } else if (v_p_train_num(train) == 24) {
    offset = 4;
  } else if (v_p_train_num(train) == 58) {
    offset = 6;
  } else if (v_p_train_num(train) == 74) {
    offset = 8;
  } else if (v_p_train_num(train) == 78) {
    offset = 10;
  } else {
    offset = 6;
  }
  if (distance < constants[offset][0]) {
    n->speed = 7;
    vals = constants[offset];
    n->time_a = SquareRoot(distance * 1000 / constants[offset][6]);
    n->dist_b = 0;
    n->time_c = constants[offset][7] * n->time_a / 1000;
  } else {
    if (true) {
      vals = constants[offset];
      n->speed = 7;
    } else {
      vals = constants[offset];
      n->speed = 12;
    }
    n->time_a = vals[2];
    n->dist_b = n->distance - vals[0];
    n->time_c = vals[3];
  }
  n->accel = vals[4];
  n->deaccel = vals[5];
  n->steady = vals[1];
  n->dist_a = n->accel * n->time_a * n->time_a / 2000;
  n->time_b = n->dist_b / n->steady;
}

uint32_t find_time(const neutron *n, uint32_t distance) {
  if (distance < n->dist_a) {
    return SquareRoot(distance * 2000 / n->accel);
  } else if (distance < n->dist_a + n->dist_b) {
    return ((distance - n->dist_a) / n->steady) + n->time_a;
  } else if (distance < n->distance) {
    return n->time_a + n->time_b + n->time_c -
           SquareRoot(n->time_c * n->time_c - 2 * distance * 1000 / n->deaccel);
  }
  KASSERT(0, "Impossible Distance");
}

void adjust_offset(neutron *n, int train, int32_t offset) {
  if (offset < 0) return;
  int len = n->accel * offset * offset / 2000;
  create_neutron(n, train, n->distance + len, true);

}

// https://stackoverflow.com/questions/1100090/looking-for-an-efficient-integer-square-root-algorithm-for-arm-thumb2
uint32_t SquareRoot(uint32_t a_nInput) {
  uint32_t op = a_nInput;
  uint32_t res = 0;
  uint32_t one = 1uL << 30; // The second-to-top bit is set: use 1u << 14 for
                            // uint16_t type; use 1uL<<30 for uint32_t type

  // "one" starts at the highest power of four <= than the argument.
  while (one > op) {
    one >>= 2;
  }

  while (one != 0) {
    if (op >= res + one) {
      op = op - (res + one);
      res = res + 2 * one;
    }
    res >>= 1;
    one >>= 2;
  }
  return res;
}
