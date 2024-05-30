#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define STACK_SIZE 32
#define RAM_SIZE 4096
#define N_REGISTERS 16
#define FONT_SIZE 80
#define FONT_ADDRESS 0x050
#define WORD_SIZE 8

uint8_t font[FONT_SIZE] = {
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

typedef struct Stack Stack;
struct Stack {
	uint16_t content[STACK_SIZE];
	size_t top;	// idx of the current top of stack
};

Stack initStack(uint16_t firstVal) {
	Stack stack = {
		.content = { [0] = firstVal },
		.top = 0
	};
	return stack;
}

void pushStack(Stack* stack, uint16_t newVal) {
	if ((stack->top + 1) >= STACK_SIZE) {
		puts("ERROR: Stack overflow\n");
		exit(EXIT_FAILURE);
	}

	stack->content[stack->top + 1] = newVal;
	stack->top += 1;
}

uint16_t popStack(Stack* stack) {
	uint16_t value = stack->content[stack->top];
	stack->content[stack->top] = 0;
	stack->top -= 1;
	return value;
}

void loadFont(uint8_t ram[RAM_SIZE], uint8_t font[FONT_SIZE]) {
	memcpy(ram + FONT_ADDRESS, font, FONT_SIZE);
}

uint16_t fetch(uint16_t* pc, uint8_t ram[RAM_SIZE]) {
	uint16_t msb = ram[*pc];
	uint16_t lsb = ram[*pc+1];
	uint16_t instruction = ((msb << WORD_SIZE) | lsb);
	*pc += 0x2;
	return instruction;
}

void decodeExecute(uint16_t instruction) {
	puts("decode - exec\n");
}

int main(int argc, char* argv[argc+1]) {
	/* Memory */
	uint8_t ram[RAM_SIZE] = { 0 };
	uint16_t pc = 0x0;
	uint16_t index = 0x0;
	Stack stack = initStack(0x0);

	/* Timers */
	uint8_t delayT = 0x0;
	uint8_t soundT = 0x0;

	/* Registers */
	uint8_t regs[N_REGISTERS] = { 0 };

	loadFont(ram, font);

	for (size_t i = 0; i < RAM_SIZE; ++i) {
		printf("%.4zu:\t%#.4x\n", i, ram[i]);
	}

	pc = 0x050;
	uint16_t inst = fetch(&pc, ram);
	printf("Instruction:\t%#.4x\n", inst);
	printf("PC:\t%#.4x\n", pc);

	return EXIT_SUCCESS;
}
