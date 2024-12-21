```

███████╗ ██████╗ ██████╗  █████╗ ███████╗██╗   ██╗██╗  ██╗ █████╗ 
██╔════╝██╔═══██╗██╔══██╗██╔══██╗██╔════╝██║   ██║██║ ██╔╝██╔══██╗
███████╗██║   ██║██████╔╝███████║███████╗██║   ██║█████╔╝ ███████║
╚════██║██║   ██║██╔══██╗██╔══██║╚════██║██║   ██║██╔═██╗ ██╔══██║
███████║╚██████╔╝██║  ██║██║  ██║███████║╚██████╔╝██║  ██╗██║  ██║
╚══════╝ ╚═════╝ ╚═╝  ╚═╝╚═╝  ╚═╝╚══════╝ ╚═════╝ ╚═╝  ╚═╝╚═╝  ╚═╝

```

# SorAsuka's CHIP-8 Emulator

A simple CHIP-8 emulator written in C++ using SDL2 for rendering.

## Prerequisites

Before you begin, ensure you have met the following requirements:

- You have installed [CMake](https://cmake.org/download/) (version 3.10 or higher).
- You have installed [SDL2](https://github.com/libsdl-org/SDL/releases/tag/release-2.30.10).

### Installing SDL2

#### macOS

You can use Homebrew to install SDL2:

```sh
brew install sdl2
```

#### Ubuntu/Linux

You can use the package manager to install SDL2. For example, on Ubuntu:

```sh
sudo apt-get update
sudo apt-get install libsdl2-dev
```

#### Windows

Say goodbye to Windows and use Linux/WSL, or buy a MacBook at [Apple's online store](https://www.apple.com.cn/mac/).

## Building the Project

1. Clone the repository and initialize submodules:

```sh
git clone --recurse-submodules https://github.com/zlt-0503/CHIP8-emulator.git
cd CHIP8-emulator
```

If you have already cloned the repository without `--recurse-submodules`, you can initialize the submodules with:

```sh
git submodule update --init --recursive
```

2. Create a build directory and navigate into it:

```sh
mkdir build
cd build
```

3. Run CMake to generate the build files:

```sh
cmake ..
```

4. Build the project:

```sh
make
```

## Running the Emulator

To run the emulator, use the following command:

```sh
./CHIP8-emulator ../chip8Archive/roms/<ROM file>
```

Replace `<ROM file>` with the name of the ROM file you want to run from the `chip8Archive/roms` directory.

## Usage

```sh
./CHIP8-emulator <ROM file>
```

## Contributing

If you want to contribute to this project, please fork the repository and create a pull request.

## License

This project is licensed under the WTFPL License.
