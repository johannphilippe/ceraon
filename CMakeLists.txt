cmake_minimum_required(VERSION 3.5)

project(aion_engine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(COMBINATOR_SRC
    ./src/combinator3000.h
    ./src/combinator3000.cpp
    ./src/faust_node.h
    ./src/asciiplotter/asciiplotter.h
    ./src/asciiplotter/asciiplotter.cpp
    ./src/halfband/halfband.cpp
    ./src/halfband/halfband.h
    )

## Library
add_library(combinator3000 SHARED
      ${COMBINATOR_SRC})

target_link_libraries(combinator3000 PRIVATE
   pthread
   )

## Test
add_executable(combinator3000_test
      ${COMBINATOR_SRC}
      ./src/faust/osc.hpp
      ./src/faust/square.hpp
      ./src/faust/filter.hpp
      ./src/test.cpp
	)
target_link_libraries(combinator3000_test PRIVATE
   pthread
   sndfile
   )