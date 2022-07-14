#include "virtual.h"

p_train_num supported_trains[MAX_NUM_TRAINS] = {1, 24, 58, 74, 78, 79};

p_train_num v_p_train_num(v_train_num num) { return supported_trains[num]; }

v_train_num p_v_train_num(p_train_num num) {
  for (int i = 0; i < MAX_NUM_TRAINS; i++) {
    if (supported_trains[i] == num) {
      return i;
    }
  }

  return -1;
}
