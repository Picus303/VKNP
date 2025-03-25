#include "VulkanContext.hpp"



VulkanContext::VulkanContext() {
    createInstance();
    pickPhysicalDevices();
    createDevicesAndQueues();
    createCommandPools();
}


VulkanContext::~VulkanContext() {
    // Destroy command pools for each device
    for (size_t i = 0; i < commandPools.size(); i++) {
        vkDestroyCommandPool(devices[i], commandPools[i], nullptr);
    }
    // Destroy logical devices
    for (auto dev : devices) {
        vkDestroyDevice(dev, nullptr);
    }
    // Destroy instance
    if (instance != VK_NULL_HANDLE) {
        vkDestroyInstance(instance, nullptr);
    }
}


void VulkanContext::createInstance() {
	// Define the application info
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "VKNP";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "VKNP_ENGINE";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_1;

	// Define instanciation info
	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	// Load instance extensions
	std::vector<const char*> enabledExtensions(instanceExtensions.begin(), instanceExtensions.end());
	createInfo.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size());
	createInfo.ppEnabledExtensionNames = enabledExtensions.data();

	// Create the instance
	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
		throw std::runtime_error("Unable to create Vulkan instance !");
	}
}


void VulkanContext::pickPhysicalDevices() {
	// Get the number of physical devices
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if (deviceCount == 0) {
        throw std::runtime_error("No GPU with Vulkan support found !");
    }
    std::vector<VkPhysicalDevice> allDevices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, allDevices.data());

    // For each device, check if it has a compute queue
    for (const auto& pd : allDevices) {
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(pd, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(pd, &queueFamilyCount, queueFamilies.data());

        for (uint32_t i = 0; i < queueFamilyCount; i++) {
            if (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
                physicalDevices.push_back(pd);
                queueFamilyIndices.push_back(i);
                break;	// Pick the first compute queue found
            }
        }
    }
    if (physicalDevices.empty()) {
        throw std::runtime_error("No GPU with compute queue found !");
    }
}


void VulkanContext::createDevicesAndQueues() {
    devices.resize(physicalDevices.size());
    queues.resize(physicalDevices.size());

	// Create a logical device for each physical device
    for (size_t i = 0; i < physicalDevices.size(); i++) {
        float queuePriority = 1.0f;

		// Define the queue creation info
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamilyIndices[i];
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount = 1;
        createInfo.pQueueCreateInfos = &queueCreateInfo;

		// Specify the device extensions to enable
		VkPhysicalDeviceFeatures deviceFeatures{};
		std::vector<const char*> enabledExtensions(deviceExtensions.begin(), deviceExtensions.end());
		createInfo.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size());
		createInfo.ppEnabledExtensionNames = enabledExtensions.data();
		createInfo.pEnabledFeatures = &deviceFeatures;

		// Create the logical device
		if (vkCreateDevice(physicalDevices[i], &createInfo, nullptr, &devices[i]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create logical device for physical device " + std::to_string(i));
		}
        vkGetDeviceQueue(devices[i], queueFamilyIndices[i], 0, &queues[i]);
    }
}


void VulkanContext::createCommandPools() {
    commandPools.resize(devices.size());

	// Create a command pool for each device
    for (size_t i = 0; i < devices.size(); i++) {

		// Define the command pool info
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = queueFamilyIndices[i];
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		// Create the command pool
        if (vkCreateCommandPool(devices[i], &poolInfo, nullptr, &commandPools[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create command pool for device " + std::to_string(i));
        }
    }
}


void VulkanContext::createBufferAndMemory(VkDevice device, VkDeviceSize size, VkBuffer& buffer, VkDeviceMemory& memory) const {
    // Define the buffer info
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |	// For compute shader storage buffer
					   VK_BUFFER_USAGE_TRANSFER_SRC_BIT |	// For copying data to the buffer
					   VK_BUFFER_USAGE_TRANSFER_DST_BIT;	// For copying data from the buffer
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	// Create the buffer
    if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create buffer !");
    }

    // Get memory requirements
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

    // Get the memory properties of the physical device
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevices[0], &memProperties);

	bool found = false;
    uint32_t memoryTypeIndex = 0;
	// Find a valid memory type for the buffer on the physical device
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if (memRequirements.memoryTypeBits & (1 << i)) {											// The memory type is supported by the buffer
            if (memProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {	// We use DEVICE_LOCAL memory -> VRAM
                memoryTypeIndex = i;
                found = true;
                break;
            }
        }
    }
    if (!found) {
        throw std::runtime_error("No valid memory type found for the buffer !");
    }

	// Define the memory allocation info
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = memoryTypeIndex;

	// Allocate memory for the buffer
    if (vkAllocateMemory(device, &allocInfo, nullptr, &memory) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate memory for the buffer !");
    }
	// Bind the buffer to the memory
    vkBindBufferMemory(device, buffer, memory, 0);
}


std::pair<VkDeviceSize, VkDeviceSize> VulkanContext::getMemoryUsage(VkPhysicalDevice device) const {
    VkPhysicalDeviceMemoryBudgetPropertiesEXT budgetProps = {};
	budgetProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_BUDGET_PROPERTIES_EXT;

	VkPhysicalDeviceMemoryProperties2 props2 = {};
	props2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;
	props2.pNext = &budgetProps;
	
    vkGetPhysicalDeviceMemoryProperties2(device, &props2);

    for (uint32_t i = 0; i < props2.memoryProperties.memoryHeapCount; ++i) {
        if (props2.memoryProperties.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
            return { budgetProps.heapUsage[i], budgetProps.heapBudget[i] };
        }
    }
    return {0, 0};
}