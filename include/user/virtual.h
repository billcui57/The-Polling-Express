#pragma once

#define MAX_NUM_TRAINS 6

typedef int v_train_num;

typedef int p_train_num;

int supported_trains[MAX_NUM_TRAINS] = {72,2,33,4,5,6};

p_train_num v_p_train_num(v_train_num num);