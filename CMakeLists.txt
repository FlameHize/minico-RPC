cmake_minimum_required(VERSION 2.8)
project(minico-rpc)

set(CMAKE_BUILD_TYPE DEBUG)
set(CMAKE_CXX_FLAGS "-std=c++11 -Wall")
set(CMAKE_CXX_FLAGS_DEBUG  "-std=c++11 -O3 -fopenmp -pthread -g -ggdb")


set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin) 
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)    

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/include/tcp)
include_directories(${PROJECT_SOURCE_DIR}/include/rpc)

add_subdirectory(src)
add_subdirectory(example)
