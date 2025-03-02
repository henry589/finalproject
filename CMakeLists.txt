cmake_minimum_required(VERSION 3.14)
project(OthelloMCTS)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Force optimization flags for Release build
if(CMAKE_BUILD_TYPE MATCHES "Release")
    message(STATUS "Enabling maximum optimizations for Release build")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native -flto -ffast-math")
endif()

add_executable(othello src/main.cpp src/mcts.cpp src/nodeManager.cpp src/bitboard.cpp src/memoryPool.cpp src/misc.cpp)
target_link_libraries(othello "${TORCH_LIBRARIES}")

# Ensure LTO (Link Time Optimization) is applied
set_property(TARGET othello PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
