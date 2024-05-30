/* C Standard Library */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

/* Third-party Libraries */
#include <SDL3/SDL.h>

/* Header files */
#include "./macros.h"
#include "./stack.h"
#include "./chip8.h"
#include "./font.h"

int main(int argc, char* argv[argc+1]) {
	/* SDL Initialization */
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		printf("SDL_Init Error: %s\n", SDL_GetError());
		return 1;
	}

	SDL_Window* window = SDL_CreateWindow("CHIP-8 Emulator", SCREEN_W, SCREEN_H, 0);
	if (window == NULL) {
		printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
		SDL_Quit();
		return EXIT_FAILURE;
	}

	SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (renderer == NULL) {
		SDL_DestroyWindow(window);
		printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
		SDL_Quit();
		return EXIT_FAILURE;
	}

	SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB332, SDL_TEXTUREACCESS_STREAMING, SCREEN_W, SCREEN_H);
	if (texture == NULL) {
		printf("SDL_CreateTexture Error: %s\n", SDL_GetError());
		SDL_Quit();
		return EXIT_FAILURE;
	}
	/* Timing Initialization */
	double clockRate = 1.0;
	struct timespec start, end;
	uint64_t delta_us;
	int64_t remainingTime;
	uint64_t timeInterval = (1 / clockRate) * 1E6;
	printf("Time int:\t%llu\n", timeInterval);

	/* Chip Initialization */
	Chip8 chip = initChip(0x200);	
	loadFont(&chip, font);
	uint16_t inst = 0x0;
	
	/* Main Loop */
	SDL_bool done = SDL_FALSE;
	while (!done) {
		/* Event Loop */
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_EVENT_QUIT) {
				done = SDL_TRUE;
			}
		}

		printf("PC:\t%#.4x - ", chip.pc);

		/* Fetch-Execute-Decode */
		inst = fetch(&chip);
		clock_gettime(CLOCK_MONOTONIC_RAW, &start);
		decodeExecute(inst, &chip, renderer, texture);
		clock_gettime(CLOCK_MONOTONIC_RAW, &end);

		printf("Instruction:\t%#.4x\n", inst);

		/* Timing Handling */
		delta_us = (end.tv_sec - start.tv_sec) * 1E6 + (end.tv_nsec	- start.tv_nsec) / 1E3;
		remainingTime = (timeInterval - delta_us);
		if (remainingTime > 0) {
			SDL_DelayNS(remainingTime * 1000);
		}

		/* Rendering */
		SDL_RenderPresent(renderer);
	}

	/* SDL Clean-up */
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();	

	/*
	for (size_t i = 0; i < RAM_SIZE; ++i) {
		printf("%.4zu:\t%#.4x\n", i, chip.ram[i]);
	}
	*/

	return EXIT_SUCCESS;
}
