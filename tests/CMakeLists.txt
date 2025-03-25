add_executable(Tests InitTest.cpp)
target_link_libraries(Tests PRIVATE VKNP)

add_test(NAME RunTests COMMAND Tests)