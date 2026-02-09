cmake_minimum_required(VERSION 3.10)
project(Wolfarch VERSION 0.3 LANGUAGES C)

set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED ON)

include_directories(include)
file(GLOB SRC_FILES "src/*.c")
add_executable(wolfarch ${SRC_FILES})

install(TARGETS wolfarch DESTINATION bin)
add_custom_target(uninstall COMMAND rm -f /usr/local/bin/wolfarch)