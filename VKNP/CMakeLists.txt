add_library(VKNP src/VKNP.cpp)
target_include_directories(VKNP PUBLIC include)

# Add required packages
find_package(Vulkan REQUIRED)
target_include_directories(VKNP PUBLIC ${Vulkan_INCLUDE_DIRS})
target_link_libraries(VKNP PUBLIC ${Vulkan_LIBRARIES})