#include <filesystem>
#include <iostream>
#include <fstream>
#include <string>
#include "inc/md5.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cout << "USAGE: md5 <path> - where path is the file you want to hash\n";
        return EXIT_FAILURE;
    }

    auto file = std::filesystem::path(argv[1]);
    std::ifstream data{ file.string(), std::ios::binary };

    auto result = hash(data);
    std::cout << result << "\n";
}
