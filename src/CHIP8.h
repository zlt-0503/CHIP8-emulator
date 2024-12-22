#ifndef CHIP8_H
#define CHIP8_H

#include <array>
#include <cstdint>
#include <random>
#include <string>
#include <SDL2/SDL.h>

#define MEMORY_SIZE 4096
#define REGISTER_COUNT 16
#define STACK_SIZE 16
#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32
#define KEYPAD_SIZE 16
#define FONT_ADDRESS 50
#define FONT_SIZE 5
#define FRAME_RATE 600 // Set to 500 Hz for smoother gameplay
#define WINDOW_SCALE 10

class CHIP8 {
  public:
    void run();
    void load_ROM(const std::string& file_name);
    CHIP8(bool legacyShift = false);
    ~CHIP8();

  private:
    bool useLegacyShift;

    std::array<uint8_t, MEMORY_SIZE> memory;
    std::array<uint8_t, REGISTER_COUNT> V;
    uint16_t I;
    uint16_t PC;
    std::array<uint16_t, STACK_SIZE> stack;
    uint8_t SP;
    std::array<bool, KEYPAD_SIZE> keypad;
    std::array<std::array<bool, DISPLAY_WIDTH>, DISPLAY_HEIGHT> display;
    uint8_t delay_timer;
    uint8_t sound_timer;

    SDL_Window* window;
    SDL_Renderer* renderer;

    struct Instruction {
        uint16_t opcode;
        uint8_t type;
        uint8_t X;
        uint8_t Y;
        uint8_t N;
        uint8_t NN;
        uint16_t NNN;
    };

    // private methods

    uint16_t fetch_opcode();
    Instruction decode_opcode(uint16_t opcode);
    void execute_opcode(const Instruction& instruction);
    void logical_and_arithmetic(const Instruction& instruction);
    void clear_display();
    void update_timers();

    void initialize();
    void emulate_cycle();
    void set_key(uint8_t key, bool is_pressed);
    const std::array<std::array<bool, DISPLAY_WIDTH>, DISPLAY_HEIGHT>& get_display() const;
    bool is_sound_playing() const;

    void render_display() const;
    void handle_input();

    void stack_push(uint16_t address);
    uint16_t stack_pop();

    void initialize_SDL();
    void cleanup_SDL();

    std::random_device rd;
    std::mt19937 gen;
    std::uniform_int_distribution<> distrib;
};

#endif /* ifndef CHIP8_H */

