// This first test only tries to initiate the VulkanContext object and check if the queues are created correctly.

#include "VulkanContext.hpp"
#include <cassert>
#include <iostream>


int main() {
    try {
        VulkanContext context;
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}