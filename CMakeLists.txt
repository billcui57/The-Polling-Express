cmake_minimum_required(VERSION 3.0)

project(kernel)

include_directories(include)

add_executable(kernel
src/main.c
src/task.c
src/hal/ts7200.c
src/hal.c
)
target_link_libraries(kernel -static)
