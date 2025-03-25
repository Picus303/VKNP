# List all test files
file(GLOB_RECURSE TEST_SOURCES *.cpp)

# Find required packages
find_package(Vulkan REQUIRED)

# Add executables
foreach(TEST_SOURCE ${TEST_SOURCES})
	get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
	add_executable(${TEST_NAME} ${TEST_SOURCE})
	target_link_libraries(${TEST_NAME} PRIVATE VKNP)

	# Add Vulkan
	target_include_directories(${TEST_NAME} PRIVATE ${Vulkan_INCLUDE_DIRS})
	target_link_libraries(${TEST_NAME} PRIVATE ${Vulkan_LIBRARIES})

	add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})

endforeach()

# Add test dependencies
set_tests_properties(MemoryTest PROPERTIES DEPENDS InitTest)