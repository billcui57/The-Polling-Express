#pragma once

#include "rpsserver.h"
#include <kprintf.h>
#include <memory.h>
#include <nameserver.h>
#include <syscall.h>

// K2 RPS
void task_k2rpsinit();
void task_k2rpsbot1();
void task_k2rpsbot2();

// K2 Perf
void task_k2perf();
void task_echo();
void task_send();
void task_calibrate();

// K1 Context switching test
void task_k1init();
void task_k1test();
