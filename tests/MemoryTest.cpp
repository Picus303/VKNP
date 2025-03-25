// This second test aims at testing the memory management methods of the VulkanContext class.

#include "VulkanContext.hpp"

#include <vulkan/vulkan.h>
#include <cassert>
#include <iostream>


int main() {
	try {
		VulkanContext context;

		// Get the logical and physical devices
		auto physicalDevices = context.getPhysicalDevices();
		auto logicalDevices = context.getDevices();

		for (auto physicalDevice : physicalDevices) {
			auto [used, budget] = context.getMemoryUsage(physicalDevice);
			std::cout << "Used memory: " << used << " bytes, Budget: " << budget << " bytes" << std::endl;

			// Create a buffer and allocate memory
			VkBuffer buffer;
			VkDeviceMemory memory;
			context.createBufferAndMemory(logicalDevices[0], 1024, buffer, memory);
			std::cout << "Buffer and memory created !" << std::endl;

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