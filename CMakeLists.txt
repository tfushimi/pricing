cmake_minimum_required(VERSION 3.16)
project(quant_finance)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(numerics)

add_executable(quant_finance main.cpp)
target_link_libraries(quant_finance PRIVATE numerics)
