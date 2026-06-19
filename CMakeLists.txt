cmake_minimum_required(VERSION 3.10)
project(Wolfarch VERSION 26.1 LANGUAGES C)

set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED ON)

include_directories(include assist)

file(GLOB SOURCES "src/*.c" "assist/*.c")

add_executable(wolfarch ${SOURCES})
