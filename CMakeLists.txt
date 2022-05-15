cmake_minimum_required(VERSION 3.0)

project(kernel C ASM)

include_directories(include)

add_executable(kernel
src/main.c
src/task.c
src/hal/ts7200.c
src/hal.c
src/hal/context.s
src/hal/context.c
src/syscall.c
src/user.c
)
target_link_libraries(kernel -static gcc)
