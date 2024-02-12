cmake_minimum_required(VERSION 3.15)

project(combinator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake/Modules/)

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
      set(LINUX ON)
endif()
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
      set(MACOS ON)
endif()
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
      set(WIN ON)
endif()

option(TESTS "Build test executable" ON)
option(BUILD_FAUST_JIT_NODE "Build Faust JIT (require LLVM) node" ON)
option(BUILD_CSOUND_NODE "Build Csound node" ON)
option(BUILD_SNDFILE_NODE "Build Sndfile nodes" ON)

set(PLUGIN_NODES_LIB "")
#############################################
#  Build submodules and corresponding nodes
#############################################

if(BUILD_FAUST_JIT_NODE)
      add_library(faust_jit STATIC 
      ./src/faust/faust_jit_node.h)
      set_target_properties(faust_jit PROPERTIES LINKER_LANGUAGE CXX)
      target_include_directories(faust_jit PUBLIC ./src)
      target_link_libraries(faust_jit PUBLIC faust)
      set(PLUGIN_NODES_LIB ${PLUGIN_NODES_LIB} faust_jit)
      add_compile_definitions(FAUST_JIT_NODE)
endif()

if(BUILD_CSOUND_NODE)
      find_package(Csound REQUIRED)
      add_library(csound_node STATIC
      ./src/csound/csound_node.h)
      set_target_properties(csound_node PROPERTIES LINKER_LANGUAGE CXX)
      target_include_directories(csound_node PUBLIC ./src ${CSOUND_INCLUDE_DIRS})
      target_link_libraries(csound_node PUBLIC ${CSOUND_LIBRARIES})
      set(PLUGIN_NODES_LIB ${PLUGIN_NODES_LIB} csound_node)
      add_compile_definitions(CSOUND_NODE)
endif()

if(BUILD_SNDFILE_NODE)
      add_library(sndfile_node STATIC
      ./src/sndfile/sndfile_node.h)
      set_target_properties(sndfile_node PROPERTIES LINKER_LANGUAGE CXX)
      target_include_directories(sndfile_node PUBLIC ./src)
      target_link_libraries(sndfile_node PUBLIC sndfile)
      set(PLUGIN_NODES_LIB ${PLUGIN_NODES_LIB} sndfile_node)
      add_compile_definitions(SNDFILE_NODE)
endif()


#############################################
#    	        Audio Engines
#############################################

add_subdirectory(./src/rtaudio)
add_subdirectory(./src/miniaudio)


#############################################
#    	Build Combinator 3000 library
#############################################
set(COMBINATOR_SRC
    ./src/combinator3000.h
    ./src/combinator3000.cpp
    ./src/faust/faust_node.h
    ./src/asciiplotter/asciiplotter.h
    ./src/asciiplotter/asciiplotter.cpp
    ./src/halfband/halfband.cpp
    ./src/halfband/halfband.h
    ./src/utilities.h
    ./include/combinator3000_api.h
    ./src/combinator3000_api.cpp
    ./src/comb_mem.h
    ./src/comb_mem.cpp
    ./src/fft/AudioFFT/AudioFFT.h 
    ./src/fft/AudioFFT/AudioFFT.cpp
    ./src/fft/fft_node.h
    )

## Library
add_library(combinator3000 SHARED
      ${COMBINATOR_SRC})

target_link_libraries(combinator3000 PUBLIC 
      ${PLUGIN_NODES_LIB} 
      miniaudio 
      rtaudio)

target_include_directories(combinator3000 PUBLIC 
      ./include 
      ./src 
      ./src/miniaudio)

if(LINUX)
      target_link_libraries(combinator3000 PRIVATE 
            pthread 
            asound 
            jack 
            pulse
            m
            dl
            atomic
            )
endif()


#############################################
#    	Build Combinator 3000 tests
#############################################

if(TESTS)
      add_executable(combinator3000_test
            ./src/faust/tests/osc.hpp
            ./src/faust/tests/square.hpp
            ./src/faust/tests/filter.hpp
            ./src/faust/tests/fftdel.hpp
            ./src/faust/tests/fftfreeze.hpp
            ./src/tests/test.cpp
            )

      target_link_libraries(combinator3000_test PUBLIC 
            pthread 
            sndfile 
            rtaudio 
            miniaudio 
            atomic
            combinator3000
            ${PLUGIN_NODES_LIB})

      target_include_directories(combinator3000_test PUBLIC 
            ./src 
            ./src/miniaudio
            )
      target_compile_definitions(combinator3000_test 
            PRIVATE 
            MINIAUDIO_IMPLEMENTATION)
endif()
