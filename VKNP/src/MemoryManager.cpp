#include "MemoryManager.hpp"
#include "VulkanContext.hpp"

#include <stdexcept>
#include <iostream>
#include <algorithm>


// #################################################################################################
// ###   MemoryManager: Singleton implementation
// #################################################################################################


// Singleton access
MemoryManager& MemoryManager::getManager() {
	static MemoryManager s_instance;
	return s_instance;
}


void MemoryManager::init(VulkanContext* context) {
	std::lock_guard<std::mutex> lock(managerMutex);
	if (context == nullptr) {
		throw std::runtime_error("Memory Manager initialized with an invalid Vulkan context");
	}
	vkContext = context;

	// ToDo: Define the configuration
}


void MemoryManager::destroy() {
	std::lock_guard<std::mutex> lock(managerMutex);

	// Destroy active buffers
	for (auto& kv : activeAllocations) {
		destroyAllocation(kv.first, false);
	}
	activeAllocations.clear();

	// Destroy cached buffers
	for (auto& kv : cachedAllocations) {
		destroyAllocation(kv.first, true);
	}
	cachedAllocations.clear();

	lruCache.clear();
}


// #################################################################################################
// ###   MemoryManager: Buffer access
// #################################################################################################


// Used by Tensors to get a buffer
MemoryHandle MemoryManager::getBuffer(VkDeviceSize size, uint32_t requestedDeviceIndex) {
	std::lock_guard<std::mutex> lock(managerMutex);

	// Check initialization and device index
	if (vkContext == nullptr) {
		throw std::runtime_error("Memory Manager not initialized");
	}
	if (requestedDeviceIndex >= vkContext->getDeviceCount()) {
		throw std::runtime_error("Unable to create a buffer for an invalid device index");
	}

	// Look in cache for a buffer of the right size and device
	for (auto it = lruCache.begin(); it != lruCache.end(); ++it) {
        auto findit = cachedAllocations.find(*it);
		if (findit == cachedAllocations.end()) {
			throw std::runtime_error("Unable to find a cached buffer in the cache");
		}

		AllocationInfo& alloc = findit->second;
		if (alloc.size == size && alloc.deviceIndex == requestedDeviceIndex) {
			// Reuse the buffer
			alloc.refCount++;
			lruCache.erase(it);

			// Move the buffer to activeAllocations
			activeAllocations[alloc.id] = alloc;
			cachedAllocations.erase(findit);

			MemoryHandle handle;
			handle.id = alloc.id;
			return handle;
		}
	}

	// Create a new buffer
	return createAllocation(size, requestedDeviceIndex);
	// ToDo: Check if there is enough memory available and empty the cache if needed
}


// Used when a view of an existing buffer is created
void MemoryManager::acquireBuffer(const MemoryHandle& handle) {
	std::lock_guard<std::mutex> lock(managerMutex);

	// Check if the handle is valid
	auto it = activeAllocations.find(handle.id);
	if (it == activeAllocations.end()) {
		throw std::runtime_error("Unable to acquire an invalid buffer handle");
	}

	it->second.refCount++;
}


// Used when a view of an existing buffer is destroyed
void MemoryManager::releaseBuffer(const MemoryHandle& handle) {
	std::lock_guard<std::mutex> lock(managerMutex);

	// Check if the handle is valid
	auto kv = activeAllocations.find(handle.id);
	if (kv == activeAllocations.end()) {
		throw std::runtime_error("Unable to release an invalid buffer handle");
	}

	if (kv->second.refCount == 0) {
		throw std::runtime_error("Unable to release a buffer with a reference count of 0");
	}

	kv->second.refCount--;

	// If the buffer is no longer used, move it to the cache
	if (kv->second.refCount == 0) {
		cachedAllocations[kv->first] = kv->second;
		lruCache.push_back(kv->first);
		activeAllocations.erase(kv);
	}
}


// #################################################################################################
// ###   MemoryManager: Buffer management
// #################################################################################################


// Remove the requested number of bytes from the cache
// If bytesToFree is 0, the entire cache is emptied
void MemoryManager::emptyCache(VkDeviceSize bytesToFree) {
	std::lock_guard<std::mutex> lock(managerMutex);

	if (bytesToFree == 0) {
		// Collect the ids of the cached buffers
		std::vector<uint64_t> keys;
		keys.reserve(cachedAllocations.size());
		for (auto& kv : cachedAllocations) {
			keys.push_back(kv.first);
		}

		// Destroy all cached buffers
		for (uint64_t key : keys) {
			destroyAllocation(key, true);
		}
		cachedAllocations.clear();
		lruCache.clear();

	} else {
		// VkDeviceSize is unsigned -> loop when bytesToFree < 0 -> infinite loop
		int64_t ToFree = static_cast<int64_t>(bytesToFree);

		// Destroy buffers until the requested number of bytes is freed
		while (ToFree > 0 && !lruCache.empty()) {
			auto it = cachedAllocations.find(lruCache.front());
			if (it == cachedAllocations.end()) {
				throw std::runtime_error("Unable to find a cached buffer in the cache");
			}

			ToFree -= it->second.size;
			destroyAllocation(it->first, true);
			cachedAllocations.erase(it);
			lruCache.pop_front();
		}
	}
}


MemoryHandle MemoryManager::createAllocation(VkDeviceSize size, uint32_t deviceIndex) {
	// Check initialization and device index
	if (vkContext == nullptr) {
		throw std::runtime_error("Memory Manager not initialized");
	}
	if (deviceIndex >= vkContext->getDeviceCount()) {
		throw std::runtime_error("Unable to create a buffer for an invalid device index");
	}

	VkDevice device = vkContext->getDevices()[deviceIndex];

	// Create the buffer
	VkBuffer buffer;
	VkDeviceMemory memory;
	vkContext -> createBufferAndMemory(device, size, buffer, memory);

	// Create the allocation info
	AllocationInfo info;
	info.id = nextBufferId++;
	info.buffer = buffer;
	info.memory = memory;
	info.device = device;
	info.size = size;
	info.deviceIndex = deviceIndex;
	info.refCount = 1;

	activeAllocations[info.id] = info;

	MemoryHandle handle;
	handle.id = info.id;
	return handle;
}


void MemoryManager::destroyAllocation(uint64_t allocId, bool inCache) {
	if (inCache) {
		// Search for the allocation in cachedAllocations
		auto itCached = cachedAllocations.find(allocId);
		if (itCached != cachedAllocations.end()) {
			AllocationInfo& info = itCached->second;

			vkDestroyBuffer(info.device, info.buffer, nullptr);
			vkFreeMemory(info.device, info.memory, nullptr);

			return;
		}
	} else {
		// Search for the allocation in activeAllocations
		auto itActive = activeAllocations.find(allocId);
		if (itActive != activeAllocations.end()) {
			AllocationInfo& info = itActive->second;

			vkDestroyBuffer(info.device, info.buffer, nullptr);
			vkFreeMemory(info.device, info.memory, nullptr);

			return;
		}
	}

	// Allocation not found, for now don't throw an error
}