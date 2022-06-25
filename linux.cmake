set(CMAKE_SYSTEM_NAME Generic)

set(CMAKE_C_COMPILER "/usr/bin/gcc")
set(CMAKE_CXX_COMPILER "/usr/bin/g++")

set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS)

set(CMAKE_C_FLAGS_INIT "-fPIC -ffreestanding -Wall -O0 -ggdb -msoft-float")
set(CMAKE_CXX_FLAGS_INIT "-fPIC -ffreestanding -Wall -msoft-float")

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)