#include "CHIP8.h"
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>

void CHIP8::stack_push(uint16_t address) {
    if (SP >= STACK_SIZE) {
        throw std::runtime_error("Stack overflow");
    }
    stack[SP++] = address;
}

uint16_t CHIP8::stack_pop() {
    if (SP == 0) {
        throw std::runtime_error("Stack underflow");
    }
    return stack[--SP];
}

uint16_t CHIP8::fetch_opcode() {
    uint16_t opcode = (memory[PC] << 8) | (memory[PC+1]);
    PC += 2;
    return opcode;
}

CHIP8::Instruction CHIP8::decode_opcode(uint16_t opcode) {
    Instruction instruction;
    instruction.opcode = opcode;
    instruction.type = (opcode | 0xF000) >> 12;
    instruction.X = (opcode | 0x0F00) >> 8;
    instruction.Y = (opcode | 0x00F0) >> 4;
    instruction.N = opcode | 0x000F;
    instruction.NN = opcode | 0x00FF;
    instruction.NNN = opcode | 0x0FFF;

    return instruction;
}

void CHIP8::logical_and_arithmetic(const CHIP8::Instruction& instruction) {
    uint8_t &VX = V[instruction.X];
    uint8_t &VY = V[instruction.Y];
    uint8_t &VF = V[0xF];
    switch (instruction.N) {
        case 0x0: // set VX to VY
            VX = VY;
            break;

        case 0x1: // binary or
            VX |= VY;
            break;

        case 0x2: // binary and
            VX &= VY;
            break;

        case 0x3: // binary xor
            VX ^= VY;
            break;

        case 0x4: // add with overflow
            VF = (VX + VY > 0xFF) ? 1 : 0;
            VX += VY;
            break;

        case 0x5: // VX - VY
            VF = (VX >= VY) ? 1: 0;
            VX -= VY;
            break;

        case 0x6: // shift right by 1
            if (useLegacyShift) VX = VY;
            VF = VX & 0x1;
            VX >>= 1;
            break;

        case 0x7: // VY - VX
            VF = (VX < VY) ? 1 : 0;
            VX = VY - VX;
            break;

        case 0xE: // shift left by 1
            if (useLegacyShift) VX = VY;
            VF = VX & 0x100;
            VX <<= 1;
            break;

        default:
            throw std::runtime_error("Unknown opcode: "
                + std::to_string(instruction.opcode));
    }
}

void CHIP8::execute_opcode(const CHIP8::Instruction& instruction) { 
    uint8_t &VX = V[instruction.X];
    uint8_t &VY = V[instruction.Y];
    uint8_t &VF = V[0xF];

    switch (instruction.type) {
        case 0x0:
            if (instruction.NN == 0xE0) { // 00E0: clear screen
                clear_display();
            } else if (instruction.NN == 0xEE) { // 00EE: subroutines
                PC = stack_pop();
            } else {
                throw std::runtime_error("Unknown opcode: "
                    + std::to_string(instruction.opcode));
            }
            break;
        
        case 0x1: // jump
            PC = instruction.NNN;
            break;

        case 0x2: // subroutines
            stack_push(PC);
            PC = instruction.NNN;
            break;

        case 0x3: // jump if VX == NN
            if (V[instruction.X] == instruction.NN) {
                PC += 2;
            }
            break;

        case 0x4: // jump if VX != NN
            if (V[instruction.X] != instruction.NN) {
                PC += 2;
            }
            break;

        case 0x5: // jump if VX == VY
            if (V[instruction.X] == V[instruction.Y]) {
                PC += 2;
            }
            break;

        case 0x6: // set VX to NN
            V[instruction.X] = instruction.NN;
            break;

        case 0x7: // add NN to VX
            V[instruction.X] += instruction.NN;
            break;

        case 0x8: // arithmetic and logical instructions
            logical_and_arithmetic(instruction);
            break;

        case 0x9: // jump if VX != VY
            if (V[instruction.X] != V[instruction.Y]) {
                PC += 2;
            }
            break;

        case 0xA: // set index
            I = instruction.NNN;
            break;

        case 0xB: // jump with offset
            PC = instruction.NNN + V[0x0];
            break;

        case 0xC: { // random 
            uint8_t random_number = distrib(gen);
            V[instruction.X] = random_number & instruction.NN;
            break;
        }

        case 0xD: { // display
            uint8_t x = VX % DISPLAY_WIDTH;
            uint8_t y = VY % DISPLAY_HEIGHT;

            VF = 0;

            for (int row = 0; row < instruction.N; ++row) {
                uint8_t sprite = memory[I + row];

                if (y + row >= DISPLAY_HEIGHT) {
                    break;
                }

                for (int col = 0; col < 0x8; ++ col) {
                    if (x + col >= DISPLAY_WIDTH) {
                        break;
                    }

                    uint8_t sprite_pixel = (sprite >> (7 - col)) & 1;
                    bool& screen_pixel = display[y + row][x + col];

                    if (sprite_pixel == 1 && screen_pixel == 1) {
                        VF = 1;
                    }

                    screen_pixel ^= sprite_pixel;
                }
            }

            break;
        }

        case 0xE:
            switch (instruction.NN) {
                case 0x9E:
                    if (keypad[VX]) {
                        PC += 2;
                    }
                    break;

                case 0xA1:
                    if (!keypad[VX]) {
                        PC += 2;
                    }
                    break;

                default:
                    throw std::runtime_error("Unknown opcode: "
                        + std::to_string(instruction.opcode));
            }
            break;

        case 0xF:
            switch (instruction.NN) {
                // timers
                case 0x07:
                    VX = delay_timer;
                    break;

                case 0x15:
                    delay_timer = VX;
                    break;

                case 0x18:
                    sound_timer = VX;
                    break;

                case 0x1E:
                    I += VX;
                    VF = (I > 0xFFF) ? 1 : 0;
                    break;

                case 0x0A: {
                    bool key_pressed = false;
                    for (uint8_t i = 0; i < KEYPAD_SIZE; ++i) {
                        if (keypad[i]) {
                            VX = i;
                            key_pressed = true;
                            break;
                        }
                    }

                    if (!key_pressed) {
                        PC -= 2;
                    }

                    break;
                }

                case 0x29:
                    I = FONT_ADDRESS + (VX * FONT_SIZE);
                    break;


                case 0x33: {
                    uint8_t hundreds = VX / 100;
                    uint8_t tens = (VX / 10) % 10;
                    uint8_t ones = VX % 10;

                    memory[I] = hundreds;
                    memory[I + 1] = tens;
                    memory[I + 2] = ones;

                    break;
                }

                case 0x55:
                    for (uint8_t i = 0; i <= instruction.X; ++i) {
                        memory[I + i] = V[i];
                    }
                    break;

                case 0x65:
                    for (uint8_t i = 0; i <= instruction.X; ++i) {
                        V[i] = memory[I + i];
                    }
                    break;
                
                default:
                    throw std::runtime_error("Unknown opcode: "
                        + std::to_string(instruction.opcode));
            }

        default:
            throw std::runtime_error("Unknown opcode: "
                + std::to_string(instruction.opcode));
    }
}

void CHIP8::initialize() {
    memory.fill(0);
    V.fill(0);
    I = 0;
    PC = 0x200;
    stack.fill(0);
    SP = 0;
    keypad.fill(false);
    for (auto& arr : display) {
        arr.fill(false);
    }
    delay_timer = 0;
    sound_timer = 0;
    
    // initialize fonts
    uint8_t font[80] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };

    std::memcpy(&memory[FONT_ADDRESS], font, 80);
}

CHIP8::CHIP8(bool legacyShift) : useLegacyShift(legacyShift), gen(rd()), distrib(0, 255) {
    initialize();
}



