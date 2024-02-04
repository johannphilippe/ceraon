cmake_minimum_required(VERSION 3.5)

project(aion_engine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake/Modules/)

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

option(TESTS "Build test executable" ON)
option(BUILD_FFT_NODE "Build FFT node" ON)
option(BUILD_FAUST_JIT_NODE "Build Faust JIT (require LLVM) node" ON)
option(BUILD_CSOUND_NODE "Build Csound node" ON)

set(PLUGIN_NODES_LIB "")
#############################################
#  Build submodules and corresponding nodes
#############################################
add_subdirectory(./src/rtaudio)

if(BUILD_FFT_NODE)
      add_library(fft_node STATIC
      ./src/fft/AudioFFT/AudioFFT.h 
      ./src/fft/AudioFFT/AudioFFT.cpp
      ./src/fft/fft_node.h)
      target_include_directories(fft_node PUBLIC ./src)
      set(PLUGIN_NODES_LIB ${PLUGIN_NODES_LIB} fft_node)
endif()

if(BUILD_FAUST_JIT_NODE)
      add_library(faust_jit STATIC 
      ./src/faust/faust_jit_node.h)
      set_target_properties(faust_jit PROPERTIES LINKER_LANGUAGE CXX)
      target_include_directories(faust_jit PUBLIC ./src)
      target_link_libraries(faust_jit PUBLIC faust)
      set(PLUGIN_NODES_LIB ${PLUGIN_NODES_LIB} faust_jit)
endif()

if(BUILD_CSOUND_NODE)
      find_package(Csound REQUIRED)
      add_library(csound_node STATIC
      ./src/csound/csound_node.h)
      set_target_properties(csound_node PROPERTIES LINKER_LANGUAGE CXX)
      target_include_directories(csound_node PUBLIC ./src ${CSOUND_INCLUDE_DIRS})
      target_link_libraries(csound_node PUBLIC ${CSOUND_LIBRARIES})
      set(PLUGIN_NODES_LIB ${PLUGIN_NODES_LIB} csound_node)
endif()

#string(STRIP ${PLUGIN_NODES_LIB} PLUGIN_NODES_LIB)

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
    )

## Library
add_library(combinator3000 SHARED
      ${COMBINATOR_SRC})

target_link_libraries(combinator3000 PRIVATE pthread)
target_link_libraries(combinator3000 PUBLIC ${PLUGIN_NODES_LIB})

#############################################
#    	Build Combinator 3000 tests
#############################################

if(TESTS)
      add_executable(combinator3000_test
            ${COMBINATOR_SRC}
            ./src/faust/tests/osc.hpp
            ./src/faust/tests/square.hpp
            ./src/faust/tests/filter.hpp
            ./src/faust/tests/fftdel.hpp
            ./src/faust/tests/fftfreeze.hpp
            ./src/tests/test.cpp
            )

      target_link_libraries(combinator3000_test PUBLIC pthread sndfile rtaudio)
      target_link_libraries(combinator3000_test PUBLIC ${PLUGIN_NODES_LIB})
      target_include_directories(combinator3000_test PUBLIC ./src)
endif()