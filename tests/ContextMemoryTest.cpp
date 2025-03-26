// Verifies the memory management methods of the VulkanContext class

#include "VulkanContext.hpp"

#include <vulkan/vulkan.h>
#include <cassert>
#include <iostream>

#define ALLOCATION_SIZE 1024	// Note: Vulkan seams to allocate blocks of 4096 bytes so this won't be the actual memory usage


int main() {
	try {
		VulkanContext& context = VulkanContext::getContext();

		// Get the logical and physical devices
		auto physicalDevices = context.getPhysicalDevices();
		auto logicalDevices = context.getDevices();

		for (auto physicalDevice : physicalDevices) {
			auto [used, budget] = context.getMemoryUsage(physicalDevice);
			std::cout << "Used memory: " << used << " bytes, Budget: " << budget << " bytes" << std::endl;

			// Create a buffer and allocate memory
			VkBuffer buffer;
			VkDeviceMemory memory;
			context.createBufferAndMemory(logicalDevices[0], ALLOCATION_SIZE, buffer, memory);
			std::cout << "Buffer and memory created" << std::endl;

			// Get memory usage again
			auto [used2, budget2] = context.getMemoryUsage(physicalDevice);
			std::cout << "Used memory: " << used2 << " bytes, Budget: " << budget2 << " bytes" << std::endl;

			// Destroy buffer and memory
			vkDestroyBuffer(logicalDevices[0], buffer, nullptr);
			vkFreeMemory(logicalDevices[0], memory, nullptr);
		}
	} catch (const std::runtime_error& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}