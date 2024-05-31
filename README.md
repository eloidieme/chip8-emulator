# CHIP-8 Emulator

## Overview

This project is a CHIP-8 emulator written in C using the SDL library for graphical output and audio. The emulator is designed to execute CHIP-8 programs and provide a platform to run classic CHIP-8 games and applications.

## Author

Eloi Dieme

## Features

- Emulation of the CHIP-8 instruction set
- Graphical rendering using SDL
- Audio output for sound timers
- Logging of emulator state for debugging purposes

## Getting Started

### Prerequisites

To build and run the CHIP-8 emulator, you need to have the following installed:

- GCC (or another compatible C compiler)
- SDL3 development libraries

### Building

1. Clone the repository:

    ```sh
    git clone https://github.com/eloidieme/chip8-emulator.git 
    cd chip8-emulator
    ```

2. Compile the project:

    ```sh
    make
    ```

3. Run the emulator:

    ```sh
    ./bin/prog <path_to_rom_file>
    ```

### Running a ROM

To run a CHIP-8 ROM, use the following command:

```sh
./chip8_emulator ./roms/your_rom.ch8
```