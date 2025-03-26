// Verifies that the memory manager can be initialized and destroyed without any error

#include "ManagerTestsCommon.hpp"


int main() {
    try {
        initContextAndManager();
        MemoryManager::getManager().destroy();
    } catch (const std::runtime_error& e) {
        std::cerr << "Erreur : " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}