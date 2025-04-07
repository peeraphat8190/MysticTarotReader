cmake_minimum_required(VERSION 3.10)
project(FinalProject)

# Set C standard
set(CMAKE_C_STANDARD 99)

# Add source files
set(SOURCES
    src/main.c
)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Add executable
add_executable(FinalProject ${SOURCES})

# Link raylib (ensure raylib is installed and accessible)
find_package(raylib REQUIRED)
target_link_libraries(FinalProject raylib)