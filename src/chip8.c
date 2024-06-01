#include "./chip8.h"

Chip8 initChip(uint16_t startingAddress) {
    Chip8 chip = {
        .ram = { 0 },
		.screen = { 0 },
        .pc = startingAddress,
        .index = 0x0,
        .stack = initStack(0x0),
        .delayT = 0x0,
        .soundT = 0x0,
        .regs = { 0 },
		.keys = { 0 }
    };
    return chip;
}

void setPixel(Chip8* chipPtr, SDL_Renderer* renderer, SDL_Texture* texture, uint8_t x, uint8_t y, uint8_t color) {
    void* pixels;
    int pitch;

    if (SDL_LockTexture(texture, NULL, &pixels, &pitch) != 0) {
        printf("SDL_LockTexture Error: %s\n", SDL_GetError());
        return;
    }

    uint8_t* pixelPtr = (uint8_t*)pixels;
    uint16_t pixelPosition = (y * pitch) + x;
    pixelPtr[pixelPosition] = color;
    SDL_UnlockTexture(texture);
}

uint16_t fetch(Chip8* chipPtr) {
	uint16_t msb = chipPtr->ram[chipPtr->pc];
	uint16_t lsb = chipPtr->ram[chipPtr->pc+1];
	uint16_t instruction = ((msb << WORD_SIZE) | lsb);
	chipPtr->pc += 0x2;
	return instruction;
}

void decodeExecute(uint16_t instruction, Chip8* chipPtr, SDL_Renderer* renderer, SDL_Texture* texture) {
	uint8_t nibbles[NIBBLE_SIZE] = {
		[0] = (instruction & 0xf000) >> 0xC,
		[1] = (instruction & 0x0f00) >> 0x8,
		[2] = (instruction & 0x00f0) >> 0x4,
		[3] = (instruction & 0x000f) >> 0x0,
	};

	switch (nibbles[0]) {
		case 0x0:
			switch (nibbles[3]) {
				case 0x0:
					// 0x00E0: Clear screen
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                    SDL_RenderClear(renderer);
					break;
				case 0xE:
					// 0x00EE: Returning from a subroutine
					chipPtr->pc = popStack(&(chipPtr->stack));
					break;
			}
			break;
		case 0x1:
			// 0x1NNN: Jump
			chipPtr->pc = (instruction & 0x0fff);
			break;
		case 0x2:
			// 0x2NNN: Call subroutine
			pushStack(&(chipPtr->stack), chipPtr->pc);
			chipPtr->pc = (instruction & 0x0fff);
			break;
		case 0x3:
			// 0x3XNN: Skip if value in VX == NN
			if (chipPtr->regs[nibbles[1]] == (instruction & 0x00ff)) {
				chipPtr->pc += 0x2;
			}
			break;
		case 0x4:
			// 0x4XNN: Skip if value in VX != NN
			if (chipPtr->regs[nibbles[1]] != (instruction & 0x00ff)) {
				chipPtr->pc += 0x2;
			}
			break;
		case 0x5:
			// 0x5XY0: Skip if values in VX and VY are equal
			if (chipPtr->regs[nibbles[1]] == chipPtr->regs[nibbles[2]]) {
				chipPtr->pc += 0x2;
			}
			break;
		case 0x6:
			// 0x6XNN: Set register VX
			chipPtr->regs[nibbles[1]] = (instruction & 0x00ff);
			break;
		case 0x7:
			// 0x7XNN: Add value to register VX
			chipPtr->regs[nibbles[1]] += (instruction & 0x00ff);
			break;
		case 0x8:
			switch (nibbles[3]) {
				case 0x0:
					// 0x8XY0: VX is set to the value of VY
					chipPtr->regs[nibbles[1]] = chipPtr->regs[nibbles[2]];
					break;
				case 0x1:
					// 0x8XY1: VX is set to VX | VY
					chipPtr->regs[nibbles[1]] = (chipPtr->regs[nibbles[1]] | chipPtr->regs[nibbles[2]]);
					break;
				case 0x2:
					// 0x8XY2: VX is set to VX & VY
					chipPtr->regs[nibbles[1]] = (chipPtr->regs[nibbles[1]] & chipPtr->regs[nibbles[2]]);
					break;
				case 0x3:
					// 0x8XY3: VX is set to VX ^ VY
					chipPtr->regs[nibbles[1]] = (chipPtr->regs[nibbles[1]] ^ chipPtr->regs[nibbles[2]]);
					break;
				case 0x4:
					// 0x8XY4: VX is set to VX + VY
					{
						uint16_t result = (chipPtr->regs[nibbles[1]] + chipPtr->regs[nibbles[2]]);
						if (result > 0xFF) {
							chipPtr->regs[0xF] = 0x1; 
						} else {
							chipPtr->regs[0xF] = 0x0;
						}
						chipPtr->regs[nibbles[1]] = result % 0x100;
					}
					break;
				case 0x5:
					// 0x8XY5: VX is set to VX - VY
					if (chipPtr->regs[nibbles[1]] >= chipPtr->regs[nibbles[2]]) {
						chipPtr->regs[0xF] = 0x1;
					} else {
						chipPtr->regs[0xF] = 0x0;
					}
					chipPtr->regs[nibbles[1]] = (chipPtr->regs[nibbles[1]] - chipPtr->regs[nibbles[2]]);
					break;
				case 0x6:
					// 0x8XY6: Shift right
					if (ORIGINAL_SHIFT) {
						chipPtr->regs[nibbles[1]] = chipPtr->regs[nibbles[2]];
					}
					uint8_t shiftedOut = chipPtr->regs[nibbles[1]] & 0x1;
					chipPtr->regs[nibbles[1]] >>= 1;
					chipPtr->regs[0xF] = shiftedOut;
					break;
				case 0x7:
					// Ox8XY7: VX is set to VY - VX
					if (chipPtr->regs[nibbles[2]] >= chipPtr->regs[nibbles[1]]) {
						chipPtr->regs[0xF] = 0x1;
					} else {
						chipPtr->regs[0xF] = 0x0;
					}
					chipPtr->regs[nibbles[1]] = (chipPtr->regs[nibbles[2]] - chipPtr->regs[nibbles[1]]);
					break;
				case 0xE:
					// 0x8XYE: Shift left
					{
						if (ORIGINAL_SHIFT) {
							chipPtr->regs[nibbles[1]] = chipPtr->regs[nibbles[2]];
						}
						uint8_t shiftedOut = (chipPtr->regs[nibbles[1]] & 0x80) >> 7;
						chipPtr->regs[nibbles[1]] <<= 1;
						chipPtr->regs[0xF] = shiftedOut;
					}
					break;
			}
			break;
		case 0x9:
			// 0x9XY0: Skip if values in VX and VY are not equal
			if (chipPtr->regs[nibbles[1]] != chipPtr->regs[nibbles[2]]) {
				chipPtr->pc += 0x2;
			}
			break;
		case 0xA:
			// 0xANNN: Set index register I
			chipPtr->index = (instruction & 0x0fff);
			break;
		case 0xB:
			// 0xBNNN: Jump with offset
			if (ORIGINAL_OFFSET_JUMP) {
				chipPtr->pc = (instruction & 0x0fff) + chipPtr->regs[0];
			} else {
				chipPtr->pc = (instruction & 0x0fff) + chipPtr->regs[nibbles[1]];
			}
			break;
		case 0xC:
			// 0xCXNN: Random
			{
				uint8_t randNbr = rand() % 256;
				chipPtr->regs[nibbles[1]] = randNbr & (instruction & 0x00ff);
			}
			break;
		case 0xD:
			// 0xDXYN: Display
			{
				uint8_t x = chipPtr->regs[nibbles[1]] & 63;
				uint8_t y = chipPtr->regs[nibbles[2]] & 31;
				chipPtr->regs[0xf] = 0x0;

				for (size_t k = 0; k < nibbles[3]; ++k) {
					if (y+k >= 32) {
						break;
					}
					uint8_t spriteByte = chipPtr->ram[chipPtr->index + k];
					for (int8_t i = 0; i < WORD_SIZE; ++i) {
						if (x+i >= 64) {
							break;
						}
						uint8_t spriteBit = spriteByte & (1 << (7 - i));
						uint8_t screenBit = chipPtr->screen[(x + i) + (y + k)*SCREEN_W];

						if (spriteBit & screenBit) {
							chipPtr->regs[0xf] = 0x1;
						}

						screenBit ^= spriteBit;
						chipPtr->screen[(x + i) + (y + k)*SCREEN_W] = screenBit;

						setPixel(chipPtr, renderer, texture, x + i, y + k, spriteBit ^ screenBit ? 0xFF : 0x00);
					}
				}
			}
			break;
		case 0xE:
			switch (nibbles[2]) {
				case 0x9:
					// 0xEX9E: Skip if key pressed
					if (chipPtr->keys[chipPtr->regs[nibbles[1]]]) {
						chipPtr->pc += 0x2;
					}
				case 0xA:
					// 0xEXA1: Skip if key not pressed
					if (!chipPtr->keys[chipPtr->regs[nibbles[1]]]) {
						chipPtr->pc += 0x2;
					}
			}
			break;
		case 0xF:
			switch (nibbles[2])
			{
			case 0x0:
				switch (nibbles[3])
				{
				case 0x7:
					// 0xFX07: Set delayT -> VX
					chipPtr->regs[nibbles[1]] = chipPtr->delayT;
					break;
				case 0xA:
					// 0xFX0A: Get key
					{
						uint8_t keyPressed = 0;
						uint8_t keyVal = 0;
						for (size_t k = 0; k < N_KEYS; ++k) {
							if (chipPtr->keys[k]) {
								keyPressed = 1;
								keyVal = chipPtr->keys[k];
							}
						}
						if (keyPressed) {
							chipPtr->regs[nibbles[1]] = keyVal;
						} 
						else {
							chipPtr->pc -= 0x2;
						}
					}
					break;
				}
				break;
			case 0x1:
				switch (nibbles[3]) {
					case 0x5:
						// 0xFX15: Set delay timer <- VX
						chipPtr->delayT = chipPtr->regs[nibbles[1]];
						break;
					case 0x8:
						// 0xFX18: Set sound timer <- VX
						chipPtr->soundT = chipPtr->regs[nibbles[1]];
						break;
					case 0xE:
						// 0xFX1E: Set index += VX
						chipPtr->index += chipPtr->regs[nibbles[1]];
						break;
				}
				break;
			case 0x2:
				// 0xFX29: Font character
				chipPtr->index = FONT_ADDRESS + 5*(chipPtr->regs[nibbles[1]] & 0x0f);
				break;
			case 0x3:
				// 0xFX33: Binary-coded decimal conversion
				{
					uint8_t value = chipPtr->regs[nibbles[1]];
					chipPtr->ram[chipPtr->index] = value / 100;
					chipPtr->ram[chipPtr->index + 1] = (value / 10) % 10;
					chipPtr->ram[chipPtr->index + 2] = 	value % 10;
				}
				break;
			case 0x5:
				// 0xFX55: Store to memory
				if (ORIGINAL_STORE_LOAD) {
					for (size_t k = 0; k <= nibbles[1]; ++k) {
						chipPtr->ram[chipPtr->index] = chipPtr->regs[k];
						chipPtr->index += 1;
					}
				} else {
					for (size_t k = 0; k <= nibbles[1]; ++k) {
						chipPtr->ram[chipPtr->index + k] = chipPtr->regs[k];
					}
				}
				break;
			case 0x6:
				// 0xFX65: Load from memory
				if (ORIGINAL_STORE_LOAD) {
					for (size_t k = 0; k <= nibbles[1]; ++k) {
						chipPtr->regs[k] = chipPtr->ram[chipPtr->index];
						chipPtr->index += 1;
					}	
				} else {
					for (size_t k = 0; k <= nibbles[1]; ++k) {
						chipPtr->regs[k] = chipPtr->ram[chipPtr->index + k];
					}
				}
				break;
			}
			break;
		default:
			puts("\nUnknown Instruction\n");
			break;
	}
}

int loadRom(Chip8* chip, const char* filename) {
	FILE* rom = fopen(filename, "rb");
	if (rom == NULL) {
		fprintf(stderr, "Failed to open ROM: %s\n", filename);
		return EXIT_FAILURE;
	}

	fseek(rom, 0, SEEK_END);
	long romSize = ftell(rom);
	rewind(rom);

	uint8_t* buffer = (u_int8_t*)malloc(sizeof(uint8_t) * romSize);
	if (buffer == NULL) {
		fprintf(stderr, "Failed to allocate memory for ROM\n");
		fclose(rom);
		return EXIT_FAILURE;
	}

	size_t bytesRead = fread(buffer, sizeof(uint8_t), romSize, rom);
	if (bytesRead != romSize) {
		fprintf(stderr, "Failed to read ROM\n");
		free(buffer);
		fclose(rom);
		return EXIT_FAILURE;
	}

	for (size_t i = 0; i < romSize; ++i) {
		chip->ram[START_ADDRESS + i] = buffer[i];
	}

	free(buffer);
	fclose(rom);

	return EXIT_SUCCESS;
}