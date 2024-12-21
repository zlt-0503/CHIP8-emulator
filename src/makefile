CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -g

# Include SDL2 if needed
# SDL_FLAGS = `sdl2-config --cflags --libs`

TARGET = chip8_emulator

SRCS = main.cpp CHIP8.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET) # $(SDL_FLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

run: $(TARGET)
	./$(TARGET) roms/Pong.ch8 # Replace with the path to your ROM file

.PHONY: all clean run
