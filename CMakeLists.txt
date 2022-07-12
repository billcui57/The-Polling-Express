cmake_minimum_required(VERSION 3.0)

project(kernel C ASM)

set(CMAKE_C_FLAGS " ${CMAKE_C_FLAGS} -O3")

include_directories(include/common)
include_directories(include/kernel)
include_directories(include/user)
include_directories(include/hal)

add_executable(kernel
  src/main.c
  src/kernel/task.c
  src/hal/ts7200.c
  src/hal/context.s
  src/hal/context.c
  src/kernel/syscall.c
  src/common/my_assert.c
  src/common/kprintf.c
  src/user/user.c
  src/hal/timer.c
  src/common/hashtable.c
  src/user/nameserver.c
  src/user/clockserver.c
  src/user/uartserver.c
  src/user/trainserver.c
  src/user/straightpathworker.c
  src/user/pathserver.c
  src/common/circularbuffer.c
  src/common/heap.c
  src/common/my_string.c
  src/common/my_memory.c
  src/user/track_data.c
  src/user/magic_numbers.c
  src/user/algorithms.c
  src/user/shell.c
  src/user/idle.c
  src/user/timerprinter.c
  src/user/switchprinter.c
  src/user/sensorcourier.c
  src/user/dispatchhub.c
  src/user/virtual.c
  src/user/sensorprinter.c
  src/user/subscribeprinter.c
)
target_link_libraries(kernel -static gcc)

# Unit Testing
# add_executable(hashtable
# tests/unit/hashtable.c
# src/hashtable.c
# )
# target_link_libraries(hashtable -static gcc)
# enable_testing()
# add_test(hashtable_test hashtable)
# add_executable(circularbuffer
# tests/unit/circularbuffer.c
# src/common/circularbuffer.c
# )
# target_link_libraries(circularbuffer)
# enable_testing()
# add_test(circularbuffer_test circularbuffer)
add_executable(algorithms
  tests/unit/algorithms.c
  src/user/algorithms.c
  src/user/track_data.c
)
target_link_libraries(algorithms)
enable_testing()
add_test(algorithms_test algorithms)
