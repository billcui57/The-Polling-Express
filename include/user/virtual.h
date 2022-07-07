#pragma once

#define MAX_NUM_TRAINS 6

typedef int v_train_num;

typedef int p_train_num;

extern p_train_num supported_trains[MAX_NUM_TRAINS];

p_train_num v_p_train_num(v_train_num num);