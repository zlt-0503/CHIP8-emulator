#ifndef CHIP8_H
#define CHIP8_H

#include <array>
#include <cstdint>
#include <random>
#include <string>

#define MEMORY_SIZE 4096
#define REGISTER_COUNT 16
#define STACK_SIZE 16
#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32
#define KEYPAD_SIZE 16
#define FONT_ADDRESS 50
#define FONT_SIZE 5
#define FRAME_RATE 60

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

    struct Instruction {
        uint16_t opcode;
        uint8_t type;
        uint8_t X;
        uint8_t Y;
        uint8_t N;
        uint8_t NN;
        uint8_t NNN;
    };

    // private methods

    /**
     * @brief Fetch the current instruction.
     *
     * Read the two-bytes instruction that PC is currently pointing at from memory.
     * And then immedietely increment the PC by 2 to get ready for the nect instruction.
     * @return uint16_t The 16-bit opcode of the current instruction.
     */
    uint16_t fetch_opcode();
    Instruction decode_opcode(uint16_t opcode);
    void execute_opcode(const Instruction& instruction);
    void logical_and_arithmetic(const Instruction& instruction);
    void clear_display();
    void draw_sprite(uint8_t x, uint8_t y, uint8_t height);
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

    std::random_device rd;
    std::mt19937 gen;
    std::uniform_int_distribution<> distrib;

};


#endif /* ifndef CHIP8_H */

