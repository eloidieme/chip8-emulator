#include "./stack.h"

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