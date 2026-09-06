cmake_minimum_required(VERSION 3.22)
project(scene_graph_project CXX)

# Graaf and modern C++ features require C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Pinocchio package in the Conda environment
find_package(pinocchio REQUIRED)

# Define your application executable
add_executable(scene_graph_app main.cpp)

# Link Pinocchio and automatically propagate Eigen and include directories
target_link_libraries(scene_graph_app PRIVATE pinocchio::pinocchio)