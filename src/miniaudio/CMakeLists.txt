cmake_minimum_required(VERSION 3.15)

project(Miniaudio LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_C_FLAGS_DEBUG "-g")
set(CMAKE_C_FLAGS_RELEASE "-O3")

add_library(miniaudio STATIC miniaudio.c miniaudio.h)

target_compile_definitions(miniaudio PUBLIC MINIAUDIO_IMPLEMENTATION)
set_target_properties(miniaudio PROPERTIES LINKER_LANGUAGE C)
