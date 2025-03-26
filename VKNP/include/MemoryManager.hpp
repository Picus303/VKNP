#pragma once

#include "VulkanContext.hpp"

#include <vulkan/vulkan.h>
#include <unordered_map>
#include <cstdint>
#include <vector>
#include <memory>
#include <deque>
#include <mutex>



// Basic structure to store a buffer ID
struct MemoryHandle {
	uint64_t id = 0;
};


// Structure to store the information to transfer to the compute pipeline
struct BufferInfo {};	// ToDo


class MemoryManager {
public:
	// Singleton access
	static MemoryManager& getManager();

	// Explicit constructors and destructors for the singleton
	// Avoids handling potentially invalid pointers when the application shuts down (VkDevice, VkInstance, etc.)
	void init(VulkanContext* context);
	void destroy();

	MemoryHandle getBuffer(VkDeviceSize size, uint32_t requestedDeviceIndex);

	// Increment / decrement the reference counter
	void acquireBuffer(const MemoryHandle& handle);
	void releaseBuffer(const MemoryHandle& handle);

	// Cache management
	void emptyCache(VkDeviceSize bytesToFree = 0);

	// Getters (required for descriptor creation)
	BufferInfo getBufferInfo(const MemoryHandle& handle) const;

private:
	// Singleton: private constructor and destructor
	MemoryManager() = default;
	~MemoryManager() = default;

	// Singleton: no copy or assignment
	MemoryManager(const MemoryManager&) = delete;
	MemoryManager& operator=(const MemoryManager&) = delete;

	// Internal methods to actually create and destroy buffers
	MemoryHandle createAllocation(VkDeviceSize size, uint32_t deviceIndex);
	void destroyAllocation(uint64_t allocId, bool inCache);

private:
	// Allocation information
	struct AllocationInfo {
		uint64_t id;
		VkBuffer buffer;
		VkDeviceMemory memory;
		VkDevice device;
		VkDeviceSize size;
		uint32_t deviceIndex;

		int refCount = 0;
	};

	VulkanContext* vkContext = nullptr;

	// Map ID -> AllocationInfo
	std::unordered_map<uint64_t, AllocationInfo> activeAllocations;
	std::unordered_map<uint64_t, AllocationInfo> cachedAllocations;

	// Store allocations in the order of their last usage
	std::deque<uint64_t> lruCache;

	uint64_t nextBufferId = 1;

	// Memory usage
	std::vector<VkDeviceSize> deviceMemoryBudgets;
	std::vector<VkDeviceSize> deviceActiveMemoryUsage;
	std::vector<VkDeviceSize> deviceCachedMemoryUsage;

	// Mutex to protect the memory manager
	std::recursive_mutex managerMutex;
};
