add_executable(Tests TestTemplate.cpp)
target_link_libraries(Tests PRIVATE VKNP)

add_test(NAME FirstTests COMMAND Tests)