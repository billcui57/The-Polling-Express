cmake_minimum_required(VERSION 3.0)

project(kernel C ASM)

include_directories(include)

add_executable(kernel
src/main.c
src/task.c
src/hal/ts7200.c
src/hal/context.s
src/hal/context.c
src/hal.c
src/syscall.c
src/my_assert.c
src/kprintf.c
src/user.c
src/timer.c
)
target_link_libraries(kernel -static gcc)


# Unit Testing
add_executable(sample
tests/unit/sample.c 
)
target_link_libraries(sample -static gcc)
enable_testing()
add_test(sample_test sample)


# Integration Testing
add_executable(context_switch_test
tests/integration/context_switch.c
src/task.c
src/hal/ts7200.c
src/hal/context.s
src/hal/context.c
src/hal.c
src/syscall.c
src/my_assert.c
src/kprintf.c
src/user.c
src/timer.c
)
target_link_libraries(context_switch_test -static gcc)

add_executable(test2_test
tests/integration/test2.c
src/task.c
src/hal/ts7200.c
src/hal/context.s
src/hal/context.c
src/hal.c
src/syscall.c
src/my_assert.c
src/kprintf.c
src/user.c
src/timer.c
)
target_link_libraries(test2_test -static gcc)
