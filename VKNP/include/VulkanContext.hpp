#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <string>


class VulkanContext {
public:
    VulkanContext();
    ~VulkanContext();

	// Set as singleton
	VulkanContext(const VulkanContext&) = delete;
	VulkanContext& operator=(const VulkanContext&) = delete;

    VkInstance getInstance() const { return instance; }

    // Getters
    const std::vector<VkPhysicalDevice>& getPhysicalDevices() const { return physicalDevices; }
    const std::vector<VkDevice>& getDevices() const { return devices; }
    const std::vector<VkQueue>& getQueues() const { return queues; }
    const std::vector<VkCommandPool>& getCommandPools() const { return commandPools; }

	// Memory management
    void createBufferAndMemory(VkDevice device, VkDeviceSize size, VkBuffer& buffer, VkDeviceMemory& memory);

private:
    void createInstance();
    void pickPhysicalDevices();
    void createDevicesAndQueues();
    void createCommandPools();

    VkInstance instance = VK_NULL_HANDLE;

    std::vector<VkPhysicalDevice> physicalDevices;
    std::vector<uint32_t> queueFamilyIndices;
    std::vector<VkDevice> devices;
    std::vector<VkQueue> queues;
    std::vector<VkCommandPool> commandPools;
};