#include "CHIP8.h"
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <ios>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

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
    uint16_t opcode = (memory[PC] << 8) | (memory[PC + 1]);
    PC += 2;
    std::cout << "Fetched opcode: 0x" << std::hex << opcode << std::dec << " at PC: 0x" << std::hex << (PC - 2) << std::dec << std::endl;
    return opcode;
}

CHIP8::Instruction CHIP8::decode_opcode(uint16_t opcode) {
    Instruction instruction;
    instruction.opcode = opcode;
    instruction.type = (opcode & 0xF000) >> 12;
    instruction.X = (opcode & 0x0F00) >> 8;
    instruction.Y = (opcode & 0x00F0) >> 4;
    instruction.N = opcode & 0x000F;
    instruction.NN = opcode & 0x00FF;
    instruction.NNN = opcode & 0x0FFF;

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
            throw std::runtime_error("Unknown opcode: 0x8"
                + std::to_string(instruction.opcode));
    }
}

void CHIP8::execute_opcode(const CHIP8::Instruction& instruction) {
    std::cout << "Executing opcode: 0x" << std::hex << instruction.opcode << std::dec << std::endl;
    uint8_t &VX = V[instruction.X];
    uint8_t &VY = V[instruction.Y];
    uint8_t &VF = V[0xF];

    switch (instruction.type) {
        case 0x0:
            if (instruction.opcode == 0x00E0) { // 00E0: clear screen
                clear_display();
            } else if (instruction.opcode == 0x00EE) { // 00EE: return from subroutine
                PC = stack_pop();
            } else {
                throw std::runtime_error("Unknown opcode: 0x0"
                    + std::to_string(instruction.opcode));
            }
            break;

        case 0x1: // 1NNN: jump to address NNN
            PC = instruction.NNN;
            break;

        case 0x2: // 2NNN: call subroutine at NNN
            stack_push(PC);
            PC = instruction.NNN;
            break;

        case 0x3: // 3XNN: skip next instruction if VX == NN
            if (VX == instruction.NN) {
                PC += 2;
            }
            break;

        case 0x4: // 4XNN: skip next instruction if VX != NN
            if (VX != instruction.NN) {
                PC += 2;
            }
            break;

        case 0x5: // 5XY0: skip next instruction if VX == VY
            if (VX == VY) {
                PC += 2;
            }
            break;

        case 0x6: // 6XNN: set VX to NN
            VX = instruction.NN;
            break;

        case 0x7: // 7XNN: add NN to VX
            VX += instruction.NN;
            break;

        case 0x8: // 8XYN: arithmetic and logical instructions
            logical_and_arithmetic(instruction);
            break;

        case 0x9: // 9XY0: skip next instruction if VX != VY
            if (VX != VY) {
                PC += 2;
            }
            break;

        case 0xA: // ANNN: set I to NNN
            I = instruction.NNN;
            break;

        case 0xB: // BNNN: jump to address NNN + V0
            PC = instruction.NNN + V[0];
            break;

        case 0xC: { // CXNN: set VX to random number & NN
            uint8_t random_number = distrib(gen);
            VX = random_number & instruction.NN;
            break;
        }

        case 0xD: { // DXYN: draw sprite at (VX, VY) with width 8 and height N
            uint8_t x = VX % DISPLAY_WIDTH;
            uint8_t y = VY % DISPLAY_HEIGHT;

            VF = 0;

            for (int row = 0; row < instruction.N; ++row) {
                uint8_t sprite = memory[I + row];

                if (y + row >= DISPLAY_HEIGHT) {
                    break;
                }

                for (int col = 0; col < 8; ++col) {
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
                case 0x9E: // EX9E: skip next instruction if key in VX is pressed
                    if (keypad[VX]) {
                        PC += 2;
                    }
                    break;

                case 0xA1: // EXA1: skip next instruction if key in VX is not pressed
                    if (!keypad[VX]) {
                        PC += 2;
                    }
                    break;

                default:
                    throw std::runtime_error("Unknown opcode: 0xE"
                        + std::to_string(instruction.opcode));
            }
            break;

        case 0xF:
            switch (instruction.NN) {
                case 0x07: // FX07: set VX to delay timer value
                    VX = delay_timer;
                    break;

                case 0x15: // FX15: set delay timer to VX
                    delay_timer = VX;
                    break;

                case 0x18: // FX18: set sound timer to VX
                    sound_timer = VX;
                    break;

                case 0x1E: // FX1E: add VX to I
                    I += VX;
                    VF = (I > 0xFFF) ? 1 : 0;
                    break;

                case 0x0A: { // FX0A: wait for a key press, store the value of the key in VX
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

                case 0x29: // FX29: set I to location of sprite for digit VX
                    I = FONT_ADDRESS + (VX * FONT_SIZE);
                    break;

                case 0x33: { // FX33: store BCD representation of VX in memory locations I, I+1, and I+2
                    uint8_t hundreds = VX / 100;
                    uint8_t tens = (VX / 10) % 10;
                    uint8_t ones = VX % 10;

                    memory[I] = hundreds;
                    memory[I + 1] = tens;
                    memory[I + 2] = ones;

                    break;
                }

                case 0x55: // FX55: store registers V0 through VX in memory starting at location I
                    for (uint8_t i = 0; i <= instruction.X; ++i) {
                        memory[I + i] = V[i];
                    }
                    break;

                case 0x65: // FX65: read registers V0 through VX from memory starting at location I
                    for (uint8_t i = 0; i <= instruction.X; ++i) {
                        V[i] = memory[I + i];
                    }
                    break;

                default:
                    throw std::runtime_error("Unknown opcode: 0xF"
                        + std::to_string(instruction.opcode));
            }
            break;

        default:
            throw std::runtime_error("Unknown opcode: "
                + std::to_string(instruction.opcode));
    }
}

void CHIP8::clear_display() {
    for (auto& row : display) {
        row.fill(false);
    }
}

void CHIP8::update_timers() {
    static auto last_time = std::chrono::high_resolution_clock::now();
    auto curr_time = std::chrono::high_resolution_clock::now();
    auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(curr_time - last_time);

    if (elapsed_time.count() >= 1000 / 60) { // Update timers at 60 Hz
        if (delay_timer > 0) --delay_timer;
        if (sound_timer > 0) --sound_timer;
        last_time = curr_time;
    }
}

bool CHIP8::is_sound_playing() const {
    return sound_timer > 0;
}

const std::array<std::array<bool, DISPLAY_WIDTH>, DISPLAY_HEIGHT>& CHIP8::get_display() const {
    return display;
}

void CHIP8::set_key(uint8_t key, bool is_pressed) {
    if (key < KEYPAD_SIZE) keypad[key] = is_pressed;
}

void CHIP8::load_ROM(const std::string& file_name) {
    std::ifstream file(file_name, std::ios::binary | std::ios::ate);
    if (!file) {
        throw std::runtime_error("Failed to load ROM file: " + file_name);
    }

    std::streamsize file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    if (file_size > (MEMORY_SIZE - 0x200)) {
        throw std::runtime_error("File Size exceeds memory size");
    }

    std::vector<char> buffer(file_size);
    if (!file.read(buffer.data(), file_size)) {
        throw std::runtime_error("Failed to read ROM file: " + file_name);
    }

    std::copy(buffer.begin(), buffer.end(), memory.begin() + 0x200);
}

void CHIP8::emulate_cycle() {
    uint16_t opcode = fetch_opcode();
    Instruction instruction = decode_opcode(opcode);
    execute_opcode(instruction);
}

void CHIP8::initialize() {
    memory.fill(0);
    V.fill(0);
    I = 0;
    PC = 0x200; // Program counter starts at 0x200
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

    initialize_SDL();
}

CHIP8::CHIP8(bool legacyShift) : useLegacyShift(legacyShift), gen(rd()), distrib(0, 255) {
    initialize();
}

CHIP8::~CHIP8() {
    cleanup_SDL();
}

void CHIP8::initialize_SDL() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        throw std::runtime_error("Failed to initialize SDL: " + std::string(SDL_GetError()));
    }

    window = SDL_CreateWindow("CHIP-8 Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, DISPLAY_WIDTH * WINDOW_SCALE, DISPLAY_HEIGHT * WINDOW_SCALE, SDL_WINDOW_SHOWN);
    if (!window) {
        throw std::runtime_error("Failed to create window: " + std::string(SDL_GetError()));
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        throw std::runtime_error("Failed to create renderer: " + std::string(SDL_GetError()));
    }
}

void CHIP8::cleanup_SDL() {
    if (renderer) {
        SDL_DestroyRenderer(renderer);
    }
    if (window) {
        SDL_DestroyWindow(window);
    }
    SDL_Quit();
}

void CHIP8::render_display() const {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (size_t y = 0; y < DISPLAY_HEIGHT; ++y) {
        for (size_t x = 0; x < DISPLAY_WIDTH; ++x) {
            if (display[y][x]) {
                SDL_Rect rect = { static_cast<int>(x * WINDOW_SCALE), static_cast<int>(y * WINDOW_SCALE), WINDOW_SCALE, WINDOW_SCALE };
                SDL_RenderFillRect(renderer, &rect);
            }
        }
    }

    SDL_RenderPresent(renderer);
}

void CHIP8::handle_input() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            exit(0);
        } else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
            bool is_pressed = (event.type == SDL_KEYDOWN);
            switch (event.key.keysym.sym) {
                case SDLK_1: set_key(0x1, is_pressed); break;
                case SDLK_2: set_key(0x2, is_pressed); break;
                case SDLK_3: set_key(0x3, is_pressed); break;
                case SDLK_4: set_key(0xC, is_pressed); break;
                case SDLK_q: set_key(0x4, is_pressed); break;
                case SDLK_w: set_key(0x5, is_pressed); break;
                case SDLK_e: set_key(0x6, is_pressed); break;
                case SDLK_r: set_key(0xD, is_pressed); break;
                case SDLK_a: set_key(0x7, is_pressed); break;
                case SDLK_s: set_key(0x8, is_pressed); break;
                case SDLK_d: set_key(0x9, is_pressed); break;
                case SDLK_f: set_key(0xE, is_pressed); break;
                case SDLK_z: set_key(0xA, is_pressed); break;
                case SDLK_x: set_key(0x0, is_pressed); break;
                case SDLK_c: set_key(0xB, is_pressed); break;
                case SDLK_v: set_key(0xF, is_pressed); break;
            }
        }
    }
}

// public methods

void CHIP8::run() {
    const auto duration = std::chrono::milliseconds(1000 / FRAME_RATE);
    auto last_time = std::chrono::high_resolution_clock::now();

    while (1) {
        auto curr_time = std::chrono::high_resolution_clock::now();
        auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(curr_time - last_time);

        if (elapsed_time >= duration) {
            last_time = curr_time;
            update_timers();
            emulate_cycle();
            render_display();
            handle_input();
        }
    }
}


