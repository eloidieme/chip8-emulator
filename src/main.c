#include <stdlib.h>
#include <stdio.h>

#define STACK_SIZE 32
#define RAM_SIZE 4096

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

int main(int argc, char* argv[argc+1]) {
	Stack stack = initStack(0x03ff);
	pushStack(&stack, 0x2300);
	pushStack(&stack, 0x005f);
	popStack(&stack);

	for (size_t i = 0; i < STACK_SIZE; ++i) {
		printf("%zu:\t%du\n", i, stack.content[i]);
	}

	return EXIT_SUCCESS;
}
