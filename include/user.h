#pragma once

#include <kprintf.h>
#include <my_memory.h>
#include <nameserver.h>
#include <syscall.h>

void task_k1init();

void task_k1test();
void task_k2perf();
void task_echo();
void task_send();

void task1();
void task2();
