cmake_minimum_required(VERSION 3.14)
project(OthelloMCTS)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define source files
set(SOURCES
    src/main.cpp
    src/nodeManager.cpp
    src/bitboard.cpp
    src/memoryPool.cpp
    src/misc.cpp
    src/mcts.cpp
)

# Create executable
add_executable(othello ${SOURCES})
