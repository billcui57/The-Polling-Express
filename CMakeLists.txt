cmake_minimum_required(VERSION 3.0)

project(kernel C ASM)

set(CMAKE_C_FLAGS " ${CMAKE_C_FLAGS} -O3")

include_directories(include)

add_executable(kernel
src/main.c
src/task.c
src/hal/ts7200.c
src/hal/context.s
src/hal/context.c
src/syscall.c
src/my_assert.c
src/kprintf.c
src/user.c
src/timer.c
src/hashtable.c
src/nameserver.c
src/clockserver.c
src/my_string.c
src/my_memory.c
)
target_link_libraries(kernel -static gcc)


# Unit Testing
add_executable(hashtable
tests/unit/hashtable.c 
src/hashtable.c
)
target_link_libraries(hashtable -static gcc)
enable_testing()
add_test(hashtable_test hashtable)

