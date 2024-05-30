#ifndef __STACK_H__
#define __STACK_H__

#include <stdlib.h>
#include <stdio.h>

#include "./macros.h"

typedef struct Stack Stack;
struct Stack {
	uint16_t content[STACK_SIZE];
	size_t top;	// idx of the current top of stack
};

Stack initStack(uint16_t firstVal);
void pushStack(Stack* stack, uint16_t newVal);
uint16_t popStack(Stack* stack);

#endif