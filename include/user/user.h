#pragma once

#include "clockserver.h"
#include "dispatchhub.h"
#include "pathserver.h"
#include "shell.h"
#include "trainserver.h"
#include "uartserver.h"
#include <kprintf.h>
#include <memory.h>
#include <my_event.h>
#include <nameserver.h>
#include <string.h>
#include <syscall.h>
#include <timer.h>

void task_k4_init();

void idle();
