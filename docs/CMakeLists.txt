cmake_minimum_required(VERSION 3.15)
project(MSDScript)

set(CMAKE_CXX_STANDARD 17)

add_library(MSDLib STATIC cont.cpp env.cpp expr.cpp macros.hpp parse.cpp step.cpp value.cpp)
add_executable(MSDScript catch.hpp cont.cpp cont.hpp env.cpp env.hpp expr.cpp expr.hpp macros.hpp parse.cpp parse.hpp step.cpp step.hpp value.cpp value.hpp main.cpp)