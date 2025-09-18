
#include "hash_ai.hpp"

int main(int argc, char** argv) {
    if (argc == 1) {
        std::string demo = "hello";
        std::cout << "hash(\"" << demo << "\") = " << custom_hash256(demo) << "\n";
        std::cout << "Provide an argument to hash (or pipe input).\n";
        return 0;
    }

    std::string s = argv[1];

    auto start = std::chrono::high_resolution_clock::now();
    std::string h = custom_hash256(s);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> ms = end - start;

    std::cout << h << "\n";
    std::cout << "Hash time: " << ms.count() << " ms\n";
    return 0;
}