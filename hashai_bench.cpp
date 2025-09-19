#include "hash_ai.hpp"

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <text_file>\n";
        return 1;
    }

    std::ifstream fin(argv[1]);
    if (!fin) {
        std::cerr << "Cannot open file: " << argv[1] << "\n";
        return 1;
    }

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(fin, line)) {
        lines.push_back(line + "\n");
    }

    std::cout << "Total lines: " << lines.size() << "\n";
    std::cout << "Lines\tBytes\tTime(ms)\n";

    size_t step = 1;
    while (step <= lines.size()) {
        std::ostringstream buffer;
        for (size_t i = 0; i < step; ++i) buffer << lines[i];
        std::string text = buffer.str();

        auto start = std::chrono::high_resolution_clock::now();
        std::string h = custom_hash256(text);
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> ms = end - start;

        std::cout << step << "\t" << text.size() << "\t" << ms.count() << "\n";

        step *= 2;
    }
}
