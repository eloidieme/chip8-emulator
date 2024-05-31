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
        .regs = { 0 }
    };
    return chip;
}

uint8_t getScreenBit(Chip8* chipPtr, uint8_t x, uint8_t y) {
    uint8_t rowIdx = x/WORD_SIZE + y*WORD_SIZE;
    uint8_t screenBit = chipPtr->screen[rowIdx] & (1 << (7 - (x%WORD_SIZE)));

    return screenBit;
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
	
	uint8_t rowIdx = x/WORD_SIZE + y*WORD_SIZE;

	if (color > 128) {
		chipPtr->screen[rowIdx] |= (1 << (7 - (x%WORD_SIZE))); 
	} else {
		chipPtr->screen[rowIdx] &= ~(1 << (7 - (x%WORD_SIZE)));
	}
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
			break;
		case 0x3:
			// 0x3XNN: Skip if value in VX == NN
			break;
		case 0x4:
			// 0x4XNN: Skip if value in VX != NN
			break;
		case 0x5:
			// 0x5XY0: Skip if values in VX and VY are equal
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
			break;
		case 0x9:
			// 0x9XY0: Skip if values in VX and VY are not equal
			break;
		case 0xA:
			// 0xANNN: Set index register I
			chipPtr->index = (instruction & 0x0fff);
			break;
		case 0xB:
			break;
		case 0xC:
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
						uint8_t screenBit = getScreenBit(chipPtr, x+i, y+k);
						if (spriteBit & screenBit) {
							setPixel(chipPtr, renderer, texture, x+i, y+k, 0x0);
							chipPtr->regs[0xf] = 0x0;
						}
						if (spriteBit & ~screenBit) {
							setPixel(chipPtr, renderer, texture, x+i, y+k, 0xFF);
						}
					}
				}
			}
			break;
		case 0xE:
			break;
		case 0xF:
			break;
		default:
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