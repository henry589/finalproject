cmake_minimum_required(VERSION 3.14)
project(OthelloMCTS)

set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} --allow-unsupported-compiler")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(Torch_DIR "C:/Users/User/source/libtorch-win-shared-with-deps-2.6.0+cu118/libtorch/share/cmake/Torch")
find_package(Torch REQUIRED)

# Force optimization flags for Release build
if(CMAKE_BUILD_TYPE MATCHES "Release")
    message(STATUS "Enabling maximum optimizations for Release build")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native -flto -ffast-math")
endif()

add_executable(othello src/main.cpp src/mcts.cpp src/nodeManager.cpp src/bitboard.cpp src/memoryPool.cpp src/misc.cpp src/othelloNet.cpp "include/residual_block.h" "test/testsuite.cpp" "include/fsmHeuristic.h")
target_link_libraries(othello "${TORCH_LIBRARIES}")

# Ensure LTO (Link Time Optimization) is applied
set_property(TARGET othello PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
add_custom_command(TARGET othello POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "C:/Users/User/source/libtorch-win-shared-with-deps-2.6.0+cu118/libtorch/lib" $<TARGET_FILE_DIR:othello>
)