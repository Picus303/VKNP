// Verifies that the VulkanContext class can be initialized without errors

#include "VulkanContext.hpp"

#include <cassert>
#include <iostream>


int main() {
    try {
        VulkanContext& context = VulkanContext::getContext();
        (void)context.getInstance();    // Avoid warning for unused variable
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}