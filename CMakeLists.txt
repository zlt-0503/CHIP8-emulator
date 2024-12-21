cmake_minimum_required(VERSION 3.10)
project(CHIP8-emulator)

set(CMAKE_CXX_STANDARD 17)

# Find SDL2
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# Add your source files
set(SOURCES
    src/CHIP8.cpp
    src/main.cpp
)

add_executable(CHIP8-emulator ${SOURCES})

# Link SDL2
target_link_libraries(CHIP8-emulator ${SDL2_LIBRARIES})
