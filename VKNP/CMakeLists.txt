# List all source files
file(GLOB_RECURSE VKNP_SOURCES src/*.cpp)

add_library(VKNP ${VKNP_SOURCES})
target_include_directories(VKNP PUBLIC include)

# Add required packages
find_package(Vulkan REQUIRED)
target_include_directories(VKNP PRIVATE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(VKNP PRIVATE ${Vulkan_LIBRARIES})