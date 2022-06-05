#pragma once

#include "rpsserver.h"
#include <kprintf.h>
#include <memory.h>
#include <my_event.h>
#include <nameserver.h>
#include <string.h>
#include <syscall.h>
#include <timer.h>

// K3
void task_k3init();
void task_k3_1();
void task_k3_2();