#include "ManagerTestsCommon.hpp"


int main() {
    try {
        initContextAndManager();
        auto& memMgr = MemoryManager::getManager();

		// 1) Empty the entire cache

        MemoryHandle h1 = memMgr.getBuffer(ALLOCATION_SIZE, 0);
        memMgr.releaseBuffer(h1);	// refCount == 0 -> move to cache
        memMgr.emptyCache(0);		// Delete all cached buffers

        MemoryHandle h2 = memMgr.getBuffer(ALLOCATION_SIZE, 0);
        assert(h1.id != h2.id);		// Buffer deleted from cache
        memMgr.releaseBuffer(h2);
		memMgr.emptyCache(0);

		// 2) Partially empty the cache

        MemoryHandle h3 = memMgr.getBuffer(ALLOCATION_SIZE, 0);
        MemoryHandle h4 = memMgr.getBuffer(ALLOCATION_SIZE, 0);
        memMgr.releaseBuffer(h3);
        memMgr.releaseBuffer(h4);

        memMgr.emptyCache(1);	// Delete one buffer from the cache
        MemoryHandle h5 = memMgr.getBuffer(ALLOCATION_SIZE, 0);
        assert(h4.id == h5.id);	// h3 deleted from cache, h4 reused
		memMgr.releaseBuffer(h5);

    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}