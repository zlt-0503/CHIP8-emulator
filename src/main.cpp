#include "CHIP8.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <ROM file>" << std::endl;
        return 1;
    }

    CHIP8 chip8;
    chip8.load_ROM(argv[1]);
    chip8.run();

    return 0;
}
