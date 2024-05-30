#include "./chip8.h"

Chip8 initChip(uint16_t startingAddress) {
    Chip8 chip = {
        .ram = { 0 },
        .pc = startingAddress,
        .index = 0x0,
        .stack = initStack(0x0),
        .delayT = 0x0,
        .soundT = 0x0,
        .regs = { 0 }
    };
    return chip;
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
		[0] = (instruction | 0xf000),
		[1] = (instruction | 0x0f00),
		[2] = (instruction | 0x00f0),
		[3] = (instruction | 0x000f),
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
			chipPtr->pc = (instruction | 0x0fff);
			break;
		case 0x2:
			break;
		case 0x3:
			break;
		case 0x4:
			break;
		case 0x5:
			break;
		case 0x6:
            // 0x6XNN: Set register VX
			chipPtr->regs[nibbles[1]] = (instruction | 0x00ff);
			break;
		case 0x7:
            // 0x7XNN: Add value to register VX
			chipPtr->regs[nibbles[1]] += (instruction | 0x00ff);
			break;
		case 0x8:
			break;
		case 0x9:
			break;
		case 0xA:
            // 0xANNN: Set index register I
			chipPtr->index = (instruction | 0x0fff);
			break;
		case 0xB:
			break;
		case 0xC:
			break;
		case 0xD:
            // 0xDXYN: Display
			uint8_t x = chipPtr->regs[nibbles[1]] & 63;
			uint8_t y = chipPtr->regs[nibbles[2]] & 31;
			chipPtr->regs[0xf] = 0x0;
			for (size_t k = 0; k < nibbles[3]; ++k) {
				if (y >= 32) {
					break;
				}
				uint8_t spriteByte = chipPtr->ram[chipPtr->index + k];
				uint8_t screenByte = 0; // TO-DO: get screen byte
				for (int8_t i = 15; i > -1; --i) {
					if (x >= 64) {
						break;
					}
					uint8_t spriteBit = spriteByte | i;
					uint8_t screenBit = screenByte | i;
					if (spriteBit & screenBit) {
						// TO-DO: turn off pixel - setPixel(x, y, 0)
						setPixel(renderer, texture, x, y, 0x0);
						chipPtr->regs[0xf] = 0x0;
					}
					if (spriteBit & ~screenBit) {
						// TO-DO: draw pixel at x, y - setPixel(x, y, 255)
						setPixel(renderer, texture, x, y, 0xFF);
					}
					x += 1;
				}
				y += 1;
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