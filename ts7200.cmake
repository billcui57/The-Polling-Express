set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_C_COMPILER "arm-none-eabi-gcc")
set(CMAKE_CXX_COMPILER "arm-none-eabi-g++")
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS )

set(CMAKE_C_FLAGS_INIT "-fPIC -ffreestanding -Wall -mcpu=arm920t -msoft-float")
set(CMAKE_CXX_FLAGS_INIT "-fPIC -ffreestanding -Wall -mcpu=arm920t -msoft-float")
set(CMAKE_EXE_LINKER_FLAGS_INIT "-T ${CMAKE_CURRENT_SOURCE_DIR}/src/linker.ld -nostdlib -Wl,-nmagic")

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
