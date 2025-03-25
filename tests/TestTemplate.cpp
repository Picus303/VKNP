#include "VKNP.hpp"
#include <cassert>
#include <iostream>

int main() {
    VulkanEngine obj;
    assert(obj.add(2, 3) == 5);
    std::cout << "Test passed!\n";
    return 0;
}