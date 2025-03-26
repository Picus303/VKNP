// Verifies that the memory manager correctly keep tracks of the reference count of the buffers.

#include "ManagerTestsCommon.hpp"


int main() {
    try {
        initContextAndManager();
        auto& memMgr = MemoryManager::getManager();

        MemoryHandle handle1 = memMgr.getBuffer(ALLOCATION_SIZE, 0);
        memMgr.acquireBuffer(handle1);		// refCount == 2
        memMgr.releaseBuffer(handle1);		// refCount == 1

        MemoryHandle handle2 = memMgr.getBuffer(ALLOCATION_SIZE, 0);
        assert(handle1.id != handle2.id);	// Not reused yet

        memMgr.releaseBuffer(handle1);		// refCount == 0 -> move to cache
        MemoryHandle handle3 = memMgr.getBuffer(ALLOCATION_SIZE, 0);
        assert(handle1.id == handle3.id);	// Reused

    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}