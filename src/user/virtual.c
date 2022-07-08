#include "virtual.h"

p_train_num supported_trains[MAX_NUM_TRAINS] = {72, 2, 33, 4, 5, 6};

p_train_num v_p_train_num(v_train_num num) { return supported_trains[num]; }