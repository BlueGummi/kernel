cmake_minimum_required(VERSION 3.16)
project(Kernel C ASM_NASM)

set(CMAKE_C_STANDARD 11)

enable_language(ASM_NASM)
set(CMAKE_ASM_NASM_FLAGS "-F dwarf -g -Wall -f elf64")

file(GLOB_RECURSE C_SOURCES src/*.c)
file(GLOB_RECURSE ASM_SOURCES src/*.S)
file(GLOB_RECURSE NASM_SOURCES src/*.asm)

include_directories(src freestnd-c-hdrs)

add_executable(kernel ${C_SOURCES} ${ASM_SOURCES} ${NASM_SOURCES})

install(TARGETS kernel DESTINATION bin-${ARCH})

