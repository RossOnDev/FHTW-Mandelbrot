cmake_minimum_required(VERSION 4.1)
project(Mandelbrot)

set(CMAKE_CXX_STANDARD 20)

find_package(OpenMP REQUIRED)

add_executable(Mandelbrot main.cpp)
target_link_libraries(Mandelbrot OpenMP::OpenMP_CXX)