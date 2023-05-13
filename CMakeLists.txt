cmake_minimum_required(VERSION 3.10)
project(MatrixMultithreading)

set(CMAKE_CXX_STANDARD 14)

add_executable(MatrixMultithreading main.cpp)

target_link_libraries(MatrixMultithreading pthread)