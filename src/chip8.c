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

void decodeExecute(uint16_t instruction, Chip8* chipPtr, SDL_Renderer* renderer) {
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
			break;
		case 0xE:
			break;
		case 0xF:
			break;
		default:
			break;
	}
}