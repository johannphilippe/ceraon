cmake_minimum_required(VERSION 3.5)

project(aion_engine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

option(TESTS "Build test executable" ON)
option(BUILD_FFT_NODE "Build FFT node" ON)

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
            ./src/tests/test.cpp
            )

      target_link_libraries(combinator3000_test PUBLIC pthread sndfile rtaudio)
      target_link_libraries(combinator3000_test PUBLIC ${PLUGIN_NODES_LIB})
      target_include_directories(combinator3000_test PUBLIC ./src)
endif()