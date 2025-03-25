cmake_minimum_required(VERSION 3.31.6)
project(VKNP_MAIN LANGUAGES CXX)

# Set the C++ standard to C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set build options
option(BUILD_SHARED_LIBS "Build as shared libraries" ON)
add_compile_options(-Wall -Wextra -pedantic)

# Enable ctest
enable_testing()

# Add the subdirectories
add_subdirectory(VKNP)
add_subdirectory(tests)