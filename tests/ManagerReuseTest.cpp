// Verifies that the memory manager reuses cached buffers when possible

#include "ManagerTestsCommon.hpp"


int main() {
    try {
        initContextAndManager();
        auto& memMgr = MemoryManager::getManager();

        MemoryHandle handle1 = memMgr.getBuffer(ALLOCATION_SIZE, 0);
        memMgr.releaseBuffer(handle1);
        MemoryHandle handle2 = memMgr.getBuffer(ALLOCATION_SIZE, 0);

        std::cout << "  handle1.id = " << handle1.id << ", handle2.id = " << handle2.id << std::endl;
        assert(handle1.id == handle2.id);

    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}