#include "virtual.h"

p_train_num supported_trains[MAX_NUM_TRAINS] = {1, 24, 58, 74, 78, 79};

p_train_num v_p_train_num(v_train_num num) { return supported_trains[num]; }
