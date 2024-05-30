#ifndef __CHIP8_H__
#define __CHIP8_H__

#include <stdlib.h>

#include <SDL3/SDL.h>

#include "./macros.h"
#include "./stack.h"

typedef struct Chip8 Chip8;
struct Chip8 {
	/* Memory */
	uint8_t ram[RAM_SIZE];	// Random Access Memory
	uint16_t pc;			// Program Counter
	uint16_t index;			// Index Register
	Stack stack;			// Address Stack

	/* Timers */
	uint8_t delayT;			// Delay Timer
	uint8_t soundT;			// Sound Timer

	/* Registers */
	uint8_t regs[N_REGISTERS];
};

Chip8 initChip(uint16_t startingAddress);
uint16_t fetch(Chip8* chipPtr);
void decodeExecute(uint16_t instruction, Chip8* chipPtr, SDL_Renderer* renderer); 

#endif