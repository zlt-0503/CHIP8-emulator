#include "CHIP8.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <ROM file>" << std::endl;
        return 1;
    }

    CHIP8 chip8;

    try {
        chip8.load_ROM(argv[1]);
    } catch (const std::exception& e) {
        std::cerr << "Failed to load ROM: " << e.what() << std::endl;
        return 1;
    }

    try {
        chip8.run();
    } catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
