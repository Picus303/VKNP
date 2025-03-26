// Common code for MemoryManager tests.

#pragma once

#include "MemoryManager.hpp"
#include "VulkanContext.hpp"

#include <iostream>
#include <cassert>

#define ALLOCATION_SIZE 1024	// Test buffer size


inline void initContextAndManager() {
    VulkanContext& context = VulkanContext::getContext();
    MemoryManager::getManager().init(&context);
}